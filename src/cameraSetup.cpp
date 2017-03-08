//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of
// Point Grey Research, Inc. ("Confidential Information"). You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the "License Agreement" that you
// entered into with PGR in connection with this software.
//
// UNLESS OTHERWISE SET OUT IN THE LICENSE AGREEMENT, THIS SOFTWARE IS
// PROVIDED ON AN “AS-IS” BASIS AND POINT GREY RESEARCH INC. MAKES NO
// REPRESENTATIONS OR WARRANTIES ABOUT THE SOFTWARE, EITHER EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES OR
// CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
// NON-INFRINGEMENT. POINT GREY RESEARCH INC. SHALL NOT BE LIABLE FOR ANY
// DAMAGES, INCLUDING BUT NOT LIMITED TO ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES, OR ANY LOSS OF PROFITS,
// REVENUE, DATA OR DATA USE, ARISING OUT OF OR IN CONNECTION WITH THIS
// SOFTWARE OR OTHERWISE SUFFERED BY YOU AS A RESULT OF USING, MODIFYING
// OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: FlyCapture2Test.cpp 300855 2016-09-30 22:48:39Z erich $
//=============================================================================

#include "cameraSetup.h"

#define NUM_BUFFERS 1
#define GPIO_NUMBER (gpio38)
#define EXT_TRIGGER 1

using namespace FlyCapture2;
using namespace std;

static bool GPIO_INIT = 0;

void PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion(&fc2Version);
    ostringstream version;
    version << "FlyCapture2 library version: " << fc2Version.major << "."
            << fc2Version.minor << "." << fc2Version.type << "."
            << fc2Version.build;
    cout << version.str() << endl;

    ostringstream timeStamp;
    timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
    cout << timeStamp.str() << endl << endl;
}

void PrintCameraInfo(CameraInfo *pCamInfo)
{
    cout << endl;
    cout << "*** CAMERA INFORMATION ***" << endl;
    cout << "Serial number - " << pCamInfo->serialNumber << endl;
    cout << "Camera model - " << pCamInfo->modelName << endl;
    cout << "Camera vendor - " << pCamInfo->vendorName << endl;
    cout << "Sensor - " << pCamInfo->sensorInfo << endl;
    cout << "Resolution - " << pCamInfo->sensorResolution << endl;
    cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
    cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl
         << endl;
}

void PrintError(Error error) { error.PrintErrorTrace(); }

bool PollForTriggerReady(Camera *pCam)
{
    const unsigned int k_softwareTrigger = 0x62C;
    Error error;
    unsigned int regVal = 0;

    do
    {
        error = pCam->ReadRegister(k_softwareTrigger, &regVal);
        if (error != PGRERROR_OK)
        {
           PrintError(error);
           return false;
        }

    } while ((regVal >> 31) != 0);

    return true;
}

bool FireSoftwareTrigger(Camera *pCam)
{
    const unsigned int k_softwareTrigger = 0x62C;
    const unsigned int k_fireVal = 0x80000000;
    Error error;

    error = pCam->WriteRegister(k_softwareTrigger, k_fireVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return false;
    }

    return true;
}

// Check for the presence of software trigger
bool CheckSoftwareTriggerPresence( Camera* pCam )
{
    const unsigned int k_triggerInq = 0x530;
    Error error;
    unsigned int regVal = 0;
    error = pCam->ReadRegister( k_triggerInq, &regVal );
    if ( error != PGRERROR_OK )
    {
        PrintError(error);
        return false;
    }
     
    if( ( regVal & 0x10000 ) != 0x10000 )
    {
        return false;
    }
                    
    return true;
}


bool initGPIO()
{
	jetsonTX1GPIONumber triggerGPIO = GPIO_NUMBER; // Ouput
	gpioUnexport(triggerGPIO);
	GPIO_INIT = gpioExport(triggerGPIO) == 0 ? 1 : 0;
	GPIO_INIT &= gpioSetDirection(triggerGPIO, outputPin) == 0 ? 1 : 0;
	if(GPIO_INIT){
   		gpioSetValue(triggerGPIO, low);
    }
	return GPIO_INIT;
}

bool triggerGPIO(unsigned int * indexArray, unsigned int indexArraySize, Camera * camArray[])
{
	jetsonTX1GPIONumber triggerGPIO = GPIO_NUMBER;
	if(GPIO_INIT){
		for(int i = 0; i < indexArraySize; i++){
		    cout << "waiting for camera " << indexArray[i] << " trigger...";
		    PollForTriggerReady(camArray[indexArray[i]]);
		    cout << "READY!" << endl;
		}
        bool triggerRes = (!gpioSetValue(triggerGPIO, high) && !gpioSetValue(triggerGPIO, low));

        cout << "Trigger..." << triggerRes << endl;

		return triggerRes;
    }
	return false;
}

