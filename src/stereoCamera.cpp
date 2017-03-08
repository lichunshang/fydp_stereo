// StereoCamera project main file

#include <stdio.h>
#include <stdint.h>
#include <string>
#include "cameraSetup.h"
#include "individual_calib.h"
#include "stereo_rectify_disp.h"
#include "stereo_rectify_disp-with_trackbars.h"
#include "stereo_calib.h"
#include "stereo_get_images.h"
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
    printf("\n");
    //testCameraArray(NUMCAMS, cameraArray);
    
    printf("initialize gpio? ... %d\n",initGPIO());
    cout << "press a button to start capture" << endl;
    //getchar();

    if (res)
    {
        startAllCapture(NUMCAMS, cameraArray);
	// camera order : 2, 1, 0
        capPictures(NUMCAMS, cameraArray); 


        //individual_calib(0, cameraArray);
        //individual_calib(1, cameraArray);
        //individual_calib(2, cameraArray);
        //stereo_get_images(0, 1, cameraArray); 
        //stereo_calib(0, 1, "stereo_calib_01.xml");
        //stereo_get_images(0, 2, cameraArray); 
        //stereo_calib(0, 2, "stereo_calib_02.xml");
        //stereo_get_images(2, 1, cameraArray); 
        //stereo_calib(2, 1, "stereo_calib_21.xml");
        //stereo_rectify_disp_t(0, 1, cameraArray); 
	//stereo_rectify_disp_t(0, 2, cameraArray);
	//stereo_rectify_disp_t(1, 2, cameraArray);
	stereo_rectify_disp(0, 1, 0, 2, cameraArray);

        teardownMultiCams(NUMCAMS, cameraArray); //only when program done
    }

    return 1;
}
