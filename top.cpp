////////////////////////////////////////////////////////////////////////////
// Canny Edge Detection Algorithm - HLS HW Implementation
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
// All of the C++ code below is synthesizable and is intended to
// be converted into RTL using Vivado HLS tools.
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
#include "ap_int.h"
#include<stdio.h>
#include<math.h>
#include "cordic.h"
//#include "cordic.cpp"


// Gradient Decomposition: Takes the two directional gradients and calculates the gradient
//                         magnitude and direction and combines them into a 16-bit pixel stream.




void gradient_decomposition(RGB_IMAGE& gx, RGB_IMAGE& gy, RGB_IMAGE_16& gd ) {
	
	HLS_SIZE_T rows = gx.rows;
	HLS_SIZE_T cols = gx.cols;

	hls::Scalar<3, unsigned char> pixel_gx;
	hls::Scalar<3, unsigned char> pixel_gy;
	hls::Scalar<3, unsigned short> element_pixel;

	unsigned char element_gx;
	unsigned char element_gy;
	unsigned char abs_gx;
	unsigned char abs_gy;
	unsigned short abs_g;
	unsigned short dir_g;
	unsigned short element_gd;

	// Calculate gradient magnitude and direction
	for( HLS_SIZE_T i = 0; i < rows; i++ ) {
		for( HLS_SIZE_T j = 0; j < cols; j++ ) {
#pragma HLS LOOP_FLATTEN OFF
#pragma HLS DEPENDENCE array inter false
#pragma HLS PIPELINE
		    // Stream in the two input directional gradients
			gx >> pixel_gx;
			gy >> pixel_gy;
			element_gx = pixel_gx.val[0];
			element_gy = pixel_gy.val[0];

			// Calculate the absolute value of the pixel's gradient
			abs_gx = hls::abs(element_gx);
			abs_gy = hls::abs(element_gy);

			// Calculate the magnitude of the gradient (approximated by the L1 norm)
			abs_g  = abs_gx + abs_gy;

			// Calculate the approximate direction of the pixel
			// 0 is 0 degrees, 1 is 45 degrees, 2 is 90 degrees, 3 is 135 degrees
			if (abs_gx > abs_gy && ((element_gx > 0 && element_gy >= 0)||(element_gx < 0 && element_gy <= 0))) {
			  if (abs_gx > (2*abs_gy)) dir_g = 0;
			  else dir_g = 1;
			}
			else if (abs_gx <= abs_gy && ((element_gx > 0 && element_gy > 0)||(element_gx < 0 && element_gy < 0))) {
			  if (abs_gy > (2*abs_gx)) dir_g = 2;
			  else dir_g = 1;
			}
			else if (abs_gx < abs_gy && ((element_gx >= 0 && element_gy < 0)||(element_gx <= 0 && element_gy > 0))) {
			  if (abs_gy > (2*abs_gx)) dir_g = 2;
			  else dir_g = 3;
			}
			else {
			  if (abs_gy > (2*abs_gx)) dir_g = 0;
			  else dir_g = 3;
			}
			// Combine the 8-bit magnitude and directions into a 16 bit pixel (direction in the LSBs)
			element_gd = ( ( abs_g << 2 ) | dir_g );
			element_pixel.val[0] = element_gd;
			// Stream out the pixel
			gd << element_pixel;
		}
	}
}