bool resetGPIO()
{
	jetsonTX1GPIONumber triggerGPIO = GPIO_NUMBER;
	return gpioSetValue(triggerGPIO, low) == 0 ? true : false;
}

cv::Mat getMatFromCameraImage(unsigned int cameraIndex, Camera * camArray[])
{
	Error error;
	Camera *cam;
	cv::Mat result;

    cam = camArray[cameraIndex];

    // Start capturing images
/*        error = cam->StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return result;
    }
*/
    Image rawImage;

    // Retrieve an image
    error = cam->RetrieveBuffer(&rawImage);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return result;
    }

// convert to rgb
    Image rgbImage;
    rawImage.Convert( FlyCapture2::PIXEL_FORMAT_BGR, &rgbImage );

    // convert to OpenCV Mat
    unsigned int rowBytes = (double)rgbImage.GetReceivedDataSize()/(double)rgbImage.GetRows();       
    cv::Mat image = cv::Mat(rgbImage.GetRows(), rgbImage.GetCols(), CV_8UC3, rgbImage.GetData(),rowBytes);            

// Stop capturing images
/*        error = cam->StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return result;
    }
*/
	image.copyTo(result);

	return result;
 }

void capPictures(unsigned int numCameras, Camera * camArray[])
{
    const int k_numImages = 1;
    Error error;
    Camera *cam;

/*    for (unsigned int i = 0; i < numCameras; i++)
    {
        printf("start %u\n", i);
        cam = camArray[i];

        // Start capturing images
        error = cam->StartCapture();
        if(error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED){
            cout << "Couldn't start camera " << i << " because interface bandwidth exceeded." << endl;
        }
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return;
        }
    }
//    StartSyncCapture(numCameras, camArray);

    printf("started capturing...\n");
    */

    Image rawImage;
    for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        if(EXT_TRIGGER){
            unsigned int idxArray[numCameras];
            for(int z = 0; z < numCameras; z++)
                idxArray[z] = z;
            bool triggerRes = triggerGPIO(idxArray, numCameras, camArray);
            cout << "Trigger... " << triggerRes << endl;
            //resetGPIO();
        }

        for (unsigned int i = 0; i < numCameras; i++)
        {
            cam = camArray[i];
            // Retrieve an image
            error = cam->RetrieveBuffer(&rawImage);
            if (error != PGRERROR_OK)
            {
                PrintError(error);
                continue;
            }

            cout << "Grabbed image " << imageCnt << endl;

            // Create a converted image
            Image convertedImage;

            // Convert the raw image
            error = rawImage.Convert(PIXEL_FORMAT_MONO8, &convertedImage);
            if (error != PGRERROR_OK)
            {
                PrintError(error);
                return;
            }

            // Create a unique filename

            ostringstream filename;
            filename << "FlyCapture2Test-Cam" << i << "-"
                     << imageCnt << ".pgm";

            // Save the image. If a file format is not passed in, then the file
            // extension is parsed to attempt to determine the file format.
            error = convertedImage.Save(filename.str().c_str());
            if (error != PGRERROR_OK)
            {
                PrintError(error);
                return;
            }
/*
            for(unsigned int i = 0; i < numCameras; i++) {
                cout << "stop " << i << endl;
                Error error;
                // Stop capturing images
                error = camArray[i]->StopCapture();
                if (error != PGRERROR_OK)
                {
                    PrintError(error);
                    return;
                }
            }
*/
            
        }
        printf("retrieved images...\n");
    }
 }

