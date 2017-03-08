cmake_minimum_required(VERSION 2.8)
project( stereoCamera )
find_package( OpenCV REQUIRED )
find_package( CUDA REQUIRED )

set(
    CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS};
    -O3 -lineinfo
    -gencode=arch=compute_30,code=sm_30
    -gencode=arch=compute_35,code=sm_35
    -gencode=arch=compute_50,code=sm_50
    -gencode=arch=compute_52,code=sm_52
    -gencode=arch=compute_53,code=sm_53
    )

find_library(FLYCAPTURE2 flycapture)

set_source_files_properties( stereoCamera.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( cameraSetup.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( individual_calib.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( stereo_calib.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( stereo_rectify_disp.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( stereo_rectify_disp-with_trackbars.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( stereo_get_images.cpp PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ )
set_source_files_properties( gpio.c PROPERTIES CUDA_SOURCE_PROPERTY_FORMAT OBJ)

cuda_add_executable( stereoCamera 
		     stereoCamera.cpp cameraSetup.cpp individual_calib.cpp stereo_calib.cpp stereo_rectify_disp.cpp stereo_get_images.cpp   
		     stereo_rectify_disp-with_trackbars.cpp do_disp.cu median_filter.cu hamming_cost.cu disparity_method.cu debug.cu costs.cu gpio.c
		     )

target_link_libraries( stereoCamera ${OpenCV_LIBS} flycapture)
