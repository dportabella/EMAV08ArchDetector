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
 * LINE DETECOR
 * Given a set of points,
 * find the score for all the possible lines connecting these points using the line hough transform.
 * The hough transform maps the set of possible lines to the space (theta, rho),
 * theta is the angle of the line (0 degrees = horizontal)
 * and rho is the distance from the center to the line.
 * It has two parameters, thetaResolutionDegrees = 10, and rhoResolution = 10.
 */

#include "util.h"
#include "HoughTransform.h"
#include <iostream>
#include <math.h>
using namespace std;


void LineHoughTransform::init(int width, int height, double thetaResolutionDegrees, double rhoResolution) {
	cout << "LineHoughTransform. init" << endl;
	this->width = width;
	this->height = height;
	
	//theta = linspace(-90, 0, ceil(90/thetaResolution) + 1);
	//theta = [theta -fliplr(theta(2:end - 1))];
    int thetaLenL = (int) (1 + ceil(90 / thetaResolutionDegrees)); 	
	thetaLen = thetaLenL + thetaLenL -1 -1;
	cout << "thetaResolutionDegrees:" << thetaResolutionDegrees << ", thetaLen:" << thetaLen << endl;
	theta = new double[thetaLen];
	for (int tIndex = 0; tIndex < thetaLen; tIndex++)
		theta[tIndex] = tIndex * (0 - -CV_PI/2) / (thetaLenL-1) + -CV_PI/2;
	for (int tIndex = 1; tIndex < thetaLen-1; tIndex++)
		theta[tIndex+thetaLen] = -theta[thetaLenL - tIndex];


	//D = sqrt((height - 1)^2 + (width - 1)^2);
	//q = ceil(D/rhoResolution);
	//nrho = 2*q - 1;
	//rho = linspace(-q*rhoResolution, q*rhoResolution, nrho);
	double D = sqrt((double)((height - 1)*(height - 1) + (width - 1)*(width - 1)));
	int q = (int)(ceil(D / rhoResolution));
	rhoLen = 2*q - 1;
	cout << "rhoResolution:" << rhoResolution << ", rhoLen:" << rhoLen << endl;

	//TODO: The hough transform of the MsWindows version hangs up here... :-?
	cout << "initializing rho..." << endl;
	rho = new double[rhoLen];
	cout << "done" << endl;

	for (int rIndex = 0; rIndex < rhoLen; rIndex++)
		rho[rIndex] = rIndex * (q * rhoResolution - -q * rhoResolution) / (rhoLen-1) + -q * rhoResolution;

	/*
	cout << "Theta: ";
	for (int i = 0; i < thetaLen; i++)
		cout << theta[i] << " ";
	cout << endl;

	cout << "D: " << D << ", q:" << q << ", rhoLen:" << rhoLen << endl;
	cout << "Rho: ";
	for (int i = 0; i < rhoLen; i++)
		cout << rho[i] << " ";
	cout << endl;
	*/
	precomputeTables();
}

//Without a width and height, the tables rhoIdxMin and rhoIdxMax will not be valid!
void LineHoughTransform::init(double *rho, int rhoLen, double *theta, int thetaLen) {
	width = -1;
	height = -1;
	this->rho = rho;
	this->rhoLen = rhoLen;
	this->theta = theta;
	this->thetaLen = thetaLen;
	
	precomputeTables();
}

void LineHoughTransform::precomputeTables() {
	H = cvCreateMat(rhoLen, thetaLen, CV_32SC1);
	Hptr = H->data.ptr;
	Hstep = H->step;

    firstRho = rho[0];

    // Precompute the sin and cos table
    cost = new double[thetaLen];
    sint = new double[thetaLen];
    for(int i=0; i < thetaLen; i++) {
        cost[i] = cos(theta[i]);
        sint[i] = sin(theta[i]);
    }

    // Compute the factor for converting back to the rho matrix index
	slope = (rhoLen - 1)/(rho[rhoLen-1] - firstRho);



	//only some combinations of thetaIdx and rhoIdx are possible (around 40% - 45%).
	//(no point in the image could be mapped there)
	//This info is not necessary to compute the hough transform, but other applications may benefit of it
	rhoIdxMin = new int[thetaLen];
	rhoIdxMax = new int[thetaLen];
	for (int thetaIdx = 0; thetaIdx < thetaLen; thetaIdx++) {
		if (theta[thetaIdx] < 0) {
			rhoIdxMin[thetaIdx] = getRhoIdx(thetaIdx, 0, height-1);
			rhoIdxMax[thetaIdx] = getRhoIdx(thetaIdx, width-1, 0);
		} else {
			rhoIdxMin[thetaIdx] = getRhoIdx(thetaIdx, 0, 0);
			rhoIdxMax[thetaIdx] = getRhoIdx(thetaIdx, width-1, height-1);
		}
	}
}


/* Given an angle (theta), and a point(x,y),
 * find the rho for line with the given angle that passes through this point.
 */ 
int LineHoughTransform::getRhoIdx(int thetaIdx, int x, int y) {
	// x*cos(theta)+y*sin(theta)=rho
	double rho = x * cost[thetaIdx] + y * sint[thetaIdx];
	double rhoIdx_ = slope * (rho - firstRho);
	int rhoIdx = (int) (rhoIdx_ + 0.5);
	return rhoIdx;
}


void LineHoughTransform::computeHough(int (*points)[2], int numPoints)
{
	cvZero(H);

	//cout << "step:" << step << "thetaLen:" << thetaLen << endl;
	//cout << "slope:" << slope << ", firstRho:" << firstRho << endl;

    // Compute the hough transform
	for(int pointIndex = 0; pointIndex < numPoints; pointIndex++) {
		//cout << "x:" << points[pointIndex][0] << ", y:" << points[pointIndex][1] << endl;
		for(int thetaIdx = 0; thetaIdx < thetaLen; thetaIdx++) {
			// x*cos(theta)+y*sin(theta)=rho
			double rho = points[pointIndex][0] * cost[thetaIdx] + points[pointIndex][1] * sint[thetaIdx];
			double rhoIdx_ = slope * (rho - firstRho);
			int rhoIdx = (int) (rhoIdx_ + 0.5);
			((int*)(Hptr + Hstep * rhoIdx))[thetaIdx]++;
			//HData[rhoIdx * thetaLen + thetaIdx]++;
			//cout << "thetaIdx:" << thetaIdx << ", rhoIdx:" << rhoIdx << endl;
        }
    }   
}



LineHoughTransform::~LineHoughTransform() {
	delete rho;
	delete theta;
	delete cost;
	delete sint;
	delete rhoIdxMin;
	delete rhoIdxMax;
	cvReleaseMat(&H);
}
