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


/* See CameraUndistort.cpp for more info */



#ifndef __CAMERA_UNDISTORT_H
#define __CAMERA_UNDISTORT_H

#include "util.h"
#include "VideoCapture.h"

class CameraUndistort : public ImageProcessor {
public:
	CameraUndistort(const char *calibFilename);
	CameraUndistort(float *_intr, float *_dist);
	CameraUndistort(float *keyValues);
	~CameraUndistort();

	void init(IplImage *current_frame);
	IplImage * processImage(IplImage *draw_image);

private:
	IplImage *  out_image;
    CvMat *intrinsics;
    CvMat *distortion_coeffs;
};

extern float Lense0KeyValues[8];
extern float Lense1KeyValues[8];
extern float Lense2KeyValues[8];
extern float Lense3KeyValues[8];


#endif
