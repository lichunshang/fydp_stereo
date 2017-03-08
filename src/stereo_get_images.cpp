#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

#include "stereo_get_images.h"

using namespace std;
using namespace FlyCapture2;
using namespace cv;

#define NUMCAMS 3
#define NUM_FRAMES 14
#define FLIP_VERTICAL 1

static void help()
{
    cout <<  "This is a camera calibration sample." << endl
         <<  "Usage: calibration configurationFile"  << endl
         <<  "Near the sample file you'll find the configuration file, which has detailed help of "
             "how to edit it.  It may be any OpenCV supported file format XML/YAML." << endl;
}
class Settings
{
public:
    Settings() : goodInput(false) {}
    Camera * cameraArray[NUMCAMS]; // Camera handles

    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

    void write(FileStorage& fs) const                        //Write serialization for this class
    {
        fs << "{" << "BoardSize_Width"  << boardSize.width
                  << "BoardSize_Height" << boardSize.height
                  << "Square_Size"         << squareSize
                  << "Calibrate_Pattern" << patternToUse
                  << "Calibrate_NrOfFrameToUse" << nrFrames
                  << "Calibrate_FixAspectRatio" << aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

                  << "Write_DetectedFeaturePoints" << bwritePoints
                  << "Write_extrinsicParameters"   << bwriteExtrinsics
                  //<< "Write_outputFileName"  << outputFileName
		  //<< "Write_outputFileName2" << outputFileName2

                  << "Show_UndistortedImage" << showUndistorsed

                  << "Input_FlipAroundHorizontalAxis" << flipVertical
                  << "Input_Delay" << delay
           << "}";
    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
        node["BoardSize_Width" ] >> boardSize.width;
        node["BoardSize_Height"] >> boardSize.height;
        node["Square_Size"]  >> squareSize;
        node["Calibrate_Pattern"] >> patternToUse;
        node["Calibrate_NrOfFrameToUse"] >> nrFrames;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Write_DetectedFeaturePoints"] >> bwritePoints;
        node["Write_extrinsicParameters"] >> bwriteExtrinsics;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
        node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
        node["Show_UndistortedImage"] >> showUndistorsed;
        node["Input_Delay"] >> delay;
        interprate();
    }
    void interprate()
    {

        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0)
        {
            cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6)
        {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }
        if (nrFrames <= 0)
        {
            cerr << "Invalid number of frames " << nrFrames << endl;
            goodInput = false;
        }

        inputType = CAMERA;
    }

    Mat nextImage(bool b)
    {

	if (b)
        	return getMatFromCameraImage(camera1ID, cameraArray);
	else
	        return getMatFromCameraImage(camera2ID, cameraArray);
    }

public:
    Size boardSize;            // The size of the board -> Number of items by width and height
    Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    bool showUndistorsed;       // Show undistorted images after calibration

    int camera1ID;
    int camera2ID;
    InputType inputType;
    bool goodInput;

private:
    string patternToUse;
};

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

int stereo_get_images(int c1, int c2, Camera * camArray[])
{
    help();
    Settings s;

    //Copy Camera Array - Temporary
    s.cameraArray[0] = camArray[0];
    s.cameraArray[1] = camArray[1];
    s.cameraArray[2] = camArray[2];
    s.camera1ID = c1;
    s.camera2ID = c2;
printf("\nHERE\n");
    const string inputSettingsFile = "individual_calib_input.xml";
    
    int mode = DETECTION;
    clock_t prevTimestamp = 0;
    const Scalar RED(0,0,255), GREEN(0,255,0);
    const char ESC_KEY = 27;
	
    Mat view1, view2;
    
    for(int i = 0; i < NUM_FRAMES;)
    {
      
      bool blinkOutput = false;

      view1 = s.nextImage(true);
      view2 = s.nextImage(false);

        if( FLIP_VERTICAL )
	{    
		if (c2 != 2)
		{
			flip( view1, view1, 0);
			flip( view2, view2, 0);
		}else {
			flip (view1, view1, 0);
		}
	} 
	

	if( mode == CAPTURING)  // For camera only take new samples after delay time
	{
	if (clock() - prevTimestamp > 3000000)
{
	std::stringstream sstm;
	sstm << i << "_Cameras_" << c1 << "_" << c2 << ".jpg";

	imwrite("Results/left" + sstm.str(), view1);
	imwrite("Results/right" + sstm.str(), view2);
	//cout << "Press Key to Continue" << endl;
	waitKey(100);
	blinkOutput = false;
	imshow("Image View", view1);
	imshow("Image View2", view2);
	waitKey(100);
	i++;
	prevTimestamp = clock();
	blinkOutput = true;
}
	}

        //----------------------------- Output Text ------------------------------------------------
        string msg = (mode == CAPTURING) ? "100/100" :
                      mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
        int baseLine = 0;
        Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
        Point textOrigin(view1.cols - 2*textSize.width - 10 - 100, view1.rows - 2*baseLine - 10 - 200);

        if( mode == CAPTURING )
        {
            if(s.showUndistorsed)
                msg = format( "%d/%d Undist", i, s.nrFrames );
            else
                msg = format( "%d/%d", i, s.nrFrames );
        }

        putText( view1, msg, textOrigin, 1, 1, mode == CALIBRATED ?  GREEN : RED);

        if( blinkOutput )
	{
	    bitwise_not(view1, view1);
	    bitwise_not(view2, view2);
	}

        
        //------------------------------ Show image and check for input commands -------------------
	
        imshow("Image View", view1);
	imshow("Image View2", view2);
        char key = (char)waitKey(50);

        if( key  == ESC_KEY )
            break;

        if( key == 'u' && mode == CALIBRATED )
           s.showUndistorsed = !s.showUndistorsed;

        if(key == 'g' )
        {
            mode = CAPTURING;
        }
    }

    return 0;
}
