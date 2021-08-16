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

#include "img_utils.h"

int getMaxPixelValue(const int pixels[], const unsigned int num_pixels)
{
  int maxpixel=-99999;
  for (unsigned int i = 0; i < num_pixels; i++) {
    if (maxpixel < pixels[i]) {
        maxpixel = pixels[i];
    }
  }
  return maxpixel;
}

int getMinPixelValue(const int pixels[], const unsigned int num_pixels)
{
  int minpixel=99999;
  for (unsigned int i = 0; i < num_pixels; i++) {
    if (minpixel > pixels[i]) {
        minpixel = pixels[i];
    }
  }
  return minpixel;
}

// Zero out pixels below threshold value. Returns the number of pixels above the threshold
unsigned int zeroPixelsBelowThreshold(int pixels[], const unsigned int num_pixels, const int threshold)
{
  int pixelsAboveThresholdCount = num_pixels;
  for (unsigned int i = 0; i < num_pixels; i++) {
    if (pixels[i] < threshold) {
      pixels[i] = 0;
      pixelsAboveThresholdCount--;
    }
  }
  return pixelsAboveThresholdCount;
}

// Caller must keep static filtpixels array
void filterLowPassPixels(int pixels[], float filtpixels[], const unsigned int num_pixels, const float alpha)
{
  for (unsigned int i=0; i< num_pixels; i++) {
    filtpixels[i] = (1.0 - alpha) * filtpixels[i] + alpha * pixels[i];
  }
  for (unsigned int i=0; i< num_pixels; i++) {
    pixels[i] = (int)filtpixels[i];
  }
}

// Implement background subtraction by subtracting long exponential smoothing average from a shorter one
// (or simply the current pixel if alpha_short_avg is set to 1.0)
// alpha_long_avg should be smaller than alpha_short_avg
// Caller must keep static shart_avg_pixels[] and long_avg_pixels[]
// The bigger alpha long is, the more aggressive the high pass filter.
void subtractBackground(int pixels[], float short_avg_pixels[], float long_avg_pixels[], const unsigned int num_pixels, const float alpha_short_avg, const float alpha_long_avg)
{
  for (unsigned int i=0; i< num_pixels; i++) {
    long_avg_pixels[i] = (1.0f - alpha_long_avg) * long_avg_pixels[i] + alpha_long_avg * pixels[i];
  }
  for (unsigned int i=0; i< num_pixels; i++) {
    short_avg_pixels[i] = (1.0f - alpha_short_avg) * short_avg_pixels[i] + alpha_short_avg * pixels[i];
  }
  for (unsigned int i=0; i< num_pixels; i++) {
    pixels[i] = short_avg_pixels[i] - (int)long_avg_pixels[i];
  }
}

void calcCenterOfMass(const int pixels[], const unsigned int xres, const unsigned int yres, float *cmx, float *cmy, int *totalmass)
{
  int cmx_numer=0, cmy_numer=0;
  for (unsigned int i = 0; i < xres*yres; i++) {
    cmx_numer += (i%xres)*pixels[i];
    cmy_numer += (i/xres)*pixels[i];
    *totalmass += pixels[i];
  }
  if (*totalmass == 0) {
    *totalmass = 1; // avoid NaN
  }
  *cmx = (float)cmx_numer/(float)(*totalmass);
  *cmy = (float)cmy_numer/(float)(*totalmass);
}

void interpn(const int pixels[], int interp_pixels[], const int w, const int h, const int interpolation_factor)
{
  int w2 = (w - 1) * interpolation_factor + 1;
  int h2 = (h - 1) * interpolation_factor + 1;
  int A, B, C, x, y;
  float x_ratio = 1.0f / (float)interpolation_factor;
  float y_ratio = 1.0f / (float)interpolation_factor;

  // First stretch in x-direction, index through each pixel of destination array. Skip rows in destination array
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w2; j++) {
      x = (int)(x_ratio * j);  // x index of original frame
      int index = i * w + x;  // pixel index of original frame
      if (x == w - 1) // last pixel on right edge of original frame
        interp_pixels[i * w2 * interpolation_factor + j] = pixels[index]; // skip rows in dest array
      else {
        A = pixels[index];
        B = pixels[index + 1];
        float x_diff = (x_ratio * j) - x; // For 2x interpolation, will be 0, 1/2, 0, 1/2...
        interp_pixels[i * w2 * interpolation_factor + j] = (int)(A + (B - A) * x_diff); // skip rows in dest array
      }
    }
  }
  // Then stretch in y-direction, index through each pixel of destination array
  for (int i = 0; i < h2; i++) {
    for (int j = 0; j < w2; j++) {
      y = (int)(y_ratio * i);  // y index of original frame
      int index = y * w2 * interpolation_factor + j;  // pixel index of frame
      if (y == h - 1) //  pixel on bottom of original frame
        interp_pixels[i * w2 + j] = interp_pixels[index];
      else {
        A = interp_pixels[index];
        C = interp_pixels[index + w2 * interpolation_factor];
        float y_diff = (y_ratio * i) - y;
        interp_pixels[i * w2 + j] = (int)(A + (C - A) * y_diff);
      }
    }
  }
}
