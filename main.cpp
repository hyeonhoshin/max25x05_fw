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

#include "mbed.h"
#include "USBSerial.h"

#include "config.h"
#include "gesture_lib.h"
#include "interface.h"
#include "controller.h"
#include "cmd.h"

#if defined(COMPILE_FOR_MAX32630)
  #include "max32630fthr.h"
  MAX32630FTHR fthr(MAX32630FTHR::VIO_3V3);
#elif defined(COMPILE_FOR_MAX32620)
  #include "MAX32620FTHR.h"
  MAX32620FTHR fthr(MAX32620FTHR::VIO_3V3);
#endif

const char *version_string = "gesture firmware framework version 1.0";

// MCU board status LEDs
DigitalOut rLED(LED1);
DigitalOut gLED(LED2);

// Device pins
DigitalOut csb(P5_5); // sensor 1
//DigitalOut csb2(P54); // sensor 2
DigitalOut sel(P3_2);

InterruptIn intb(P5_3); // sensor 1
//InterruptIn intb2(P3_3); // sensor 2

#if USE_SPI
  SPI spi(P5_1, P5_2, P5_0);
#else
  I2C i2c(P3_4, P3_5);
#endif

uint32_t i2c_addr_select = 0;

uint32_t serial_mode = SPI_MODE;

static uint32_t read_sensor_frames_enabled = 0;
static uint32_t data_stream_enabled = 0;

static uint32_t send_pixel_data_with_stream = 1;

// Data ready flags
static uint32_t sensorDataReadyFlag = 0; // Data ready flag, set by the end-of-conversion interrupt

// Declare functions called in main
static void processFrame(int pixels[]);
void set_default_register_settings();

int main()
{
  static int pixels[NUM_SENSOR_PIXELS];
  gLED = LED_OFF;
  rLED = LED_ON;

  #if USE_SPI
    spi.frequency(6e6);
    serial_mode = SPI_MODE;
    spi_init();
  #else
    i2c.frequency(400e3);
    serial_mode = I2C_MODE;
    i2c_init();
  #endif

  // Write register settings to device
  set_default_register_settings();

  // Set FTHR board status LEDs
  gLED = LED_ON;
  rLED = LED_OFF;

  // Configure gesture library
  // Declare a configuration object
  GestureConfig gestCfg;
  // First get a GestureConfig struct that is populated with default values
  initConfigStructToDefaults(&gestCfg);
  // Then pass this struct to the configGesture function to initialize the gesture library
  configGesture(&gestCfg);
  //configGesture(NULL); // If configGesture is called with NULL, then default parameters will be used.

  // Enable reading of sensor frames
  enable_read_sensor_frames();

  while (1) {
    // Check if a command was received over the serial interface
    checkUserCmd();

    // If using INTB interrupt, the sensorDataReadyFlag will be set when the end-of-conversion occurs
    if (sensorDataReadyFlag) {
        getSensorPixels(pixels, getGestureConfigPtr()->flip_sensor_pixels);
        processFrame(pixels);
        sensorDataReadyFlag = 0;
    }
  }
}

/*
* This function calls the gesture library for a single sensor frame
* It builds a data stream frame from the gesture results
* And sends the stream over the serial connection
*/
GestureResult gesResult;
void processFrame(int pixels[])
{
  runGesture(pixels, &gesResult);
  // For raw pixels, instead uncomment out the following line
  //memset(&gesResult, 0, sizeof(GestureResult));

  if (data_stream_enabled) {

    uint8_t frm_data[NUM_SENSOR_PIXELS*2+NUM_INFO_BYTES];

    // The frame data is per the serial API used by the gesture EVKit GUI
    // SYNC bits are used by receiver to know the start of frame
    frm_data[0] = 255;  // SYNC
    frm_data[1] = 255;  // SYNC2
    frm_data[3] = gesResult.state;
    frm_data[4] = gesResult.n_sample;
    frm_data[6] = (gesResult.maxpixel>>8) & 0xFF;  // maxpixel high byte
    frm_data[7] = gesResult.maxpixel & 0xFF;      // maxpixel low byte
    float x = gesResult.x;
    memcpy(frm_data+10, &x, 4);
    float y = gesResult.y;
    memcpy(frm_data+15, &y, 4);

    for (uint32_t i=0; i<NUM_SENSOR_PIXELS; i++) {
      frm_data[2*i + NUM_INFO_BYTES] = (pixels[i]>>8) & 0xFF;
      frm_data[2*i+1 + NUM_INFO_BYTES] = pixels[i] & 0xFF;
    }

    if (send_pixel_data_with_stream)
      sendDataStream(frm_data, NUM_SENSOR_PIXELS*2 + NUM_INFO_BYTES);
    else
      sendDataStream(frm_data, NUM_INFO_BYTES);
  }
}


void set_default_register_settings()
{
  uint32_t ledDrvLevel = 0x0F;
  reg_write(0x01, 0x04);
  reg_write(0x02, 0x02);
  #if defined(MAX25405_DEVICE)
  reg_write(0x03, 0x24); // SDLY=2, TIM=2
  reg_write(0x04, 0x8C); // NRPT=4, NCDS=3
  #elif defined(MAX25205_DEVICE)
  reg_write(0x03, 0x04); // SDLY=0, TIM=2
  reg_write(0x04, 0xAC); // NRPT=5, NCDS=3
  ledDrvLevel = 0x0A;
  #endif
  reg_write(0x05, 0x08);
  reg_write(0x06, ledDrvLevel); // LED power
  reg_write(0xa5, 0x88);
  reg_write(0xa6, 0x88);
  reg_write(0xa7, 0x88);
  reg_write(0xa8, 0x88);
  reg_write(0xa9, 0x88);
  reg_write(0xc1, 0x0A); // PWM LED driver
}

/*
* This is the interrupt handler to handle end-of-conversion interrupts on the INTB pin
*/
void intb_handler()
{
  sensorDataReadyFlag = 1;
}

/*
* This function starts the monitoring of the INTB interrupt
*/
void enable_read_sensor_frames()
{
  resetGesture();

  intb.fall(&intb_handler); // Add INTB interrupt handler

  // Read status reg to clear interrupt
  uint8_t status_reg;
  reg_read(0x00, 1, &status_reg);

  // Turn off the status LEDs on the MCU board
  gLED = LED_OFF;
  rLED = LED_OFF;
  read_sensor_frames_enabled = 1;
}

/*
* This function stops the monitoring of the INTB interrupt or polling
*/
void disable_read_sensor_frames()
{
  intb.fall(0); // Remove interrupt handler

  // Turn on the status LEDs on the MCU board
  gLED = LED_ON;
  rLED = LED_OFF;
  read_sensor_frames_enabled = 0;
}

/*
* These functions set the data reporting mode and starts sending of data
*/
void set_stream_on(uint32_t send_pixel_data)
{
  send_pixel_data_with_stream = send_pixel_data;
  data_stream_enabled = 1;
  if (read_sensor_frames_enabled == 0)
    enable_read_sensor_frames(); // make sure
}

void set_stream_off()
{
  data_stream_enabled = 0;
}
