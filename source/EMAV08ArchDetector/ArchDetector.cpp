/*
 * EMAV08ArchDetector is a computer vision software to detect the arches
 * from a video-stream for the EMAV08 competition.
 *
 *  The rules of the EMAV08 competition are given in
 *  http://www.dgon.de/content/pdf/emav_2008_Rules_v07.pdf
 *  (attached also in this zip file)
 *  See also the EMAV08 website for more details: http://www.dgon.de/emav2008.htm
 *
 * EMAV08ArchDetector
 * Copyright (C) 2008 David Portabella Clotet
 *
 * To contact the author:
 * email: david.portabella@gmail.com
 * web: http://david.portabella.name
 * 
 *
 * This file is part of EMAV08ArchDetector
 *  
 * EMAV08ArchDetector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EMAV08ArchDetector is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EMAV08ArchDetector.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * BLOB DETECTOR
 * The blob detector finds a set of points, each representing the center of the ballons/plates of the arch.
 *
 *
 * LINE DETECOR
 * Given this set of points,
 * find the score for all the possible lines connecting these points using the line hough transform.
 * The hough transform maps the set of possible lines to the space (theta, rho),
 * theta is the angle of the line (0 degrees = horizontal)
 * and rho is the distance from the center to the line.
 * It has two parameters, thetaResolutionDegrees = 10, and rhoResolution = 10.
 *
 *
 * HORIZON DETECTOR
 * It detects the horizon line, based on the paper:
 * "Aircraft attitude estimation from horizon video."
 * IEE Electronics Letters -- 22 June 2006 -- Volume 42, Issue 13, p. 744-745
 * T.D. Cornall, G.K. Egan, and A. Price
 *
 *
 * ARCH DETECTOR
 * The ArchDetector looks for two parallel lines (normal to the horizon line) given the set of possible lines.
 * It detects:
 *  - the angle of the arch (the angle of the two parallel lines)
 *  - the position of the first line (rho1, using the hough transform space)
 *  - the position of the second line (rho2)
 * 
 * The arch detection exploits the video sequence info using dynamic programming,
 * in the sene that the position of arch should not change too much from frame to frame.
 * The arch in each frame is characterized by a state, from a possible set of states.
 * A state defines:
 *  - the angle of the arch
 *  - the position of the first line (rho1)
 *  - the distance of the second line to the first line (rhoDistance = rho2 - rho1)
 *
 * To limit the set of possible states, we compute the horizon and we look for arch vertical to the horizon line,
 * with a range margin of +- 30 degress (angleDegreesMargin).
 * For instance, if the horizon line angle is 10 degrees (0 degrees = horizontal),
 * then we look for two parallel lines (the arch), between 10+90+30 and 10+90-30, so, between 130 and 70 degrees.
 *   
 * For instance, given the parameters:
 *   int thetaResolutionDegrees = 10;
 *   int rhoResolution = 10;
 *   int angleDegreesMargin = 30;
 *   int rhoDistanceMin = 4;
 *   int rhoDistanceMax = 11;
 *   bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage = false;
 *
 * we have a maximum of 1720 possible states for a given frame.
 * For instance, state 1 could be thetha = 130 degrees, rho1 = -40, rhoDistance = 30;
 *
 * Given the set of possible lines, we compute the "cost" of each state as follows:
 *   stateCost = 6 - hough(theta, rho1) - hough(theta, rho1 + rhoDistance);
 *   (hough is limited to 3, as there are only three red plates at each side of the arch)
 *
 * then, for each state in the current frame, we compute the transition cost from the set of all states of the
 * previous frames to this given state, as follows (simplifiying the details):
 *   transitionCost = trasnsitionCostTheta + trasnsitionCostRho1 + trasnsitionCostRhoDistance,
 *   transitionCostTheta = abs(newTheta - prevTheta)
 *   transitionCostRho1 =  abs(newRho1 - prevRho1)
 *   transitionCostRhoDistance = abs(newRhoDistance - prevRhoDistance)
 *
 * the total cost for each pair (state in the current frame, state in the previous frame) is computed as:
 *   totalCost(state, previousState) = stateCost(state) + transitionCost(previousState, state) + accumulatedCost(previousState);
 *
 * for the first frame, accumlatedCost(state) is just the stateCost(state).
 * for the following frames, accumulatedCost(state) is the minimum of totalCost(state, previousState)
 *
 * the selected state for the current frame is simply the state with minimum accumulated cost. 
 *
 * Thus, given the previous example, 
 * computing the newAccumulatedCost for the current frame may require 1720 * 1720 totalCost computations!!
 * (that's quite expensive).
 */


