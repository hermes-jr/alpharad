#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include <linux/videodev2.h>
#include <stdbool.h>

#include <openssl/sha.h>
#include <signal.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
    u_int8_t *start;
    size_t length;
};

static const bool strict = false;
static char *dev_name;
static int fd = -1;
struct buffer *buffers;
static u_int8_t n_buffers;
static u_long frame_number = 0;
static u_long bytes = 0;
static struct timeval start, checkpoint;
static const u_int WIDTH = 640;
static const u_int HEIGHT = 480;
//static const u_int WIDTH = 160;
//static const u_int HEIGHT = 120;
static const u_int DWIDTH = WIDTH * 2;
FILE *out_file;
FILE *stat_file;

static void errno_exit(const char *s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg) {
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static bool is_pixel_lit(const u_int8_t *p, u_int idx) { return p[idx] > 4u; }

static void push_xy(u_int x, u_int y) {
    char z[32];
    sprintf(z, "%d:%d\n", x, y);
    fputs(z, stat_file);
    fflush(stat_file);

//    printf("%d of %d; conv: %d; bytes so far: %07lu\n", idx, size, conv, bytes);
}

static void push_byte(int size, u_int idx, uint8_t conv) {
    fputc(conv, out_file);
    fflush(out_file);

//    printf("%d of %d; conv: %d; bytes so far: %07lu\n", idx, size, conv, bytes);
}

static void process_image(const u_int8_t *p, int size) {
    for (u_int idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) { // we're on black
            continue;
        }

        if (strict) {
            // check neighbors
            if (idx > DWIDTH) {
                // this is not the first row
                if (is_pixel_lit(p, idx - DWIDTH)) {
                    // N is on
                    continue;
                }
                if (idx % DWIDTH != 0 && is_pixel_lit(p, idx - DWIDTH - 2)) {
                    // not the leftmost column, NW is on
                    continue;
                }
                if ((idx + 2) % DWIDTH != 0 && is_pixel_lit(p, idx - DWIDTH + 2)) {
                    // not the rightmost column, NE is on
                    continue;
                }
            }
            if (idx % DWIDTH != 0 && is_pixel_lit(p, idx - 2)) {
                // not the leftmost column, W is on
                continue;
            }
            bytes += 2;
            u_int x = (idx % DWIDTH) / 2;
            uint8_t conv = ((double) x) / WIDTH * (UINT8_MAX + 1);
            push_byte(size, idx, conv);

            u_int y = idx / DWIDTH;
            conv = ((double) y) / HEIGHT * (UINT8_MAX + 1);
            push_byte(size, idx, conv);
//        printf("Flash at %3d:%3d\n", x, y);
            push_xy(x, y);
        } else {
            u_char hash[SHA512_DIGEST_LENGTH];
            SHA512(p, size, hash);

/*
            printf("hash: ");
            for (int x = 0; x < SHA512_DIGEST_LENGTH; x++) {
                printf("%02x", hash[x]);
            }
            putchar('\n');
*/
            bytes += SHA512_DIGEST_LENGTH;
            for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
                fputc(hash[i], out_file);
            }
            return;
        }
    }

}

static void print_perf_stats(void) {
    gettimeofday(&checkpoint, NULL);
    double time_spent = (double) (checkpoint.tv_usec - start.tv_usec) / 1.0e6 +
                        (double) (checkpoint.tv_sec - start.tv_sec);
    printf("Time: %d, FPS: %f, BPM: %f\n",
           (int) time_spent,
           ((double) frame_number) / time_spent,
           ((double) bytes) / (time_spent / 60));
}

static int read_frame(void) {
    struct v4l2_buffer buf;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                errno_exit("VIDIOC_DQBUF");
        }
    }

    assert(buf.index < n_buffers);

    frame_number++;
//    printf("Processing frame %d\n", frame_number);

    process_image(buffers[buf.index].start, buf.bytesused);

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        errno_exit("VIDIOC_QBUF");


    return 1;
}

static void mainloop(void) {

    for (; bytes < 1.0e6;) {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;
            errno_exit("select");
        }

        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }

        if (!read_frame()) {
            fprintf(stderr, "recording error\n");
            break;
        }
    }
}

static void stop_capturing(void) {
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
        errno_exit("VIDIOC_STREAMOFF");
}

static void start_capturing(void) {
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
        errno_exit("VIDIOC_STREAMON");

}

static void uninit_device(void) {
    unsigned int i;

    for (i = 0; i < n_buffers; ++i)
        if (-1 == munmap(buffers[i].start, buffers[i].length))
            errno_exit("munmap");

    free(buffers);
}

static void init_mmap(void) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support "
                            "memory mapping\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n",
                dev_name);
        exit(EXIT_FAILURE);
    }

    buffers = calloc(req.count, sizeof(*buffers));

    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit("mmap");
    }
}

static void init_device(void) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n",
                    dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n",
                dev_name);
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming i/o\n",
                dev_name);
        exit(EXIT_FAILURE);
    }


    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else {
        /* Errors ignored. */
    }


    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH; //replace
    fmt.fmt.pix.height = HEIGHT; //replace
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU; //replace
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        errno_exit("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    init_mmap();

}

static void close_device(void) {
    if (-1 == close(fd))
        errno_exit("close");

    fd = -1;
}

static void open_device(void) {
    struct stat st;

    if (-1 == stat(dev_name, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n",
                dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void signal_usr1_handler(int signnum) {
    if (SIGUSR1 == signnum) {
        print_perf_stats();
    }
}

int main(int argc, char **argv) {
    signal(SIGUSR1, signal_usr1_handler);

    dev_name = "/dev/video0";
    out_file = fopen("myout.dat", "wba");
    stat_file = fopen("stat.dat", "wa");

    gettimeofday(&start, NULL);

    open_device();
    init_device();
    start_capturing();
    mainloop();
    stop_capturing();
    uninit_device();
    close_device();

    fclose(out_file);

    fprintf(stderr, "\n");
    return EXIT_SUCCESS;
}
