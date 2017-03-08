/**
    This file is part of sgm. (https://github.com/dhernandez0/sgm).

    Copyright (c) 2016 Daniel Hernandez Juarez.

    sgm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sgm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with sgm.  If not, see <http://www.gnu.org/licenses/>.

**/

#include "do_disp.h"

bool directory_exists(const char* dir) {
	DIR* d = opendir(dir);
	bool ok = false;
	if(d) {
	    closedir(d);
	    ok = true;
	}
	return ok;
}

void disparity_errors(cv::Mat estimation, const char* gt_file, int *n, int *n_err) {
	int nlocal = 0;
	int nerrlocal = 0;

	cv::Mat gt_image = cv::imread(gt_file, cv::IMREAD_UNCHANGED);
	if(!gt_image.data) {
		std::cerr << "Couldn't read the file " << gt_file << std::endl;
		exit(EXIT_FAILURE);
	}
	if(estimation.rows != gt_image.rows || estimation.cols != gt_image.cols) {
		std::cerr << "Ground truth must have the same dimesions" << std::endl;
		exit(EXIT_FAILURE);
	}
	const int type = estimation.type();
	const uchar depth = type & CV_MAT_DEPTH_MASK;
	for(int i = 0; i < gt_image.rows; i++) {
		for(int j = 0; j < gt_image.cols; j++) {
			const uint16_t gt = gt_image.at<uint16_t>(i, j);
			if(gt > 0) {
				const float gt_f = ((float)gt)/256.0f;
				float est;
				if(depth == CV_8U) {
					est = (float) estimation.at<uint8_t>(i, j);
				} else {
					est = estimation.at<float>(i, j);
				}
				const float err = fabsf(est-gt_f);
				const float ratio = err/fabsf(gt_f);
				if(err > ABS_THRESH && ratio > REL_THRESH) {
					nerrlocal++;
				}
				nlocal++;
			}
		}
	}
	*n += nlocal;
	*n_err += nerrlocal;
}

bool check_directories_exist(const char* directory, const char* left_dir, const char* right_dir, const char* disparity_dir) {
	char left_dir_sub[PATH_MAX];
	char right_dir_sub[PATH_MAX];
	char disparity_dir_sub[PATH_MAX];
	sprintf(left_dir_sub, "%s/%s", directory, left_dir);
	sprintf(right_dir_sub, "%s/%s", directory, right_dir);
	sprintf(disparity_dir_sub, "%s/%s", directory, disparity_dir);

	return directory_exists(left_dir_sub) && directory_exists(right_dir_sub) && directory_exists(disparity_dir_sub);
}

cv::Mat do_disp(int param1, int param2, cv::Mat lf, cv::Mat rf) {

	std::vector<float> times;

	init_disparity_method(param1, param2);
	cv::Mat disparity_im;

	cv::Mat h_im0 = lf;
	if(!h_im0.data) {
		std::cerr << "Couldn't read the file " << std::endl;
	}
	cv::Mat h_im1 = rf;
	if(!h_im1.data) {
		std::cerr << "Couldn't read the file " << std::endl;
	}

	// Convert images to grayscale
	if (h_im0.channels()>1) {
		cv::cvtColor(h_im0, h_im0, CV_RGB2GRAY);
	}

	if (h_im1.channels()>1) {
		cv::cvtColor(h_im1, h_im1, CV_RGB2GRAY);
	}

	if(h_im0.rows != h_im1.rows || h_im0.cols != h_im1.cols) {
		std::cerr << "Both images must have the same dimensions" << std::endl;
	}

#if LOG
	std::cout << "processing: " << std::endl;
#endif
	// Compute
	float elapsed_time_ms;
	disparity_im = compute_disparity_method(h_im0, h_im1, &elapsed_time_ms);
#if LOG
	std::cout << "done" << std::endl;
#endif
	times.push_back(elapsed_time_ms);

	finish_disparity_method();

	double mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

	std::cout << "It took an average of " << mean << " miliseconds, " << 1000.0f/mean << " fps" << std::endl;

	return disparity_im;
}
