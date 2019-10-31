/**
 * I2CDAC.c
 *
 * This example demonstrates how to use an I2C host supported by LibTiePie.
 * It shows how to control an Analog Devices AD5667 dual 16-bit DAC.
 *
 * Find more information on http://www.tiepie.com/LibTiePie .
 */

#include <stdlib.h>
#include <stdio.h>
#include <libtiepie.h>
#include "CheckStatus.h"
#include "PrintInfo.h"
#include "Utils.h"

// AD5667 address:
#define AD5667_ADDRESS 12

// AD5667 registers:
#define AD5667_REG_DAC_A   0x00
#define AD5667_REG_DAC_B   0x01
#define AD5667_REG_DAC_ALL 0x07

// AD5667 commands:
#define AD5667_CMD_WRITE            0x00
#define AD5667_CMD_UPDATE           0x08
#define AD5667_CMD_WRITE_UPDATE_ALL 0x10
#define AD5667_CMD_WRITE_UPDATE     0x18
#define AD5667_CMD_POWER            0x20
#define AD5667_CMD_RESET            0x28
#define AD5667_CMD_LDAC_SETUP       0x30
#define AD5667_CMD_REF_SETUP        0x38

int main(int argc, char* argv[])
{
  int status = EXIT_SUCCESS;

  // Initialize library:
  LibInit();

  // Print library information:
  printLibraryInfo();

  // Enable network search:
  NetSetAutoDetectEnabled(BOOL8_TRUE);
  CHECK_LAST_STATUS();

  // Update device list:
  LstUpdate();
  CHECK_LAST_STATUS();

  // Try to open an I2C host:
  LibTiePieHandle_t i2c = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_I2CHOST))
    {
      i2c = LstOpenI2CHost(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      if(i2c)
      {
        break;
      }
    }
  }

  if(i2c != LIBTIEPIE_HANDLE_INVALID)
  {
    // Print I2C host info:
    printDeviceInfo(i2c);

    // Turn on internal reference for DAC A:
    I2CWriteByteWord(i2c, AD5667_ADDRESS, AD5667_CMD_REF_SETUP | AD5667_REG_DAC_A, 1);
    CHECK_LAST_STATUS();

    // Set DAC A to mid level:
    I2CWriteByteWord(i2c, AD5667_ADDRESS, AD5667_CMD_WRITE_UPDATE | AD5667_REG_DAC_A, 0x8000);
    CHECK_LAST_STATUS();

    // Close I2C host:
    ObjClose(i2c);
    CHECK_LAST_STATUS();
  }
  else
  {
    fprintf(stderr, "No I2C host available!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
