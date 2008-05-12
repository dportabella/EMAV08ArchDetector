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
 * Plays from a VideoCapture.
 *
 * constructor: VideoPlayer(VideoCapture *vc, const char *output_file, bool printFPS=false);
 * play(bool allowInteraction, bool startPaused, bool quitAtEndIfNotInteraction); 
 *
 * For instance:
 * VideoCapture *vc = new VideoFileReader("video.avi");
 * VideoPlayer player(vc, NULL);
 * player.play(true, true, false); 
 */

#include <cassert>
#include <iostream>
#include <time.h>

#include "util.h"
#include "VideoPlayer.h"
#include "testCircle.h"
#include "VideoCapture.h"

using namespace std;

char *playerInstructions =
	"\n"
	"PLAYER. Key bindings:\n"
	"i  prints help\n"
	"q  quit player\n"
	"\n"
	"If interaction allowed:\n"
	"x  pause video\n"
	"c  play normal (forward, normal speed)\n"
	"\n"
	"If random access video:\n"
	"v  play backward (pressing again 'v', plays backward even faster\n"
	"b  play forward (pressing again 'b', plays forward even faster\n"
	"n  goes one frame backward\n"
	"m  goes one frame forward\n"
	"h  goes 10 frames backward\n"
	"j  goes 10 frames forward\n"
	"z  goes 100 frames backward\n"
	"u  goes 100 frames forward\n"
	"6  goes 1000 frames backward\n"
	"7  goes 1000 frames forward\n"
	"5  goes to the first frame\n"
	"8  goes to the last frame\n"
	"r  reload video\n";

const char  *WINDOW_NAME  = "Player";
bool debug = false;

//TODO: as option, play the video at the orginal fps
// (in this project, the video had to be played at maximum speed)

VideoPlayer::VideoPlayer(VideoCapture *vc, const char *output_file, bool _printFPS) {
	this->vc = vc;
	randomAccess = vc->allowsRandomAccess();

	this->_printFPS = _printFPS;
	localTimes_pos = 0;

    cout << "VideoPlayer created" << endl;
    IplImage * current_frame = vc->cvQueryFrame();
    assert (current_frame);
	cout << "VideoPlayer. image size: " << current_frame->width << " x " << current_frame->height << ", depth: " << current_frame->depth << ", channels: " << current_frame->nChannels << endl;
	draw_image    = cvCreateImage(cvSize(current_frame->width, current_frame->height), current_frame->depth, current_frame->nChannels);
	//cout << "VideoPlayer. image size: " << draw_image->width << " x " << draw_image->height << ", depth: " << draw_image->depth << ", channels: " << draw_image->nChannels << endl;
	//draw_image = cvCloneImage(current_frame);
	assert (draw_image);

	if (debug)
		cout << "VideoPlayer::VideoPlayer. P2" << endl;
    cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);

    if (debug)
		cout << "VideoPlayer::VideoPlayer. P3" << endl;
	utilInit();

    if (debug)
		cout << "VideoPlayer::VideoPlayer. P4" << endl;


	video_out = NULL;
	if(output_file) {
		//remove(output_file);
		cout << "VideoPlayer. Creating video output file: " << output_file << endl;
		video_out = cvCreateVideoWriter(output_file, 0, 25, cvSize(draw_image->width, draw_image->height));
		if (video_out == NULL)
			throw "VideoPlayer. Error creating file";
		//CV_FOURCC('P', 'I', 'M', '1') for MPEG-1,
		//CV_FOURCC('M', 'J', 'P', 'G') for motion-JPEG,
		//CV_FOURCC('D', 'I', 'B', ' ') for RGB avi files, or
		//CV_FOURCC('I', 'Y', 'U', 'V') for uncompressed YUV, 4:2:0 chroma subsampled.
		//Currently these only work in Windows. 
	}
}

VideoPlayer::~VideoPlayer() {
	cvReleaseImage(&draw_image);
	closeVideoOut();
}

void VideoPlayer::closeVideoOut() {
	if (video_out) {
		cvReleaseVideoWriter(&video_out);
		printf("Output video created\n");
	}
}


