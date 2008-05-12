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
 * Main file. 
 * You can find a short description of this software in ArchDetector.cpp.
 */

#include <cassert>
#include <iostream>

#include "util.h"
#include "VideoPlayer.h"
#include "testCircle.h"
#include "VideoCapture.h"
#include "CameraUndistort.h"
#include "CombineFrames.h"
#include "HorizonDetector.h"
#include "MorphBlobDetector.h"
#include "HoughTransform.h"
#include "ArchDetector.h"
#include "kk.h"


using namespace std;

char *info = 
    "EMAV08ArchDetector is a computer vision software to detect the arches\n"
    "from a video-stream for the EMAV08 competition.\n"
    "\n"
    " The rules of the EMAV08 competition are given in\n"
	" http://www.dgon.de/content/pdf/emav_2008_Rules_v07.pdf\n"
    " (attached also in this zip file)\n"
    " See also the EMAV08 website for more details: http://www.dgon.de/emav2008.htm\n"
    "\n"
    "EMAV08ArchDetector\n"
    "Copyright (C) 2008 David Portabella Clotet\n"
    "\n"
    "To contact the author:\n"
    "email: david.portabella@gmail.com\n"
    "web: http://david.portabella.name\n"
    "\n"
    "\n"
    "This file is part of EMAV08ArchDetector\n"
    "\n"
    "EMAV08ArchDetector is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n"
    "\n"
    "EMAV08ArchDetector is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with EMAV08ArchDetector.  If not, see <http://www.gnu.org/licenses/>.\n";

	
char *instructions = 
	"EMAV08ArchDetector\n" 
	" -if <filename>     reads video from the file.\n"
	" -ic                reads video from the video camera.\n"
	" -cup <id>          undistorts the video using predefined calibrations.\n"
	"                    id = lense0 | lense1 | lense2 | lense3\n"
    " -cuf <filename>    undistorts the video using the calibration specified in filename.\n" 
    " -d <id>            image detector.\n"
	"                    id = horion | blob | arch | none. default = arch.\n"
    " -of <filename>     saves the video output into filename, no video compression.\n"
    "\n"
    " (either -if or -ic is mandatory, all the other options are optional)\n"
	"For instance:\n"
	"EMAV08ArchDetector -if video1-ranges2.avi -cup lense0 -d arch\n";


//ARCH DETECTOR PARAMETERS
int thetaResolutionDegrees = 10;
int rhoResolution = 10;
//int angleDegreesMargin = 20;
int angleDegreesMargin = 10;
int rhoDistanceMin = 4;
int rhoDistanceMax = 11;
bool allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage = false;
	
	
int main1(int argc, char * const argv[])
{
	cout << info << endl;

	VideoCapture *vc1 = NULL;
	CameraUndistort *cameraUndistortProcessor = NULL;
	ImageProcessor *imageProcessor = NULL;
	bool imageProcessorDefined = false;
	char *videoOutputFilename = NULL;

	try {
		for (int i = 1; i < argc; i++) {
			//video input filename
			if (strcmp(argv[i], "-if") == 0) {
				if ((argc - 1) < (i + 1))
					throw "-if needs a filename.";
				i++;
				vc1 = new VideoFileReader(argv[i]);

			//camera input
			} else if (strcmp(argv[i], "-ic") == 0) {
				vc1 = new VideoCameraCapture(0);

			//camera undistortion predefined
			} else if (strcmp(argv[i], "-cup") == 0) {

				if ((argc - 1) < (i + 1))
					throw "-cup needs an identification.";
				i++;
				if (strcmp(argv[i], "lense0") == 0) {
					cameraUndistortProcessor = new CameraUndistort(Lense0KeyValues);
				} else if (strcmp(argv[i], "lense0") == 0) {
					cameraUndistortProcessor = new CameraUndistort(Lense1KeyValues);
				} else if (strcmp(argv[i], "lense1") == 0) {
					cameraUndistortProcessor = new CameraUndistort(Lense2KeyValues);
				} else if (strcmp(argv[i], "lense3") == 0) {
					cameraUndistortProcessor = new CameraUndistort(Lense3KeyValues);
				} else {
					throw "Unknown cup id";
				} 
			//camera undistortion, config filename
			} else if (strcmp(argv[i], "-cuf") == 0) {
				if ((argc - 1) < (i + 1))
					throw "-cuf needs a filename.";
				i++;
				cameraUndistortProcessor = new CameraUndistort(argv[i]);

			//image processor
			} else if (strcmp(argv[i], "-d") == 0) {
				if ((argc - 1) < (i + 1))
					throw "-d needs an identification.";
				i++;
				if (strcmp(argv[i], "horizon") == 0) {
					imageProcessor = new HorizonDetector();
				} else if (strcmp(argv[i], "blob") == 0) {
					imageProcessor = new MorphBlobDetector();
				} else if (strcmp(argv[i], "arch") == 0) {
					imageProcessor = new ArchDetector(thetaResolutionDegrees, rhoResolution, angleDegreesMargin, rhoDistanceMin, rhoDistanceMax, allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage);
				} else if (strcmp(argv[i], "none") == 0) {
					imageProcessor = NULL;
				} else {
					throw "Unknown d";
				} 
				imageProcessorDefined = true;
			//output filename
			} else if (strcmp(argv[i], "-of") == 0) {
				if ((argc - 1) < (i + 1))
					throw "-of needs a filename.";
				i++;
				videoOutputFilename = argv[i];
			} else {
				throw "unknown option";
			}		
		}

		if (!vc1)
			throw "video input is mandatory";

	} catch (char const *msg) {
		cout << "Option error: " << msg << endl;
		cout << instructions << endl;
		return 1;
	}

	//Undistort the image
	VideoCapture *vc2 = (cameraUndistortProcessor == NULL) ? vc1 : new FilterVideoCapture(vc1, cameraUndistortProcessor);

	//If not specified, use the arch detector
	if (imageProcessor == NULL && imageProcessorDefined == false)
		imageProcessor = new ArchDetector(thetaResolutionDegrees, rhoResolution, angleDegreesMargin, rhoDistanceMin, rhoDistanceMax, allowOneOfTheTwoLinesToBeMomentaryOutsideTheImage);

	//Detector: horizon, blob, arch or none
	VideoCapture *vc3 = (imageProcessor == NULL) ? vc2 : new FilterVideoCapture(vc2, imageProcessor);


	//PLAYER
	VideoPlayer player(vc3, videoOutputFilename, true);
	//player.play(bool allowInteraction, bool startPaused, bool quitAtEndIfNotInteraction); 
	if (vc3->allowsRandomAccess())
		player.play(true, true, false);
	else
		player.play(true, false, true);


	//END
	if (imageProcessor != NULL) {
		delete vc3;
		delete imageProcessor;
	}
	if (cameraUndistortProcessor != NULL) {
		delete vc2; 
		delete cameraUndistortProcessor;
	}
	delete vc1;

    return 0;
}




