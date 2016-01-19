#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#include <Windows.h>
#include <tchar.h>

/*
  filter:
	FILTER_BOX		  = 0,	//! Box, pulse, Fourier window, 1st order (constant) b-spline
	FILTER_BICUBIC	  = 1,	//! Mitchell & Netravali's two-param cubic filter
	FILTER_BILINEAR   = 2,	//! Bilinear filter
	FILTER_BSPLINE	  = 3,	//! 4th order (cubic) b-spline
	FILTER_CATMULLROM = 4,	//! Catmull-Rom spline, Overhauser spline, default
	FILTER_LANCZOS3	  = 5	//! Lanczos3 filter
*/

void image_debug_enable(int enable);

int image_size_get(LPCTSTR file, int& width, int& height);

int image_clipping(LPCTSTR dest, LPCTSTR src, int left, int top, int right, int bottom, int width, int height, int filter);



#endif //__IMAGE_PROCESS_H__