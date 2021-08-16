/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
*
* This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

#ifndef GESTURE_COMMON_H_INCLUDED
#define GESTURE_COMMON_H_INCLUDED

#include "img_utils.h"
#include "gesture_lib.h"

//#define USE_MBED

#ifdef USE_MBED
	#include "mbed.h"
#else
	#include <math.h>
	#include <string.h>
	#include <stdlib.h>
	#define TRUE 1
	#define FALSE 0
#endif

// Sensor constants. Declared as constants so arrays can be statically sized
#define SENSOR_XRES 10
#define SENSOR_YRES 6
#define NUM_SENSOR_PIXELS (SENSOR_XRES * SENSOR_YRES)

// Interpolation used in gesture algorithm
#define INTERP_FACTOR 4
#define INTERP_XRES ((SENSOR_XRES-1)*INTERP_FACTOR+1)
#define INTERP_YRES ((SENSOR_YRES-1)*INTERP_FACTOR+1)
#define NUM_INTERP_PIXELS (INTERP_XRES*INTERP_YRES)

#define DY_PIXEL_SCALE  1.42857f /*10.0f/7.0f*/


// Error condition constants
static const float MAX_ABS_ACCUM_DX = SENSOR_XRES*2; 	// Total x motion should not exceed 2x the screen size
static const float MAX_ABS_ACCUM_DY = SENSOR_YRES*2 * DY_PIXEL_SCALE; 	// Total x motion should not exceed 2x the screen size

// Structure to store tracking results
typedef struct {
	uint32_t state;             // 0: inactive; 1: object detected
	int maxpixel;               // Maximum pixel value for this frame
	float x;                    // Object x-position
	float y;                    // Object y-position
} TrackingResult;

// Functions in tracking.cpp
void configTracking(const uint32_t _sampleT, const uint32_t _adc_full_scale, const TrackingConfig *cfg);
void runTracking(const TrackingConfig *cfg, int in_pixels[], TrackingResult *gesResult);
void resetTracking();
void clearTrackingCalibration();

// Structure to store dynamic gesture results
typedef struct {
	uint32_t state;             // 0: inactive; 1: object detected; 2: rotation in progress
	uint32_t n_sample;          // The current sample number of this gesture
	int maxpixel;               // Maximum pixel value for this frame
	float x;                    // Object x-position
	float y;                    // Object y-position
} DynamicGestureResult;

#endif
