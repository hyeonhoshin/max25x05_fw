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

#include "cmd.h"
#include <stdlib.h>
#include <string.h>
#include "USBSerial.h"
#include "controller.h"
#include "interface.h"
#include "gesture_lib.h"
#include "config.h"

extern const char *version_string;

#if ENABLE_USER_FRAMES
extern uint32_t userDataFrameFlag;
extern int user_pixels[60];
int cmd_frame(char *toks[], const unsigned int tokCount);
#endif

#if USE_UART_INTERFACE
extern Serial *serial;
#else
extern USBSerial *serial;
#endif

commandTable cmdTable[] =
{
  {"ping","Ping the processor.", cmd_ping},
  {"help","Display help.", cmd_help},
  {"ver","Display firmware version", cmd_ver},
  {"reg", "reg <read/write> <addr> <num/value>. Read or write to a device register.", cmd_reg},
  {"stream", "stream <on/off> [nopixels]. Enable data streaming mode. Use nopixels parameter to suppress pixel data.", cmd_stream},
  {"forcecal", "Force bias calibration (tracking mode).", cmd_force_tracking_cal},
  {"reset", "reset device register settings.", cmd_reset},
  {"poll", "Request gesture results", cmd_poll},
  {CMD_TABLE_END, "", NULL} // last command must be NULL
};

// Parses command string and calls appropriate function, passing arguments
int processCmdString(char *st)
{
  char *toks[MAX_TOKENS];
  unsigned int tokCount = 0;
  int cmdIndex = 0;

  // Get tokens
  toks[tokCount] = strtok(st, " ");
  while((toks[tokCount] != NULL) && (tokCount < MAX_TOKENS)) {
    tokCount++;
    toks[tokCount] = strtok(NULL, " ");
  }

  // Find a valid command
  cmdIndex = lookupCmd(toks[0]);

  // If command found call the function
  if(cmdIndex >= 0) {
    return cmdTable[cmdIndex].fp(toks, tokCount);
  }
  return CMD_NACK;
}

// Returns index of command if found. Returns -1 if no command found
int lookupCmd(const char *str)
{
  int i = 0;
  while(1) {
    if(strcmp(cmdTable[i].cmd, CMD_TABLE_END) == 0) // no command found
      return -1;
    if(strcmp(str, cmdTable[i].cmd) == 0)
      return(i);
    i++;
  }
}

// Custom implementation of polling of serial port
// to build a command string and pass it to processCmdString()
int checkUserCmd()
{
  static int index = 0;
  static char cmdString[CMD_BUFFER_SIZE];
  int status = 0;
  while ((*serial).readable()) {
    char ch = (char)(*serial).getc();
    if (ch == 0x08 && index > 0) { // got a backspace, delete the previous character
      index--;
    }
    else if (ch == '\r') {/*ignore*/}
    else if (ch == '\n') {    // Command string is ready to be processed
      cmdString[index] = 0;   // NULL terminate the string
      index = 0;              // reset the index
      if (processCmdString(cmdString) == CMD_NACK) {
        (*serial).printf("Invalid command received: %s\n", cmdString);
      }
      else {
        status = 1;
      }
    }
    else {
        cmdString[index++] = ch;
    }
  }
  return status;
}

int cmd_ping(char *toks[], const unsigned int tokCount)
{
  (*serial).puts("ack\n");
  return CMD_ACK;
}

int cmd_ver(char *toks[], const unsigned int tokCount)
{
  (*serial).puts(version_string);
  (*serial).puts("\n");
  return CMD_ACK;
}

