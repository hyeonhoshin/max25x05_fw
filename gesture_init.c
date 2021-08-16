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

#include "gesture_lib.h"
#include "config.h"
#include "gesture_config.h"

// Initialize configuration structs to default values

void initTrackingConfigStructToDefaults(TrackingConfig *cfg)
{
  cfg->enable_auto_bias_calibration = ENABLE_AUTO_BIAS_CALIBRATION;
  cfg->static_state_bias_ms = STATIC_STATE_BIAS_MS;
  cfg->low_pass_filter_alpha = TRACKING_LOW_PASS_FILTER_ALPHA;
  cfg->zero_clamp_threshold = TRACKING_ZERO_CLAMP_THRESHOLD;
  cfg->zero_clamp_threshold_factor = TRACKING_ZERO_CLAMP_THRESHOLD_FACTOR;
  cfg->start_detection_threshold = TRACKING_START_DETECTION_THRESHOLD;
  cfg->end_detection_threshold = TRACKING_END_DETECTION_THRESHOLD;
  cfg->static_state_bias_delta_max = STATIC_STATE_BIAS_DELTA_MAX;
  cfg->bias_fullscale_factor_max = BIAS_FULLSCALE_FACTOR_MAX;
  cfg->track_width = TRACK_WIDTH;
  cfg->track_height = TRACK_HEIGHT;
  cfg->enable_gain_correction = ENABLE_GAIN_CORRECTION;
  cfg->gain_factor_0 = GAIN_FACTOR_0;
  cfg->gain_factor_1 = GAIN_FACTOR_1;
  cfg->gain_factor_2 = GAIN_FACTOR_2;
}

void initConfigStructToDefaults(GestureConfig *cfg)
{
  cfg->flip_sensor_pixels = FLIP_SENSOR_PIXELS;
  cfg->pixel_data_mode = PIXEL_DATA_MODE;
  cfg->sample_period_ms = SAMPLE_PERIOD_MS;
  cfg->adc_full_scale = ADC_FULL_SCALE;
  cfg->background_filter_alpha = BACKGROUND_FILTER_ALPHA;
  cfg->low_pass_filter_alpha = LOW_PASS_FILTER_ALPHA;
  cfg->zero_clamp_threshold = ZERO_CLAMP_THRESHOLD;
  cfg->zero_clamp_threshold_factor = ZERO_CLAMP_THRESHOLD_FACTOR;
  cfg->enable_window_filter = ENABLE_WINDOW_FILTER;
  cfg->window_filter_alpha = WINDOW_FILTER_ALPHA;
  cfg->start_detection_threshold = START_DETECTION_THRESHOLD;
  cfg->end_detection_threshold = END_DETECTION_THRESHOLD;

  // Initialize tracking config strucutre
  initTrackingConfigStructToDefaults(&cfg->trackingConfig);
}
