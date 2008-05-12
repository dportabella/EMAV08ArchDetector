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
 * Undistorts the image coming from a camera with a fisheye lense
 *
 * The camera needs to be calibrated before, using the CameraCalibrate software,
 * which takes several images of a predefined chessboard.
 */
 

#include <iostream>
using namespace std;

#include "util.h"
//#include <OpenCV/cxcore.h>
//#include <OpenCV/cv.h>
//#include <OpenCV/highgui.h>
#include "CameraUndistort.h"

float Lense0KeyValues[8] = {684.6002f, 386.9515f, 698.2037f,  242.8590f, -0.2724f, 0.0201f,  0.0069f, -0.0089f};
float Lense1KeyValues[8] = {820.3981f, 101.1703f, 855.8934f, 1845.4957f, -0.0072f, 0.0002f, -0.0047f,  0.0052f};
float Lense2KeyValues[8] = {586.4049f, 363.6123f, 597.3917f,  291.7093f, -0.2946f, 0.0048f, -0.0072f,  0.0108f};
float Lense3KeyValues[8] = {419.6732f, 382.9352f, 428.9744f,  276.1003f, -0.2826f, 0.0818f,  0.0008f, -0.0072f};


void readCameraCalibration(const char *calibFilename, CvMat **_intrinsics, CvMat **_distortion_coeffs);
void createUndistortMatrix(float *keyValues, CvMat **_intrinsics, CvMat **_distortion_coeffs);
void createUndistortMatrix(float *_intr, float *_dist, CvMat **_intrinsics, CvMat **_distortion_coeffs);


void readCameraCalibration(const char *calibFilename, CvMat **_intrinsics, CvMat **_distortion_coeffs)
{
    cout << "read_camera_calibration. START" << endl;
	
	float keyValues[8];
	
	FILE *calib_file = fopen (calibFilename, "r");
	if (!calib_file)
		throw "Cannot read camera calibration file.";

    int res = fscanf(calib_file, "%f %f %f %f %f %f %f %f\n", &keyValues[0], &keyValues[1], &keyValues[2], &keyValues[3], &keyValues[4], &keyValues[5], &keyValues[6], &keyValues[7]);
    fclose (calib_file);
	if (res != 8)
		throw "Camera calibration file not correct.";
	
	createUndistortMatrix(keyValues, _intrinsics, _distortion_coeffs);
    cout << "read_camera_calibration. END" << endl;
}


void createUndistortMatrix(float *keyValues, CvMat **_intrinsics, CvMat **_distortion_coeffs)
{
	float *_intr = new float[9];
	float *_dist = new float[4];
    _intr[1] = 0; _intr[3] = 0; _intr[6] = 0; _intr[7] = 0; _intr[8] = 1; 
    _intr[0] = keyValues[0]; _intr[2] = keyValues[1]; _intr[4] = keyValues[2]; _intr[5] = keyValues[3];
	_dist[0] = keyValues[4]; _dist[1] = keyValues[5]; _dist[2] = keyValues[6]; _dist[3] = keyValues[7];

	createUndistortMatrix(_intr, _dist, _intrinsics, _distortion_coeffs);
}


void createUndistortMatrix(float *_intr, float *_dist, CvMat **_intrinsics, CvMat **_distortion_coeffs)
{
    CvMat *intrinsics;
    CvMat *distortion_coeffs;
    intrinsics 		= cvCreateMat(3,3,CV_32FC1);
    distortion_coeffs 	= cvCreateMat(1,4,CV_32FC1);
    (*intrinsics) = cvMat(3,3,CV_32FC1, _intr);
    (*distortion_coeffs) = cvMat(4,1,CV_32FC1, _dist);

	*(_intrinsics) = intrinsics;
	*(_distortion_coeffs) = distortion_coeffs;


	//test
	float intr[3][3] = {0.0};
	float dist[4] = {0.0};
  
	for ( int i = 0; i < 3; i++)
		for ( int j = 0; j < 3; j++)
			intr[i][j] = ((float*)(intrinsics->data.ptr + intrinsics->step*i))[j];

	for ( int i = 0; i < 4; i++)
		dist[i] = ((float*)(distortion_coeffs->data.ptr))[i];
	
	printf ("%6.4f %6.4f %6.4f %6.4f %6.4f %6.4f %6.4f %6.4f\n", intr[0][0], intr[0][2], intr[1][1], intr[1][2], dist[0], dist[1], dist[2], dist[3]);
	printf("-----------------------------------------\n");
	printf("INTRINSIC MATRIX: \n");
	printf("[ %6.4f %6.4f %6.4f ] \n", intr[0][0], intr[0][1], intr[0][2]);
	printf("[ %6.4f %6.4f %6.4f ] \n", intr[1][0], intr[1][1], intr[1][2]);
	printf("[ %6.4f %6.4f %6.4f ] \n", intr[2][0], intr[2][1], intr[2][2]);
	printf("-----------------------------------------\n");
	printf("DISTORTION VECTOR: \n");
	printf("[ %6.4f %6.4f %6.4f %6.4f ] \n", dist[0], dist[1], dist[2], dist[3]);
	printf("-----------------------------------------\n");
}


CameraUndistort::CameraUndistort(const char *calibFilename) {
    readCameraCalibration(calibFilename, &intrinsics, &distortion_coeffs);
}

CameraUndistort::CameraUndistort(float *_intr, float *_dist) {
	createUndistortMatrix(_intr, _dist, &intrinsics, &distortion_coeffs);
}

CameraUndistort::CameraUndistort(float *keyValues) {
	createUndistortMatrix(keyValues, &intrinsics, &distortion_coeffs);
}

void CameraUndistort::init(IplImage *current_frame) {
	out_image    = cvCreateImage(cvSize (current_frame->width, current_frame->height), IPL_DEPTH_8U, 3);
	assert (out_image);
}

IplImage * CameraUndistort::processImage(IplImage *src_image)
{
	cvUndistort2(src_image, out_image, intrinsics, distortion_coeffs);
	return out_image;
}

CameraUndistort::~CameraUndistort() {
	cvReleaseImage(&out_image);
    cvReleaseMat(&intrinsics);
    cvReleaseMat(&distortion_coeffs);
}
