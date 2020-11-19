/*
Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>

This file is part of alpharad project.

alpharad is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

alpharad is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with alpharad.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "settings.h"
#include "logger.h"

struct settings settings = {
        .dev_name = S_DEFAULT_DEV_NAME,
        .file_out_name = S_DEFAULT_FILE_OUT_NAME,
        .file_out = S_DEFAULT_FILE_OUT,
        .file_hits_name = S_DEFAULT_FILE_HITS_NAME,
        .file_hits = S_DEFAULT_FILE_HITS,
        .frame_processor = S_DEFAULT_FRAME_PROCESSOR,
        .width = S_DEFAULT_WIDTH,
        .height = S_DEFAULT_HEIGHT,
        .crop = S_DEFAULT_CROP,
        .threshold = S_DEFAULT_THRESHOLD,
        .verbose = S_DEFAULT_VERBOSE,
        .yes_to_all = S_DEFAULT_YES_TO_ALL
};

static const struct option
        long_options[] = {
        {"device",     required_argument, NULL, 'd'},
        {"geometry",   required_argument, NULL, 'g'},
        {"border",     optional_argument, NULL, 'b'},
        {"threshold",  optional_argument, NULL, 't'},
        {"hits-file",  required_argument, NULL, 'l'},
        {"out-file",   required_argument, NULL, 'o'},
        {"mode",       required_argument, NULL, 'm'},
        {"list-modes", no_argument,       NULL, 'M'},
        {"yes",        no_argument,       NULL, 'y'},
        {"verbose",    optional_argument, NULL, 'v'},
        {"help",       no_argument,       NULL, 'h'},
        {0, 0, 0,                               0}
};
static const char short_options[] = "d:g:b:t:l:o:m:Myv:h";

void print_usage(FILE *ofp, char *self_name) {
    fprintf(ofp,
            "Usage: %s [options]\n\n"
            "Version %s\n\n"
            "Options:\n"
            "-d, --device=PATH             Video device PATH [%s]\n"
            "-g, --geometry=WIDTH:HEIGHT   Frame dimensions [%d:%d]\n"
            "-b, --border=N                Ignore everything that happens within N pixels thick border\n"
            "-t, --threshold=N             Ignore pixels with luminosity below N [%d]\n"
            "-l, --hits-file=LOGFILE       Log detected flashes to LOGFILE. Disabled by default\n"
            "-o, --out-file=FILE           Write processed data to FILE [%s] \n"
            "-m, --mode=MODE               Set one of available modes\n"
            "-M, --list-modes              List currently supported modes\n"
            "-y, --yes-to-all              Modify existing output files\n"
            "-v, --verbose=LEVEL           Set verbosity level\n"
            "-h, --help                    Print this message\n"
            "",
            self_name, PROJECT_VERSION, settings.dev_name, settings.width, settings.height, settings.threshold,
            settings.file_out_name);
}

void print_supported_modes(FILE *ofp) {
    for (short i = 0; i >= 0; i++) {
        frame_processor_t p = registered_processors[i];
        if (is_processor_null(&p)) {
            break;
        }
        if (p.code != NULL && p.description != NULL) {
            fprintf(ofp, "%-30s %s\n", p.code, p.description);
        }
    }
}

bool is_processor_null(frame_processor_t *p) {
    return (*p).description == NULL && (*p).code == NULL && (*p).execute == NULL;
}

/**
 * Handle user input and attempt to fill in fields of settings object
 *
 * @param argc passthrough from main method
 * @param argv passthrough from main method
 * @param ofp file pointer to write messages to. For the sake of test coverage
 * @return 0 if no action should be taken; -1 if should exit with error code; 1 if should exit with success
 */