void VideoPlayer::registerLocalFPS() {
#ifdef WIN32
	localTimes[localTimes_pos] = clock();
#else
	gettimeofday(&localTimes[localTimes_pos],NULL);
#endif

	localTimes_pos++; if (localTimes_pos == 10) localTimes_pos = 0;
}

double VideoPlayer::getLocalFPS() {
    int last_pos = (localTimes_pos+10-1)%10;
#ifdef WIN32
	double secs = (localTimes[last_pos] - localTimes[localTimes_pos])/(double)CLK_TCK;
#else
	double secs = localTimes[last_pos].tv_sec - localTimes[localTimes_pos].tv_sec + (double) (localTimes[last_pos].tv_usec - localTimes[localTimes_pos].tv_usec) / 1000000.0;
#endif

	double fps = 10/secs;
	cout << "local fps: " << fps << "\n";
	return fps;
}

void VideoPlayer::printLocalFPS(IplImage *image, int frame) {
	sprintf(str,"Frame: %d  fps:%.2lf", frame, getLocalFPS());
	cvPutText(image,str,cvPoint(15,30),&font, CV_BLACK);
	//cvPutText(image,str,cvPoint(15,30),&font, CV_WHITE);
}

void VideoPlayer::initGlobalFPS() {
#ifdef WIN32
	localTimes[0] = clock();
#else
	gettimeofday(&localTimes[0],NULL);
#endif

	globalFrames = 0;
}

void VideoPlayer::registerGlobalFPS() {
#ifdef WIN32
	localTimes[1] = clock();
#else
	gettimeofday(&localTimes[1],NULL);
#endif

	globalFrames++;
}

double VideoPlayer::getGlobalFPS() {
#ifdef WIN32
	double secs = (localTimes[1] - localTimes[0])/(double)CLK_TCK;
#else
	double secs = localTimes[1].tv_sec - localTimes[0].tv_sec + (double) (localTimes[1].tv_usec - localTimes[0].tv_usec) / 1000000.0;
#endif

	double fps = globalFrames/secs;
	cout << "global fps: " << fps << ", frames: " << globalFrames << ", secs: " << secs << endl;
	return fps;
}


// returns false if end of video
bool VideoPlayer::showFrame() {
	
	IplImage *current_frame = randomAccess ? vc->cvQueryFrame(frame) : vc->cvQueryFrame();
	if (current_frame == NULL)
		return false;
	
	//if (debug)
	cout << "VideoPlayer. frame:" << (frame+1) << "/" << numFrames << endl;
	registerLocalFPS();
	registerGlobalFPS();
	
	if(video_out) {
		cout << "VideoPlayer. Writing frame... ";
		cvWriteFrame(video_out, current_frame);
		cout << "done" << endl;
	}

	
	//cvFlip (current_frame, draw_image, 1);
	cvCopy (current_frame, draw_image);
	
	if (debug)
		cout << "VideoPlayer::showFrame. P3" << endl;
	if (_printFPS)
		printLocalFPS(draw_image, frame+1);
	getGlobalFPS(); //prints only in the console
	
	if (debug)
		cout << "VideoPlayer::showFrame. P4" << endl;
	cvShowImage (WINDOW_NAME, draw_image);
	
	return true;
}


void VideoPlayer::play() {
	if (vc->allowsRandomAccess())
		play(true, true, false);
	else
		play(true, false, true);
}

