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


#include <cassert>
#include <iostream>
#include <time.h>
#include "kk.h"

using namespace std;
				 

void kk_test() {
	while (true) {
		cout << "READY." << endl;
		char cc; scanf ("%c",&cc);  //just to pause
		
		cout << "START." << endl;
#ifdef WIN32
		clock_t timeStart = clock();
#else
		timeval timeStart; gettimeofday(&timeStart,NULL);
#endif
		/*
		int c=0;
		for (int a = 0; a < 1720; a++)
			for (int b = 0; b < 1720; b++)
				c++;
		cout << "c=" << c << endl;
		*/
		scanf ("%c",&cc);  //just to pause

#ifdef WIN32
		clock_t timeEnd = clock();
		double secs = (timeEnd - timeStart)/(double)CLK_TCK;
#else
		timeval timeEnd; gettimeofday(&timeEnd,NULL);
		double secs = timeEnd.tv_sec - timeStart.tv_sec + (double) (timeEnd.tv_usec - timeStart.tv_usec) / 1000000.0;
#endif

		cout << "END. time: " << secs << " secs" << endl;
	}
}
