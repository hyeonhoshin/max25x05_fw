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

#include "gesture_common.h"

static uint32_t reset_flag = TRUE;
static uint32_t reset_bias_flag = TRUE;

// Calculated from parameters
static uint32_t sampleT;
static uint32_t adc_full_scale;
static uint32_t static_state_bias_n;

static uint32_t force_calibration_flag = FALSE;

typedef enum { INACTIVE_STATE, TRACKING_STATE } TrackingState;

void configTracking(const uint32_t _sampleT, const uint32_t _adc_full_scale, const TrackingConfig *cfg)
{
  sampleT = _sampleT;
  adc_full_scale = _adc_full_scale;
  static_state_bias_n = cfg->static_state_bias_ms/sampleT;
  resetTracking();

  // Reset calibration only if sample period or full-scale changed, so calibration is not cleared.
  static uint32_t last_sampleT = 0;
  static uint32_t last_adc_full_scale = 0;
  if ((sampleT != last_sampleT) || (adc_full_scale != last_adc_full_scale)) {
    clearTrackingCalibration();
  }
  last_sampleT = sampleT;
  last_adc_full_scale = adc_full_scale;
}

// Force a single bias configuration
void forceTrackingCalibration()
{
  force_calibration_flag = TRUE;
}

void clearTrackingCalibration()
{
  reset_bias_flag = TRUE;
}

// Reset the state machine and clear the filter
void resetTracking()
{
  reset_flag = TRUE;
}