void nonmax_suppression(RGB_IMAGE_16& gd, RGB_IMAGE& dst) {
	
	HLS_SIZE_T rows = gd.rows;
	HLS_SIZE_T cols = gd.cols;

	// Line buffer can only handle images of up to 1920 pixels wide
	hls::LineBuffer<2, 1920, unsigned short> linebuff;
	// 3x3 computation kernel
	hls::Window<3, 3, unsigned short> win;                         //edited this line
	
	hls::Scalar<3, unsigned short> pixel_gd;
	hls::Scalar<3, unsigned char> out_pixel;

	unsigned short element_gd;
	unsigned char out_pixel_val;
	unsigned char current_dir;
	unsigned char current_grad;
	unsigned char ga;
	unsigned char gb;
	unsigned short tmp0;
	unsigned short tmp1;

	for( HLS_SIZE_T i = 0; i < rows+1; i++ ) {
		for( HLS_SIZE_T j = 0; j < cols+1; j++ ) {
#pragma HLS LOOP_FLATTEN OFF
#pragma HLS DEPENDENCE array inter false
#pragma HLS PIPELINE

		  // read pixels from the input stream only if within the bounds of the image
		  if ( i < rows && j < cols ) {
			gd >> pixel_gd;
			element_gd = pixel_gd.val[0];
		  }
	   
		  // Save the values in the line buffer
		  // and then shift up the values in the current column of the linebuffer
		  if( j < cols ) {
			tmp1 = linebuff.getval(1, j);
			tmp0 = linebuff.getval(0, j);
			// shift values up 1 row
			linebuff.val[1][j] = tmp0;
		  }
		  // Insert a new pixel into the bottom right of the line buffer if it is present
		  if( j < cols && i < rows ){
			linebuff.insert_bottom( element_gd, j );
		  }
		  
		  // Shifting the window right
		  win.shift_right();
		  
		  // Copying values from the linebuffer to the window	
		  if( j < cols ) {
			win.insert( element_gd, 0, 0 );
			win.insert( tmp0, 1, 0 );
			win.insert( tmp1, 2, 0 );
		  }
		  		  
		  // separate the 16-bit value into the gradient direction and magnitude
		  current_dir = win.getval(1, 1) & 3;
		  current_grad = win.getval(1, 1) >> 2;
		  
		  // Calculate output by checking if the pixel is a local maximum
		  // Only calculate an output if the entire kernel uses valid pixels
		  if( i <= 1 || j <= 1 || i > rows-1 || j > cols-1 ) {
			out_pixel_val = 0;
		  }
		  else {
			// Checking in the east-west axis
			if ( current_dir == 0 ) {
			  ga = win.getval( 1, 0 )>>2; 
			  gb = win.getval( 1, 2 )>>2; 
			}
			// Checking in the northEast-southWest axis
			else if ( current_dir == 1 ){
			  ga = win.getval( 2, 0 )>>2; 
			  gb = win.getval( 0, 2 )>>2; 
			}
			// Checking in the north-south axis
			else if ( current_dir == 2 ){
			  ga = win.getval( 0, 1 )>>2; 
			  gb = win.getval( 2, 1 )>>2; 
			}
			// Checking in the northWest-southEast axis
			else { // ( current_dir == 3 )
			  ga = win.getval( 2, 2 )>>2; 
			  gb = win.getval( 0, 0 )>>2; 
			}
			
			// The pixel is an edge only if it is a local maximum along any axis
			if( current_grad > ga && current_grad > gb ) {
			  out_pixel_val = current_grad;
			}
			else {
			  out_pixel_val = 0;
			}
		  }
		  
		  // Output a pixel only if it is part of the actual image
		  if( j > 0 && i > 0 ) {
			out_pixel.val[0] = out_pixel_val;
			dst << out_pixel;
		  }
		  
		}
	}
}

void hysteresis( RGB_IMAGE& src, RGB_IMAGE& dst, int threshold_low, int threshold_high ) {
	
	HLS_SIZE_T rows = src.rows;
	HLS_SIZE_T cols = src.cols;

	// Line buffer can only handle images of up to 1920 pixels wide
	hls::LineBuffer<2, 1920, unsigned char> linebuff;
	// 3x3 computation kernel
	hls::Window<3, 3, unsigned char> win;
	
	hls::Scalar<3, unsigned char> pixel_gd;
	hls::Scalar<3, unsigned char> out_pixel;

	unsigned char element_gd;
	    
	unsigned char out_pixel_val;
	unsigned char current_dir;
	unsigned char current_grad;
	unsigned char ga;
	unsigned char gb;
	unsigned char tmp0;
	unsigned char tmp1;

	for( HLS_SIZE_T i = 0; i < rows+1; i++ ) {
		for( HLS_SIZE_T j = 0; j < cols+1; j++ ) {
#pragma HLS LOOP_FLATTEN OFF
#pragma HLS DEPENDENCE array inter false
#pragma HLS PIPELINE

		  // read pixels from the input stream only if within the bounds of the image
		  if ( i < rows && j < cols ) {
		    src >> pixel_gd;
			element_gd = pixel_gd.val[0];
		  }
	   
		  // Save the values in the line buffer
		  // and then shift up the values in the current column of the linebuffer
		  if( j < cols ) {
			tmp1 = linebuff.getval(1, j);
			tmp0 = linebuff.getval(0, j);
			// shift values up 1 row
			linebuff.val[1][j] = tmp0;
		  }
		  // Insert a new pixel into the bottom right of the line buffer if it is present
		  if( j < cols && i < rows ){
			linebuff.insert_bottom( element_gd, j );
		  }
		  
		  // Shifting the window right
		  win.shift_right();
		  
		  // Copying values from the linebuffer to the window	
		  if( j < cols ) {
			win.insert( element_gd, 0, 0 );
			win.insert( tmp0, 1, 0 );
			win.insert( tmp1, 2, 0 );
		  }

		  // Calculate if this pixel is an edge by comparing it to 
		  // the high and low thresholds and its neighbors in the kernel
		  // Only calculate an output if the entire kernel uses valid pixels
		  if( i <= 1 || j <= 1 || i > rows-1 || j > cols-1 ) {
			out_pixel_val = 0;
		  }
		  else {
			if( win.getval(1,1) < threshold_low ){
			  out_pixel_val = 0;
			}
			else if( 	win.getval(1,1) > threshold_high || 
						win.getval(0,0) > threshold_low  || 
						win.getval(0,1) > threshold_low  || 
						win.getval(0,2) > threshold_low  || 
						win.getval(1,0) > threshold_low  || 
						win.getval(1,2) > threshold_low  || 
						win.getval(2,0) > threshold_low  || 
						win.getval(2,1) > threshold_low  || 
						win.getval(2,2) > threshold_low  ) {
			  out_pixel_val = 255;
			}
			else {
			  out_pixel_val = 0;
			} 
		  }		
		  
		  // Output a pixel only if it is part of the actual image
		  if( j > 0 && i > 0 ) {
			out_pixel.val[0] = out_pixel_val;
			dst << out_pixel;
		  }
		}
	}
}