// If startPaused == true, allowInteraction must be true
// If startPaused == true, quitAtEndIfNotInteraction is ignored
void VideoPlayer::play(bool allowInteraction, bool startPaused, bool quitAtEndIfNotInteraction) 
{
	assert(!(startPaused == true && allowInteraction == false));
	bool hasUserInteracted = false;

	frame = 0;
	int frameStep = 1;
	if (randomAccess)
		numFrames = vc->getNumFrames();

	bool paused = startPaused;
	initGlobalFPS();

    if (debug)
		cout << "VideoPlayer::play. P2" << endl;

	bool firstTime = true;
	bool showFrameNow = true;

    while (true) {
	
		//show frame
		if (showFrameNow) {
			bool ok = showFrame();
			if (debug)
				cout << "VideoPlayer::play. P3" << endl;

			if (!ok) {
				if (!randomAccess) {
					cout << "VideoPlayer. Video End." << endl << "Press 'q' to quit." << endl;
					if (quitAtEndIfNotInteraction && !hasUserInteracted) {
						break;
					} else {
						paused = true;
					}
				} else {
					cout << "VideoPlayer. ERROR: randomAcess = true, however cvQueryFrame returned NULL" << endl;
					paused = true;
				}
			}
		}

		if (firstTime) {
			cout << playerInstructions << endl;
			if (startPaused == false) {
				// There is a bug on opencv -> cvWaitKey(1).
				// At the beginning, this will wait forever (not just 1 millisecond) until we move the mouse of press a key
				cout << "VideoPlayer:" << endl << ">>> Because of a bug in opencv, PLEASE JUST MOVE THE MOUSE or press a key to continue. <<<" << endl;
			}
		}
	

		//a command from the user?
		//cout << "Waiting for a key. paused=" << paused << "." << endl;
        int key = paused ? cvWaitKey() : cvWaitKey(1);
		if (firstTime) {
			initGlobalFPS();
			firstTime = false;
		}
		
		//cout << "Key = " << key << endl;
		showFrameNow = false;
		
		if (key != -1 && allowInteraction)
			hasUserInteracted = true;
		
		if (key == -1) {
			frame += frameStep;
			showFrameNow = true;
			
        } else if (key == 'q' || key == 'Q') {
            break;

		} else if (allowInteraction == true && (key == 'x' || key == 'X')) {
			paused = true;
				
		} else if (allowInteraction == true && (key == 'c' || key == 'C')) {
			paused = false;
			frameStep = 1;
			frame += frameStep;
			initGlobalFPS();
			showFrameNow = true;
			
		} else if (randomAccess == true && allowInteraction == true) {
			if (key == 'r' || key == 'R') {
				paused = true;
				vc->reload();
				showFrameNow = true;
				
			} else if (key == 'v' || key == 'V') {
				if (!paused || frameStep > 0)
					frameStep--;
				paused = false;
				
				if (frameStep==0) frameStep--;
				frame += frameStep;
				
				initGlobalFPS();
				showFrameNow = true;
				
			} else if (key == 'b' || key == 'B') {
				if (!paused || frameStep < 0)
					frameStep++;
				paused = false;
				
				if (frameStep==0) frameStep++;
				frame += frameStep;
				
				initGlobalFPS();
				showFrameNow = true;
				
			} else if (key == 'n' || key == 'N') {
				paused = true;
				frame--;
				showFrameNow = true;
				
			} else if (key == 'm' || key == 'M') {
				paused = true;
				frame++;
				showFrameNow = true;
				
			} else if (key == 'h' || key == 'H') {
				paused = true;
				frame-=10;
				showFrameNow = true;
				
			} else if (key == 'j' || key == 'J') {
				paused = true;
				frame+=10;
				showFrameNow = true;
				
			} else if (key == 'z' || key == 'Z') {
				paused = true;
				frame-=100;
				showFrameNow = true;
				
			} else if (key == 'u' || key == 'U') {
				paused = true;
				frame+=100;
				showFrameNow = true;
				
			} else if (key == '6') {
				paused = true;
				frame-=1000;
				showFrameNow = true;
				
			} else if (key == '7') {
				paused = true;
				frame+=1000;
				showFrameNow = true;
				
			} else if (key == '5') {
				paused = true;
				frame-=0;
				showFrameNow = true;
				
			} else if (key == '8') {
				paused = true;
				frame=numFrames-1;
				showFrameNow = true;
				
			} else {
				cout << playerInstructions << endl;
			}
		} else {
			cout << playerInstructions << endl;
		}
		
		if (randomAccess && frame < 0) {
			frame = 0;
			paused = true;

		} else if (randomAccess && frame >= numFrames) {
			cout << playerInstructions << endl << "VideoPlayer. Video End." << endl;

			if (quitAtEndIfNotInteraction && hasUserInteracted == false)
				break;			

			frame = numFrames-1;
			paused = true;
			
		}
				
		if (debug)
			cout << "VideoPlayer::play. P10" << endl;
    }


	if (debug)
		cout << "VideoPlayer::play. P11" << endl;
	closeVideoOut();

	if (debug)
		cout << "VideoPlayer::play. P12" << endl;
    cout << "VideoPlayer::play. END" << endl;
}