#include <cmath>
#include <cassert>
#include <iostream>
using namespace std;

#include "util.h"
#include "ArchDetector.h"
#include "CombineFrames.h"
#include "CameraUndistort.h"
#include "MorphBlobDetector.h"
#include "HoughTransform.h"
#include "HorizonDetector.h"




ArchDetector::ArchDetector() {
	thetaResolutionDegrees = 10;
	rhoResolution = 10;
	angleDegreesMargin = 20;
	//angleDegreesMargin = 10;
	rhoDistanceMin = 4;
	rhoDistanceMax = 11;
	allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage = false;
}

ArchDetector::ArchDetector(int thetaResolutionDegrees, int rhoResolution, int angleDegreesMargin, int rhoDistanceMin, int rhoDistanceMax, bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage) {
	this->thetaResolutionDegrees = thetaResolutionDegrees;
	this->rhoResolution = rhoResolution;
	this->angleDegreesMargin = angleDegreesMargin;
	this->rhoDistanceMin = rhoDistanceMin;
	this->rhoDistanceMax = rhoDistanceMax;
	this->allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage = allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage;
}

void ArchDetector::init(IplImage *current_frame) {
	cout << "ArchDetector. init" << endl;
	width = current_frame->width;
	height = current_frame->height;

	multipleImage    = _cvCreateImage(cvSize(width*2, height*2), IPL_DEPTH_8U, 3);
	mixedImage       = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	temp3CImage1     = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	temp3CImage2     = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	blobDetector = new MorphBlobDetector();
	blobDetector->init(current_frame);
	
	horizonDetector = new HorizonDetector();
	horizonDetector->init(current_frame);

	hough = new LineHoughTransform();
	hough->init(width, height, thetaResolutionDegrees, rhoResolution);
	tempH = cvCreateImage(cvSize(hough->thetaLen, hough->rhoLen), IPL_DEPTH_8U, 1);
	//tempH = cvCreateImage(cvSize(18, 91), IPL_DEPTH_8U, 1);
	HImage  = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	initDynamicProgrammingTables();
}

ArchDetector::~ArchDetector() {
	delete blobDetector;
	delete horizonDetector;
	delete hough;
	delete thetaIdxMin;
	delete thetaIdxMax;
	delete rho1IdxMin;
	delete rho1IdxMax;
	delete rhoDistanceMaxTable;
	delete numStates;
	delete stateDescription;
	delete accumulatedCost0; 
	delete accumulatedCost1; 
	cvReleaseImage(&mixedImage);
	cvReleaseImage(&multipleImage);
	cvReleaseImage(&temp3CImage1);
	cvReleaseImage(&temp3CImage2);
	cvReleaseImage(&tempH);
	cvReleaseImage(&HImage);
}


