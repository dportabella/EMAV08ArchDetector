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
 * Combine2x1Frames simply puts two images one next to the other
 * Combine2x2Frames simply puts two images on the top, and two images on the bottom
 */

#include <cassert>
#include <iostream>

#include "util.h"
#include "CombineFrames.h"

Combine2x1Frames::Combine2x1Frames(ImageProcessor *imageProcessor1, ImageProcessor *imageProcessor2) {
	this->imageProcessor1 = imageProcessor1;
	this->imageProcessor2 = imageProcessor2;
}

void Combine2x1Frames::init(IplImage *current_frame) {
	out_image    = cvCreateImage(cvSize (current_frame->width*2, current_frame->height), IPL_DEPTH_8U, 3);
	assert (out_image);

	if (imageProcessor1)
		imageProcessor1->init(current_frame);

	if (imageProcessor2)
		imageProcessor2->init(current_frame);
}

Combine2x1Frames::~Combine2x1Frames() {
	cvReleaseImage(&out_image);
}

IplImage * Combine2x1Frames::processImage(IplImage *src_image)
{
	IplImage *im1 = imageProcessor1 ? imageProcessor1->processImage(src_image) : src_image;
	IplImage *im2 = imageProcessor2 ? imageProcessor2->processImage(src_image) : src_image;

	combine2x1(out_image, im1, im2);

	return out_image;
}

//TODO: it could merge 1C with 3C images...
void Combine2x1Frames::combine2x1(IplImage *outImage, IplImage *im1, IplImage *im2) {
	int    width        = im1->width;
	int    height       = im1->height;

	char   *im1Data     = im1->imageData;
	int    iwdIm1       = im1->widthStep;
	int    channels     = im1->nChannels;

	char   *im2Data     = im2->imageData;
	assert(width    == im2->width);
	assert(height   == im2->height);
	assert(iwdIm1   == im2->widthStep);
	assert(channels == im2->nChannels);

	char   *outData     = outImage->imageData;
	int    iwdOut       = outImage->widthStep;
	assert(width*2  == outImage->width);
	assert(height   == outImage->height);
	assert(iwdIm1*2 == iwdOut);
	assert(channels == outImage->nChannels);

	//TODO: this can be faster
	for (int x=0;x<width;x++) {
		for (int y=0;y<height;y++) {
			for (int channel = 0; channel < channels; channel++) {
				outData[y*iwdOut + x*channels + channel]         = im1Data[y*iwdIm1 + x*channels + channel];
				outData[y*iwdOut + (x+width)*channels + channel] = im2Data[y*iwdIm1 + x*channels + channel];
			}
		}
	}
}




Combine2x2Frames::Combine2x2Frames(ImageProcessor *imageProcessor1, ImageProcessor *imageProcessor2, ImageProcessor *imageProcessor3, ImageProcessor *imageProcessor4) {
	this->imageProcessor1 = imageProcessor1;
	this->imageProcessor2 = imageProcessor2;
	this->imageProcessor3 = imageProcessor3;
	this->imageProcessor4 = imageProcessor4;
}

void Combine2x2Frames::init(IplImage *current_frame) {
	out_image    = cvCreateImage(cvSize (current_frame->width*2, current_frame->height*2), IPL_DEPTH_8U, 3);
	assert (out_image);

	if (imageProcessor1)
		imageProcessor1->init(current_frame);

	if (imageProcessor2)
		imageProcessor2->init(current_frame);

	if (imageProcessor3)
		imageProcessor3->init(current_frame);

	if (imageProcessor4)
		imageProcessor4->init(current_frame);
}

Combine2x2Frames::~Combine2x2Frames() {
	cvReleaseImage(&out_image);
}

IplImage * Combine2x2Frames::processImage(IplImage *src_image)
{
	IplImage *im1 = imageProcessor1 ? imageProcessor1->processImage(src_image) : src_image;
	IplImage *im2 = imageProcessor2 ? imageProcessor2->processImage(src_image) : src_image;
	IplImage *im3 = imageProcessor3 ? imageProcessor3->processImage(src_image) : src_image;
	IplImage *im4 = imageProcessor4 ? imageProcessor4->processImage(src_image) : src_image;

	combine2x2(out_image, im1, im2, im3, im4);

	return out_image;
}

