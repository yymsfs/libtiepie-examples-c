/**
 * OscilloscopeCombineHS3HS4.c
 *
 * This example demonstrates how to create and open a combined instrument of all found Handyscope HS3, Handyscope HS4 and/or Handyscope HS4 DIFF's.
 *
 * Find more information on http://www.tiepie.com/LibTiePie .
 */

#include <stdlib.h>
#include <stdio.h>
#include <libtiepie.h>
#include "CheckStatus.h"
#include "PrintInfo.h"
#include "Utils.h"

int main(int argc, char* argv[])
{
  int status = EXIT_SUCCESS;

  // Initialize library:
  LibInit();

  // Print library information:
  printLibraryInfo();

  // Update device list:
  LstUpdate();
  CHECK_LAST_STATUS();

  // Allocate memory for storing device handles:
  uint32_t deviceCount = LstGetCount();
  CHECK_LAST_STATUS();
  LibTiePieHandle_t* deviceHandles = malloc(sizeof(LibTiePieHandle_t) * deviceCount);
  uint32_t deviceHandleCount = 0;

  // Try to open all HS3/HS4(D) oscilloscopes:
  for(uint32_t index = 0; index < deviceCount; index++)
  {
    uint32_t productId = LstDevGetProductId(IDKIND_INDEX, index);
    CHECK_LAST_STATUS();

    if((productId == PID_HS3 || productId == PID_HS4 || productId == PID_HS4D) &&
        LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_OSCILLOSCOPE))
    {
      LibTiePieHandle_t scp = LstOpenOscilloscope(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      if(scp != LIBTIEPIE_HANDLE_INVALID)
      {
        // Get name:
        uint32_t length = DevGetName(scp, NULL, 0) + 1; // Add one for the terminating zero
        CHECK_LAST_STATUS();
        char* name = malloc(sizeof(char) * length);
        length = DevGetName(scp, name, length);
        CHECK_LAST_STATUS();

        printf("Found: %s, s/n: %u" NEWLINE, name, DevGetSerialNumber(scp));
        CHECK_LAST_STATUS();

        free(name);

        deviceHandles[deviceHandleCount] = scp;
        deviceHandleCount++;
      }
    }
  }

  if(deviceHandleCount > 1)
  {
      // Create and open combined instrument:
      LibTiePieHandle_t scp = LstCreateAndOpenCombinedDevice(deviceHandles, deviceHandleCount);
      CHECK_LAST_STATUS();

      // Release HS3/HS4(D) handles, not required anymore:
      for(uint32_t i = 0; i < deviceHandleCount; i++)
      {
        ObjClose(deviceHandles[i]);
        CHECK_LAST_STATUS();
      }
      free(deviceHandles);
      deviceHandles = NULL;

      // Print combined oscilloscope info:
      printDeviceInfo(scp);

      // Get serial number, required for removing:
      uint32_t serialNumber = DevGetSerialNumber(scp);
      CHECK_LAST_STATUS();

      // Close combined oscilloscope:
      ObjClose(scp);
      CHECK_LAST_STATUS();

      // Remove combined oscilloscope from the device list:
      LstRemoveDevice(serialNumber);
      CHECK_LAST_STATUS();
  }
  else
  {
    fprintf(stderr, "Not enough HS3/HS4(D)\'s found, at least two required!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Release HS3/HS4(D) handles:
  if(deviceHandles)
  {
    for(uint32_t i = 0; i < deviceHandleCount; i++)
    {
      ObjClose(deviceHandles[i]);
    }
    free(deviceHandles);
  }

  // Exit library:
  LibExit();

  return status;
}