int cmd_help(char *toks[], const unsigned int tokCount)
{
  if (tokCount == 1) {
    (*serial).printf("Commands available. Type 'help <command>' for additional info: ");
    int i = 0;
    while (strcmp(cmdTable[i].cmd, CMD_TABLE_END) != 0) {
        (*serial).printf("%s ", cmdTable[i++].cmd);
    }
    (*serial).printf("\n");
  }
  else {
    int cmdIndex = lookupCmd(toks[1]);
    if(cmdIndex == -1) {
      (*serial).printf("No help on that topic. Type 'help' for list of commands\n");
    }
    else {
      (*serial).printf("%s: ", cmdTable[cmdIndex].cmd);
      (*serial).printf(cmdTable[cmdIndex].helpStr);
      (*serial).printf("\n");
    }
  }
  return CMD_ACK;
}

int cmd_reg(char *toks[], const unsigned int tokCount)
{
  if (tokCount < 4)
    return CMD_NACK;
  char * cmd = toks[1];
  uint8_t reg_addr = strtoul(toks[2], NULL, 0);

  if (strcmp(cmd, "read") == 0) {
    uint8_t num_bytes = strtoul(toks[3], NULL, 0);
    uint8_t reg_vals[MAX_REG_READ_SIZE];
    uint8_t output[MAX_REG_OUTPUT_LENGTH]; // char array to store string output
    char *output_idx = (char*) output; // used to iterate through char array, must be a char for sprintf

    reg_read(reg_addr, num_bytes, reg_vals);
    for(int i=0; i<num_bytes; i++) {
      sprintf(output_idx, "%02X", reg_vals[i]);
      output_idx += 2; // increment by two characters
      if (i<num_bytes-1) {
        sprintf(output_idx++, " "); // values separated by a space
      }
    }
    sprintf(output_idx, "\n"); // add a newline to indicate end-of-frame

    sendDataStream(output, num_bytes * 3); // Send as a data stream for better speed, in case application is polling frame data.
  }
  else if (strcmp(cmd, "write") == 0) {
    uint8_t reg_val = strtoul(toks[3], NULL, 0);
    reg_write(reg_addr, reg_val);
  }
  else {
    return CMD_NACK;
  }
  return CMD_ACK;
}

// Enable data streaming mode, based on hardware interrupt from the INTb pin */
int cmd_stream(char *toks[], const unsigned int tokCount)
{
  if (tokCount < 2) {
    return CMD_NACK;
  }
  char * enable_stream = toks[1];
  if (strcmp(enable_stream, "on") == 0) {
    if (tokCount > 2) {
      char * no_data_parameter = toks[2];
      if (strcmp(no_data_parameter, "nopixels") == 0) {
        set_stream_on(0);
      }
      else {
        return CMD_NACK;
      }
    }
    else {
      set_stream_on(1);
    }
  }
  else if (strcmp(enable_stream, "off") == 0) {
    set_stream_off();
  }
  else {
    return CMD_NACK;
  }
  return CMD_ACK;
}

int cmd_track(char *toks[], const unsigned int tokCount)
{
  uint32_t cols, rows;
  if (tokCount < 2) {
    cols = 3;
    rows = 2;
  }
  else {
    cols = strtoul(toks[1], NULL, 0);
    rows = strtoul(toks[2], NULL, 0);
  }
  return CMD_ACK;
}

int cmd_force_tracking_cal(char *toks[], const unsigned int tokCount)
{
  forceTrackingCalibration();
  return CMD_ACK;
}

int cmd_reset(char *toks[], const unsigned int tokCount)
{
  set_default_register_settings();
  return CMD_ACK;
}

extern GestureResult gesResult;
extern GestureEvent latched_gesture;
int cmd_poll(char *toks[], const unsigned int tokCount)
{
  (*serial).printf("%d,%d,%d,%d,%.2f,%.2f,%.2f,%d,%d,%d\n",
    gesResult.int_placeholder,
    gesResult.state,
    gesResult.n_sample,
    gesResult.maxpixel,
    gesResult.x,
    gesResult.y,
    gesResult.float_placeholder,
    gesResult.int_placeholder,
    gesResult.int_placeholder,
    gesResult.int_placeholder
  );
}
