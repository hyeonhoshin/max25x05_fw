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

#ifndef CMDS_H_INCLUDED
#define CMDS_H_INCLUDED

#include "mbed.h"

#define MAX_TOKENS          16 /*Increased for frame read over usb. Can reduce to 10*/
#define CMD_BUFFER_SIZE     129 /*Increased size for frame read over usb. Can reduce to 65*/
#define CMD_TABLE_END       "NULL"
#define CMD_ACK             0
#define CMD_NACK            1

#define MAX_REG_OUTPUT_LENGTH 512
#define MAX_REG_READ_SIZE 256

extern void enable_read_sensor_frames();
extern void disable_read_sensor_frames();
extern void set_stream_on(uint32_t send_pixel_data);
extern void set_stream_off();
extern void set_default_register_settings();

typedef struct {
    char cmd[128];
    const char *helpStr;
    int (*fp)(char *toks[], const unsigned int tokCount);
}commandTable;

int processCmdString(char *st);
int lookupCmd(const char *str);
int checkUserCmd();

int cmd_ping(char *toks[], const unsigned int tokCount);
int cmd_ver(char *toks[], const unsigned int tokCount);
int cmd_help(char *toks[], const unsigned int tokCount);
int cmd_reg(char *toks[], const unsigned int tokCount);
int cmd_stream(char *toks[], const unsigned int tokCount);
int cmd_force_tracking_cal(char *toks[], const unsigned int tokCount);
int cmd_poll(char *toks[], const unsigned int tokCount);
int cmd_reset(char *toks[], const unsigned int tokCount);
#endif
