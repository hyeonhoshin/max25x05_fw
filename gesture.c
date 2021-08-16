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

// Gesture states
typedef enum {STATE_INACTIVE, GESTURE_IN_PROGRESS} GestureState;

static uint32_t reset_flag = TRUE;

// Create a static instance of the configuration to maintain current config parameters
static GestureConfig gestCfg;

static void runDynamicGesture(const GestureConfig *cfg, int _pixels[], DynamicGestureResult *gesResult);
void noiseWindow3Filter(int pixels[], const float alpha, const uint32_t reset_flag);

void resetGesture()
{
  reset_flag = TRUE;
  // Reset submodules
  resetTracking();
}

// Get a copy of the config struct
void getGestureConfig(GestureConfig *_cfg)
{
  *_cfg = gestCfg;
}

// Get a pointer to the config struct
GestureConfig * getGestureConfigPtr()
{
  return &gestCfg;
}

// Copy the user's config struct to the local struct, and initialize calculated values
void configGesture(const GestureConfig *_cfg)
{
  if (!_cfg) {
    initConfigStructToDefaults(&gestCfg); // Use default configuration if pointer is NULL
  }
  else {
    gestCfg = *_cfg; // make a copy of the struct
  }

  configTracking(gestCfg.sample_period_ms, gestCfg.adc_full_scale, &gestCfg.trackingConfig);

  resetGesture();
}

void runGesture(int pixels[], GestureResult *gesResult)
{
  // Initialize result structure
  memset(gesResult, 0, sizeof(GestureResult));
  gesResult->state = STATE_INACTIVE;
  gesResult->gesture = GEST_NONE;

  // Noise filter
  if (gestCfg.enable_window_filter) {
    noiseWindow3Filter(pixels, gestCfg.window_filter_alpha, reset_flag);
  }

  // Process pixels for dynamic gesture
  if (1) {
    DynamicGestureResult dynamicResult;
    runDynamicGesture(&gestCfg, pixels, &dynamicResult);
    gesResult->state = dynamicResult.state;
    gesResult->n_sample = dynamicResult.n_sample;
    gesResult->maxpixel = dynamicResult.maxpixel;
    gesResult->x = dynamicResult.x;
    gesResult->y = dynamicResult.y;
  }
  // Process pixels for tracking
  if (0) {
    TrackingResult trackResult;
    runTracking(&gestCfg.trackingConfig, pixels, &trackResult);
    gesResult->state = trackResult.state;
    gesResult->maxpixel = trackResult.maxpixel; // Will override dynamic result if any
    gesResult->x = trackResult.x; // Will override dynamic result if any
    gesResult->y = trackResult.y; // Will override dynamic result if any
  }
}

static void runDynamicGesture(const GestureConfig *cfg, int pixels[], DynamicGestureResult *gesResult)
{
  memset(gesResult, 0, sizeof(DynamicGestureResult));

  static GestureState state = STATE_INACTIVE;
  GestureEvent gest_event = GEST_NONE;
  static uint32_t n_sample = 0, n_frame =0;

  if (reset_flag) {
    state = STATE_INACTIVE;
  }

  int rawmaxpixel = getMaxPixelValue(pixels, NUM_SENSOR_PIXELS);

  // Static background subtraction
  {
    static float foreground_pixels[NUM_SENSOR_PIXELS], background_pixels[NUM_SENSOR_PIXELS];
    if (reset_flag) {
      for(uint32_t i=0; i<NUM_SENSOR_PIXELS; i++) {
        foreground_pixels[i] = pixels[i]; // clear the filter
        background_pixels[i] = pixels[i]; // clear the filter
      }
    }

    float background_alpha = cfg->background_filter_alpha;

    subtractBackground(pixels, foreground_pixels, background_pixels, NUM_SENSOR_PIXELS, cfg->low_pass_filter_alpha, background_alpha);
  }

  // Clear the reset flag. All reset activity should be done by now
  if (reset_flag) {
    reset_flag = FALSE;
  }

  // Find post-filter max pixel
  int maxpixel=getMaxPixelValue(pixels, NUM_SENSOR_PIXELS);

  float cmx,cmy;
  {
    #if INTERP_FACTOR == 1
      int *interp_pixels;
      interp_pixels = pixels;
    #else
      static int interp_pixels[NUM_INTERP_PIXELS];
      interpn(pixels, interp_pixels, SENSOR_XRES, SENSOR_YRES, INTERP_FACTOR);
    #endif

    // Thresholding
    zeroPixelsBelowThreshold(interp_pixels,NUM_INTERP_PIXELS,(int)(maxpixel/cfg->zero_clamp_threshold_factor)); // zero out pixels below some percent of peak
    zeroPixelsBelowThreshold(interp_pixels,NUM_INTERP_PIXELS,cfg->zero_clamp_threshold); // and also zero out below fixed threshold

    // Center of mass
    int totalmass=0;
    if (maxpixel >= cfg->end_detection_threshold) {
      calcCenterOfMass(interp_pixels, INTERP_XRES, INTERP_YRES, &cmx, &cmy, &totalmass); // Only calculate COM if there is a pixel above the noise (avoid divide-by-zero)
      cmx = cmx/INTERP_FACTOR;
      cmy = cmy/INTERP_FACTOR * DY_PIXEL_SCALE; // scale y so it has same unit dimension as x
    }
  }

  n_frame++;

  gesResult->n_sample = n_sample;
  gesResult->maxpixel = maxpixel; //rawmaxpixel;
  gesResult->state = maxpixel >= cfg->end_detection_threshold ? GESTURE_IN_PROGRESS : STATE_INACTIVE;
  gesResult->x = maxpixel >= cfg->end_detection_threshold ? cmx : -1.00;
  gesResult->y = maxpixel >= cfg->end_detection_threshold ? cmy : -1.00;
}

void noiseWindow3Filter(int pixels[], const float alpha, const uint32_t reset_flag)
{
  static int nwin[3][NUM_SENSOR_PIXELS];
  if (reset_flag) {
    for(uint32_t i=0; i<NUM_SENSOR_PIXELS; i++) {
      nwin[0][i] = pixels[i]; // clear the filter
      nwin[1][i] = pixels[i]; // clear the filter
      nwin[2][i] = pixels[i]; // clear the filter
    }
  }
  else {
    for (uint32_t i=0; i<NUM_SENSOR_PIXELS; i++) {
      nwin[0][i] = nwin[1][i];
      nwin[1][i] = nwin[2][i];
      nwin[2][i] = pixels[i];
      pixels[i] = alpha * nwin[1][i] + (1-alpha)*(nwin[0][i] + nwin[2][i])/2;
    }
  }
}
