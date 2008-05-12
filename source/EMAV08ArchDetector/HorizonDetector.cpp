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
 * HORIZON DETECTOR
 * It detects the horizon line, based on the paper:
 * "Aircraft attitude estimation from horizon video."
 * IEE Electronics Letters -- 22 June 2006 -- Volume 42, Issue 13, p. 744-745
 * T.D. Cornall, G.K. Egan, and A. Price
 */

#include <cmath>
#include <cassert>
#include <iostream>
using namespace std;

#include "util.h"
#include "HorizonDetector.h"


void HorizonDetector::init(IplImage *current_frame) {
	cout << "HorizonDetector. init" << endl;
	width = current_frame->width;
	height = current_frame->height;

	grayImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	assert(grayImage);
	assert(grayImage->nChannels == 1);
	assert(grayImage->widthStep == width);
	grayData     = (uchar*) grayImage->imageData;

	horizonImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	assert(horizonImage);
	horizonData     = (uchar*) grayImage->imageData;


	//initialize the circle
	//  circleX[20][0] is the x point where the circle starts at line 20
	//  circleX[20][1] is the width of the circle at line 20
	circleX = new int[height][2];
	int xc = width / 2;
	int yc = height / 2;
	int d2 = height * height / 4; // (height/2)^2
	circlePixels = 0;
	for (int y = 0; y < height; y++) {
		int y0 = yc - y;
		int x0 = (int) sqrt((double)(d2 - y0*y0));
		circleX[y][0] = xc - x0;
		int width = 2 * x0 + 1;
		circleX[y][1] = width;
		circlePixels += width;
	}
	/*
	circleImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	for (int y=0;y<height;y++)
		for (int x=0;x<width;x++)
			circleData[y*width + x] = ((x-xc)*(x-xc) + (y-yc)*(y-yc)) < d2;
	*/

	horizon.image = grayImage;
}


Horizon* HorizonDetector::computeHorizon(IplImage *srcImage)
{
	//cout << "H1" << endl;
	uchar  *srcData     = (uchar*) srcImage->imageData;
	int    srcIwd       = srcImage->widthStep;
	int    srcChannels  = srcImage->nChannels;
	assert(srcChannels == 3);
	assert(srcIwd == width*srcChannels);

	//compute gray image
	uchar *dataYX = srcData;
	uchar *grayDataYX = grayData;
	for (int y=0;y<height;y++) {
		for (int x=0;x<width;x++) {
			/*
			int index = y*srcIwd + x*srcChannels;
			int r = srcData[index+2];
			int g = srcData[index+1];
			int b = srcData[index];
			*/
			int b = *(dataYX++);
			int g = *(dataYX++);
			int r = *(dataYX++);

			int sum = r + g + b;
			int o;
			if (sum == 0) {
				o = 0;
			} else {
				o = 3 * b * b / sum;
				if (o > 255) o = 255;
			}

			//grayData[y*width + x] = o;
			*(grayDataYX++) = o;
		}
	}

	//cout << "H2" << endl;
	//compute black and white image
    cvThreshold(grayImage,grayImage,-1,255,CV_THRESH_BINARY+CV_THRESH_OTSU);       


	//cout << "H3" << endl;
	//find the sky and ground center
	//make sure that the image is small enough to accumate the mean, or change the code
	int skyXAccum = 0,    skyYAccum = 0;
	int groundXAccum = 0, groundYAccum = 0;
	int skyPixels = 0;
	for (int y = 0; y < height; y++) {
		int x = circleX[y][0];
		uchar *grayDataYX = grayData + y*width + x;
		uchar *grayDataYXMax = grayDataYX + circleX[y][1];
		for (; grayDataYX < grayDataYXMax; x++, grayDataYX++) {
			if (*grayDataYX) {
				skyYAccum += y;
				skyXAccum += x;
				skyPixels++;
			} else {
				groundYAccum += y;
				groundXAccum += x;
			}
		} 
	}

	//cout << "H4" << endl;
	horizon.skyX = (double)skyXAccum / skyPixels;
	horizon.skyY = (double)skyYAccum / skyPixels;
	int groundPixels = circlePixels - skyPixels;
	horizon.groundX = (double)groundXAccum / groundPixels;
	horizon.groundY = (double)groundYAccum / groundPixels;
	//cout << "skyPixels:" << skyPixels << ", circlePixels:" << circlePixels << "groundPixels:" << groundPixels << endl;
	cout << "skyX=" << horizon.skyX << ", skyY=" << horizon.skyY << endl;
	cout << "groundX=" << horizon.groundX << ", groundY=" << horizon.groundY << endl;

	//compute the angle
	horizon.angleR = CV_PI/2 + atan(-(horizon.groundY - horizon.skyY) / (horizon.groundX - horizon.skyX));
	horizon.angleD = horizon.angleR * 180 / CV_PI;
	double prop = (double)skyPixels / (skyPixels + groundPixels);
	horizon.x0 = (int)(horizon.skyX + (horizon.groundX-horizon.skyX) * prop);
	horizon.y0 = (int)(horizon.skyY + (horizon.groundY-horizon.skyY) * prop);
	horizon.a = tan(horizon.angleR);
	horizon.b = (height-horizon.y0) - horizon.a*horizon.x0;
	cout << "angleR=" << horizon.angleR << ", angleD=" << horizon.angleD << ", a=" << horizon.a << ", b=" << horizon.b << endl;

	return &horizon;
}

IplImage * HorizonDetector::showImage()
{
	//put black pixels outside the circle
	uchar *grayDataY = grayData;
	for (int y = 0; y < height; y++, grayDataY += width) {
		/*
		int x = circleX[y][0];
		int index = y*width;
		int indexMax = index + x;
		for (; index < indexMax; x++, index++) grayData[index] = 0;  //TODO: there is a faster memset function somewhere
		*/
		int circleStartX = circleX[y][0];
		memset(grayDataY, 0, circleStartX);

		/*
		index = indexMax + circleX[y][1];
		indexMax = y*width + width;
		for (; index < indexMax; x++, index++) grayData[index] = 0;  //TODO: there is a faster memset function somewhere
		*/
		int circleWidth = circleX[y][1];
		circleStartX += circleWidth;
		memset(grayDataY + circleStartX, 0, width - circleStartX);
	}

	//draw the lines and points
	cvCvtColor (grayImage, horizonImage, CV_GRAY2BGR);
	cvLine(horizonImage, cvPoint((int)horizon.skyX, (int)horizon.skyY), cvPoint((int)horizon.groundX, (int)horizon.groundY), CV_RED, 1);
	cvLine(horizonImage, cvPoint(0, height - (int)horizon.b), cvPoint(width, height - (int)horizon.b - (int)(horizon.a * width)), CV_GREEN, 2);
	drawSymbol(horizonImage, 1, horizon.x0, horizon.y0, CV_BLUE);
	drawSymbol(horizonImage, 1, (int)horizon.skyX, (int)horizon.skyY, CV_RED);
	drawSymbol(horizonImage, 1, (int)horizon.groundX, (int)horizon.groundY, CV_GREEN);
	
	return horizonImage;
}

IplImage * HorizonDetector::processImage(IplImage *srcImage)
{
	computeHorizon(srcImage);
	showImage();

	return horizonImage;
}


HorizonDetector::~HorizonDetector() {
	cvReleaseImage(&grayImage);
	cvReleaseImage(&horizonImage);

	delete[] circleX;
}