IplImage * ArchDetector::processImage(IplImage *srcImage)
{
	bool showAll = true;
	
	//BLOB DETECTOR
	blobDetector->findBlobs(srcImage);

	//HORIZON DETECTOR
	horizonDetector->computeHorizon(srcImage);

	//HOUGH TRANSFORM
	hough->computeHough(blobDetector->blobCentroid, blobDetector->numBlobs);
	cvMinS(hough->H, 3, hough->H);

	//DYNAMIC PROGRAMMING
	computeNewAccumulatedCost();

	if (showAll) {
		//COMBINE IMAGES
		//mixed image. draw blobs
		cvCopy(srcImage, mixedImage);
		cvCvtColor(blobDetector->blobsImage, temp3CImage1, CV_GRAY2BGR);
		cvCopy(temp3CImage1, mixedImage, blobDetector->blobsImage);

		//mixed image. draw horizon
		cvLine(mixedImage, cvPoint(0, height - (int)horizonDetector->horizon.b), cvPoint(width, height - (int)horizonDetector->horizon.b - (int)(horizonDetector->horizon.a * width)), CV_GREEN, 2);

		//mixed image. draw arch
		double thisRho1 = rho[currentStateDesc->rho1Idx];
		double thisRho2 = rho[currentStateDesc->rho1Idx + currentStateDesc->rhoDistance];
		int thisThetaIdx = currentStateDesc->thetaIdx;
		if (sint[thisThetaIdx] != 0) {
			cvLine(mixedImage, cvPoint(0, (int)(thisRho1/sint[thisThetaIdx])), cvPoint(width-1, (int)((thisRho1 - (width-1)*cost[thisThetaIdx])/sint[thisThetaIdx])), CV_RED, 2);
			cvLine(mixedImage, cvPoint(0, (int)(thisRho2/sint[thisThetaIdx])), cvPoint(width-1, (int)((thisRho2 - (width-1)*cost[thisThetaIdx])/sint[thisThetaIdx])), CV_BLUE, 2);
		} else {
			cvLine(mixedImage, cvPoint((int)thisRho1, 0), cvPoint((int)thisRho1, height-1), CV_RED, 2);
			cvLine(mixedImage, cvPoint((int)thisRho2, 0), cvPoint((int)thisRho2, height-1), CV_BLUE, 2);
		}

		//mixed image. draw blob centrois
		for (int i = 0; i < blobDetector->numBlobs; i++)
			drawSymbol(mixedImage, 1, blobDetector->blobCentroid[i][0], blobDetector->blobCentroid[i][1], CV_GREEN);


		//horizon
		IplImage *horizonImage = horizonDetector->showImage();

		//canny
		IplImage *canny3CImage = temp3CImage1;
		cvCvtColor(blobDetector->cannyImage, canny3CImage, CV_GRAY2BGR);

		//hough
		cvConvertScale(hough->H, tempH, 255/3, 0);
		cvResize(tempH, HImage, CV_INTER_NN);
		IplImage *H3CImage = temp3CImage2;
		cvCvtColor(HImage, H3CImage, CV_GRAY2BGR);

		//combine
		Combine2x2Frames::combine2x2(multipleImage, mixedImage, horizonImage, canny3CImage, H3CImage);
		return multipleImage;
		//return mixedImage;
	} else {
		return srcImage;
	}
}

