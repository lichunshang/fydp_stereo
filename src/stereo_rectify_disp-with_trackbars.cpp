/* This is sample from the OpenCV book. The copyright notice is below */

#include "stereo_rectify_disp-with_trackbars.h"
#include "do_disp.h"

using namespace cv;
using namespace std;

#define NUMCAMS 3


static const int p1_s_max = 128;
static const int p2_s_max = 10000;
static int p1_s;
static int p2_s;
static Mat disp = imread("Results/left0_Cameras_0_1.jpg", 0);
static Mat canvas, croppedl, croppedr;
static Rect vroi2;
static double sf;
static int w, h;
static Mat img = imread("Results/left0_Cameras_0_1.jpg", 0), cimg;
static int c1, c2;
static Camera * cArray[NUMCAMS];
static Mat rmap[2][2];
static Mat canvasPart;
static Rect vroi;
static bool isVerticalStereo = false;
static Mat R, T, R1, R2, P1, P2, Q;
static Rect validRoi[2];


void on_trackbar( int, void* )
{
	
	for(int k = 0; k < 2; k++ )
		{

		if (k == 0)
		    img = getMatFromCameraImage(c1, cArray);

		else
		    img = getMatFromCameraImage(c2, cArray);
		if (k == 0 || (k== 1 && c2 != 2))
	    		flip( img, img, 0);

            remap(img, cimg, rmap[k][0], rmap[k][1], CV_INTER_LINEAR);

            //cvtColor(rimg, cimg, COLOR_GRAY2BGR);
            canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));

            resize(cimg, canvasPart, canvasPart.size(), 0, 0, CV_INTER_AREA);

		vroi.x = cvRound(validRoi[k].x*sf);
		vroi.y = cvRound(validRoi[k].y*sf);
		vroi.width = cvRound(validRoi[k].width*sf);
		vroi.height = cvRound(validRoi[k].height*sf);
		
		// Crop to ROI
		if (k == 0)
		{			
			vroi2 = vroi;
			vroi2.height -= 60;
			vroi2.width -= 60;
			vroi2.x += 20;
			vroi2.y += 20;
			vroi2.height -= vroi2.height%4;
			vroi2.width -= vroi2.width%4;
			//croppedl = canvasPart.clone();
			croppedl = canvasPart(vroi2);
		}
		else
		{
			//croppedr = canvasPart.clone();
			croppedr = canvasPart(vroi2);
		}
		rectangle(canvasPart, vroi2, Scalar(0,0,255), 1, 8);
            
	}

        if( !isVerticalStereo )
            for(int j = 0; j < canvas.rows; j += 16 )
                line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
        else
            for(int j = 0; j < canvas.cols; j += 16 )
                line(canvas, Point(j, 0), Point(j, canvas.rows), Scalar(0, 255, 0), 1, 8);

	disp = do_disp(p1_s, p2_s, croppedl, croppedr);

        imshow("rectified", canvas);
//	imshow("croppedl", croppedl);
//	imshow("croppedr", croppedr);
        imshow("disparity", disp);	
}

static void
StereoRectifyDisp(int camera1ID, int camera2ID, Camera * camArray[])
{
c1 = camera1ID;
c2 = camera2ID;

cArray[0] = camArray[0];
cArray[1] = camArray[1];
cArray[2] = camArray[2];

 /// Create Windows
 namedWindow("disparity", 1);

    p1_s = 16;
    p2_s = 1000;

	char Trackbar1Name[50];
	char Trackbar2Name[50];
	 sprintf( Trackbar1Name, "Param1 x %d", p1_s_max );
	 createTrackbar( Trackbar1Name, "disparity", &p1_s, p1_s_max, on_trackbar );
	 sprintf( Trackbar2Name, "Param2 x %d", p2_s_max );
	 createTrackbar( Trackbar2Name, "disparity", &p2_s, p2_s_max, on_trackbar );



    Size imageSize = img.size();
    Mat cameraMatrix[2], distCoeffs[2];
	// Todo: Alter this based on CameraIDs to read proper file
    // Read intrinsic parameters
    std::stringstream sstm, sstm2;
	sstm << "Results/intrinsics_" << camera1ID << camera2ID << ".yml";
	sstm2 << "Results/extrinsics_" << camera1ID << camera2ID << ".yml";

    FileStorage fs(sstm.str(), CV_STORAGE_READ);
    if( fs.isOpened() )
    {
	fs["M1"] >> cameraMatrix[0];
	fs["D1"] >> distCoeffs[0];
        fs["M2"] >> cameraMatrix[1];
	fs["D2"] >> distCoeffs[1];
	fs.release();
    }
    else
        cout << "Error: can not load the intrinsic parameters\n";

    fs.open(sstm2.str(), CV_STORAGE_READ);
    if( fs.isOpened() )
    {
	fs["R"] >> R;
	fs["T"] >> T;
	fs["R1"] >> R1;
	fs["R2"] >> R2;
	fs["P1"] >> P1;
	fs["P2"] >> P2;
	fs["Q"] >> Q;
        fs["ValidROI1"] >> validRoi[0];
	fs["ValidROI2"] >> validRoi[1];
        fs.release();
    }
    else
        cout << "Error: can not laod the extrinsic parameters\n";

    // OpenCV can handle left-right
    // or up-down camera arrangements

    //Precompute maps for cv::remap()
    initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
    initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);

    
    if( !isVerticalStereo )
    {
        //sf = 600./MAX(imageSize.width, imageSize.height);
	sf = 1;
        w = cvRound(imageSize.width*sf);
        h = cvRound(imageSize.height*sf);
        canvas.create(h, w*2, CV_8UC3);
    }
    else
    {
        //sf = 300./MAX(imageSize.width, imageSize.height);
	sf = 1;
        w = cvRound(imageSize.width*sf);
        h = cvRound(imageSize.height*sf);
        canvas.create(h*2, w, CV_8UC3);
    }

	//int ctr =1 ;
//while (1){

	 /// Show some stuff
	 on_trackbar( p1_s, 0 );


	//std::stringstream sstm3;
	//sstm3 << "Disparity_" << ctr << ".jpg";
	//ctr++;

	//imwrite(sstm3.str(), disp);

	// To show cropped image
	
        /*char c = (char)waitKey(100);
        if( c == 27 || c == 'q' || c == 'Q' )
            break;    
		
	}*/
        char c = (char)waitKey();
        if( c == 27 || c == 'q' || c == 'Q' )
            return;    

}

int stereo_rectify_disp_t(int camera1ID, int camera2ID, Camera * camArray[])
{
    StereoRectifyDisp(camera1ID, camera2ID, camArray);
    return 0;
}
