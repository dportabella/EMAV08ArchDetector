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


/* See CombineFrames.cpp for more info */


#include "util.h"
#include "VideoCapture.h"

class Combine2x1Frames : public ImageProcessor {
public:
	Combine2x1Frames(ImageProcessor *imageProcessor1, ImageProcessor *imageProcessor2);
	~Combine2x1Frames();
	void init(IplImage *current_frame);
	IplImage * processImage(IplImage *draw_image);

	static void combine2x1(IplImage *outImage, IplImage *im1, IplImage *im2);

private:
	IplImage *out_image;	
	ImageProcessor *imageProcessor1;
	ImageProcessor *imageProcessor2;
};

class Combine2x2Frames : public ImageProcessor {
public:
	Combine2x2Frames(ImageProcessor *imageProcessor1, ImageProcessor *imageProcessor2, ImageProcessor *imageProcessor3, ImageProcessor *imageProcessor4);
	~Combine2x2Frames();
	void init(IplImage *current_frame);
	IplImage * processImage(IplImage *draw_image);

	static void combine2x2(IplImage *outImage, IplImage *im1, IplImage *im2, IplImage *im3, IplImage *im4);

private:
	IplImage *out_image;	
	ImageProcessor *imageProcessor1;
	ImageProcessor *imageProcessor2;
	ImageProcessor *imageProcessor3;
	ImageProcessor *imageProcessor4;
};
