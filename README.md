# CS 126 Final Project: Air Piano
## Overview
Air Piano enables users to play the piano on the computer using hand gestures. Users can press a key by simply bending their finger - just like playing a normal piano! 

## Build Requirements:
To run/edit this project, you would need:
* Visual Studio 2015 (Community)
* Cinder (v0.9.2)
* OpenCV (v 3.0)
## Additional Requirements:
* A webcam
* A pair of gloves(optional but recommended).

## Installation

####Installing Cinder + VS
1) Install Visual Studio 2015 from [here](https://my.visualstudio.com/Downloads?q=visual%20studio%202015&wt.mc_id=o%7Emsft%7Evscom%7Eolder-downloads)
2) Install Cinder [here](https://libcinder.org/download).
3) After downloading the .zip file from Cinder, extract the folder to a convenient location (we'll call the extracted folder ~Cinder throughout this installation guide). 
4) Build the ~Cinder folder using an IDE 
5) Create a folder named "my-projects" inside the ~Cinder folder. Clone this repo in the my-projects folder.

####Installing OpenCV
1) Follow the installation instructions for OpenCV from [here](https://docs.opencv.org/master/d3/d52/tutorial_windows_install.html). You must build static library files.
2) To build static library files, set the "BUILD_SHARED_LIBS" option in CMakeGUI to false.
3) In the project CMake file, add your OpenCV installation path to line Number 28, after "OPENCV_DIR".

## Usage
* Create a file named "assets" in project directory, and download all the audio files [here](https://drive.google.com/drive/folders/1maoL-CzKkF1AZgK4RKIQjbxkHjMYfokx?usp=sharing) in that folder.
* Before running the program, follow the instructions on Line 28 in gesture_wrapper.h and 
* For optimal performance, use gloves and sit in a static background(no moving objects in the background) with constant lighting. The user's face can appear in the webcam stream, as the program will automatically filter it out.
* If not using gloves, then choose an environment with a plain background (or one that has colors different from that of your hands)
* After running the program, first press the 'H' key to filter out the background using HSV calibration. Move the trackbars around and try to remove as much of the background as possible. Complete Background removal is not necessary. Press the H key to end the HSV calibration.
* Next, press the 'B' key. A background subtraction window will pop up. Slightly move to your left and right while background subtraction takes place. This is to take account of slight movements you make while playing the piano.
* Once you are done, press the B key again to end background subtraction. Only your hands must appear in the "Combined Window".
* Press the 'Y' key to start playing the piano!
* To change the window size, or any other adjustable variables, simply change their values in the config.json file provided. The window's size variables are saved as output_window_length/height in the config file.

