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

#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include "gesture_common.h"

extern DigitalOut rLED;
extern DigitalOut gLED;
extern DigitalOut csb; // sensor 1
extern DigitalOut sel;

enum ser_modes {SPI_MODE, I2C_MODE};
extern uint32_t serial_mode;

// Serial Interfaces to device
extern I2C i2c;
extern SPI spi;

void i2c_init();
void spi_init();
int reg_read(const uint8_t reg_addr, const uint8_t num_bytes, uint8_t reg_vals[]);
int reg_write(const uint8_t reg_addr, const uint8_t reg_val);
int i2c_read(const uint8_t reg_addr, uint8_t const num_bytes, uint8_t reg_vals[]);
int i2c_write(const uint8_t reg_addr, const uint8_t reg_val);
int spi_read(const uint8_t reg_addr, const uint8_t num_bytes, uint8_t reg_vals[]);
int spi_write(const uint8_t reg_addr, const uint8_t reg_val);
void getSensorPixels(int pixels[], const uint8_t flip_sensor_pixels);
int convertTwoUnsignedBytesToInt(const unsigned char hi_byte, const unsigned char lo_byte);

#endif
