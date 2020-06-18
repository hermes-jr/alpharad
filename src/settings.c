#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "settings.h"

struct settings settings = {
        .dev_name = "/dev/video0",
        .file_out_name = "out.dat",
        .file_hitlog_name = "points.log",
        .frame_processor = PROC_DEFAULT,
        .width = 640,
        .height = 480,
        .threshold = 8u,
        .verbose = 0
};

static const struct option
        long_options[] = {
        {"device",      required_argument, NULL, 'd'},
        {"geometry",    required_argument, NULL, 'g'},
        {"hitlog-file", required_argument, NULL, 'l'},
        {"out-file",    required_argument, NULL, 'o'},
        {"mode",        required_argument, NULL, 'm'},
        {"list-modes",  no_argument,       NULL, 'M'},
        {"verbose",     optional_argument, NULL, 'v'},
        {"help",        no_argument,       NULL, 'h'},
        {0, 0, 0,                                0}
};
static const char short_options[] = "d:g:l:o:m:Mv:h";

void print_usage(char *self_name) {
    fprintf(stdout,
            "Usage: %s [options]\n\n"
            "Version 0.5\n"
            "Options:\n"
            "-d | --device name             Video device name [%s]\n"
            "-g | --geometry WIDTHxHEIGHT   Frame dimensions [%dx%d]\n"
            "-l | --hitlog-file logfile     Log detected flashes to logfile. Disabled by default\n"
            "-o | --out-file file           Write processed data to file [%s] \n"
            "-m | --mode mode               Set one of available modes\n"
            "-M | --list-modes              List currently supported modes\n"
            "-v | --verbose                 Verbosity level\n"
            "-h | --help                    Print this message\n"
            "",
            self_name, settings.dev_name, settings.width, settings.height, settings.file_out_name);
}

void populate_settings(int argc, char **argv) {

    for (;;) {
        int idx;
        int c = getopt_long(argc, argv,
                            short_options, long_options, &idx);

        if (-1 == c)
            break;

        // d:g:l:o:m:Mv:h
        switch (c) {
            case 0: /* getopt_long() flag */
                break;

            case 'd':
                settings.dev_name = optarg;
                break;

            case 'g': {
                char provided_mode[32];
                char *mode_token;

                strncpy(provided_mode, optarg, sizeof(provided_mode));
                mode_token = strtok(provided_mode, "x");
                if (mode_token != NULL) {
                    settings.width = strtol(mode_token, NULL, 0);
                } else {
                    printf("Couldn't parse width");
                    exit(EXIT_FAILURE);
                }
                mode_token = strtok(NULL, "x");
                if (mode_token != NULL) {
                    settings.height = strtol(mode_token, NULL, 0);
                } else {
                    printf("Couldn't parse height");
                    exit(EXIT_FAILURE);
                }
                printf("w h %d %d\n", settings.width, settings.height);
                break;
            }

            case 'l':
                settings.file_hitlog_name = optarg;
                break;

            case 'o':
                settings.file_out_name = optarg;
                break;

            case 'm':

                settings.height = strtol(optarg, NULL, 0);
                if (errno)
                    printf("Couldn't parse option 'g' %s\n", optarg);
                break;

            case 'M':
                // FIXME: implement
                //  print_supported_modes();
                printf("Supported modes: implement.");
                exit(EXIT_SUCCESS);

            case 'v':
                settings.verbose = strtol(optarg, NULL, 0);
                if (errno)
                    printf("Couldn't parse option 'v' %s\n", optarg);
                break;

            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);


            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

}

// Fixme: this is only a test, must be removed
int add(int a, int b) {
    return a + b;
}