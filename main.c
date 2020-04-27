#include <errno.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

uint8_t *buffer;

const int WIDTH = 640;
const int HEIGHT = 480;

int poke_camera(int device) {
    struct v4l2_capability cap = {};

    if (-1 == ioctl(device, VIDIOC_QUERYCAP, &cap)) {
        perror("Unable to get camera capabilities");
        return 1;
    }

    return 0;
}

int setup_format(int device) {
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
    format.fmt.pix.width = WIDTH;
    format.fmt.pix.height = HEIGHT;

    if (-1 == ioctl(device, VIDIOC_S_FMT, &format)) {
        perror("Unable to set camera format");
        return 1;
    }

    return 0;
}

int init_mmap(int fd) {
    struct v4l2_requestbuffers bufRequest;
    bufRequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufRequest.memory = V4L2_MEMORY_MMAP;
    bufRequest.count = 1;

    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &bufRequest)) {
        perror("Failed to request a buffer");
        return 1;
    }

    struct v4l2_buffer bufInfo = {0};
    bufInfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufInfo.memory = V4L2_MEMORY_MMAP;
    bufInfo.index = 0;
    if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &bufInfo)) {
        perror("Failed to query buffer");
        return 1;
    }

    buffer = mmap(NULL, bufInfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bufInfo.m.offset);
    if (MAP_FAILED == buffer) {
        perror("MMAP failed");
        return 1;
    }
    memset(buffer, 0, bufInfo.length);

    return 0;
}

void processYuy2Frame(long frameNum) {
    int spikes = 0;
    int yValIdx = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (buffer[yValIdx] >= 50) {
                printf("Glow at %d:%d\n", x, y);
                spikes++;
            }
            yValIdx += 2; // Skip color data
        }
    }

    if (spikes > 0) {
        printf("Frame: %04ld, pixels lit: %d\n", frameNum, spikes);
    }
}

void capture_video(int device) {
    struct v4l2_buffer bufInfo;
    memset(&bufInfo, 0, sizeof(bufInfo));

    bufInfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufInfo.memory = V4L2_MEMORY_MMAP;
    bufInfo.index = 0; /* Queueing buffer index 0. */

    // Activate streaming
    int type = bufInfo.type;
    if (-1 == ioctl(device, VIDIOC_STREAMON, &type)) {
        perror("Could not enable streaming");
        exit(1);
    }

    /* Here is where you typically start two loops:
     * - One which runs for as long as you want to
     *   capture frames (shoot the video).
     * - One which iterates over your buffers everytime. */
    for (long frameNum = 0;; ++frameNum) {

        // Put the buffer in the incoming queue.
        if (-1 == ioctl(device, VIDIOC_QBUF, &bufInfo)) {
            perror("VIDIOC_QBUF");
            exit(1);
        }

        // The buffer's waiting in the outgoing queue.
        if (-1 == ioctl(device, VIDIOC_DQBUF, &bufInfo)) {
            perror("VIDIOC_QBUF");
            exit(1);
        }

        processYuy2Frame(frameNum);

        usleep(33333); // around 30 fps
    }
}

int main() {
    int device;

    device = open("/dev/video0", O_RDWR);
    if (-1 == device) {
        perror("Could not open video device");
        return EXIT_FAILURE;
    }

    if (poke_camera(device)) {
        return EXIT_FAILURE;
    }

    if (setup_format(device)) {
        return EXIT_FAILURE;
    }

    if (init_mmap(device)) {
        return EXIT_FAILURE;
    }

    capture_video(device);

/*
    if (-1 == ioctl(device, VIDIOC_STREAMOFF)) {
        perror("Failed to stop streaming");
        return EXIT_FAILURE;
    }
*/

    close(device);
    return EXIT_SUCCESS;
}