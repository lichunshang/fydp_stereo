// StereoCamera project main file

#include <stdio.h>
#include <stdint.h>
#include "cameraSetup.h"
#include "../include/FlyCapture2.h"

using namespace FlyCapture2;
using namespace std;

#define NUMCAMS 3

int main(int /*argc*/, char ** /*argv*/)
{
    printf("BUILT PEOPLE\n");
    Camera * cameraArray[NUMCAMS];
    bool res = initMultiCams(NUMCAMS, cameraArray);
    printf("result: %d\n", res);

    if (res)
    {
        capPictures(NUMCAMS, cameraArray);
        teardownMultiCams(NUMCAMS, cameraArray);
    }

    return 1;
}
