#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "settings.h"

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
        .verbose = S_DEFAULT_VERBOSE
};

static const struct option
        long_options[] = {
        {"device",     required_argument, NULL, 'd'},
        {"geometry",   required_argument, NULL, 'g'},
        {"border",     optional_argument, NULL, 'b'},
        {"hits-file",  required_argument, NULL, 'l'},
        {"out-file",   required_argument, NULL, 'o'},
        {"mode",       required_argument, NULL, 'm'},
        {"list-modes", no_argument,       NULL, 'M'},
        {"verbose",    optional_argument, NULL, 'v'},
        {"help",       no_argument,       NULL, 'h'},
        {0, 0, 0,                               0}
};
static const char short_options[] = "d:g:b:l:o:m:Mv:h";

void print_usage(FILE *ofp, char *self_name) {
    fprintf(ofp,
            "Usage: %s [options]\n\n"
            "Version %s\n\n"
            "Options:\n"
            "-d, --device=PATH             Video device PATH [%s]\n"
            "-g, --geometry=WIDTH:HEIGHT   Frame dimensions [%d:%d]\n"
            "-b, --border=N                Ignore everything that happens within N pixels thick border\n"
            "-l, --hits-file=LOGFILE       Log detected flashes to LOGFILE. Disabled by default\n"
            "-o, --out-file=FILE           Write processed data to FILE [%s] \n"
            "-m, --mode=MODE               Set one of available modes\n"
            "-M, --list-modes              List currently supported modes\n"
            "-v, --verbose=LEVEL           Set verbosity level\n"
            "-h, --help                    Print this message\n"
            "",
            self_name, PROJECT_VERSION, settings.dev_name, settings.width, settings.height, settings.file_out_name);
}

/**
 * Handle user input and attempt to fill in fields of settings object
 *
 * @param argc passthrough from main method
 * @param argv passthrough from main method
 * @param ofp file pointer to write messages to. For the sake of test coverage
 * @return 0 if no action should be taken; -1 if should exit with error code; 1 if should exit with success
 */
int populate_settings(int argc, char **argv, FILE *ofp) {

    for (;;) {
        int idx;
        int c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (-1 == c) {
            break;
        }

        // d:g:b:l:o:m:Mv:h
        switch (c) {
            case 0: /* getopt_long() flag */
                break;

            case 'd':
                settings.dev_name = optarg;
                break;

            case 'g': {
                int buffer_size = 31;
                char provided_mode[buffer_size + 1];
                char *mode_token;

                strncpy(provided_mode, optarg, buffer_size);
                provided_mode[buffer_size] = '\0';

                char *delimiter = ":";
                /* Get width */
                mode_token = strtok(provided_mode, delimiter);
                if (validated_long_parse(ofp, &(settings.width), mode_token, "Couldn't parse width\n")) {
                    return -1;
                }
                /* Get height */
                mode_token = strtok(NULL, delimiter);
                if (validated_long_parse(ofp, &(settings.height), mode_token, "Couldn't parse height\n")) {
                    return -1;
                }

                D(fprintf(ofp, "w h %d %d\n", settings.width, settings.height));
                break;
            }

            case 'b': {
                if (validated_long_parse(ofp, &(settings.crop), optarg, "Couldn't parse option 'b'\n")) {
                    return -1;
                }
                break;
            }

            case 'l':
                settings.file_hits_name = optarg;
                break;

            case 'o':
                settings.file_out_name = optarg;
                break;

            case 'm':
                // FIXME: implement mode selection
                fprintf(ofp, "Mode select: implement.\n");
                break;

            case 'M':
                // FIXME: implement print_supported_modes();
                fprintf(ofp, "Supported modes: implement.\n");
                return 1;

            case 'v':
                settings.verbose = strtol(optarg, NULL, 0);
                if (errno)
                    fprintf(ofp, "Couldn't parse option 'v' %s\n", optarg);
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
        *target = strtol(input, &verify_end, 0);
    }
    if (input == NULL || (errno && *target == 0) || verify_end == input) {
        fprintf(ofp, "%s", err_msg);
        return -1;
    }
    return 0;
}