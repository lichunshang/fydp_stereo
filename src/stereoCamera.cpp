// StereoCamera project main file

#include <stdio.h>
#include <stdint.h>
#include <string>
#include "cameraSetup.h"
#include "individual_calib.h"
#include "stereo_rectify_disp.h"
#include "stereo_calib.h"
#include "stereo_get_images.h"
#include "../include/FlyCapture2.h"

using namespace FlyCapture2;
using namespace std;

#define NUMCAMS 1

int main(int /*argc*/, char ** /*argv*/)
{
    printf("BUILT PEOPLE\n");
    Camera * cameraArray[NUMCAMS];
    bool res = initMultiCams(NUMCAMS, cameraArray);
    printf("initialize gpio? ... %d\n",initGPIO());
    printf("result: %d\n", res);

    cout << "press a button for gpio on" << endl;
    getchar();
    triggerGPIO(NUMCAMS, cameraArray);

    if (res)
    {
while(1);
//	capPictures(NUMCAMS, cameraArray); 
	
	//individual_calib(0, cameraArray);
	//individual_calib(1, cameraArray);
	//individual_calib(2, cameraArray);
	//stereo_get_images(0, 1, cameraArray); 
	//stereo_get_images(0, 2, cameraArray); 
	//stereo_get_images(1, 2, cameraArray); 
	//stereo_calib(0, 1, "stereo_calib.xml");
	//stereo_calib(0, 2, "stereo_calib_02.xml");
	//stereo_calib(1, 2, "stereo_calib_12.xml");
    
//	stereo_rectify_disp(0, 1, cameraArray); 
    }

    teardownMultiCams(NUMCAMS, cameraArray); //only when program done

    return 1;
}
