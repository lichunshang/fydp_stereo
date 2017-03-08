#ifndef STR_RECTIFY_T
#define STR_RECTIFY_T

#include "../include/FlyCapture2.h"

#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "cameraSetup.h"

int stereo_rectify_disp_t(int camera1ID1, int camera1ID2, Camera * camArray[]);

#endif

