// Canny Edge Detection Algorithm - OpenCV SW Implementation
// Created by: Aadeetya Shreedhar and Alexander Wang
// ECE 5775, Fall 2013
// Dec 5th, 2013
// 
// This file contains all of the subroutines which perform the
// Canny Edge Detection algorithm. It takes an image's pixel
// stream and pushes it's pixels one by one through the functional
// blocks below to perform the 5 stages of Canny Edge Detection.
// In the end it produces an output image highlighting just the edges
// of the input image.
//
// All of the C++ code below is non-synthesizable and uses subroutines
// from the OpenCV video processing libraries.
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

#include "opencv_top.h"
#include "top.h"
#include "highgui.hpp"
#include "imgproc.hpp"



using namespace cv;
using namespace std;




// This is doing the image filter using OpenCV code in SW
void opencv_image_filter(IplImage* src, IplImage* dst) {
    IplImage* tmp = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	// copy the input stream
    cvCopy(src, tmp);
	// convert to a Matrix
	cv::Mat srcMat(tmp);
	cv::Mat dstMat(dst);
        //cv::Mat cdstMat(dst);


	// Convert to grayscale (1 channel)
	cvtColor(srcMat, dstMat, CV_BGR2GRAY);
	// Perform Gaussian Blur with 5x5 kernel and sigma of 1.4
	cv::GaussianBlur(dstMat, dstMat, cv::Size(5,5), 1.4, 1.4);
	// Perform Canny filtering
        Canny(dstMat, dstMat, 100, 140);
	// Convert back to RGB (3 channel)
//	cvtColor(dstMat, dstMat, CV_GRAY2RGB, src->nChannels);
       
        //Hough transform

 /*       vector<Vec2f> lines;
        HoughLines(dstMat, lines, 1, 1, 500, 0, 0 ); 
    
     for( size_t i = 0; i < lines.size(); i++ )
  {
     float rho = lines[i][0], theta = lines[i][1];
     Point pt1, pt2;
     double a = cos(theta), b = sin(theta);
     double x0 = a*rho, y0 = b*rho;
     pt1.x = cvRound(x0 + 1000*(-b));
     pt1.y = cvRound(y0 + 1000*(a));
     pt2.x = cvRound(x0 - 1000*(-b));
     pt2.y = cvRound(y0 - 1000*(a));
     line( dstMat, pt1, pt2, Scalar(255,0,0), 3, CV_AA);
  }
*/



 vector<Vec4i> lines;
  HoughLinesP(dstMat, lines, 1, CV_PI/180, 1000, 2000, 5 );
  for( size_t i = 0; i < lines.size(); i++ )
  {
    Vec4i l = lines[i];
    line( dstMat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
  }     






//	cvtColor(dstMat, dstMat, CV_GRAY2RGB, src->nChannels);  

      cvtColor(dstMat, dstMat, CV_GRAY2BGR);



   // imshow("original", srcMat);   
    //imshow("detected lines", dstMat);



	IplImage dstImg = dstMat;
	// Convert back to IplImage and output
	cvCopy(&dstImg, dst);
	cvReleaseImage(&tmp);    
}

// This is doing the image filter using HLS code in SW
void sw_image_filter(IplImage* src, IplImage* dst) {
    AXI_STREAM src_axi, dst_axi;
    IplImage2AXIvideo(src, src_axi);
    image_filter(src_axi, dst_axi, src->height, src->width);
    AXIvideo2IplImage(dst_axi, dst);
}
