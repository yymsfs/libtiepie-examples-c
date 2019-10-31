/**
 * GeneratorGatedBurst.c
 *
 * This example generates a 10 kHz square waveform, 10 Vpp when the external trigger input is active.
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

  // Try to open a generator with gated burst support:
  LibTiePieHandle_t gen = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_GENERATOR))
    {
      gen = LstOpenGenerator(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and arbitrary support:
      if(gen != LIBTIEPIE_HANDLE_INVALID && (GenGetModesNative(gen) & GM_GATED_PERIODS) && DevTrGetInputCount(gen) > 0)
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
    GenSetSignalType(gen, ST_SQUARE);
    CHECK_LAST_STATUS();

    // Set frequency:
    GenSetFrequency(gen, 10e3); // 10 kHz
    CHECK_LAST_STATUS();

    // Set amplitude:
    GenSetAmplitude(gen, 5); // 5 V
    CHECK_LAST_STATUS();

    // Set offset:
    GenSetOffset(gen, 0); // 0 V
    CHECK_LAST_STATUS();

    // Set mode:
    GenSetMode(gen, GM_GATED_PERIODS);
    CHECK_LAST_STATUS();

    // Locate trigger input:
    const uint16_t index = DevTrGetInputIndexById(gen, TIID_EXT1);
    CHECK_LAST_STATUS();

    if(index != LIBTIEPIE_TRIGGERIO_INDEX_INVALID)
    {
      // Enable trigger input:
      DevTrInSetEnabled(gen, index, BOOL8_TRUE);
      CHECK_LAST_STATUS();

      // Enable output:
      GenSetOutputOn(gen, BOOL8_TRUE);
      CHECK_LAST_STATUS();

      // Print generator info:
      printDeviceInfo(gen);

      // Start signal generation:
      GenStart(gen);
      CHECK_LAST_STATUS();

      // Wait for keystroke:
      printf("Press any key to stop signal generation..." NEWLINE);
      waitForKeyStroke();

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
      fprintf(stderr, "Unknown trigger input!" NEWLINE);
      status = EXIT_FAILURE;
    }
  }
  else
  {
    fprintf(stderr, "No generator available with gated burst support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