void ArchDetector::initDynamicProgrammingTables() {
	//given that we are looking for lines of angle T, we actually look for lines
	//at angle T-AngleMargin to T+AngleMargin in order to accomdate the noise of the horiton detector

	thetaLen = hough->thetaLen;
	rhoLen = hough->rhoLen;
	theta = hough->theta;
	rho = hough->rho;
	sint = hough->sint;
	cost = hough->cost;
	

	thetaIdxMin = new int[thetaLen];
	thetaIdxMax = new int[thetaLen];
	int TIdxMargin = (int)(ceil(angleDegreesMargin / thetaResolutionDegrees));
	for (int thetaIdx = 0; thetaIdx < thetaLen; thetaIdx++) {
		thetaIdxMin[thetaIdx] = Max(0, thetaIdx - (TIdxMargin-1));    //when using this, tIdx needs to be computed as fix(tIdx)
		thetaIdxMax[thetaIdx] = Min(thetaLen-1, thetaIdx + TIdxMargin);
	}

	rho1IdxMin = new int[thetaLen];
	rho1IdxMax = new int[thetaLen];
	for (int thetaIdx = 0; thetaIdx < thetaLen; thetaIdx++) {
		int thisRho1IdxMin = hough->rhoIdxMin[thetaIdx];
		if (allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage)
			thisRho1IdxMin = Max(0, thisRho1IdxMin - (rhoDistanceMax-1));
		rho1IdxMin[thetaIdx] = thisRho1IdxMin;

		int thisRho1IdxMax = hough->rhoIdxMax[thetaIdx];
		if (allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage)
			thisRho1IdxMax = Min(rhoLen-1, thisRho1IdxMax + (rhoDistanceMax-1));
		thisRho1IdxMax -= rhoDistanceMin;
		rho1IdxMax[thetaIdx] = thisRho1IdxMax;
	}

	rhoDistanceMaxTable = new int[thetaLen*rhoLen];
	//not all the table is used, but we get faster access
	//no need to initialize the unused values to zero, as they will never be read.
	for (int thetaIdx = 0; thetaIdx < thetaLen; thetaIdx++) {
		for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= rho1IdxMax[thetaIdx]; rho1Idx++) {
			int max = Min(rhoDistanceMax, rho1IdxMax[thetaIdx] + rhoDistanceMin - rho1Idx);
			rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx] = max;
		}
	}	
	

	//get the number of states by thetaCenterIdx
	int maxNumStates = -1;
	numStates = new int[thetaLen];
	for (int thetaCenterIdx = 0; thetaCenterIdx < thetaLen; thetaCenterIdx++) {
		int thisNumStates = 0;
		for (int thetaIdx = thetaIdxMin[thetaCenterIdx]; thetaIdx <= thetaIdxMax[thetaCenterIdx]; thetaIdx++) {
			for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= rho1IdxMax[thetaIdx]; rho1Idx++) {
				thisNumStates += rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx] - rhoDistanceMin + 1;
			}
		}

		numStates[thetaCenterIdx] = thisNumStates;
		cout << "Theta: " << theta[thetaCenterIdx]*180/CV_PI << "  numStates: " << thisNumStates << endl;
		if (thisNumStates > maxNumStates)
			maxNumStates = thisNumStates;
	}	
	cout << "MaxNumStates: " << maxNumStates << endl;


	//enumate all the states, for fast indirect access afterwards
	typedef StateDescription* StateDescriptionPtr;
	stateDescription = new StateDescriptionPtr[thetaLen];
	for (int thetaCenterIdx = 0; thetaCenterIdx < thetaLen; thetaCenterIdx++) {
		stateDescription[thetaCenterIdx] = new StateDescription[numStates[thetaCenterIdx]];
		int state = 0;
		int thisThetaIdxMax = thetaIdxMax[thetaCenterIdx];
		for (int thetaIdx = thetaIdxMin[thetaCenterIdx]; thetaIdx <= thisThetaIdxMax; thetaIdx++) {
			int thisRho1IdxMax = rho1IdxMax[thetaIdx];
			for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= thisRho1IdxMax; rho1Idx++) {
				int thisRhoDistanceMax = rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx];
				for (int rhoDistance = rhoDistanceMin; rhoDistance <= thisRhoDistanceMax; rhoDistance++) {
					StateDescription desc;
					desc.thetaIdx = thetaIdx;
					desc.rho1Idx = rho1Idx;
					desc.rhoDistance = rhoDistance;
					stateDescription[thetaCenterIdx][state++] = desc;
				}
			}
		}
	}


	//initialize the main dynamic programming vector
	accumulatedCost0 = new int[maxNumStates]; 
	accumulatedCost1 = new int[maxNumStates]; 
	previousAccumulatedCostVector = -1;
	
	/*
	int thetaCenterIdx = 0;
	cout << thetaIdxMin[thetaCenterIdx] << ", " << thetaIdxMax[thetaCenterIdx] << endl;
	int thetaIdx = thetaIdxMin[thetaCenterIdx];
	cout << rho1IdxMin[thetaIdx] << ", " << rho1IdxMax[thetaIdx] << endl;
	int rho1Idx = rho1IdxMin[thetaIdx];
	cout << rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx] << endl;
	cout << rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx+1] << endl;

	for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= rho1IdxMax[thetaIdx]; rho1Idx++)
		cout << rho1Idx << ", d:" << rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx] << endl;

	cout << hough->rhoIdxMax[0] << endl;
	*/
}