void runTracking(const TrackingConfig *cfg, int pixels[], TrackingResult *gesResult)
{
  static TrackingState state = INACTIVE_STATE;
  static uint32_t calibration_done = FALSE;
  static uint32_t static_state_bias_count = 0;

  static uint32_t reset_filter_flag = TRUE;
  static uint32_t reset_linger_flag = TRUE;

  memset(gesResult, 0, sizeof(TrackingResult));

  // A reset will reset the calibration, so filters and static state counters must also be reset once a calibration is performed
  if (reset_flag) {
    state = INACTIVE_STATE;
    static_state_bias_count = 0; // Reset the bias calibration counter but don't clear the bias cal.
    reset_filter_flag = TRUE;
    reset_flag = FALSE;
  }

  // -----------------------------------------
  // Bias Compenstation
  // -----------------------------------------
  {
    static int biaspixels[NUM_SENSOR_PIXELS];
    static float refpixels[NUM_SENSOR_PIXELS];

    int max_raw_pixel=getMaxPixelValue(pixels, NUM_SENSOR_PIXELS);
    int min_raw_pixel=getMinPixelValue(pixels, NUM_SENSOR_PIXELS);

    if (reset_bias_flag) {
      for(uint32_t i=0; i<NUM_SENSOR_PIXELS; i++) {
        biaspixels[i] = 0;         // clear the bias compensation
        refpixels[i] = pixels[i];  // reset reference pixels
      }
      static_state_bias_count = 0;
      reset_bias_flag = FALSE;
      calibration_done = FALSE;
    }

    if (cfg->enable_auto_bias_calibration) {
      // Compare current pixel to reference
      int maxdelta = -99999;
      for (uint32_t i = 0; i < NUM_SENSOR_PIXELS; i++) {
        int delta = abs(pixels[i]  - refpixels[i]);
        if (maxdelta < delta) {
          maxdelta = delta;
        }
      }
      // Check for static condition
      if (maxdelta < (int)cfg->static_state_bias_delta_max
        && max_raw_pixel-min_raw_pixel < (int)adc_full_scale/(int)cfg->bias_fullscale_factor_max
        && state != TRACKING_STATE)
      {
        static_state_bias_count++;
      }
      else {
        // Sensor not static, reset the counter and set new reference
        static_state_bias_count = 0;
        for(uint32_t i=0; i<NUM_SENSOR_PIXELS; i++) {
          refpixels[i] = pixels[i];
        }
      }
      // If static condition, recalculate bias compenstation
      if (static_state_bias_count > static_state_bias_n) {
        for (uint32_t i = 0; i < NUM_SENSOR_PIXELS; i++) {
          biaspixels[i] = refpixels[i];
        }
        static_state_bias_count = 0;
        calibration_done = TRUE;
      }
    }
    // If force_calibration_flag is set, set bias to current pixel values
    if (force_calibration_flag) {
      for (uint32_t i = 0; i < NUM_SENSOR_PIXELS; i++) {
        biaspixels[i] = pixels[i];
      }
      calibration_done = TRUE;
      force_calibration_flag = FALSE;
    }
    // Apply bias compensation
    for (uint32_t i = 0; i < NUM_SENSOR_PIXELS; i++) {
      pixels[i] -= biaspixels[i];
    }
  }

  // -----------------------------------------
  // Low pass filter
  // -----------------------------------------
  if (calibration_done) {
    static float filtpixels[NUM_SENSOR_PIXELS];
    if (reset_filter_flag) {
      for(uint32_t i=0; i<NUM_SENSOR_PIXELS; i++)
      {
        filtpixels[i] = pixels[i]; // Clear the filter
      }
      reset_filter_flag = FALSE;
    }
    filterLowPassPixels(pixels, filtpixels, NUM_SENSOR_PIXELS, cfg->low_pass_filter_alpha);
  }

  // -----------------------------------------
  // Software gain
  // -----------------------------------------
  {
    double gain_factor = 0.0f;
    for (uint32_t i = 0; i < NUM_SENSOR_PIXELS; i++) {
      if (i%SENSOR_XRES == 0 || i%SENSOR_XRES == 9 || i%SENSOR_XRES == 1 || i%SENSOR_XRES == 8 || i/SENSOR_XRES == 0 || i/SENSOR_XRES == 5)
        gain_factor = cfg->gain_factor_2;
      else if (i%SENSOR_XRES == 2 || i%SENSOR_XRES == 7 || i/SENSOR_XRES == 1 || i/SENSOR_XRES == 4)
        gain_factor = cfg->gain_factor_1;
      else if (i%SENSOR_XRES == 3 || i%SENSOR_XRES == 6 || i%SENSOR_XRES == 4 || i%SENSOR_XRES == 5)
        gain_factor = cfg->gain_factor_0;

      pixels[i] *= gain_factor;
    }
  }

  // -----------------------------------------
  // Determine tracking state
  // -----------------------------------------

  // Get max/min pixel after applying bias compensation and filtering
  int maxpixel=getMaxPixelValue(pixels, NUM_SENSOR_PIXELS);

  // Determine state
  if (calibration_done
    && ((state == INACTIVE_STATE
    && maxpixel > cfg->start_detection_threshold) || (state == TRACKING_STATE
    && maxpixel > cfg->end_detection_threshold)))
  {
    state = TRACKING_STATE;
  }
  else {
    state = INACTIVE_STATE;
  }

  // -----------------------------------------
  // Object position
  // -----------------------------------------
  float x_scaled = -1.0, y_scaled = -1.0;
  if (state == TRACKING_STATE) {
    #if INTERP_FACTOR == 1
      int *interp_pixels;
      interp_pixels = pixels;
    #else
      int interp_pixels[NUM_INTERP_PIXELS];
      interpn(pixels, interp_pixels, SENSOR_XRES, SENSOR_YRES, INTERP_FACTOR);
    #endif

    // Zero out low pixels to reduce artifacts and noise
    zeroPixelsBelowThreshold(interp_pixels,NUM_INTERP_PIXELS,(int)(maxpixel/cfg->zero_clamp_threshold_factor)); // zero out pixels below some percent of peak
    zeroPixelsBelowThreshold(interp_pixels,NUM_INTERP_PIXELS,cfg->zero_clamp_threshold);

    // Find center of mass.
    float cmx,cmy;
    if (maxpixel > 0) {
      int totalmass=0;
      calcCenterOfMass(interp_pixels, INTERP_XRES, INTERP_YRES, &cmx, &cmy, &totalmass); // only calculate COM if there is a pixel above the noise (avoid divide-by-zero)
      cmx = cmx/(float)INTERP_FACTOR;
      cmy = cmy/(float)INTERP_FACTOR;
    }

    // Scale position according to tracking width/height parameter. This scales to values (0,9) in x, (0,5) in y
    x_scaled = (cmx - ((float)SENSOR_XRES - cfg->track_width)/2.0f) * (SENSOR_XRES-1)/(cfg->track_width-1);
    x_scaled = x_scaled >= (SENSOR_XRES-1) ? SENSOR_XRES - 1.001 : x_scaled < 0.0f ? 0.0f : x_scaled;
    y_scaled = (cmy - ((float)SENSOR_YRES - cfg->track_height)/2.0f) * (SENSOR_YRES-1)/(cfg->track_height-1);
    y_scaled = y_scaled >= (SENSOR_YRES-1) ? SENSOR_YRES - 1.001 : y_scaled < 0.0f ? 0.0f : y_scaled;
  }

  // Update gesture result struct
  gesResult->state = state == TRACKING_STATE ? 1 : 0;
  gesResult->x = state == TRACKING_STATE ? x_scaled : -1.00f;
  gesResult->y = state == TRACKING_STATE ? y_scaled : -1.00f;
  gesResult->maxpixel = maxpixel;
}
