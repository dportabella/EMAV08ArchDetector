EMAV08ArchDetector is a computer vision software to detect the arches
from a video-stream for the EMAV08 competition.

 The rules of the EMAV08 competition are given in
 http://www.dgon.de/content/pdf/emav_2008_Rules_v07.pdf
 (attached also in this zip file)
 See also the EMAV08 website for more details: http://www.dgon.de/emav2008.htm

EMAV08ArchDetector
Copyright (C) 2008 David Portabella Clotet

To contact the author:
email: david.portabella@gmail.com
web: http://david.portabella.name


This file is part of EMAV08ArchDetector

EMAV08ArchDetector is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EMAV08ArchDetector is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EMAV08ArchDetector.  If not, see <http://www.gnu.org/licenses/>.


HOW IT WORKS
+++++++++++++++++++++++++++++++++++++++++
BLOB DETECTOR
The blob detector finds a set of points, each representing the center of the ballons/plates of the arch.


LINE DETECOR
Given this set of points,
find the score for all the possible lines connecting these points using the line hough transform.
The hough transform maps the set of possible lines to the space (theta, rho),
theta is the angle of the line (0 degrees = horizontal)
and rho is the distance from the center to the line.
It has two parameters, thetaResolutionDegrees = 10, and rhoResolution = 10.


HORIZON DETECTOR
It detects the horizon line, based on the paper:
"Aircraft attitude estimation from horizon video."
IEE Electronics Letters -- 22 June 2006 -- Volume 42, Issue 13, p. 744-745
T.D. Cornall, G.K. Egan, and A. Price


ARCH DETECTOR
The ArchDetector looks for two parallel lines (normal to the horizon line) given the set of possible lines.
It detects:
 - the angle of the arch (the angle of the two parallel lines)
 - the position of the first line (rho1, using the hough transform space)
 - the position of the second line (rho2)

The arch detection exploits the video sequence info using dynamic programming,
in the sene that the position of arch should not change too much from frame to frame.
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
  int thetaResolutionDegrees = 10;
  int rhoResolution = 10;
  int angleDegreesMargin = 30;
  int rhoDistanceMin = 4;
  int rhoDistanceMax = 11;
  bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage = false;

we have a maximum of 1720 possible states for a given frame.
For instance, state 1 could be thetha = 130 degrees, rho1 = -40, rhoDistance = 30;

Given the set of possible lines, we compute the "cost" of each state as follows:
  stateCost = 6 - hough(theta, rho1) - hough(theta, rho1 + rhoDistance);
  (hough is limited to 3, as there are only three red plates at each side of the arch)

then, for each state in the current frame, we compute the transition cost from the set of all states of the
previous frames to this given state, as follows (simplifiying the details):
  transitionCost = trasnsitionCostTheta + trasnsitionCostRho1 + trasnsitionCostRhoDistance,
  transitionCostTheta = abs(newTheta - prevTheta)
  transitionCostRho1 =  abs(newRho1 - prevRho1)
  transitionCostRhoDistance = abs(newRhoDistance - prevRhoDistance)

the total cost for each pair (state in the current frame, state in the previous frame) is computed as:
  totalCost(state, previousState) = stateCost(state) + transitionCost(previousState, state) + accumulatedCost(previousState);

for the first frame, accumlatedCost(state) is just the stateCost(state).
for the following frames, accumulatedCost(state) is the minimum of totalCost(state, previousState)

the selected state for the current frame is simply the state with minimum accumulated cost. 

Thus, given the previous example, 
computing the newAccumulatedCost for the current frame may require 1720 * 1720 totalCost computations!!
(that's quite expensive).



HOW TO RUN THE SOFTWARE
+++++++++++++++++++++++++++++++++++++++++
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

For instance: EMAV08ArchDetector -if video1-ranges2.avi -cup lense0 -d arch
For MacOSX: ./EMAV08ArchDetector.app/Contents/MacOS/EMAV08ArchDetector -if video1-ranges2.avi -cup lense0 -d arch



END OF README.txt.
