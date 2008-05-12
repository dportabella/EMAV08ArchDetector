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
 * A VideoCapture basically provides an image each time we call cvQueryFrame.
 * This image can come from a video file (as in VideoFileReader)
 * or it can be computed (as in FilterVideoCapture, which produces an image given a VideoCapture and an imageProcessor).
 * 
 * A VideoCapture can have random access to frames (as in VideoFileReader),
 * or it cannot, as in VideoCameraCapture (a real time camera stream).
 */

#include <cassert>
#include <iostream>
using namespace std;

#include "util.h"
#include "VideoCapture.h"

//#define DEBUG

VideoFileReader::VideoFileReader(const char *videoFilename) {
	this->videoFilename = videoFilename;
	reload();

	frameRanges = NULL;
	frame = -1;
	framesToNextRange = -1;
#ifdef DEBUG
	cout << "VideoFileReader::VideoFileReader. cvGetCaptureProperty-CV_CAP_PROP_FRAME_COUNT..." << endl;
#endif
	numFrames = (int) cvGetCaptureProperty(video, CV_CAP_PROP_FRAME_COUNT);
#ifdef DEBUG
	cout << "numFrames: " << numFrames << endl;
#endif
}


VideoFileReader::VideoFileReader(const char *videoFilename, int *frameRanges, int frameRangesLength) {
	this->videoFilename = videoFilename;
	reload();

	this->frameRanges = frameRanges;
	this->frameRangesLength = frameRangesLength;
	
	frame = -1;
	rangeNumber = -1;
	framesToNextRange = 1;

#ifdef DEBUG
	cout << "VideoFileReader::VideoFileReader. cvGetCaptureProperty-CV_CAP_PROP_FRAME_COUNT..." << endl;
#endif
	int realNumFrames = (int) cvGetCaptureProperty(video, CV_CAP_PROP_FRAME_COUNT);
#ifdef DEBUG
	cout << "realNumFrames: " << realNumFrames << endl;
#endif

	numFrames = 0;
	for (int rangeNumber = 0; rangeNumber < frameRangesLength; rangeNumber++) {
		int from = frameRanges[rangeNumber*2];
		int to = frameRanges[rangeNumber*2 + 1];
		if (to < from)
			throw "range invalid. to < from";
		if (to >= realNumFrames)
			throw "range invalid. to > real num frames";

		numFrames += frameRanges[rangeNumber*2 + 1] - frameRanges[rangeNumber*2];
	}
}

int VideoFileReader::reload() {
	cout << "VideoFileReader. Reload video." << endl;

	bool exists = fileExistsAndIsReadable(videoFilename);
	if (!exists)
		throw "VideoFileReader. Cannot read file.";

	video = cvCreateFileCapture(videoFilename);
	if (!video) 
		throw "VideoFileReader. CvCapture not valid.";

	return 0;
}

IplImage * VideoFileReader::cvQueryFrame() {
	return cvQueryFrame(frame);
}

IplImage * VideoFileReader::cvQueryFrame(int _frame) {
	frame = _frame;
	goToFrame(frame);
	cout << "frame: " << (frame+1) << "\n";

	if (frame +1 < numFrames)
		frame++;

/*
	cout << "framesToNextRange:" << framesToNextRange << ", rangeNumber:" << rangeNumber << endl;

cout << "a" << endl;
	if (framesToNextRange != -1) {
cout << "b" << endl;
		if (framesToNextRange > 0) {
cout << "c" << endl;
			framesToNextRange--;
		} else {
cout << "d. frameRangesLength:" << frameRangesLength << endl;
			if (rangeNumber + 1 >= frameRangesLength) {  //finished
cout << "e" << endl;
				return NULL;
			}
	
cout << "f" << endl;
			rangeNumber++;
			frame = frameRanges[rangeNumber*2];
			cout << "change range. frame=" << frame << endl;
			::cvSetCaptureProperty(video, CV_CAP_PROP_POS_FRAMES, frame);	

			int rangeFrameEnd = frameRanges[rangeNumber*2+1];
			framesToNextRange = (rangeFrameEnd == -1) ? -1 : (1 + rangeFrameEnd - frame);
		}
	}
*/

#ifdef DEBUG
	cout << "VideoFileReader::cvQueryFrame(" << frame << "). cvQueryFrame..." << endl;
#endif
	IplImage *image = ::cvQueryFrame(video);
#ifdef DEBUG
	cout << "image != NULL:" << (image!=NULL) << endl;
#endif
	if (image == NULL)
		throw "VideoFileReader. cvQueryFrame = NULL";
	return image;
}


int VideoFileReader::goToFrame(int numFrame) {
	if (numFrame >= numFrames)
		throw "VideoFileReader::goToFrame. numFrames out of range.";

	this->frame = numFrame;

	int realNumFrame;
	if (frameRanges == NULL) {
		realNumFrame = numFrame;
	} else {
		int framesLeft = numFrame;
		for (int rangeNumber = 0; rangeNumber < frameRangesLength; rangeNumber++) {
			int framesInRange = frameRanges[rangeNumber*2 + 1] - frameRanges[rangeNumber*2];
			if (framesLeft < framesInRange) {
				realNumFrame = frameRanges[rangeNumber*2] + framesLeft;
				break;
			} else {
				framesLeft -= framesInRange;
			}
		}
	}


	//OPENCV bug: sometimes, opencv has problems going backwards.
	//this extra call is a workaround sometimes,
	//however the opencv bug should be solved!!!
	if (realNumFrame > 0) {
#ifdef DEBUG
		cout << "VideoFileReader::goToFrame. BUG FIX AROUND. cvSetCaptureProperty-CV_CAP_PROP_POS_FRAMES, realNumFrame-1=" << (realNumFrame-1) << "..." << endl;
#endif
		cvSetCaptureProperty(video, CV_CAP_PROP_POS_FRAMES, realNumFrame-1);
#ifdef DEBUG
		cout << "done" << endl;
#endif
	}

#ifdef DEBUG
	cout << "VideoFileReader::goToFrame. cvSetCaptureProperty-CV_CAP_PROP_POS_FRAMES, realNumFrame=" << realNumFrame << "..." << endl;
#endif
	int res = cvSetCaptureProperty(video, CV_CAP_PROP_POS_FRAMES, realNumFrame);
#ifdef DEBUF
	cout << "successful: " << (res != 0) << endl;
#endif
	if (res == 0) {
		cout << "Failed re-positioning." << endl;
		cout << "Trying to reload the video." << endl;
		reload();
		res = cvSetCaptureProperty(video, CV_CAP_PROP_POS_FRAMES, realNumFrame);
		if (res == 0) {
			cout << "Failed re-positioning." << endl;
		}
	}
	return 0;
}

int VideoFileReader::getNumFrames() {
	return numFrames;
}



