/*
 * The most part of this code is from V4L2 video capture example
 *
 * This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

#include <stdlib.h>
#include <string.h> /* define CLEAR(x) */
#include <sys/stat.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "settings.h"
#include "v4l2_util.h"
#include "logger.h"

extern struct settings settings;
static uint8_t n_buffers;
static struct buffer *buffers;
static int frame_number;
extern int device;

int xioctl(int dev, int request, void *arg) {
    int r;

    do {
        r = ioctl(dev, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

int read_frame(void (*callback)(const uint8_t *, uint)) {
    struct v4l2_buffer buf;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(device, VIDIOC_DQBUF, &buf)) {
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

    callback(buffers[buf.index].start, buf.bytesused);

    if (-1 == xioctl(device, VIDIOC_QBUF, &buf))
        errno_exit("VIDIOC_QBUF");

    return 1;
}

void stop_capturing(void) {
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(device, VIDIOC_STREAMOFF, &type))
        errno_exit("VIDIOC_STREAMOFF");
}

void start_capturing(void) {
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(device, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(device, VIDIOC_STREAMON, &type))
        errno_exit("VIDIOC_STREAMON");

}

void init_mmap(void) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(device, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            log_p(LOG_FATAL, "%s does not support memory mapping. Error %d, %s\n",
                  settings.dev_name, errno, strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        log_p(LOG_FATAL, "Insufficient buffer memory on %s\n",
              settings.dev_name);
        exit(EXIT_FAILURE);
    }

    buffers = calloc(req.count, sizeof(*buffers));

    if (!buffers) {
        log_p(LOG_FATAL, "Out of memory\n",
              settings.dev_name);
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(device, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     device, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit("mmap");
    }
}

void uninit_device(void) {
    unsigned int i;

    for (i = 0; i < n_buffers; ++i)
        if (-1 == munmap(buffers[i].start, buffers[i].length))
            errno_exit("munmap");

    free(buffers);
}

void init_device(void) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(device, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            log_p(LOG_FATAL, "%s is no V4L2 device. Error %d, %s\n",
                  settings.dev_name, errno, strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        log_p(LOG_FATAL, "%s is no video capture device\n",
              settings.dev_name);
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        log_p(LOG_FATAL, "%s does not support streaming i/o\n",
              settings.dev_name);
        exit(EXIT_FAILURE);
    }


    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(device, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(device, VIDIOC_S_CROP, &crop)) {
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
    fmt.fmt.pix.width = settings.width;
    fmt.fmt.pix.height = settings.height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (-1 == xioctl(device, VIDIOC_S_FMT, &fmt))
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

void close_device(void) {
    if (-1 == close(device))
        errno_exit("close");

    device = -1;
}

void open_device(void) {
    struct stat st;

    if (-1 == stat(settings.dev_name, &st)) {
        log_p(LOG_FATAL, "Cannot identify %s. Error %d, %s\n",
              settings.dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode)) {
        log_p(LOG_FATAL, "Bad device %s. Error %d, %s\n",
              settings.dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    device = open(settings.dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == device) {
        log_p(LOG_FATAL, "Can't open %s. Error %d, %s\n",
              settings.dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void errno_exit(const char *s) {
    log_p(LOG_FATAL, "%s. Error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}
