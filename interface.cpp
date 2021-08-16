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

#include "interface.h"
#include "USBSerial.h"
#include "controller.h"
#include "config.h"

// Serial port for full interface with streaming data. Select USB or UART with USE_UART_INTERFACE constant.
// Files that use these objects should declare them as externs.
#if USE_UART_INTERFACE
Serial *serial = new Serial(P3_1,P3_0,UART_BAUD_RATE);
#else
USBSerial *serial = new USBSerial(0x0b6a, 0x4360, 0x0001, false);
#endif

// UART over daplink, 9600 baud. This can be used for debugging
//Serial daplink(P2_1, P2_0);

// USBSerial putc and printf are slow. Use writeBlock to stream data over USB
void sendDataStream(uint8_t *str, const unsigned int num_chars)
{
#if USE_UART_INTERFACE

  for (int i=0; i<num_chars; i++) {
    (*serial).putc(str[i]);
  }

#else // This section needs to be #defined because writeBlock does not exist for UART serial object
  uint8_t *output_ptr = (uint8_t *)str; // writeBlock requires pointer to uint8_t
  unsigned int bytes_remaining = num_chars;

  while (bytes_remaining > 0) {
    if (bytes_remaining < USB_BLOCK_SIZE) {
      (*serial).writeBlock(output_ptr, bytes_remaining);
      bytes_remaining = 0;
    }
    else {
      (*serial).writeBlock(output_ptr, USB_BLOCK_SIZE);
      bytes_remaining -= USB_BLOCK_SIZE;
      output_ptr += USB_BLOCK_SIZE;
    }
  }
#endif
}
