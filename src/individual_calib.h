#ifndef IND_CALIB
#define IND_CALIB

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

int individual_calib(int cameraID, Camera * camArray[]);

#endif

