/* This is sample from the OpenCV book. The copyright notice is below */

#include "stereo_rectify_disp.h"
#include "do_disp.h"

using namespace cv;
using namespace std;

static void
StereoRectify(int camera1ID, int camera2ID, Camera * camArray[])
{
    int param1 = 1;
    int param2 = 5000;
    Mat img = imread("Results/left0.jpg", 0), rimg, cimg, disp;
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

    Mat R, T, R1, R2, P1, P2, Q;
    Rect validRoi[2];
    
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
    bool isVerticalStereo = false;

    Mat rmap[2][2];

    //Precompute maps for cv::remap()
    initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
    initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);

    Mat canvas, croppedl, croppedr;
    Rect vroi2;
    double sf;
    int w, h;
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

int ctr =1 ;
	for (int t = 0; t < 100 ; t++){

        for(int k = 0; k < 2; k++ )
        {
		if (k == 0)
		    img = getMatFromCameraImage(camera1ID, camArray);
		else
		    img = getMatFromCameraImage(camera2ID, camArray);
		
		flip( img, img, 0);

            remap(img, cimg, rmap[k][0], rmap[k][1], CV_INTER_LINEAR);
            //cvtColor(rimg, cimg, COLOR_GRAY2BGR);
            Mat canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));
            resize(cimg, canvasPart, canvasPart.size(), 0, 0, CV_INTER_AREA);
            
                Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
                          cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf));               
		
		// Crop to ROI
		if (k == 0)
		{			
			vroi2 = vroi;
			vroi2.height -= vroi2.height%4;
			vroi2.width -= vroi2.width%4;
			croppedl = canvasPart.clone();
			croppedl = croppedl(vroi2);
		}
		else
		{
			croppedr = canvasPart.clone();
			croppedr = croppedr(vroi2);
		}
		rectangle(canvasPart, vroi, Scalar(0,0,255), 3, 8);
            
	}


        /*if( !isVerticalStereo )
            for(int j = 0; j < canvas.rows; j += 16 )
                line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
        else
            for(int j = 0; j < canvas.cols; j += 16 )
                line(canvas, Point(j, 0), Point(j, canvas.rows), Scalar(0, 255, 0), 1, 8);*/

	disp = do_disp(param1, param2, croppedl, croppedr);

        //imshow("rectified", canvas);
        imshow("disparity", disp);

	//std::stringstream sstm3;
	//sstm3 << "Disparity_" << ctr << ".jpg";
	//ctr++;

	//imwrite(sstm3.str(), disp);

	// To show cropped image
//	imshow("croppedl", croppedl);
//	imshow("croppedr", croppedr);
        char c = (char)waitKey(1000);
        if( c == 27 || c == 'q' || c == 'Q' )
            break;    
		
	}
        char c = (char)waitKey();
        if( c == 27 || c == 'q' || c == 'Q' )
            return;    
}

int stereo_rectify_disp(int camera1ID, int camera2ID, Camera * camArray[])
{

    StereoRectify(camera1ID,camera2ID, camArray);
    return 0;
}
