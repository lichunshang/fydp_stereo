#ifndef CAMERAS
#define CAMERAS

#if defined(LINUX32) || defined(LINUX64)
#define LINUX
#endif

#ifdef LINUX
#include <time.h>
#include <unistd.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "stdafx.h"
#include "../include/FlyCapture2.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "gpio.h"

using namespace FlyCapture2;
using namespace std;

void PrintBuildInfo();
bool initMultiCams(unsigned int numCameras, Camera * camArray[]);
cv::Mat getMatFromCameraImage(unsigned int cameraIndex, Camera * camArray[]);
void capPictures(unsigned int numCameras, Camera * camArray[]);
void teardownMultiCams(unsigned int numCameras, Camera * camArray[]);
bool initGPIO();
bool triggerGPIO(unsigned int * indexArray, unsigned int indexArraySize, Camera * camArray[]);
void testCameraArray(unsigned int numCameras, Camera * camArray[]);
bool startAllCapture(unsigned int numCameras, Camera * camArray[]);

#endif