int populate_settings(FILE *ofp, char **argv, int argc) {

    for (;;) {
        int idx;
        int c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (-1 == c) {
            break;
        }

        // d:g:b:t:l:o:m:Mv:h
        switch (c) {
            case 0: /* getopt_long() flag */
                break;

            case 'd':
                settings.dev_name = optarg;
                break;

            case 'g': {
                char provided_mode[BUFSIZ];
                char *mode_token;

                strncpy(provided_mode, optarg, BUFSIZ - 1);
                provided_mode[BUFSIZ - 1] = '\0';

                char *delimiter = ":";
                /* Get width */
                mode_token = strtok(provided_mode, delimiter);
                if (validated_long_parse(ofp, &(settings.width), mode_token, "Couldn't parse width")) {
                    return -1;
                }
                /* Get height */
                mode_token = strtok(NULL, delimiter);
                if (validated_long_parse(ofp, &(settings.height), mode_token, "Couldn't parse height")) {
                    return -1;
                }

                log_fp(LOG_DEBUG, ofp, "Dimensions: %d:%d\n", settings.width, settings.height);
                break;
            }

            case 'b': {
                if (validated_long_parse(ofp, &(settings.crop), optarg, "Couldn't parse option 'b'")) {
                    return -1;
                }
                log_fp(LOG_DEBUG, ofp, "Border crop: %d\n", settings.crop);
                break;
            }

            case 't': {
                if (validated_short_parse(ofp, &(settings.threshold), optarg, "Couldn't parse option 't'")) {
                    return -1;
                }
                log_fp(LOG_DEBUG, ofp, "Threshold: %d\n", settings.threshold);
                break;
            }

            case 'l':
                settings.file_hits_name = optarg;
                log_fp(LOG_DEBUG, ofp, "Hits file: %s\n", settings.file_hits_name);
                break;

            case 'o':
                settings.file_out_name = optarg;
                log_fp(LOG_DEBUG, ofp, "Out file: %s\n", settings.file_hits_name);
                break;

            case 'm': {
                frame_processor_t selected;
                for (short i = 0; i >= 0; i++) {
                    selected = registered_processors[i];
                    // TODO: feels somewhat unsafe here
                    if (is_processor_null(&selected) || strcasecmp(selected.code, optarg) == 0) {
                        break;
                    }
                }
                if (is_processor_null(&selected)) {
                    fprintf(ofp, "Unsupported processor requested. Retry with -M option\n");
                    return -1;
                }
                settings.frame_processor = selected;
                log_fp(LOG_DEBUG, ofp, "Processor: %s\n", settings.frame_processor.code);
                break;
            }

            case 'M':
                print_supported_modes(ofp);
                return 1;

            case 'y':
                settings.yes_to_all = true;
                log_fp(LOG_DEBUG, ofp, "Permission granted to modify existing output files\n");
                break;

            case 'v':
                if (validated_short_parse(ofp, &(settings.verbose), optarg, "Couldn't parse option 'v'")) {
                    return -1;
                }
                log_fp(LOG_DEBUG, ofp, "Verbosity: %d\n", settings.verbose);
                break;

            case 'h':
                print_usage(ofp, argv[0]);
                return 1;

            default:
                print_usage(ofp, argv[0]);
                return -1;
        }
    }

    return 0;

}

/* Attempt to parse value and report in case of error */
int validated_long_parse(FILE *ofp, uint *target, char *input, const char *err_msg) {
    char *verify_end = NULL;
    if (input != NULL) {
        *target = strtoul(input, &verify_end, 0);
    }
    if (input == NULL || (errno && *target == 0) || verify_end == input) {
        log_fp(LOG_FATAL, ofp, "%s\n", err_msg);
        return -1;
    }
    return 0;
}

/* Attempt to parse value and report in case of error */
int validated_short_parse(FILE *ofp, uint8_t *target, char *input, const char *err_msg) {
    uint tmp = 0u;
    if (validated_long_parse(ofp, &tmp, input, err_msg)) {
        return -1;
    }
    if (tmp > UINT8_MAX) {
        log_fp(LOG_FATAL, ofp, "%s\n", err_msg);
        return -1;
    }
    *target = tmp;
    return 0;
}