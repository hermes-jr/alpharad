#!/bin/sh
v4l2-ctl --set-ctrl=exposure_auto=1
v4l2-ctl --set-ctrl=exposure_absolute=255
v4l2-ctl --set-ctrl=exposure_auto_priority=0
v4l2-ctl --set-ctrl=sharpness=255
v4l2-ctl --set-ctrl=contrast=255
