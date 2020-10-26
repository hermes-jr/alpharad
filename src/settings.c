#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "settings.h"

struct settings settings = {
        .dev_name = "/dev/video0",
        .file_out_name = "out.dat",
        .file_hits_name = "points.log",
        .frame_processor = PROC_DEFAULT,
        .width = 640,
        .height = 480,
        .threshold = 8u,
        .verbose = 0
};

static const struct option
        long_options[] = {
        {"device",     required_argument, NULL, 'd'},
        {"geometry",   required_argument, NULL, 'g'},
        {"hits-file",  required_argument, NULL, 'l'},
        {"out-file",   required_argument, NULL, 'o'},
        {"mode",       required_argument, NULL, 'm'},
        {"list-modes", no_argument,       NULL, 'M'},
        {"verbose",    optional_argument, NULL, 'v'},
        {"help",       no_argument,       NULL, 'h'},
        {0, 0, 0,                               0}
};
static const char short_options[] = "d:g:l:o:m:Mv:h";

void print_usage(char *self_name) {
    fprintf(stdout,
            "Usage: %s [options]\n\n"
            "Version %s\n\n"
            "Options:\n"
            "-d, --device=PATH             Video device PATH [%s]\n"
            "-g, --geometry=WIDTH:HEIGHT   Frame dimensions [%d:%d]\n"
            "-l, --hits-file=LOGFILE       Log detected flashes to LOGFILE. Disabled by default\n"
            "-o, --out-file=FILE           Write processed data to FILE [%s] \n"
            "-m, --mode=MODE               Set one of available modes\n"
            "-M, --list-modes              List currently supported modes\n"
            "-v, --verbose=LEVEL           Set verbosity level\n"
            "-h, --help                    Print this message\n"
            "",
            self_name, PROJECT_VERSION, settings.dev_name, settings.width, settings.height, settings.file_out_name);
}

int populate_settings(int argc, char **argv, FILE *ofp) {

    for (;;) {
        int idx;
        int c = getopt_long(argc, argv,
                            short_options, long_options, &idx);

        if (-1 == c) {
            break;
        }

        // d:g:l:o:m:Mv:h
        switch (c) {
            case 0: /* getopt_long() flag */
                break;

            case 'd':
                settings.dev_name = optarg;
                break;

            case 'g': {
                int buffer_size = 31;
                char provided_mode[buffer_size + 1];
                char *verify_end = NULL;
                char *mode_token;

                strncpy(provided_mode, optarg, buffer_size);
                provided_mode[buffer_size] = '\0';

                char *delimiter = ":";
                /* Get width */
                mode_token = strtok(provided_mode, delimiter);
                if (mode_token != NULL) {
                    settings.width = strtol(mode_token, &verify_end, 0);
                }
                if (mode_token == NULL || (errno && settings.width == 0) || verify_end == mode_token) {
                    fprintf(ofp, "Couldn't parse width\n");
                    return -1;
                }
                /* Get height */
                mode_token = strtok(NULL, delimiter);
                if (mode_token != NULL) {
                    settings.height = strtol(mode_token, &verify_end, 0);
                }
                if (mode_token == NULL || (errno && settings.height == 0) || verify_end == mode_token) {
                    fprintf(ofp, "Couldn't parse height\n");
                    return -1;
                }
                D(fprintf(ofp, "w h %d %d\n", settings.width, settings.height));
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
                print_usage(argv[0]);
                return 1;


            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    return 0;

}