void image_filter(AXI_STREAM& input, AXI_STREAM& output, int rows, int cols) {

unsigned char element_hd; //Inserted this line Akshay
float a,b,x0;
int p,q;
ap_fixed<16,2> c,s;
typedef ap_fixed<12,12> pxtype;
pxtype y0,d;
ap_fixed<16,3> theta;
ap_fixed<12,12> mag;

    //Create AXI streaming interfaces for the core
#pragma HLS RESOURCE variable=input core=AXIS metadata="-bus_bundle INPUT_STREAM"
#pragma HLS RESOURCE variable=output core=AXIS metadata="-bus_bundle OUTPUT_STREAM"

#pragma HLS RESOURCE core=AXI_SLAVE variable=rows metadata="-bus_bundle CONTROL_BUS"
#pragma HLS RESOURCE core=AXI_SLAVE variable=cols metadata="-bus_bundle CONTROL_BUS"
#pragma HLS RESOURCE core=AXI_SLAVE variable=return metadata="-bus_bundle CONTROL_BUS"

#pragma HLS INTERFACE ap_stable port=rows
#pragma HLS INTERFACE ap_stable port=cols

    RGB_IMAGE src(rows, cols);    
    RGB_IMAGE src_bw(rows, cols);
    RGB_IMAGE src_blur(rows, cols);
    RGB_IMAGE src1(rows, cols);
    RGB_IMAGE src2(rows, cols);
    RGB_IMAGE sobel_gx(rows, cols);
    RGB_IMAGE sobel_gy(rows, cols);
    RGB_IMAGE_16 grad_gd(rows, cols);
    RGB_IMAGE suppressed(rows, cols);
    RGB_IMAGE thresholded(rows, cols);
    RGB_IMAGE canny_edges(rows, cols);
    RGB_IMAGE canny_edges1(rows, cols);    //inserted these lines Akshay
    RGB_IMAGE canny_edges2(rows, cols);
    RGB_IMAGE canny_edges5(rows, cols);
    RGB_IMAGE f1(rows, cols);
    RGB_IMAGE f2(rows, cols);
    RGB_PIXEL pix(255, 0, 0);
    
#pragma HLS dataflow
    // AXI to RGB_IMAGE stream
	hls::AXIvideo2Mat( input, src );
        hls::Duplicate(src, f1, f2);
  // Grayscaling
	hls::CvtColor<HLS_RGB2GRAY>( f1, src_bw );
	// Gaussian Blur Noise Reduction
	hls::GaussianBlur<5,5>( src_bw, src_blur, 1.4, 1.4 );
	// Duplicate the streams
	hls::Duplicate( src_blur, src1, src2 );
	// Calculate gradients in x and y direction using Sobel filter
    hls::Sobel<1,0,3>( src1, sobel_gx );
    hls::Sobel<0,1,3>( src2, sobel_gy );
	// Calculate gradient magnitude and direction
	gradient_decomposition( sobel_gx, sobel_gy, grad_gd );
	// Perform non-maximum suppression for edge thinning
	nonmax_suppression( grad_gd, suppressed );
	// Perform hysteresis thresholding for edge tracing
	hysteresis( suppressed, canny_edges, 160, 220 );

	hls::Duplicate( canny_edges, canny_edges1, canny_edges2);   //making multiple copies of canny edges for future purposes
	hls::Polar_< ap_fixed<16,3>, ap_fixed<12,12> > lines[500];                       //Hough transform
  hls::HoughLines2<1, 5>(canny_edges1, lines, 300);

  hls::Scalar<3, unsigned char>pixelh;       //pixel to add to hough image
  hls::Scalar<3, unsigned char>pixela;       //pixel to read image
  
    for(pxtype y=0;y<1080;y++)
    {
    for(pxtype x=0;x<1920;x++)
    {
    canny_edges2 >> pixela;
    p=0;
    if(pixela.val[0] != 0)
    {
    for(int i=0;i<500;i++)
    {
//    theta = lines[i].angle;
//    mag = lines[i].rho;
    cordic(lines[i].angle, s, c);
    y0 = (lines[i].rho - x*c) /s;
    d = y-y0;
    if(d == 0)
    p = 1;
    }
    if(p==1)
    {
    pixelh.val[0] = 255;
    }
    else
    {
    pixelh.val[0] = 0;
    }
    }
    else
    {
    pixelh.val[0] = 0;
    }
    canny_edges5 << pixelh;
    //f2<< pixelh;
    }
    } 
   hls::Scalar<3, unsigned char> color(0,0,255);
   hls::PaintMask(f2,canny_edges5,f2,color); 
	// RGB_IMAGE to AXI stream 

    hls::Mat2AXIvideo(f2, output );
}