// other tests...
#define PATH="./videos/"

inline VideoFileReader* getVideo1() { 
	const char *videoFilename = PATH "video1.avi";
	return new VideoFileReader(videoFilename);
}

inline VideoFileReader* getVideo1Ranges() { 
	const char *videoFilename = PATH "video1.avi";
	static const int frameRanges[][2] = { {4650,4850}, {5170,5400}, {6170,6400}, {6670,6850}, {7100,7300}, {7600,7700}, {8030,8200}};
	return new VideoFileReader(videoFilename, (int*)frameRanges, sizeof(frameRanges)/sizeof(frameRanges[0]));
}

inline VideoFileReader* getVideo1Ranges2() { 
	const char *videoFilename = PATH "video1.avi";
	static const int frameRanges[][2] = { {8083,8113}, {5216,5274}};
	return new VideoFileReader(videoFilename, (int*)frameRanges, sizeof(frameRanges)/sizeof(frameRanges[0]));
}


int main2(int argc, char * const argv[])
{
	VideoCapture *vc1 = new VideoFileReader(PATH "video1-ranges2.avi");
	//VideoCapture *vc1 = new VideoFileReader(PATH "lense0.avi");
	//VideoCapture *vc1 = new VideoFileReader(PATH "lense2.avi");
	//VideoCapture *vc1 = new VideoFileReader(PATH "lense2b.avi");
	//VideoCapture *vc1 = new VideoFileReader(PATH "lense3.avi");
	//VideoCapture *vc1 = new ImageVideoCapture(PATH "test_image.bmp");
	//VideoCapture *vc1 = getVideo1Ranges2();


	//CameraUndistort *cameraUndistortProcessor = NULL;
	CameraUndistort *cameraUndistortProcessor = new CameraUndistort(Lense0KeyValues);
	VideoCapture *vc2 = (!cameraUndistortProcessor) ? vc1 : new FilterVideoCapture(vc1, cameraUndistortProcessor);


	//HorizonDetector imageProcessor = HorizonDetector();
	//MorphBlobDetector imageProcessor = MorphBlobDetector();
	ImageProcessor *imageProcessor = new ArchDetector;
	VideoCapture *vc3 = new FilterVideoCapture(vc2, imageProcessor);


	const char *output_file = NULL;
	//const char *output_file = PATH "undistor-2.avi";

	VideoPlayer player(vc3, output_file, true);
	player.play();
	//playVideo(vc, output_file, NULL, true);

	delete vc1;
	if (cameraUndistortProcessor)
		delete vc2; 
	delete vc3;

    return 0;
}




