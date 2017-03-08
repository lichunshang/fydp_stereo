/* This is sample from the OpenCV book. The copyright notice is below */

#include "stereo_rectify_disp.h"
#include "do_disp.h"

using namespace cv;
using namespace std;

#define NUMCAMS 3

static int p1_s, p3_s;
static int p2_s, p4_s;
static Mat disp = imread("Results/left0_Cameras_0_1.jpg", 0);
static Mat disp2 = imread("Results/left0_Cameras_0_1.jpg", 0);
static Mat fdisp;
static Mat img = imread("Results/left0_Cameras_0_1.jpg", 0), cimg;
static Mat canvas, croppedl, croppedr;
static double sf;
static int w, h;
static Camera * cArray[NUMCAMS];
static Mat rmap1[2][2], rmap2[2][2];
static Mat canvasPart;
static Rect vroi_1, vroi_2, vroi2_1, vroi2_2, fuse_roi;
static bool isVerticalStereo = false;
static Mat R_1, T_1, R1_1, R2_1, P1_1, P2_1, Q_1;
static Mat R_2, T_2, R1_2, R2_2, P1_2, P2_2, Q_2;
static Rect validRoi1[2], validRoi2[2];
static Size imageSize = img.size();
static Mat cameraMatrix1[2], distCoeffs1[2], cameraMatrix2[2], distCoeffs2[2];

cv::Mat getDispMat(int c1ID, int c2ID, int pair)
{
	for(int k = 0; k < 2; k++ )
	{
		if (k == 0)
		    img = getMatFromCameraImage(c1ID, cArray);
		else
		    img = getMatFromCameraImage(c2ID, cArray);

		if (k == 0 || (k== 1 && c2ID != 2))
			flip( img, img, 0);

		if (pair == 1)
			remap(img, cimg, rmap1[k][0], rmap1[k][1], CV_INTER_LINEAR);
		else
			remap(img, cimg, rmap2[k][0], rmap2[k][1], CV_INTER_LINEAR);

		canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));
		resize(cimg, canvasPart, canvasPart.size(), 0, 0, CV_INTER_AREA);

		// Crop to ROI
		if (pair == 1)
		{
			if (k == 0)
				croppedl = canvasPart(fuse_roi);
			else
				croppedr = canvasPart(fuse_roi);
			rectangle(canvasPart, vroi2_1, Scalar(0,0,255), 1, 8);
		}
		else
		{
			if (k == 0)
				croppedl = canvasPart(fuse_roi);
			else
				croppedr = canvasPart(fuse_roi);
			rectangle(canvasPart, vroi2_2, Scalar(0,0,255), 1, 8);
		}

	}

	/*        if( !isVerticalStereo )
	for(int j = 0; j < canvas.rows; j += 16 )
	line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
	else
	for(int j = 0; j < canvas.cols; j += 16 )
	line(canvas, Point(j, 0), Point(j, canvas.rows), Scalar(0, 255, 0), 1, 8);*/

	//imshow("rectified", canvas);

	//std::stringstream sstm3;
	//sstm3 << "Disparity_" << ctr << ".jpg";
	//ctr++;

	//imwrite(sstm3.str(), disp);

	// To show cropped image
	//imshow("croppedl", croppedl);
	//imshow("croppedr", croppedr);
	if (pair == 1)
		return do_disp(p1_s, p2_s, croppedl, croppedr).clone();
	else
		return do_disp(p3_s, p4_s, croppedl, croppedr).clone();
}

