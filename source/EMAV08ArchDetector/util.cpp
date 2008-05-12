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


#include "util.h"

#include <cassert>
#include <iostream>

using namespace std;


CvFont font;
CvScalar CV_BLACK, CV_RED, CV_GREEN, CV_BLUE, CV_WHITE;
void utilInit() {
	cvInitFont(&font,CV_FONT_VECTOR0,0.8,0.8,0.0,2);
	CV_BLACK = CV_RGB(0,0,0);
	//CV_BLACK = CV_RGB(255,255,255);
	CV_RED = CV_RGB(255,0,0);
	CV_GREEN = CV_RGB(0,255,0);
	CV_BLUE = CV_RGB(0,0,255);
	CV_WHITE = CV_RGB(255,255,255);
//	printf("CLOCKS_PER_SEC: %d\n", CLOCKS_PER_SEC);
}


//TODO: the symbols should be cropped if partially outside the image!!
//TODO: optimize it!
void drawSymbol(IplImage *image, int symbolId, int xc, int yc, CvScalar color) {
	uchar  *srcData  = (uchar*) image->imageData;
	int    iwd       = image->widthStep;
	int    channels  = image->nChannels;
	assert(channels == 3);

	//TODO: this can be done faster. or maybe there is function already done somewhere...
	for (int y = yc-4; y <= yc+4; y++) {
		int index = y*iwd + xc*channels;
		srcData[index+2] = (uchar)color.val[2];
		srcData[index+1] = (uchar)color.val[1];
		srcData[index] = (uchar)color.val[0];
	}		
	for (int x = xc-4; x <= xc+4; x++) {
		int index = yc*iwd + x*channels;
		srcData[index+2] = (uchar)color.val[2];
		srcData[index+1] = (uchar)color.val[1];
		srcData[index] = (uchar)color.val[0];
	}		
}

/* 
 * Computes the center of the blob.
 * The blob consists on all the pixels with value != 0
 *
 * Returns *xc = -1 if there are not such pixels (blob area = 0)
 */
void computeCentroid(IplImage *image, int *xc, int *yc) {
	uchar  *data  = (uchar*) image->imageData;
	int    iwd       = image->widthStep;
	assert(image->nChannels == 1);

	int width = image->width;
	int height = image->height;

	//make sure that the image is small enough to accumate the mean, or change the code
	int xAccum = 0, yAccum = 0;
	int numPixels = 0;
	uchar *dataY = data;
	for (int y = 0; y < height; y++, dataY+=iwd) {
		uchar *dataX = dataY;
		for (int x = 0; x < width; x++, dataX++) {
			if (*dataX) {
				xAccum += x;
				yAccum += y;
				numPixels++;
			}
		}	
	}
	if (numPixels == 0) {
		*xc = -1;
		return;
	}

	*xc = xAccum / numPixels;
	*yc = yAccum / numPixels;
	//cout << "xAccum:" << xAccum << ", yAccum:" << yAccum << ", numPixels:" << numPixels << endl;
}
