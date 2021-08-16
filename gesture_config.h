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

  /* Paramaters for gesture configuration */
  #define FLIP_SENSOR_PIXELS 0
  #define PIXEL_DATA_MODE 0
  #define SAMPLE_PERIOD_MS 19.8F /*Changed from 12.5f for 400um device*/
  #define ADC_FULL_SCALE 16384 /*Changed from 8192 for 400um device*/
  #define BACKGROUND_FILTER_ALPHA 0.05F /*Changed from 0.10f for 400um device*/
  #define LOW_PASS_FILTER_ALPHA 1.0F
  #define ZERO_CLAMP_THRESHOLD 10
  #define ZERO_CLAMP_THRESHOLD_FACTOR 6
  #define ENABLE_WINDOW_FILTER 1
  #define WINDOW_FILTER_ALPHA 0.5F
  #define START_DETECTION_THRESHOLD 150 /*Changed from 400 for 400um device*/
  #define END_DETECTION_THRESHOLD 50 /*Changed from 250 for 400um device*/



  /* Paramaters for tracking configuration */
  #define ENABLE_AUTO_BIAS_CALIBRATION 1
  #define STATIC_STATE_BIAS_MS 2000
  #define TRACKING_LOW_PASS_FILTER_ALPHA 1.0F
  #define TRACKING_ZERO_CLAMP_THRESHOLD 50
  #define TRACKING_ZERO_CLAMP_THRESHOLD_FACTOR 6
  #define TRACKING_START_DETECTION_THRESHOLD 400
  #define TRACKING_END_DETECTION_THRESHOLD 300
  #define STATIC_STATE_BIAS_DELTA_MAX 200
  #define BIAS_FULLSCALE_FACTOR_MAX 2
  #define TRACK_WIDTH 6.0F
  #define TRACK_HEIGHT 3.0F
  #define ENABLE_GAIN_CORRECTION 1
  #define GAIN_FACTOR_0 0.5;
  #define GAIN_FACTOR_1 0.75;
  #define GAIN_FACTOR_2 1.0;
