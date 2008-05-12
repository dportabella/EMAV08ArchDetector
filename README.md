# EMAV08ArchDetector
The EMAV 2008 competition requires a Micro Aerial Vehicule to autonomously fly a course passing through several arches that are decorated with red ballons to help visibility.
EMAV08ArchDetector is a computer vision software that detects the position of the arches from a video-stream.
See the [EMAV08 competition rules](https://raw.githubusercontent.com/dportabella/EMAV08ArchDetector/master/emav_2008_Rules_v07.pdf) and the [EMAV08 website](http://www.dgon.de/emav2008.htm) for more details.

Sample processed video showing the detected position of the arches (click to watch):

[![input video](https://raw.githubusercontent.com/dportabella/EMAV08ArchDetector/master/EMAV08ArchDetector-screenshot.png)](https://www.youtube.com/watch?v=xI54RqRsWV0)

Competition course:

![Competition course](https://raw.githubusercontent.com/dportabella/EMAV08ArchDetector/master/emav_2008_course.png)

Sample input video (click to watch):

[![input video](http://img.youtube.com/vi/Py1LLplwxzE/0.jpg)](https://www.youtube.com/watch?v=Py1LLplwxzE)



## Contents

1. How to run it
2. Binaries and videos
3. How it works
4. How to compile
5. TODO + Known bugs


## 1. HOW TO RUN IT
The MacOSX compiled version already includes the OpenCV framework.
To run the MsWindows version, you need to install OpenCV first. See HOW TO COMPILE->For MsWindows->Steps 1 to 3.

```
EMAV08ArchDetector
 -if <filename>     reads video from the file.
 -ic                reads video from the video camera.
 -cup <id>          undistorts the video using predefined calibrations.
                    id = lense0 | lense1 | lense2 | lense3
 -cuf <filename>    undistorts the video using the calibration specified in filename.
 -d <id>            image detector.
                    id = horion | blob | arch | none. default = arch.
 -of <filename>     saves the video output into filename, no video compression.

 (either -if or -ic is mandatory, all the other options are optional)

For instance:
for MacOSX:    ./EMAV08ArchDetector.app/Contents/MacOS/EMAV08ArchDetector -if videos/video1.avi -cup lense0 -d arch
for MsWindows: EMAV08ArchDetector.exe -if videos\video1.avi -cup lense0 -d blob
```

## 2. BINARIES AND VIDEOS
The binaries and videos can be found [here](https://www.dropbox.com/sh/vp9mcoytpdhhtfh/AAAnh4vtlwnFFW2ffo_B_nGRa?dl=0).

I didn't manage to convert the videos to a "standard" format, compatible both with OpenCV and VLC, for both MsWindows and OSX.
Try using [VLC](http://www.videolan.org/vlc/), QuickTime, MsWindows Media Player. You can also use our player based on [OpenCV codecs](http://opencvlibrary.sourceforge.net/VideoCodecs), which allows watching a video frame by frame:
```
For MsWindows: EMAV08ArchDetector.exe -d none -if videos\video1-ranges2.avi
For MacOSX:    ./EMAV08ArchDetector.app/Contents/MacOS/EMAV08ArchDetector -d none -if videos/video1-ranges2.avi
``` 
Some usefull commands to convert between video formats:
```
$ mencoder in.avi -ovc raw -vf format=i420 -o out.avi
$ ffmpeg -i video1_apple_codec.avi -qscale 0  video1.avi
# slow done the video:
$ ffmpeg -i video1.RESULT2.avi -qscale 0 -filter:v "setpts=8.0*PTS" video1.RESULT2_slow.avi
```

```
SOURCE VIDEOS
video1.avi
video2.avi
video3.m2v
video4-lense-62degrees.m2v
video5-lense-62degrees.m2v
video6-lense-78degrees.m2v

OUTPUT OF THE ARCH DETECTOR:
video1.RESULT1.avi           (Realtime video, using EMAV08ArchDetector)
video1.RESULT2.avi           (Better result, because of the better blob detector)
video1.RESULT2.avi.ff.mpg.avi
  (same as video1.RESULT2.avi, but it is readable by our frame by frame player in OSX)
```

The second result was produced by the Matlab version of the EMAV08ArchDetector.
The code for the blob detector can be found in `source/EMAV08ArchDetector/MorphBlobDetector_alternative.m`.
Anyway, the blob detector needs to be redone from scratch, as it does not work with the other videos,
and currently is the most expensive component of the arch detector (even more than dynamic programming!!)

---
The test input videos have been produced by the team "EPFL-AILE A RIEN", Copyright 2008.
Team: Adrien Briod, Alexandre Habersaat, Laurent Coutard, David Portabella.
With the support of the EPFL-LIS laboratory: Severin Leven and Jean-Christophe Zufferey.


## 3. HOW IT WORKS
### BLOB DETECTOR
The blob detector finds a set of points, each representing the center of the ballons/plates of the arch.


### LINE DETECOR
Given this set of points,
find the score for all the possible lines connecting these points using the line hough transform.
The hough transform maps the set of possible lines to the space (theta, rho),
theta is the angle of the line (0 degrees = horizontal)
and rho is the distance from the center to the line.
It has two parameters, thetaResolutionDegrees = 10, and rhoResolution = 10.


### HORIZON DETECTOR
It detects the horizon line, based on the paper:
"Aircraft attitude estimation from horizon video."
IEE Electronics Letters -- 22 June 2006 -- Volume 42, Issue 13, p. 744-745
T.D. Cornall, G.K. Egan, and A. Price


### ARCH DETECTOR
The ArchDetector looks for two parallel lines (normal to the horizon line) given the set of possible lines.
It detects:
 - the angle of the arch (the angle of the two parallel lines)
 - the position of the first line (rho1, using the hough transform space)
 - the position of the second line (rho2)

The arch detection exploits the video sequence info using dynamic programming,
in the sense that the position of arch should not change too much from frame to frame.
The arch in each frame is characterized by a state, from a possible set of states.
A state defines:
 - the angle of the arch
 - the position of the first line (rho1)
 - the distance of the second line to the first line (rhoDistance = rho2 - rho1)

To limit the set of possible states, we compute the horizon and we look for arch vertical to the horizon line,
with a range margin of +- 30 degress (angleDegreesMargin).
For instance, if the horizon line angle is 10 degrees (0 degrees = horizontal),
then we look for two parallel lines (the arch), between 10+90+30 and 10+90-30, so, between 130 and 70 degrees.
  
For instance, given the parameters:
```
  int thetaResolutionDegrees = 10;
  int rhoResolution = 10;
  int angleDegreesMargin = 30;
  int rhoDistanceMin = 4;
  int rhoDistanceMax = 11;
  bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage = false;
```
we have a maximum of 1720 possible states for a given frame.
For instance, state 1 could be thetha = 130 degrees, rho1 = -40, rhoDistance = 30;

Given the set of possible lines, we compute the "cost" of each state as follows:
```
stateCost = 6 - hough(theta, rho1) - hough(theta, rho1 + rhoDistance);
```
  (hough is limited to 3, as there are only three red plates at each side of the arch)

then, for each state in the current frame, we compute the transition cost from the set of all states of the
previous frames to this given state, as follows (simplifiying the details):
```
  transitionCost = trasnsitionCostTheta + trasnsitionCostRho1 + trasnsitionCostRhoDistance,
  transitionCostTheta = abs(newTheta - prevTheta)
  transitionCostRho1 =  abs(newRho1 - prevRho1)
  transitionCostRhoDistance = abs(newRhoDistance - prevRhoDistance)
```
the total cost for each pair (state in the current frame, state in the previous frame) is computed as:
```
  totalCost(state, previousState) = stateCost(state) + transitionCost(previousState, state) + accumulatedCost(previousState);
```
for the first frame, accumlatedCost(state) is just the stateCost(state).
for the following frames, accumulatedCost(state) is the minimum of totalCost(state, previousState)

the selected state for the current frame is simply the state with minimum accumulated cost. 

Thus, given the previous example, 
computing the newAccumulatedCost for the current frame may require 1720 * 1720 totalCost computations!!
(that's quite expensive).


## 4. HOW TO COMPILE
### For MacOSX:

1. Download the [pre-built version of OpenCV for OSX](http://www.ient.rwth-aachen.de/~asbach/OpenCV-Private-Framework-1.1.dmg)
   [More info](http://opencvlibrary.sourceforge.net/Mac_OS_X_OpenCV_Port)

2. Place the directory "OpenCV.framework" into source/

3. Open source/EMAV08ArchDetector/EMAV08ArchDetector.xcodeproj with XCode (default) and compile

DONE.


### For MsWindows:

1. Download [OpenCV for MsWindows](http://downloads.sourceforge.net/opencvlibrary/OpenCV_1.0.exe?modtime=1161287502&big_mirror=1)
   [more info](http://sourceforge.net/projects/opencvlibrary/)

2. Install it by executing OpenCV_1.0.exe

3. Include C:\Program Files\OpenCV\bin in the PATH environment variable

4. Temporaly copy C:\Program Files\OpenCV into source\

5. Open source\EMAV08ArchDetector/EMAV08ArchDetector.sln with Ms Visual Studio and compile

DONE.



## 5. TODO + KNOWN BUGS
```
MorphBlobDetector.cpp:	//IMPORTANT!!!! TODO: try different thresholds or choose the thresholds automatically!!!
MorphBlobDetector.cpp:	//TODO: faster implementation to find the centroids.

The aim of the blob detector is to find the balloons and plates of the arch.
The matlab version (MorphBlobDetector_alternative.m) provides better results than the c++ version.
Anyway, the blob detector needs to be redone from scratch, as it does not work with the other videos,
and currently is the most expensive component of the arch detector (even more than dynamic programming!!)


ArchDetector.cpp:       //TODO: Set a weight for each thetaIdx (e.g. the vertical line should have more weight than the "vertical-10 degress" line)
ArchDetector.cpp:       //TODO: Prevent accumulatedCost0/1 to overflow...


HoughTransform.cpp:     //TODO: The hough transform of the MsWindows version hangs up
                        //      and so the ArchDetector does not work.
                        //      (the blob and horizon detector run fine).


VideoCapture.h:         //TODO: count drop frames
VideoCapture.h:         //TODO: VideoCameraCapture. re-look at the documentation, and re-implement this class accordingly...
VideoCapture.h:         //TODO: some videos have the images origin topleft or bottomleft.
                        //      take this into account!! e.g. by flipping images if they are not topleft.


VideoPlayer.cpp:        //TODO: as option, play the video at the orginal fps
                        //      (in this project, the video had to be played at maximum speed)


util.cpp:               //TODO: drawSymbol. the symbols should be cropped if partially outside the image!!
util.cpp:               //TODO: drawSymbol. optimize it!

CombineFrames.cpp:      //TODO: It could merge 1C with 3C images...
CombineFrames.cpp:	    //TODO: The implementation of Combine2x1 can be faster
```
