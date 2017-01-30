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
// $Id: AsyncTriggerEx.cpp 300855 2016-09-30 22:48:39Z erich $
//=============================================================================

#include "stdafx.h"

#if defined(LINUX32) || defined(LINUX64)
#define LINUX
#endif

#ifdef LINUX
#include <time.h>
#include <unistd.h>
#endif

#include "FlyCapture2.h"
#include <iostream>
#include <sstream>

//
// Software trigger the camera instead of using an external hardware trigger
//
#define SOFTWARE_TRIGGER_CAMERA

using namespace FlyCapture2;
using namespace std;

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

bool CheckSoftwareTriggerPresence(Camera *pCam)
{
    const unsigned int k_triggerInq = 0x530;

    Error error;
    unsigned int regVal = 0;

    error = pCam->ReadRegister(k_triggerInq, &regVal);

    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return false;
    }

    if ((regVal & 0x10000) != 0x10000)
    {
        return false;
    }

    return true;
}

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

int main(int /*argc*/, char ** /*argv*/)
{
    PrintBuildInfo();

    const int k_numImages = 5;

    Error error;
    Error error2;
    Error error3;

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    cout << "Number of cameras detected: " << numCameras << endl;

    if (numCameras < 1)
    {
        cout << "Insufficient number of cameras... exiting" << endl;
        return -1;
    }

    PGRGuid guid;
    PGRGuid guid2;
    PGRGuid guid3;

    error = busMgr.GetCameraFromIndex(0, &guid);
    error2 = busMgr.GetCameraFromIndex(1, &guid2);
    error3 = busMgr.GetCameraFromIndex(2, &guid3);
    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
        printf("Error getting camera.");
        PrintError(error);
        return -1;
    }


    Camera cam;
    Camera cam2;
    Camera cam3;

    // Connect to a camera
    error = cam.Connect(&guid);
    error2 = cam2.Connect(&guid2);
    error3 = cam3.Connect(&guid3);

    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
	printf("Could not connect to a camera.\n");
        PrintError(error);
        return -1;
    }

    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error = cam.WriteRegister(k_cameraPower, k_powerVal);
    error2 = cam2.WriteRegister(k_cameraPower, k_powerVal);
    error3 = cam3.WriteRegister(k_cameraPower, k_powerVal);

    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
        printf("Error powering on cameras.");
        PrintError(error);
        return -1;
    }

    const unsigned int millisecondsToSleep = 100;
    unsigned int regVal = 0;
    unsigned int retries = 10;

    // Wait for camera to complete power-up
    do
    {
#if defined(_WIN32) || defined(_WIN64)
        Sleep(millisecondsToSleep);
#elif defined(LINUX)
        struct timespec nsDelay;
        nsDelay.tv_sec = 0;
        nsDelay.tv_nsec = (long)millisecondsToSleep * 1000000L;
        nanosleep(&nsDelay, NULL);
#endif
        error = cam.ReadRegister(k_cameraPower, &regVal);
	error2 = cam2.ReadRegister(k_cameraPower, &regVal);
	error3 = cam3.ReadRegister(k_cameraPower, &regVal);

        if ((error == PGRERROR_TIMEOUT) || (error2 == PGRERROR_TIMEOUT) || (error3 == PGRERROR_TIMEOUT))
        {
            // ignore timeout errors, camera may not be responding to
            // register reads during power-up
        }
        else if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error != PGRERROR_OK))
        {
	    printf("Error waiting for camera to complete powerup\n");
            PrintError(error);
            return -1;
        }

        retries--;
    } while ((regVal & k_powerVal) == 0 && retries > 0);

    // Check for timeout errors after retrying
    if ((error == PGRERROR_TIMEOUT) || (error2 == PGRERROR_TIMEOUT) || (error3 == PGRERROR_TIMEOUT))
    {
	printf("Error camera timeout after retrying\n");
        PrintError(error);
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    CameraInfo camInfo2;
    CameraInfo camInfo3;

    error = cam.GetCameraInfo(&camInfo);
    error2 = cam2.GetCameraInfo(&camInfo2);
    error3 = cam3.GetCameraInfo(&camInfo3);

    if ((error != PGRERROR_OK) ||(error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
	printf("Error could not get camera info.\n");
        PrintError(error);
        return -1;
    }

    PrintCameraInfo(&camInfo);
    PrintCameraInfo(&camInfo2);
    PrintCameraInfo(&camInfo3);

#ifndef SOFTWARE_TRIGGER_CAMERA
    // Check for external trigger support
    TriggerModeInfo triggerModeInfo;
    error = cam.GetTriggerModeInfo(&triggerModeInfo);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    if (triggerModeInfo.present != true)
    {
        cout << "Camera does not support external trigger! Exiting..." << endl;
        return -1;
    }
#endif

    // Get current trigger settings
    TriggerMode triggerMode;
    TriggerMode triggerMode2;
    TriggerMode triggerMode3;

    error = cam.GetTriggerMode(&triggerMode);
    error2 = cam2.GetTriggerMode(&triggerMode2);
    error3 = cam3.GetTriggerMode(&triggerMode3);
    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
        printf("Error getting triggering modes.");
        PrintError(error);
        return -1;
    }

    // Set camera to trigger mode 0
    triggerMode.onOff = true;
    triggerMode.mode = 0;
    triggerMode.parameter = 0;

    triggerMode2.onOff = true;
    triggerMode2.mode = 0;
    triggerMode2.parameter = 0;

    triggerMode3.onOff = true;
    triggerMode3.mode = 0;
    triggerMode3.parameter = 0;

	

#ifdef SOFTWARE_TRIGGER_CAMERA
    // A source of 7 means software trigger
    triggerMode.source = 7;
    triggerMode2.source = 7;
    triggerMode3.source = 7;
#else
    // Triggering the camera externally using source 0.
    triggerMode.source = 0;
    triggerMode2.source = 0;
    triggerMode3.source = 0;
#endif

    error = cam.SetTriggerMode(&triggerMode);
    error2 = cam2.SetTriggerMode(&triggerMode2);
    error3 = cam3.SetTriggerMode(&triggerMode3);
    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
        printf("Error setting triggering modes.\n");
        PrintError(error);
        return -1;
    }



    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady(&cam);
    retVal &= PollForTriggerReady(&cam2);
    retVal &= PollForTriggerReady(&cam3);

    if (!retVal)
    {
        cout << endl;
        cout << "Error polling for trigger ready!" << endl;
        return -1;
    }

    // Get the camera configuration
    FC2Config config;
    FC2Config config2;
    FC2Config config3;

    error = cam.GetConfiguration(&config);
    error2 = cam2.GetConfiguration(&config2);
    error3 = cam3.GetConfiguration(&config3);
    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
        printf("Error getting configuration.\n");
        PrintError(error);
        return -1;
    }

    // Set the grab timeout to 5 seconds
    config.grabTimeout = 5000;
    config2.grabTimeout = 5000;
    config3.grabTimeout = 5000;

    // Set the camera configuration
    error = cam.SetConfiguration(&config);
    error2 = cam2.SetConfiguration(&config2);
    error3 = cam3.SetConfiguration(&config3);

    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
        printf("Error grabbing configurations.");
        PrintError(error);
        return -1;
    }


    // Camera is ready, start capturing images
    error = cam.StartCapture();
