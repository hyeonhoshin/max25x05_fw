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
#include "controller.h"
#include "config.h"

static uint32_t i2c_device_addr = 0x9E; // LSB justified

#define I2C_ADDR_SELECT 0

void i2c_init()
{
  if (I2C_ADDR_SELECT) {
    csb = 0; // set low for default i2c address
    i2c_device_addr = 0x9E;
  }
  else {
    csb = 1;
    i2c_device_addr = 0xA0;
  }
  sel = 1; // set high for i2c
}

void spi_init()
{
  sel = 0; // set low for SPI
}

int reg_read(const uint8_t reg_addr, const uint8_t num_bytes, uint8_t reg_vals[])
{
  if (serial_mode == SPI_MODE) {
    spi_read(reg_addr, num_bytes, reg_vals);
  }
  else {
    i2c_read(reg_addr, num_bytes, reg_vals);
  }
  return 0;
}

int reg_write(const uint8_t reg_addr, const uint8_t reg_val)
{
  int result = (serial_mode == SPI_MODE ? spi_write(reg_addr, reg_val) : i2c_write(reg_addr, reg_val));
  return result;
}

int i2c_read(const uint8_t reg_addr, const uint8_t num_bytes, uint8_t reg_vals[])
{
  #if !USE_SPI
  char read_addr = (char)reg_addr;
  i2c.write(i2c_device_addr, &read_addr, 1);
  i2c.read(i2c_device_addr, (char*)reg_vals, (int)num_bytes);
  #endif
  return 0;
}

int i2c_write(const uint8_t reg_addr, const uint8_t reg_val)
{
  #if !USE_SPI
  char data[2];
  data[0] = reg_addr;
  data[1] = reg_val;
  i2c.write(i2c_device_addr, data, 2);
  #endif
  return 0;
}

int spi_read(const uint8_t reg_addr, const uint8_t num_bytes, uint8_t reg_vals[])
{
  #if USE_SPI
  csb = 0;
  spi.write(reg_addr);                // byte 1: register address
  spi.write(0x80);                    // byte 2: read command 0x80
  for(int i=0; i<num_bytes; i++) {
    reg_vals[i] = spi.write(0x00);  // byte3: read byte
  }
  csb = 1;
  #endif
  return 0;
}

int spi_write(const uint8_t reg_addr, const uint8_t reg_val)
{
  #if USE_SPI
  csb = 0;
  spi.write(reg_addr);    // byte1: register address
  spi.write(0x00);        // byte2: write command 0x00
  spi.write(reg_val);     // byte3: write byte
  csb = 1;
  #endif
  return 0;
}

void getSensorPixels(int pixels[], const uint8_t flip_sensor_pixels)
{
  unsigned char reg_vals[NUM_SENSOR_PIXELS*2];
  reg_read(0x10, NUM_SENSOR_PIXELS*2, reg_vals);

  for (int i = 0; i < NUM_SENSOR_PIXELS; i++) {
    pixels[i] = convertTwoUnsignedBytesToInt(reg_vals[2 * i], reg_vals[2 * i + 1]);
  }

  if (flip_sensor_pixels) {
    for (int i = 0; i < NUM_SENSOR_PIXELS/2; i++) {
      int temp = pixels[i];
      pixels[i] = pixels[NUM_SENSOR_PIXELS-1-i];
      pixels[NUM_SENSOR_PIXELS-1-i] = temp;
    }
  }
}

int convertTwoUnsignedBytesToInt(uint8_t hi_byte, uint8_t lo_byte)
{
  int intval = (int)(hi_byte << 8 | lo_byte);
  // Convert unsigned to 2's complement
  if ((1 << 15) < intval)
    intval -= (1 << 16);
  return intval;
}
