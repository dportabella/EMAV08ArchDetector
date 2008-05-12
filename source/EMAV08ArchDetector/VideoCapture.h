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


/* See VideoCapture.cpp for more info */


#ifndef __VIDEO_CAPTURE_H
#define __VIDEO_CAPTURE_H


#include <cassert>
#include <iostream>
using namespace std;
#include "util.h"

#ifdef WIN32
  #include <cvcam.h>
#endif

//TODO: some videos have the images origin topleft or bottomleft.
//      take this into account!! e.g. by flipping images if they are not topleft.


class VideoCapture {
public:
    virtual IplImage * cvQueryFrame() = 0;
    virtual IplImage * cvQueryFrame(int frame) { goToFrame(frame); return cvQueryFrame(); }
	virtual bool allowsRandomAccess() { return false; }
	virtual int reload() { cout << "Reload not permitted" << endl; return -1; }
	virtual int goToFrame(int numFrame) { cout << "goToFrame not permitted" << endl; return -1; }
	virtual int getNumFrames() { cout << "getNumFrames not permitted" << endl; return -1; }
};


//TODO: re-look at the documentation, and re-implement this class accordingly...
//TODO: count drop frames
class VideoCameraCapture : public VideoCapture {
public:

	VideoCameraCapture(int index = -1) {
#ifdef WIN32
		vc = cvCreateFileCapture("");
		//IplImage *frame = cvQueryFrame(vc);
		int numCams = cvcamGetCamerasCount(); //cvcam initialization
		if (index == -1) index = 0;
		cout << "VideoCameraCapture. There are " << numCams << " cameras available." << endl;
		cout << "Taking index=" << index << endl;
		if (index >= numCams)
			throw "this camera not available.";		cvcamSetProperty(index, CVCAM_PROP_ENABLE , CVCAMTRUE);
		if( !cvcamInit() ) throw "Cannot initialize the video camera.";
		cvcamGetProperty(index, CVCAM_VIDEOFORMAT, NULL);
		cvcamStart();
#else
		vc = cvCreateCameraCapture(index);
#endif
	}

	~VideoCameraCapture() {
#ifdef WIN32
		cvcamStop();
		cvcamExit();
#else
		cvReleaseCapture(&vc);
#endif
	}
	
	IplImage * cvQueryFrame()  { return ::cvQueryFrame(vc);}


private:
	CvCapture *vc;
};







class VideoFileReader : public VideoCapture {
	const char *videoFilename;
    CvCapture *video;
    int *frameRanges;
	int frameRangesLength;
	int rangeNumber;
	int framesToNextRange;
	int numFrames;
	int frame;

public:
	VideoFileReader(const char *videoFilename);
	VideoFileReader(const char *videoFilename, int *frameRanges, int frameRangesLength);

	IplImage * cvQueryFrame();
	IplImage * cvQueryFrame(int frame);

	int goToFrame(int numFrame);
	int getNumFrames();
	int reload();
	bool allowsRandomAccess() { return true; }
};


class ImageProcessor {
public:
	virtual void init(IplImage *src_image) = 0;
	virtual IplImage * processImage(IplImage *src_image) = 0;
};


class FilterVideoCapture : public VideoCapture {
public:
	FilterVideoCapture(VideoCapture *vc, ImageProcessor *imageProcessor) {
		this->vc = vc;
		this->imageProcessor = imageProcessor;
		inited = false;
	}

    IplImage *cvQueryFrame() {
		IplImage * image = vc->cvQueryFrame();
		if (!inited) {
			imageProcessor->init(image);
			inited = true;
		}
		if (image == NULL)
			return NULL;
		return imageProcessor->processImage(image);
	}

    IplImage *cvQueryFrame(int frame) {
		IplImage * image = vc->cvQueryFrame(frame);
		if (!inited) {
			imageProcessor->init(image);
			inited = true;
		}
		return imageProcessor->processImage(image);
	}


	int reload() { return vc->reload(); }
	int goToFrame(int numFrame) { return vc->goToFrame(numFrame); }
	int getNumFrames() { return vc->getNumFrames(); }
	bool allowsRandomAccess() { return vc->allowsRandomAccess(); }
private:
	VideoCapture *vc;
	ImageProcessor *imageProcessor;
	bool inited;
};



class ImageVideoCapture : public VideoCapture {
public:
	ImageVideoCapture(const char *fileName) { image = cvLoadImage(fileName); }
	~ImageVideoCapture() { cvReleaseImage(&image); }
	IplImage * cvQueryFrame() { return image; }
	int getNumFrames() { return 1; }
	bool allowsRandomAccess() { return true; }
	int goToFrame(int numFrame) { return 0; }

private:
	IplImage *image;
};



#endif