/*
//TODO: it could merge 1C with 3C images...
void Combine2x2Frames::combine2x2(IplImage *outImage, IplImage *im1, IplImage *im2, IplImage *im3, IplImage *im4) {
	int    width        = im1->width;
	int    height       = im1->height;

	int    iwdIm1       = im1->widthStep;
	int    channels     = im1->nChannels;

	assert(width    == im2->width);
	assert(height   == im2->height);
	assert(iwdIm1   == im2->widthStep);
	assert(channels == im2->nChannels);

	assert(width    == im3->width);
	assert(height   == im3->height);
	assert(iwdIm1   == im3->widthStep);
	assert(channels == im3->nChannels);

	assert(width    == im4->width);
	assert(height   == im4->height);
	assert(iwdIm1   == im4->widthStep);
	assert(channels == im4->nChannels);

	int    iwdOut       = outImage->widthStep;
	assert(width*2  == outImage->width);
	assert(height*2 == outImage->height);
	assert(iwdIm1*2 == iwdOut);
	assert(channels == outImage->nChannels);

	//TODO: this can be faster. memcpy operations...
	char *out1YData = outImage->imageData;
	char *out2YData = outImage->imageData + width * channels;
	char *out3YData = outImage->imageData + height * iwdOut;
	char *out4YData = outImage->imageData + height * iwdOut + width * channels;
	char *im1YData = im1->imageData;
	char *im2YData = im2->imageData;
	char *im3YData = im3->imageData;
	char *im4YData = im4->imageData;

	char *out1XData, *out2XData, *out3XData, *out4XData, *im1XData, *im2XData, *im3XData, *im4XData;
	for (int y=0;y<height;y++) {
		out1XData = out1YData;
		out2XData = out2YData;
		out3XData = out3YData;
		out4XData = out4YData;
		im1XData = im1YData;
		im2XData = im2YData;
		im3XData = im3YData;
		im4XData = im4YData;
		for (int x=0;x<width;x++) {
			for (int channel = 0; channel < channels; channel++) {
				out1XData[channel] = im1XData[channel];
				out2XData[channel] = im2XData[channel];
				out3XData[channel] = im3XData[channel];
				out4XData[channel] = im4XData[channel];
			}
			out1XData += channels;
			out2XData += channels;
			out3XData += channels;
			out4XData += channels;
			im1XData += channels;
			im2XData += channels;
			im3XData += channels;
			im4XData += channels;
		}
		out1YData += iwdOut;
		out2YData += iwdOut;
		out3YData += iwdOut;
		out4YData += iwdOut;
		im1YData += iwdIm1;
		im2YData += iwdIm1;
		im3YData += iwdIm1;
		im4YData += iwdIm1;
	}
}
*/


//TODO: it could merge 1C with 3C images...
void Combine2x2Frames::combine2x2(IplImage *outImage, IplImage *im1, IplImage *im2, IplImage *im3, IplImage *im4) {
	int    width        = im1->width;
	int    height       = im1->height;

	int    iwdIm1       = im1->widthStep;
	int    channels     = im1->nChannels;

	assert(width    == im2->width);
	assert(height   == im2->height);
	assert(iwdIm1   == im2->widthStep);
	assert(channels == im2->nChannels);

	assert(width    == im3->width);
	assert(height   == im3->height);
	assert(iwdIm1   == im3->widthStep);
	assert(channels == im3->nChannels);

	assert(width    == im4->width);
	assert(height   == im4->height);
	assert(iwdIm1   == im4->widthStep);
	assert(channels == im4->nChannels);

	int    iwdOut       = outImage->widthStep;
	assert(width*2  == outImage->width);
	assert(height*2 == outImage->height);
	assert(iwdIm1*2 == iwdOut);
	assert(channels == outImage->nChannels);

	//cout << "width: " << width << ", channels: " << channels << ", iwdIm1: " << iwdIm1 << ", iwdOut: " << iwdOut << endl;

	char *out1YData = outImage->imageData;
	char *out2YData = outImage->imageData + width * channels;
	char *out3YData = outImage->imageData + height * iwdOut;
	char *out4YData = outImage->imageData + height * iwdOut + width * channels;
	char *im1YData = im1->imageData;
	char *im2YData = im2->imageData;
	char *im3YData = im3->imageData;
	char *im4YData = im4->imageData;
	int numLineBytes = width * channels;
	for (int y=0;y<height;y++) {
		memcpy(out1YData, im1YData, numLineBytes);
		memcpy(out2YData, im2YData, numLineBytes);
		memcpy(out3YData, im3YData, numLineBytes);
		memcpy(out4YData, im4YData, numLineBytes);
		
		out1YData += iwdOut;
		out2YData += iwdOut;
		out3YData += iwdOut;
		out4YData += iwdOut;
		im1YData += iwdIm1;
		im2YData += iwdIm1;
		im3YData += iwdIm1;
		im4YData += iwdIm1;
	}
}

