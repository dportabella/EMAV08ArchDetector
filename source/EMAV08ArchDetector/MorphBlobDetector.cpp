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
 * The matlab implementation was a little better, but still,
 * it was miserably failing for the new videos we took.
 * The blob detector needs to be really improved.
 */

#include <cmath>
#include <cassert>
#include <iostream>
using namespace std;

#include "util.h"
#include "MorphBlobDetector.h"
#include "CombineFrames.h"


void MorphBlobDetector::init(IplImage *current_frame) {
	cout << "MorphBlobDetector. init" << endl;
	width = current_frame->width;
	height = current_frame->height;

	grayImage    = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	cannyImage   = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	blobsImage   = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	mixedImage    = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	multipleImage    = _cvCreateImage(cvSize(width*2, height), IPL_DEPTH_8U, 3);

	temp1CImage  = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	temp3CImage  = _cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	blobCentroid = NULL;
}


MorphBlobDetector::~MorphBlobDetector() {
	cvReleaseImage(&grayImage);
	cvReleaseImage(&cannyImage);
	cvReleaseImage(&blobsImage);
	cvReleaseImage(&mixedImage);
	cvReleaseImage(&multipleImage);
	cvReleaseImage(&temp1CImage);
	cvReleaseImage(&temp3CImage);
	delete[] blobCentroid;
}

//cvZero(cannyImage);
//cvCircle(cannyImage, cvPoint(150,200), 50, cvRealScalar(255));
//cvCircle(cannyImage, cvPoint(150,200), 20, cvRealScalar(255));
void MorphBlobDetector::findBlobs(IplImage *srcImage)
{
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contour = NULL;

	cvCvtColor (srcImage, grayImage, CV_BGR2GRAY);
	
	//edge detector
	//IMPORTANT!!!! TODO: try different thresholds or choose the thresholds automatically!!!
	cvCanny(grayImage, cannyImage, 100, 200); 
	cvCopy(cannyImage, temp1CImage);

	//get all the blob countours
	int contoursFound = cvFindContours(temp1CImage, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

	//prepare vector blobcentroids, maximum size
	if (blobCentroid) delete[] blobCentroid;
	blobCentroid = new int[contoursFound][2];
	numBlobs = 0;
	
	cvZero(blobsImage);
	//const double roundnessThreshold = 0.1;
	const double roundnessThreshold = 0.05;
	while(contour) {
		double area = -cvContourArea(contour, CV_WHOLE_SEQ);
		bool keep = (area != 0);

		if (keep) {
			// remove the blobs that are not "round", according to the roundness metric.
			double length = cvArcLength(contour, CV_WHOLE_SEQ, -1);
			double roundnessMetric = 4 * CV_PI * area / (length*length);
			keep = roundnessMetric > roundnessThreshold;
			//cout << "area:" << area << ", length:" << length << ", roundnessMetric:" << roundnessMetric << endl;
		}

		if (keep) {
			//find the centroids. TODO: faster implementation!
			int x, y;
			cvZero(temp1CImage);
			cvDrawContours(temp1CImage, contour, CV_WHITE, CV_WHITE, -1, CV_FILLED, 8);
			computeCentroid(temp1CImage, &x, &y);
			//cout << "x=" << x << ", y=" << y << endl;
			if (x == -1) {
				cout << "There is something wrong. area = " << area << ", however cvDrawCountours drawed nothing." << endl;
			} else {
				blobCentroid[numBlobs][0] = x;
				blobCentroid[numBlobs][1] = y;
				numBlobs++;
			
				//just for visualizing
				cvCopy(temp1CImage, blobsImage, temp1CImage);   //add this blob to the blobsImage
				//cvDrawContours(blobsImage, contour, CV_WHITE, CV_WHITE, -1, CV_FILLED, 8);
			}
		}
		contour = contour->h_next;
	}
	cvReleaseMemStorage(&storage); // desallocate CvSeq as well.
}


IplImage * MorphBlobDetector::processImage(IplImage *srcImage)
{
	findBlobs(srcImage);

	//mixed image. draw blobs
	cvCopy(srcImage, mixedImage);
	cvCvtColor (blobsImage, temp3CImage, CV_GRAY2BGR);
	cvCopy(temp3CImage, mixedImage, blobsImage);

	//mixed image. draw blob centrois
	for (int i = 0; i < numBlobs; i++)
		drawSymbol(mixedImage, 1, blobCentroid[i][0], blobCentroid[i][1], CV_GREEN);


	//mixed image + canny
	cvCvtColor (cannyImage, temp3CImage, CV_GRAY2BGR);
	Combine2x1Frames::combine2x1(multipleImage, mixedImage, temp3CImage);

	//return edgesImage;
	return multipleImage;
}