bool initMultiCams(unsigned int numSetupCams, Camera * camArray[]) 
{
    PrintBuildInfo();

    Error error;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    FILE *tempFile = fopen("test.txt", "w+");
    if (tempFile == NULL)
    {
        cout << "Failed to create file in current folder.  Please check "
                "permissions."
             << endl;
        return false;
    }
    fclose(tempFile);
    remove("test.txt");

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return false;
    }

    cout << "Number of cameras detected: " << numCameras << endl;

    if (numCameras < numSetupCams)
    {
        printf("ERROR: Want to set up %d cameras, but only detected %d cameras/\n", numSetupCams, numCameras);
        return false;
    }

    for (unsigned int i = 0; i < numSetupCams; i++)
    {
        cout << "setting up..." << endl;
        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);

        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }
        Error error;    

        // Connect to a camera
        Camera *cam = new Camera;
        error = cam->Connect(&guid);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

        // Get the camera information
        CameraInfo camInfo;
        error = cam->GetCameraInfo(&camInfo);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

        PrintCameraInfo(&camInfo);

        // Check for external trigger support
        TriggerModeInfo triggerModeInfo;
        error = cam->GetTriggerModeInfo(&triggerModeInfo);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

        if (triggerModeInfo.present != true)
        {
            cout << "Camera does not support external trigger! Exiting..." << endl;
            return false;
        }
	else
	{
	    cout << "Found camera trigger support." << endl;
	}

        // Set trigger mode
        TriggerMode triggerMode;

        error = cam->GetTriggerMode(&triggerMode);
        if (error != PGRERROR_OK)
        {
            printf("Error getting triggering modes.");
            PrintError(error);
            return false;
        }

	if(EXT_TRIGGER){
		triggerMode.onOff = true;
		triggerMode.mode = 0;
		triggerMode.parameter = 0;
		triggerMode.source = 2; //external trigger
		triggerMode.polarity = 0; //falling edge
	}
	else{
		triggerMode.onOff = false;
	}

        // Set the trigger settings
	
	error = cam->SetTriggerMode(&triggerMode);
        if (error != PGRERROR_OK)
        {
            printf("Error setting trigger modes.\n");
            PrintError(error);
            return false;
        }
        else
        {
            cout << "Set trigger mode." << endl;
        }

        // Power on the camera
        const unsigned int k_cameraPower = 0x610;
        const unsigned int k_powerVal = 0x80000000;
        error  = cam->WriteRegister( k_cameraPower, k_powerVal );
                    
        if ( error != PGRERROR_OK )
        {
            printf("Error: Could not power on camera.\n");
            PrintError( error );
            return false;
        }
	cout << "Camera on." << endl;

        // Poll for trigger to be ready
	if(EXT_TRIGGER){
		bool retVal = PollForTriggerReady(cam);
		if (!retVal)
		{
		    cout << endl;
		    cout << "Error polling for trigger ready!" << endl;
		    return false;
		}
	}

	cout << "Trigger ready." << endl;

        // Get the camera configuration
        FC2Config config;
        error = cam->GetConfiguration(&config);
        if (error != PGRERROR_OK)
        {
            printf("Error getting configuration.\n");
            PrintError(error);
            return false;
        }

        // Set the number of driver buffers used to 10.
        config.numBuffers = 2;
        // Set the grab timeout to 5 seconds
        config.grabTimeout = 1000;
//        config.highPerformanceRetrieveBuffer = true;
//        config.grabMode = DROP_FRAMES;
//        config.asyncBusSpeed = BUSSPEED_S_FASTEST;

        // Set the camera configuration
        error = cam->SetConfiguration(&config);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

        printf("FINISHED CONFIGURING\n");
        
        Format7ImageSettings imageSettings;
        unsigned int packetSize;
        float percentage;
        cout << "Setting image format..." << endl;
        error = cam->GetFormat7Configuration(&imageSettings, &packetSize, &percentage);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

        //imageSettings.pixelFormat = PIXEL_FORMAT_MONO8;
        error = cam->SetFormat7Configuration(&imageSettings, packetSize);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

        cout << "Finished setting image format." << endl;

	//cam->SetOutputVerticalFlip(true);

        camArray[i] = cam;
//        RunSingleCamera(*guid);
    }

    cout << "Press Enter to exit..." << endl;
    cin.ignore();

    return true;
}

void teardownMultiCams(unsigned int numCameras, Camera * camArray[])
{
    for (unsigned int i = 0; i < numCameras; i++)
    {
        Camera * cam = camArray[i];
        Error error;
        // Stop capturing images
        error = cam->StopCapture();
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return;
        }

        // Disconnect the camera
        error = cam->Disconnect();
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return;
        }
    }
}

void testCameraArray(unsigned int numCameras, Camera * camArray[]){
    for(int i = 0; i < numCameras; i++){
        // Get the camera information
        CameraInfo camInfo;
        Error error;
        error = camArray[i]->GetCameraInfo(&camInfo);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return;
        }
        PrintCameraInfo(&camInfo);
    }
}

bool startAllCapture(unsigned int numCameras, Camera * camArray[]){
    Error error;
    Camera *cam;

    for (unsigned int i = 0; i < numCameras; i++)
    {
        printf("start %u\n", i);
        cam = camArray[i];

        // Start capturing images
        error = cam->StartCapture();
        if(error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED){
            cout << "Couldn't start camera " << i << " because interface bandwidth exceeded." << endl;
        }
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }
    }

    printf("started capturing for all...\n");
    return true;
}