void ArchDetector::computeNewAccumulatedCost() {

	//find the thetaCenter, according to the horizon
	double thetaCenter = mod(horizonDetector->horizon.angleR + CV_PI/2, CV_PI);
	thetaCenter = CV_PI/2 - thetaCenter; //angle reference in the hough transform
	int thetaCenterIdx = (int) ((thetaCenter - theta[0]) * (thetaLen -1) / (theta[thetaLen-1] - theta[0]));

	//TODO: set a weight for each thetaIdx (e.g. the vertical line should have more weight than the "vertical-10 degress" line)


	//compute the newAccumulatedCost
	int minCostState = -1;
	if (previousAccumulatedCostVector == -1) {
		int *newAccumulatedCost = accumulatedCost0;
		previousAccumulatedCostVector = 0;
		
		int minCost = 999999; //TODO: prevent accumulatedCost0/1 to overflow...
		int state = 0;
		int thisThetaIdxMax = thetaIdxMax[thetaCenterIdx];
		for (int thetaIdx = thetaIdxMin[thetaCenterIdx]; thetaIdx <= thisThetaIdxMax; thetaIdx++) {
			int thisRho1IdxMax = rho1IdxMax[thetaIdx];
			for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= thisRho1IdxMax; rho1Idx++) {
				int partialStateCost = 6 - hough->getHAt(thetaIdx, rho1Idx);
				int thisRhoDistanceMax = rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx];
				for (int rhoDistance = rhoDistanceMin; rhoDistance <= thisRhoDistanceMax; rhoDistance++) {
					int thisStateCost = partialStateCost - hough->getHAt(thetaIdx, rho1Idx + rhoDistance);
					if (thisStateCost < minCost) {
						minCost = thisStateCost;
						minCostState = state;
					}
					newAccumulatedCost[state++] = thisStateCost; 
				}
			}
		}
		cout << "FIRST. minCost: " << minCost << ", minCostState:" << minCostState << endl;
	
	} else {
		int *previousAccumulatedCost, *newAccumulatedCost;
		if (previousAccumulatedCostVector == 0) {
			previousAccumulatedCost = accumulatedCost0;
			newAccumulatedCost = accumulatedCost1;
			previousAccumulatedCostVector = 1;
		} else {
			previousAccumulatedCost = accumulatedCost1;
			newAccumulatedCost = accumulatedCost0;
			previousAccumulatedCostVector = 0;
		}

		int minCost = 999999; //TODO: prevent accumulatedCost0/1 to overflow...
		int state = 0;
		int thisThetaIdxMax = thetaIdxMax[thetaCenterIdx];
		for (int thetaIdx = thetaIdxMin[thetaCenterIdx]; thetaIdx <= thisThetaIdxMax; thetaIdx++) {
			int thisRho1IdxMax = rho1IdxMax[thetaIdx];
			for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= thisRho1IdxMax; rho1Idx++) {
				int partialStateCost = 6 - hough->getHAt(thetaIdx, rho1Idx);
				int thisRhoDistanceMax = rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx];
				for (int rhoDistance = rhoDistanceMin; rhoDistance <= thisRhoDistanceMax; rhoDistance++) {
					int stateCost = partialStateCost - hough->getHAt(thetaIdx, rho1Idx + rhoDistance);
					int thisNewAccumulatedCost = computeThisNewAccumulatedCost(previousAccumulatedCost, stateCost, thetaIdx, rho1Idx, rhoDistance);
					if (thisNewAccumulatedCost < minCost) {
						minCost = thisNewAccumulatedCost;
						minCostState = state;
					}
					newAccumulatedCost[state++] = thisNewAccumulatedCost;
				}
			}
		}
		cout << "minCost: " << minCost << ", minCostState:" << minCostState << endl;
	}
	previousThetaCenterIdx = thetaCenterIdx;

	currentStateDesc = &stateDescription[thetaCenterIdx][minCostState];
	cout << "thetaIdx:" << currentStateDesc->thetaIdx << ", rho1Idx:" << currentStateDesc->rho1Idx << ", rhoDistance:" << currentStateDesc->rhoDistance << endl;
}


