### Overview
Alpharad is a C based open source software aimed at helping hobbyists to conduct experiments on quantum events like alpha particles emission by radioactive Am-241 with DIY hardware (e.g. [1](https://blog.cyllene.net/2011/04/alpha-radiation-camera/), [2](http://www.inventgeek.com/alpha-radiation-visualizer/)).

This software is powered by [V4L2](https://www.linuxtv.org/) library and [OpenSSL ](https://www.openssl.org/).

A bunch of helper utilities are provided along with alpharad and can be found in `qa` directory.

### Getting started

Clone this repository
```bash
git clone https://github.com/hermes-jr/alpharad.git
```

Install dependencies
```bash
sudo apt install cmake libv4l-dev
# The following are optional
sudo apt install openssl libssl-dev libcunit1-dev
```

Build main application
```bash
cd alpharad/
mkdir cmake-build
cd cmake-build/
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

Ignore the testing phase build errors if there are any - they're not important. `alpharad` executable should appear in `bin/` now.

At this point you might need to tune your camera for better results, check the [troubleshooting](#camera-tuning) section below.

Gather some data
```bash
# Return to the project directory
cd ..
cmake-build/bin/alpharad -v 4 --hits-file=points.log
```

You should expect the following to happen
```bash
Verbosity: 4
Hits file: points.log
Initializing FPS buffer
Opening video device
Initializing video device
Start capturing

Analyzing neighbors of 582:188: { 582:188, }
Analyzing neighbors of 26:108: { 26:108, }
Analyzing neighbors of 180:398: { 180:398, 181:398, 180:399, 181:399, }
Analyzing neighbors of 613:291: { 613:291, }
...
```

The program can be interrupted with <kbd>Ctrl</kbd>+<kbd>C</kbd> or `pkill -SIGINT alpharad` at any time.

OK, if all went fine, you have `alpharad` up and running and there should be some data collected in *points.log* and a stream of random bytes saved to *out.dat*.

To analyze collected data we need to set up the QA portion of the project.

```bash
cd qa/
```

Most of our additional utils are written in Python and you might take either option to make them work:
1. Using a virtual environment. It is a cleaner approach but might require installing additional system packages and learning about venv usage.
    
    Under Ubuntu the following might be required 
    ```bash
    sudo apt-get install python3-venv
    ```
    
    Next, create and activate a clean virtual environment and install all required packages within it  
    ```bash
    cd qa/
    python3 -m venv env
    source env/bin/activate
    pip install -r requirements.txt
    ```

2. Simply install Python dependencies system-wide or locally for your user. Drawback here is the possibility of package conflicts, so this way won't necessary make your life easier.
    ```bash
    pip install -r requirements.txt
    ```

If all went fine again, running
```bash
python3 plot_data.py
```
should generate a plot for you.

![example-plot](https://user-images.githubusercontent.com/3757084/98234271-42f43c00-1f71-11eb-8a4f-f7014d84e67c.png)

If it works, you're all set. Happy experimenting!

### Usage
Check FPS, bytes per minute and some other stats:
```bash
pkill -SIGUSR1 alpharad
```
<!-- TODO: explain more options later -->

### Troubleshooting
#### Camera tuning
With default camera settings you might get many false positive detections or very low FPS due to unnecessarily long exposure times.

These packages might be required:
```bash
sudo apt install v4l-utils ffmpeg
```

Webcam tuning
```bash
v4l2-ctl --set-ctrl=exposure_auto=1
v4l2-ctl --set-ctrl=exposure_absolute=255
v4l2-ctl --set-ctrl=exposure_auto_priority=0
v4l2-ctl --set-ctrl=sharpness=255
v4l2-ctl --set-ctrl=contrast=255
```

Check if everything works
```bash
ffplay -f v4l2 -framerate 30 -video_size 640x480 -v verbose -i /dev/video0
```

![expected-capture](https://user-images.githubusercontent.com/3757084/98728581-c1564100-23aa-11eb-898b-2aba34b005e3.gif)

#### Some modes are missing
If you are missing SHA256 modes in `alpharad -M`, it means that the program was built without crypto support. Cmake is configured so that if it fails to detect OpenSSL library, the program would be successfully built without extra features. So make sure the required libraries are available and that `Compiling with OpenSSL crypto support` message appears on building stage.

#### Other bugs
Please feel free to report any bugs by [opening an issue](https://github.com/hermes-jr/alpharad/issues/new/choose).

### License
Alpharad is [GPL-3.0](https://www.gnu.org/licenses/gpl-3.0.html) licensed
