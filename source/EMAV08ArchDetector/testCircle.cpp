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
 * Find all the circles in the image
 */

#include <cassert>
#include <iostream>

#include "util.h"
#include "testCircle.h"


void TestCircle::init(IplImage *current_frame) {
	storage = cvCreateMemStorage(0);
	assert (storage);
	
	draw_image    = cvCreateImage(cvSize (current_frame->width, current_frame->height), IPL_DEPTH_8U, 3);
	assert (draw_image);
	gray_image    = cvCreateImage(cvSize (current_frame->width, current_frame->height), IPL_DEPTH_8U, 1);
	assert (gray_image);
}

TestCircle::~TestCircle() {
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&draw_image);
	cvReleaseImage(&gray_image);
}


IplImage * TestCircle::processImage(IplImage *src_image)
{
	cvCvtColor (src_image, gray_image, CV_BGR2GRAY);
	//cvSmooth( gray_image, gray_image, CV_GAUSSIAN, 9, 9 ); // smooth it, otherwise a lot of false circles may be detected
	cvSmooth( gray_image, gray_image, CV_GAUSSIAN, 3, 3 ); // smooth it, otherwise a lot of false circles may be detected
	//CvSeq* circles = cvHoughCircles( gray_image, storage, CV_HOUGH_GRADIENT, 2, gray_image->height/4, 200, 100 );
	CvSeq* circles = cvHoughCircles( gray_image, storage, CV_HOUGH_GRADIENT, 1, gray_image->height/4, 100, 70 );
	
	cvCopy (src_image, draw_image);
	for(int i = 0; i < circles->total; i++ )
	{
		float* p = (float*)cvGetSeqElem( circles, i );
		cvCircle( draw_image, cvPoint(cvRound(p[0]),cvRound(p[1])), 3, CV_RGB(0,255,0), -1, 8, 0 );
		cvCircle( draw_image, cvPoint(cvRound(p[0]),cvRound(p[1])), cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );
	}
	
	sprintf(str,"Circles: %d", circles->total);
	cvPutText(draw_image,str,cvPoint(15,70),&font, CV_BLACK);
	
	return draw_image;
}