int ArchDetector::computeThisNewAccumulatedCost(int *previousAccumulatedCost, int newStateCost, int newThetaIdx, int newRho1Idx, int newRhoDistance) {
	int minCost = 999999; //TODO: prevent accumulatedCost0/1 to overflow...
	
	//see the function getTransitionCost for some info about computing transitionCost.
	//getTransitionCost has been integrated in this loop to avoid repeting some calculations...
	int state = 0;
	int thisThetaIdxMax = thetaIdxMax[previousThetaCenterIdx];
	for (int thetaIdx = thetaIdxMin[previousThetaCenterIdx]; thetaIdx <= thisThetaIdxMax; thetaIdx++) {
		int diffTheta = newThetaIdx - thetaIdx;
		int transitionCostTheta = (diffTheta < 0) ? -diffTheta : diffTheta;

		int thisRho1IdxMax = rho1IdxMax[thetaIdx];
		for (int rho1Idx = rho1IdxMin[thetaIdx]; rho1Idx <= thisRho1IdxMax; rho1Idx++) {
			int transitionCostThetaAndRho1 = transitionCostTheta;
			int diffRho1 = newRho1Idx - rho1Idx;
			if (diffTheta > 0 && diffRho1 > 0) {
				transitionCostThetaAndRho1 += max(0, diffRho1 - 3);
			} else if (diffTheta < 0 && diffRho1 < 0) {
				transitionCostThetaAndRho1 += max(0, -diffRho1 - 3);
			} else if (diffRho1 < 0) {
				transitionCostThetaAndRho1 += -diffRho1;
			} else {
				transitionCostThetaAndRho1 += diffRho1;
			}

			int thisRhoDistanceMax = rhoDistanceMaxTable[thetaIdx * rhoLen + rho1Idx];
			for (int rhoDistance = rhoDistanceMin; rhoDistance <= thisRhoDistanceMax; rhoDistance++) {
				int thisPreviousAccumulatedCost = previousAccumulatedCost[state++];

				//int transitionCost = getTransitionCost(thetaIdx, rho1Idx, rhoDistance, newThetaIdx, newRho1Idx, newRhoDistance);
				int transitionCost = min(3, transitionCostThetaAndRho1 + absminus(newRhoDistance, rhoDistance));
				   //limit transitioncost to 3, otherwise a changing scenario may take too much effort.
				
				int cost = thisPreviousAccumulatedCost + transitionCost + newStateCost;
				if (cost < minCost)
					minCost = cost;
			}
		}
	}
	return minCost;
}



/*
//this function has been integrated in the loops, to avoid repeting some calculations...
inline int getTransitionCost(int prevThetaIdx, int prevRho1Idx, int prevRhoDistance, int newThetaIdx, int newRho1Idx, int newRhoDistance) {
	int costTheta = absminus(newThetaIdx, prevThetaIdx);
    int costRho1 =  absminus(newRho1Idx, prevRho1Idx);
    int costRhoDistance = absminus(newRhoDistance, prevRhoDistance);
    int cost = costTheta + costRho1 + costRhoDistance;
	return cost;
}
*/

/*
//this function works better than the previous one.
//this function has been integrated in the loops, to avoid repeting some calculations...
inline int getTransitionCost(int prevThetaIdx, int prevRho1Idx, int prevRhoDistance, int newThetaIdx, int newRho1Idx, int newRhoDistance) {
	int diffTheta = newThetaIdx - prevThetaIdx;
	int costTheta = (diffTheta < 0) ? -diffTheta : diffTheta;

	int difftRho1 = newRho1Idx - prevRho1Idx;
	int costRho1;
	
	//when there is a change in the angle, the rho is not longer "easily" comparable.
	//based on experimentation, it can change up to 3*rhoResolution
	//depending on the position of the points of the arch (at the top or at the bottom)
	if (diffTheta > 0 && diffRho1 > 0) {  
		costRho1 = max(0, difftRho1 - 3);
	} else if (diffTheta < 0 && diffRho1 < 0) {
		costRho1 = max(0, -diffRho1 - 3);
	} else if (diffRho1 < 0) {
		costRho1 = -diffRho1;
	} else {
		costRho1 = diffRho1;
	}

    int costRhoDistance = absminus(newRhoDistance, prevRhoDistance);

    int cost = costTheta + costRho1 + costRhoDistance;
	if (cost > 3)
		cost = 3;

	return cost;
}
*/


















