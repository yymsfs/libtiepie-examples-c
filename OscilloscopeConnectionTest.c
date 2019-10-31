/**
 * OscilloscopeConnectionTest.c
 *
 * This example performs a connection test.
 *
 * Find more information on http://www.tiepie.com/LibTiePie .
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
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

  // Enable network search:
  NetSetAutoDetectEnabled(BOOL8_TRUE);
  CHECK_LAST_STATUS();

  // Update device list:
  LstUpdate();
  CHECK_LAST_STATUS();

  // Try to open an oscilloscope with connection test support:
  LibTiePieHandle_t scp = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_OSCILLOSCOPE))
    {
      scp = LstOpenOscilloscope(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and connection test support:
      if(scp != LIBTIEPIE_HANDLE_INVALID && ScpHasConnectionTest(scp))
      {
        break;
      }
      else
      {
        scp = LIBTIEPIE_HANDLE_INVALID;
      }
    }
  }

  if(scp != LIBTIEPIE_HANDLE_INVALID)
  {
    // Get the number of channels:
    const uint16_t channelCount = ScpGetChannelCount(scp);
    CHECK_LAST_STATUS();

    // Enable all channels that support connection testing:
    for(uint16_t ch = 0; ch < channelCount; ch++)
    {
      bool8_t b = ScpChHasConnectionTest(scp, ch);
      CHECK_LAST_STATUS();
      ScpChSetEnabled(scp, ch, b);
      CHECK_LAST_STATUS();
    }

    // Start connection test on current active channels:
    ScpStartConnectionTest(scp);
    CHECK_LAST_STATUS();

    // Wait for connection test to complete:
    while(!ScpIsConnectionTestCompleted(scp) && !ObjIsRemoved(scp))
    {
      sleepMiliSeconds(10); // 10 ms delay, to save CPU time.
    }

    // Create data buffer:
    LibTiePieTriState_t* data = malloc(sizeof(LibTiePieTriState_t) * channelCount);

    // Get data:
    ScpGetConnectionTestData(scp, data, channelCount);
    CHECK_LAST_STATUS();

    // Print results:
    printf("Connection test result:" NEWLINE);
    for(uint16_t ch = 0; ch < channelCount; ch++)
    {
      printf("Ch%" PRIu16 " = ", ch + 1);

      switch(data[ch])
      {
        case LIBTIEPIE_TRISTATE_UNDEFINED:
          printf("undefined" NEWLINE);
          break;

        case LIBTIEPIE_TRISTATE_FALSE:
          printf("false" NEWLINE);
          break;

        case LIBTIEPIE_TRISTATE_TRUE:
          printf("true" NEWLINE);
          break;

        default:
          printf("unknown state" NEWLINE);
      }
    }

    // Free data buffer:
    free(data);

    // Close oscilloscope:
    ObjClose(scp);
    CHECK_LAST_STATUS();
  }
  else
  {
    fprintf(stderr, "No oscilloscope available with connection test support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