int main3(int argc, char * const argv[])
{
	VideoFileReader *vc1 = getVideo1Ranges2();
    //VideoFileReader *vc1 = new VideoFileReader(cvCreateFileCapture(PATH "video1.avi"));
	//VideoFileReader *vc1 = new VideoFileReader(cvCreateCameraCapture(CV_CAP_ANY));

	const char *output_file = NULL;
	//const char *output_file = PATH "undistor-2.avi";

	//TestCircle imageProcessor = TestCircle();
	//playVideo(vc, output_file, NULL);

	CameraUndistort imageProcessor = CameraUndistort(PATH "lense_original_calib.txt");

	VideoCapture *vc2 = new FilterVideoCapture(vc1, &imageProcessor);
	VideoPlayer player(vc2, output_file, true);
	player.play();

	delete vc1;
	delete vc2;
    return 0;
}



int main4(int argc, char * const argv[])
{
	VideoFileReader *vc1 = getVideo1Ranges();
	//const char *output_file = PATH "undistor-2.avi";
	const char *output_file = NULL;

	CameraUndistort imageProcessor1 = CameraUndistort(PATH "lense_original_calib.txt");
	Combine2x1Frames imageProcessor2 = Combine2x1Frames(NULL, &imageProcessor1);

	VideoCapture *vc2 = new FilterVideoCapture(vc1, &imageProcessor2);
	VideoPlayer player(vc2, output_file, true);
	player.play();

	delete vc1;
	delete vc2;
    return 0;
}


int main5(int argc, char * const argv[])
{
	//const char *calib_file = PATH "lense_original_calib.txt";
	const char *calib_file = PATH "lense_3_calib.txt";

	const char *inputFilename = PATH "lense3.avi";

	//const char *output_file = PATH "undistor-2.avi";
	const char *output_file = NULL;

	VideoFileReader *vc1 = new VideoFileReader(inputFilename);
	CameraUndistort imageProcessor1 = CameraUndistort(calib_file);
	Combine2x1Frames imageProcessor2 = Combine2x1Frames(NULL, &imageProcessor1);

	VideoCapture *vc2 = new FilterVideoCapture(vc1, &imageProcessor2);
	VideoPlayer player(vc2, output_file, true);
	player.play();

	delete vc1;
	delete vc2;
    return 0;
}

int main6(int argc, char * const argv[])
{
	//const char *input_file = PATH "SAMPLE.AVI";
	//static const int frameRanges[][2] = {{5,10}, {20, 30}};
	//const char *output_file = PATH "out.avi";

	const char *input_file = "video1-src-unc.avi";
	static const int frameRanges[][2] = {{8082,8123}, {5215, 5273}};
	const char *output_file = "video1-ranges2-test.avi";

	//const char *input_file = PATH "vid_24avr/lense_orig.avi";
	//static const int frameRanges[][2] = { {258,337}, {583,691}, {941,1094}, {1953,2127},{2390,2532}};
	//const char *output_file = PATH "vid_24avr/lense0-cut.avi";

	//const char *input_file = PATH "vid_24avr/lense2.avi";
	//static const int frameRanges[2][2] = { {1221,1364}, {1653,1847}};
	//const char *output_file = PATH "vid_24avr/lense2-cut.avi";

	//const char *input_file = PATH "vid_24avr/lense2_b.avi";
	//static const int frameRanges[][2] = { {696,877}, {1242,1478}, {1932,2186}, {2731,2920}};
	//const char *output_file = PATH "vid_24avr/lense2b-cut.avi";

	//const char *input_file = PATH "vid_24avr/lense3.avi";
	//static const int frameRanges[1][2] = { {1055,1502}};
	//const char *output_file = PATH "vid_24avr/lense3-cut.avi";


	VideoFileReader *vc = new VideoFileReader(input_file, (int*)frameRanges, sizeof(frameRanges)/sizeof(frameRanges[0]));
	//VideoFileReader *vc = new VideoFileReader(input_file);
	VideoPlayer player(vc, output_file);
	player.play();

	delete vc;
    return 0;
}

int main7(int argc, char * const argv[])
{
	VideoCapture *vc = getVideo1Ranges2();
	const char *output_file = PATH "video1/src-ranges2.avi";
	VideoPlayer player(vc, output_file);
	player.play();

	delete vc;
    return 0;
}

int main8(int argc, char * const argv[])
{
	VideoCapture *vc = new VideoFileReader(argv[1]);
	VideoPlayer player(vc, NULL);
	player.play();

	delete vc;
    return 0;
}


int main(int argc, char * const argv[]) {
	try {
		main1(argc, argv); 

		//or enter here the command line arguments. some tests:
		//char *argv[] = {"aaa", "-ic", "-d", "blob"};
		//char *argv[] = {"aaa", "-if", PATH "video1-ranges2.avi"};
		//char *argv[] = {"aaa", "-if", PATH "video1-ranges2.avi", "-cup", "lense0"};
		//char *argv[] = {"aaa", "-d", "none", "-if", PATH "video1-ranges2.avi"};
		//char *argv[] = {"aaa", "-if", PATH "video1-ranges2.avi", "-of", PATH "out.avi"};
		//main1(sizeof(argv)/sizeof(argv[0]), argv); 
		//kk_test();	
		//main6(argc, argv); 

	} catch (char const *msg) {
		cout << "Expception: " << msg << endl;
	}
}
