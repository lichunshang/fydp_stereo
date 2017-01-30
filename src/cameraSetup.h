#ifndef CAMERAS
#define CAMERAS

#if defined(LINUX32) || defined(LINUX64)
#define LINUX
#endif

#ifdef LINUX
#include <time.h>
#include <unistd.h>
#endif

#include "stdafx.h"
#include "../include/FlyCapture2.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace FlyCapture2;
using namespace std;

void PrintBuildInfo();
bool initMultiCams(unsigned int numCameras, Camera * camArray[]);
void capPictures(unsigned int numCameras, Camera * camArray[]);
void teardownMultiCams(unsigned int numCameras, Camera * camArray[]);

#endif
