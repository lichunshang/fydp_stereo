#ifndef STR_CALIB
#define STR_CALIB

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

using namespace std;

int stereo_calib(int camera1ID, int camera2ID, string imagelistfn);

#endif

