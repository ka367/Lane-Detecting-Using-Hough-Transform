#ifndef _TOP_H_
#define _TOP_H_

#include "hls_video.h"
#include "hls_math.h"

// maximum image size
#define MAX_WIDTH  640
#define MAX_HEIGHT 480

// I/O Image Settings
#define INPUT_IMAGE           "test_1080p.bmp"
#define OUTPUT_IMAGE          "result_1080p.bmp"
#define OUTPUT_IMAGE_GOLDEN   "result_1080p_golden.bmp"

// typedef video library core structures
typedef hls::stream<ap_axiu<34,1,1,1> >               AXI_STREAM;
typedef hls::Scalar<3, unsigned char>                 RGB_PIXEL;
typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC3>     RGB_IMAGE;
typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC1>     RGBSINGLE_IMAGE;
// Same as RGB image, but with 16 bit pixels, since the data is wider for gd
typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_16UC3>     RGB_IMAGE_16;

// custom function blocks for Canny
void gradient_decomposition( RGB_IMAGE& gx, RGB_IMAGE& gy, RGB_IMAGE_16& gd );
void nonmax_suppression(RGB_IMAGE_16& gd, RGB_IMAGE& dst);
void hysteresis(RGB_IMAGE& gd, RGB_IMAGE& dst, int threshold_low, int threshold_high);
template<unsigned int max>
void Hough_plotting(RGBSINGLE_IMAGE &src, RGBSINGLE_IMAGE &dst,hls::Polar_< float, float > (&lines)[max]);
// top level function for HW synthesis
void image_filter(AXI_STREAM & src_axi, AXI_STREAM & dst_axi, int rows, int cols);

#endif

