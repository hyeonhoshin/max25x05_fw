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

#ifndef GESTURE_LIB_H_INCLUDED
#define GESTURE_LIB_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
* Enumerate gesture events here
*/
typedef enum {
	GEST_NONE,
	GEST_PLACEHOLDER
} GestureEvent;

/*
* Structure to store gesture results.
*/
typedef struct {
	GestureEvent gesture;        // Gesture event reported for this processed frame
	uint32_t state;              // 0: inactive; 1: object detected; 2: rotation in progress
	uint32_t n_sample;           // The current sample number of the gesture in progress
	int maxpixel;                // Maximum pixel value for this frame
	float x;                     // Object x-position. Only accurate if TRACKING_ENABLE mask is set.
	float y;                     // Object y-position. Only accurate if TRACKING_ENABLE mask is set.
	float float_placeholder;
	uint32_t int_placeholder;

} GestureResult;


/*
* Structure for configuration parameters that apply to tracking mode, region selection, and linger-to-click.
* An instance of this structure is a member of the GestureConfig stucture.
*/
typedef struct {
	uint32_t enable_auto_bias_calibration;  // Enable automatic bias calibration
	uint32_t static_state_bias_ms;          // How many milliseconds of static condition before performing an automoatic bias calibration
	float low_pass_filter_alpha;            // Smoothing factor for low pass filter. The lower the value, the more low pass filtering
	int zero_clamp_threshold;               // Any pixel below this value is clamped to zero. Must be less than detection thresholds
	float zero_clamp_threshold_factor;   // Any pixel below maxpixel divided by this factor is clamped to zero. This is to reduce optical clutter and background noise
	int start_detection_threshold;           // Pixel activation (bias corrected) to start gesture tracking
	int end_detection_threshold;      // Pixel activation (bias corrected) to stop gesture tracking
	uint32_t static_state_bias_delta_max;   // Max current pixel to reference delta to be considered static state for bias calibration
	uint32_t bias_fullscale_factor_max;     // Only calibrate if raw pixel range below this factor of ADC FS
	float track_width;                      // Width (in pixels) of sensor array over which to scale tracking cursor (thereby ignoring edge pixels)
	float track_height;                     // Height (in pixels) of sensor array over which to scale tracking cursor (thereby ignoring edge pixels)
	uint32_t enable_gain_correction;        // Enable pixel gain correction
	float gain_factor_0; 										// Inner gain factor
	float gain_factor_1; 										// Mid gain factor
	float gain_factor_2;										// Outer gain factor
} TrackingConfig;

/*
* Struture for gesture configuration parameters
* An instance of this structure can be modified by the application and passed to the configGesture function
*/
typedef struct {
	uint32_t flip_sensor_pixels;              // Set true to if device is mounted upside-down
	uint32_t pixel_data_mode;									// Set to 0 for raw data, 1 for bias-compensated pixels, 2 for background subtracted (gesture) pixels
	float sample_period_ms;                   // Sample period of sensor in milliseconds
	uint32_t adc_full_scale;                  // ADC full scale of sensor for current register configuration
	float background_filter_alpha;            // Smoothing factor for dynamic background cancellation. Larger value results in more aggressive high pass filtering
	float low_pass_filter_alpha;              // Smoothing factor for low pass filter. Set to 1 for no filtering
	int zero_clamp_threshold;                 // Any pixel below this value (background corrected) is clamped to zero. Must be less than detection thresholds
	float zero_clamp_threshold_factor;     		// Any pixel below maxpixel/x is clamped to zero. This is to reduce optical clutter and background noise
	uint32_t enable_window_filter;
	float window_filter_alpha;
	int start_detection_threshold;            // Pixel activation level (background corrected) to start gesture tracking
	int end_detection_threshold;              // Pixel threshold (background corrected) to end gesture tracking
	TrackingConfig trackingConfig;
} GestureConfig;


/**
* This function obtains a copy of the GestureConfig structure.
* Obtain a copy in order to modify parameters and then pass the struct back using configGesture.
*
* Parameters
* cfg: A pointer to a GestureConfig structure
*
* Return Value
* None
*/
void getGestureConfig(GestureConfig *_cfg);

/**
* This function obtains a pointer to the GestureConfig structure.
* Obtain a pointer to use the VALUE of specific parameter. Parameters should never be SET directly using the pointer
* to the Config struct. Instead use getGestureConfig() above.
*
* Parameters
* none
*
* Return Value
* cfg: A pointer to the GestureConfig structure contained in the gesture library
*/

GestureConfig * getGestureConfigPtr();


/**
* This function initializes the gesture algorithm with the parameters defined in the GestureConfig structure.
*
* Parameters
* cfg: A pointer to the GestureConfig structure. If the pointer is NULL, the algorithm will be configured with default values.
*
* Return Value
* None
*/
void configGesture(const GestureConfig *cfg);


/**
* This function initializes an instance of a GestureConfig structure to the default values.
* After calling this function, the structure instance can then be modified selectively by the application
* and passed into the configGesture function to implement a custom configuration.
*
* Parameters
* cfg: A pointer to the GestureConfig structure
*
* Return Value
* None
*/
void initConfigStructToDefaults(GestureConfig *cfg);


/**
* This function executes the algorithm for a single frame. This function should be called for every sample period of the sensor.
*
* Parameters
* pixels:    An integer array of sensor pixel data; length is determined by the sensor resolution defined in gesture_common.h
* gesResult: A pointer to a GestureResult struct instance; this instance will be populated with results by the algorithm.
*
* Return Value
* None
*/
void runGesture(int pixels[], GestureResult *gesResult);


/**
* This function resets the gesture algorithm by resetting the state machine and clearing filters.
* Note that the initialization parameters are not changed
*
* Parameters
* None
*
* Return Value
* None
*/
void resetGesture();


/**
* This function forces a tracking mode calibration.
* Note that the calibration only applies to tracking mode (region selection and linger to click); it is not used
* for dynamic gesture mode.
*
* Parameters
* None
*
* Return Value
* None
*/
void forceTrackingCalibration();

#ifdef __cplusplus
} // extern "C"
#endif

#endif
