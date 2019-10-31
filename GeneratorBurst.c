/**
 * GeneratorBurst.c
 *
 * This example generates a 50 Hz sine waveform, 4 Vpp, 100 periods.
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

  // Enable network search:
  NetSetAutoDetectEnabled(BOOL8_TRUE);
  CHECK_LAST_STATUS();

  // Update device list:
  LstUpdate();
  CHECK_LAST_STATUS();

  // Try to open a generator with burst support:
  LibTiePieHandle_t gen = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_GENERATOR))
    {
      gen = LstOpenGenerator(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and burst support:
      if(gen != LIBTIEPIE_HANDLE_INVALID && (GenGetModesNative(gen) & GM_BURST_COUNT))
      {
        break;
      }
      else
      {
        gen = LIBTIEPIE_HANDLE_INVALID;
      }
    }
  }

  if(gen != LIBTIEPIE_HANDLE_INVALID)
  {
    // Set signal type:
    GenSetSignalType(gen, ST_SINE);
    CHECK_LAST_STATUS();

    // Set frequency:
    GenSetFrequency(gen, 50); // 50 Hz
    CHECK_LAST_STATUS();

    // Set amplitude:
    GenSetAmplitude(gen, 2); // 2 V
    CHECK_LAST_STATUS();

    // Set offset:
    GenSetOffset(gen, 0); // 0 V
    CHECK_LAST_STATUS();

    // Set mode:
    GenSetMode(gen, GM_BURST_COUNT);
    CHECK_LAST_STATUS();

    // Set burst count:
    GenSetBurstCount(gen, 100); // 100 periods
    CHECK_LAST_STATUS();

    // Enable output:
    GenSetOutputOn(gen, BOOL8_TRUE);
    CHECK_LAST_STATUS();

    // Print Generator info:
    printDeviceInfo(gen);

    // Start signal generation:
    GenStart(gen);
    CHECK_LAST_STATUS();

    // Wait for burst to complete:
    while(GenIsBurstActive(gen))
    {
      sleepMiliSeconds(10); // 10 ms delay, to save CPU time.
    }

    // Stop generator:
    GenStop(gen);
    CHECK_LAST_STATUS();

    // Disable output:
    GenSetOutputOn(gen, BOOL8_FALSE);
    CHECK_LAST_STATUS();

    // Close generator:
    ObjClose(gen);
    CHECK_LAST_STATUS();
  }
  else
  {
    fprintf(stderr, "No generator available with burst support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
