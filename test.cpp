////////////////////////////////////////////////////////////////////////////
// Canny Edge Detection Algorithm - Simulation testbench
// Created by: Aadeetya Shreedhar and Alexander Wang
// ECE 5775, Fall 2013
// Dec 5th, 2013
// 
// This file takes an input static image and performs Canny Edge
// Detection on the image using both the HLS "HW" implementation
// and the OpenCV SW implementation, producing two output images.
//
// This file is modified off a tutorial created by Xilinx, and as such
// we retain the below copyright notice and disclaimer.
////////////////////////////////////////////////////////////////////////////

/***************************************************************************

*   © Copyright 2013 Xilinx, Inc. All rights reserved. 

*   This file contains confidential and proprietary information of Xilinx,
*   Inc. and is protected under U.S. and international copyright and other
*   intellectual property laws. 

*   DISCLAIMER
*   This disclaimer is not a license and does not grant any rights to the
*   materials distributed herewith. Except as otherwise provided in a valid
*   license issued to you by Xilinx, and to the maximum extent permitted by
*   applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH
*   ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, 
*   EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES
*   OF MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR 
*   PURPOSE; and (2) Xilinx shall not be liable (whether in contract or 
*   tort, including negligence, or under any other theory of liability)
*   for any loss or damage of any kind or nature related to, arising under
*   or in connection with these materials, including for any direct, or any
*   indirect, special, incidental, or consequential loss or damage (including
*   loss of data, profits, goodwill, or any type of loss or damage suffered 
*   as a result of any action brought by a third party) even if such damage
*   or loss was reasonably foreseeable or Xilinx had been advised of the 
*   possibility of the same. 

*   CRITICAL APPLICATIONS 
*   Xilinx products are not designed or intended to be fail-safe, or for use
*   in any application requiring fail-safe performance, such as life-support
*   or safety devices or systems, Class III medical devices, nuclear facilities,
*   applications related to the deployment of airbags, or any other applications
*   that could lead to death, personal injury, or severe property or environmental
*   damage (individually and collectively, "Critical Applications"). Customer
*   assumes the sole risk and liability of any use of Xilinx products in Critical
*   Applications, subject only to applicable laws and regulations governing 
*   limitations on product liability. 

*   THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT 
*   ALL TIMES.

***************************************************************************/
#include "top.h"
#include "opencv_top.h"


int main (int argc, char** argv) {

    // Load input image
    IplImage* src = cvLoadImage(INPUT_IMAGE);
	// The open_CV image will be 3 channel RGB
    IplImage* dst_opencv = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	// The HLS image will be 1 channel grayscale
    IplImage* dst_hls = cvCreateImage(cvGetSize(src), src->depth, 3);

	// Process the input image using the HLS implementation
    AXI_STREAM  src_axi, dst_axi;
    IplImage2AXIvideo(src, src_axi);
    image_filter(src_axi, dst_axi, src->height, src->width);
    AXIvideo2IplImage(dst_axi, dst_hls);
    cvSaveImage(OUTPUT_IMAGE, dst_hls);
    
	// Process the input image using the OpenCV implementation
    opencv_image_filter(src, dst_opencv);
    cvSaveImage(OUTPUT_IMAGE_GOLDEN, dst_opencv);

    cvReleaseImage(&src);
    cvReleaseImage(&dst_hls);
    cvReleaseImage(&dst_opencv);

	// Compare the resulting images (they will differ)
    char tempbuf[2000];
    sprintf(tempbuf, "diff --brief -w %s %s", OUTPUT_IMAGE, OUTPUT_IMAGE_GOLDEN);
    int ret = system(tempbuf);
	printf("Outputs generated!\n");
    return ret;

}
