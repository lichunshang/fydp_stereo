#ifndef GET_IMAGES
#define GET_IMAGES

#include "../include/FlyCapture2.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <string>

#include "cameraSetup.h"

int stereo_get_images(int c1, int c2, Camera * camArray[]);

#endif