//    error2 = error;
    error2 = cam2.StartCapture();
    error3 = cam3.StartCapture();
//	error3 = error;

    if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
    {
       printf("Error start capturing.\n");
       PrintError(error);
       return -1;
    }

#ifdef SOFTWARE_TRIGGER_CAMERA
    if (!CheckSoftwareTriggerPresence(&cam))
    {
        cout << "SOFT_ASYNC_TRIGGER not implemented on this camera! Stopping "
                "application"
             << endl;
        return -1;
    }
#else
    cout << "Trigger the camera by sending a trigger pulse to GPIO"
         << triggerMode.source << endl;

#endif

    Image image;
    Image image2;
    Image image3;

    cout << "Going to take " << k_numImages << " images..." << endl;

    for (int imageCount = 0; imageCount < k_numImages; imageCount++)
    {

#ifdef SOFTWARE_TRIGGER_CAMERA
        // Check that the trigger is ready
        PollForTriggerReady(&cam);
	PollForTriggerReady(&cam2);
	PollForTriggerReady(&cam3);

        cout << "Press the Enter key to initiate a software trigger" << endl;
        cin.ignore();

        // Fire software trigger
        bool retVal = FireSoftwareTrigger(&cam);
        retVal &= FireSoftwareTrigger(&cam2);
        retVal &= FireSoftwareTrigger(&cam3);

        if (!retVal)
        {
            cout << endl;
            cout << "Error firing software trigger" << endl;
            return -1;
        }
#endif

        // Grab image
        error = cam.RetrieveBuffer(&image);
	error2 = cam2.RetrieveBuffer(&image2);
	error3 = cam3.RetrieveBuffer(&image3);

        if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
        {
            printf("Error grabbing images.");
            PrintError(error);
            return -1;
        }

        cout << "." << endl;

	Image convertedImage;
	Image convertedImage2;
	Image convertedImage3;
/*
	error = image.Convert(PIXEL_FORMAT_MONO8, &convertedImage);
	//error2 = image2.Convert(PIXEL_FORMAT_MONO8, &convertedImage2);
	//error3 = image3.Convert(PIXEL_FORMAT_MONO8, &convertedImage3);
        if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
        {
            printf("Error converting images.");
            PrintError(error);
            return -1;
        }


	ostringstream filename;
	ostringstream filename2;
	ostringstream filename3;

	filename << "FlyCapture2Test-" << "Camera_1" << "-" << "pic-" << imageCount << ".pgm";
	filename2 << "FlyCapture2Test-" << "Camera_2" << "-" << "pic-" << imageCount << ".pgm";
	filename3 << "FlyCapture2Test-" << "Camera_3" << "-" << "pic-" << imageCount << ".pgm";

	error = convertedImage.Save(filename.str().c_str());
	error2 = convertedImage.Save(filename2.str().c_str());
	error3 = convertedImage.Save(filename3.str().c_str());
        if ((error != PGRERROR_OK) || (error2 != PGRERROR_OK) || (error3 != PGRERROR_OK))
        {
	    printf("Error saving images.");
            PrintError(error);
            return -1;
        }
*/
    }

    // Turn trigger mode off.
    triggerMode.onOff = false;
    error = cam.SetTriggerMode(&triggerMode);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }
    cout << endl;
    cout << "Finished grabbing images" << endl;

    // Stop capturing images
    error = cam.StopCapture();
    cam2.StopCapture();
    cam3.StopCapture();

    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Disconnect the camera
    error = cam.Disconnect();
    cam2.Disconnect();
    cam3.Disconnect();

    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    cout << "Done! Press Enter to exit..." << endl;
    cin.ignore();

    return 0;
}
