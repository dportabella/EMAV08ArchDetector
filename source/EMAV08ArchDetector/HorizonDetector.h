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


/* See HorizonDetector.cpp for more info */


#ifndef __HORIZON_DETECTOR_H
#define __HORIZON_DETECTOR_H

#include "util.h"
#include "VideoCapture.h"

struct Horizon {
	IplImage *image;
	double skyX, skyY;
	double groundX, groundY;
	double angleR;  //angle in radians
	double angleD;  //angle in degress
	double a;       //y = a*x + b
	double b;
	int x0, y0;     //a point, intersecting the line center sky - center ground, and the horizon line
};


class HorizonDetector : public ImageProcessor {
public:
	void init(IplImage *current_frame);
	Horizon * computeHorizon(IplImage *srcImage);
	IplImage * showImage();
	IplImage * processImage(IplImage *srcImage);
	~HorizonDetector();

	Horizon horizon;

private:
	int width, height;
	IplImage *grayImage;    uchar *grayData;
	IplImage *horizonImage; uchar *horizonData;

	int (*circleX)[2];  //this is the awkward syntax in C++ for later having circleX = new int[height][2];
	int circlePixels;
};

#endif