void init(int c1ID, int c2ID, int pair)
{
	// Read intrinsic parameters
	std::stringstream sstm, sstm2;
	sstm << "Results/intrinsics_" << c1ID << c2ID << ".yml";
	sstm2 << "Results/extrinsics_" << c1ID << c2ID << ".yml";

	FileStorage fs(sstm.str(), CV_STORAGE_READ);
	if( fs.isOpened() )
	{
	if (pair == 1)
	{
		fs["M1"] >> cameraMatrix1[0];
		fs["D1"] >> distCoeffs1[0];
		fs["M2"] >> cameraMatrix1[1];
		fs["D2"] >> distCoeffs1[1];
	}
	else
	{
		fs["M1"] >> cameraMatrix2[0];
		fs["D1"] >> distCoeffs2[0];
		fs["M2"] >> cameraMatrix2[1];
		fs["D2"] >> distCoeffs2[1];
	}
	fs.release();
	}
	else
	cout << "Error: can not load the intrinsic parameters\n";

	fs.open(sstm2.str(), CV_STORAGE_READ);
	if( fs.isOpened() )
	{
	if (pair == 1)
	{
		fs["R"] >> R_1;
		fs["T"] >> T_1;
		fs["R1"] >> R1_1;
		fs["R2"] >> R2_1;
		fs["P1"] >> P1_1;
		fs["P2"] >> P2_1;
		fs["Q"] >> Q_1;
		fs["ValidROI1"] >> validRoi1[0];
		fs["ValidROI2"] >> validRoi1[1];
	}
	else
	{
		fs["R"] >> R_2;
		fs["T"] >> T_2;
		fs["R1"] >> R1_2;
		fs["R2"] >> R2_2;
		fs["P1"] >> P1_2;
		fs["P2"] >> P2_2;
		fs["Q"] >> Q_2;
		fs["ValidROI1"] >> validRoi2[0];
		fs["ValidROI2"] >> validRoi2[1];
	}
	fs.release();
	}
	else
	cout << "Error: can not laod the extrinsic parameters\n";

	//Precompute maps for cv::remap()
	if (pair == 1)
	{
		initUndistortRectifyMap(cameraMatrix1[0], distCoeffs1[0], R1_1, P1_1, imageSize, CV_16SC2, rmap1[0][0], rmap1[0][1]);
		initUndistortRectifyMap(cameraMatrix1[1], distCoeffs1[1], R2_1, P2_1, imageSize, CV_16SC2, rmap1[1][0], rmap1[1][1]);
		vroi_1.x = cvRound(validRoi1[0].x*sf);
		vroi_1.y = cvRound(validRoi1[0].y*sf);
		vroi_1.width = cvRound(validRoi1[0].width*sf);
		vroi_1.height = cvRound(validRoi1[0].height*sf);
		vroi2_1 = vroi_1;
		vroi2_1.height -= 60;
		vroi2_1.width -= 60;
		vroi2_1.x += 20;
		vroi2_1.y += 20;
		vroi2_1.height -= vroi2_1.height%4;
		vroi2_1.width -= vroi2_1.width%4;

	}
	else
	{
		initUndistortRectifyMap(cameraMatrix2[0], distCoeffs2[0], R1_2, P1_2, imageSize, CV_16SC2, rmap2[0][0], rmap2[0][1]);
		initUndistortRectifyMap(cameraMatrix2[1], distCoeffs2[1], R2_2, P2_2, imageSize, CV_16SC2, rmap2[1][0], rmap2[1][1]);
		vroi_2.x = cvRound(validRoi2[0].x*sf);
		vroi_2.y = cvRound(validRoi2[0].y*sf);
		vroi_2.width = cvRound(validRoi2[0].width*sf);
		vroi_2.height = cvRound(validRoi2[0].height*sf);
		vroi2_2 = vroi_2;
		vroi2_2.height -= 60;
		vroi2_2.width -= 60;
		vroi2_2.x += 20;
		vroi2_2.y += 20;
		vroi2_2.height -= vroi2_2.height%4;
		vroi2_2.width -= vroi2_2.width%4;
	}

	fuse_roi.x = vroi2_1.x;
	fuse_roi.y = vroi2_1.y;
	if (vroi2_1.width <= vroi2_2.width)
		fuse_roi.width = vroi2_1.width;
	else
		fuse_roi.width = vroi2_2.width;

	if (vroi2_1.height <= vroi2_2.height)
		fuse_roi.height = vroi2_1.height;
	else
		fuse_roi.height = vroi2_2.height;

}

static void
StereoRectifyDisp(int camera1ID1, int camera1ID2, int camera2ID1, int camera2ID2, Camera * camArray[])
{
	cArray[0] = camArray[0];
	cArray[1] = camArray[1];
	cArray[2] = camArray[2];

	p1_s = 0;
	p2_s = 90;
	p3_s = 0;
	p4_s = 150;

	sf = 1;

	init(camera1ID1, camera1ID2, 1);
	init(camera2ID1, camera2ID2, 2);

	if( !isVerticalStereo )
	{
	//sf = 600./MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width*sf);
	h = cvRound(imageSize.height*sf);
	canvas.create(h, w*2, CV_8UC3);
	}
	else
	{
	//sf = 300./MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width*sf);
	h = cvRound(imageSize.height*sf);
	canvas.create(h*2, w, CV_8UC3);
	}

	while (1){
	    // use external trigger before grabbing images
    	unsigned int idxArray[] = {(unsigned int) camera1ID1, (unsigned int) camera1ID2, (unsigned int) camera2ID1, (unsigned int) camera2ID2};
	    triggerGPIO(idxArray, 4 , cArray);
        
		disp = getDispMat(camera1ID1, camera1ID2, 1);
        disp2 = disp;
//		disp2 = getDispMat(camera2ID1, camera2ID2, 2);
		addWeighted( disp, 0.5, disp2, 0.5, 0.0, fdisp);	
		imshow("disparity1", disp);
		imshow("disparity2", disp2);	
		imshow("fused", fdisp);	
		char c = (char)waitKey(30);
		if( c == 27 || c == 'q' || c == 'Q' )
		    break;   
	} 
}

int stereo_rectify_disp(int camera1ID1, int camera1ID2, int camera2ID1, int camera2ID2, Camera * camArray[])
{
    StereoRectifyDisp(camera1ID1, camera1ID2, camera2ID1, camera2ID2, camArray);
    return 0;
}
