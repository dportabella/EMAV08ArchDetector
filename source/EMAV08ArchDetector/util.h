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


#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>

#ifdef WIN32                    //MsWindows
  #include <cv.h>
  #include <highgui.h>
#else                           //MacOSX
  #include <OpenCV/OpenCV.h>
#endif

extern CvFont font;
extern CvScalar CV_BLACK, CV_RED, CV_GREEN, CV_BLUE, CV_WHITE;
void utilInit();

void drawSymbol(IplImage *image, int symbolId, int xc, int yc, CvScalar color);

inline IplImage* _cvCreateImage(CvSize cvSize, int depth, int channels) {
	IplImage* image = cvCreateImage(cvSize, IPL_DEPTH_8U, channels);
	assert(image);
	assert(image->nChannels == channels);
	assert(image->widthStep == cvSize.width * channels);
	return image;
}

void computeCentroid(IplImage *image, int *xc, int *yc);

inline int Max(int x, int y) { return ( x > y ) ? x : y; }
inline int Min(int x, int y) { return ( x < y ) ? x : y; }

inline int absminus(int a, int b) {
	if (a < b)
		return b-a;
	return a-b;
}

//from matlab:
//MOD    Modulus after division.
//    MOD(x,y) is x - n.*y where n = floor(x./y) if y ~= 0.  If y is not an
//    integer and the quotient x./y is within roundoff error of an integer,
//    then n is that integer.  The inputs x and y must be real arrays of the
//    same size, or real scalars.
// 
//    The statement "x and y are congruent mod m" means mod(x,m) == mod(y,m).
// 
//    By convention:
//       MOD(x,0) is x.
//       MOD(x,x) is 0.
//       MOD(x,y), for x~=y and y~=0, has the same sign as y.
 inline double mod(double x, double y) {
	if (y == 0)
		return x;
	return x - floor(x / y) * y;
}



inline bool fileExistsAndIsReadable(const char *filename) {
	FILE *fp = fopen(filename,"r");
	if( fp ) {
		fclose(fp);
		return true;
	} else {
		return false;
	}
}

#endif
