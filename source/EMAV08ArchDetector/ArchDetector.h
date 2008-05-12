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
 
 
/* See ArchDetector.cpp for more info */



#include "util.h"
#include "CameraUndistort.h"
#include "MorphBlobDetector.h"
#include "HoughTransform.h"
#include "HorizonDetector.h"
#include "VideoCapture.h"

class ArchDetector : public ImageProcessor {
public:
	ArchDetector();
	ArchDetector(int thetaResolutionDegrees, int rhoResolution, int angleDegreesMargin, int rhoDistanceMin, int rhoDistanceMax, bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage);
	void init(IplImage *current_frame);
	IplImage * processImage(IplImage *srcImage);


private:
	~ArchDetector();
	void initDynamicProgrammingTables();
	void computeNewAccumulatedCost();
	int  computeThisNewAccumulatedCost(int *previousAccumulatedCost, int newStateCost, int newThetaIdx, int newRho1Idx, int newRhoDistance);

	MorphBlobDetector *blobDetector;
	HorizonDetector *horizonDetector;

	double thetaResolutionDegrees, rhoResolution;
	int angleDegreesMargin;
	int rhoDistanceMin, rhoDistanceMax;
	bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage;
	
	LineHoughTransform *hough;
	double *rho, *theta;
	int rhoLen, thetaLen;
	double *sint, *cost;
	
	int *thetaIdxMin, *thetaIdxMax;
	int *rho1IdxMin, *rho1IdxMax;
	int *rhoDistanceMaxTable;
	int *numStates;
	struct StateDescription {
		int thetaIdx;
		int rho1Idx;
		int rhoDistance;
	};
	typedef StateDescription* StateDescriptionPtr;
	StateDescriptionPtr *stateDescription;
	StateDescription *currentStateDesc;
	int *accumulatedCost0; 
	int *accumulatedCost1; 
	int previousAccumulatedCostVector;
	int previousThetaCenterIdx; 
	

	int width, height;
	IplImage *mixedImage, *multipleImage;
	IplImage *temp3CImage1, *temp3CImage2;
	IplImage *tempH;
	IplImage *HImage;
};
