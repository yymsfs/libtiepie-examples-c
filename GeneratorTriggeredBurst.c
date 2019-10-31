/**
 * GeneratorTriggeredBurst.c
 *
 * This example generates a 100 kHz square waveform, 25% duty cycle, 0..5 V, 20 periods, this waveform is triggered by the external trigger.
 * Connect the external trigger to GND to trigger the burst.
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

  // Try to open a generator with triggered burst support:
  LibTiePieHandle_t gen = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_GENERATOR))
    {
      gen = LstOpenGenerator(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and triggered burst support:
      if(gen != LIBTIEPIE_HANDLE_INVALID && (GenGetModesNative(gen) & GM_BURST_COUNT) && DevTrGetInputCount(gen) > 0)
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
    GenSetFrequency(gen, 100e3); // 100 kHz
    CHECK_LAST_STATUS();

    // Set amplitude:
    GenSetAmplitude(gen, 2.5); // 2.5 V
    CHECK_LAST_STATUS();

    // Set offset:
    GenSetOffset(gen, 2.5); // 2.5 V
    CHECK_LAST_STATUS();

    // Set symmetry (duty cycle):
    GenSetSymmetry(gen, 0.25); // 25 %
    CHECK_LAST_STATUS();

    // Set mode:
    GenSetMode(gen, GM_BURST_COUNT);
    CHECK_LAST_STATUS();

    // Set burst count:
    GenSetBurstCount(gen, 20); // 20 periods
    CHECK_LAST_STATUS();

    // Locate trigger input:
    uint16_t index = DevTrGetInputIndexById(gen, TIID_EXT1);
    CHECK_LAST_STATUS();

    if(index == LIBTIEPIE_TRIGGERIO_INDEX_INVALID)
    {
      index = DevTrGetInputIndexById(gen, TIID_EXT2);
      CHECK_LAST_STATUS();
    }

    if(index != LIBTIEPIE_TRIGGERIO_INDEX_INVALID)
    {
      // Enable trigger input:
      DevTrInSetEnabled(gen, index, BOOL8_TRUE);
      CHECK_LAST_STATUS();

      // Set trigger input kind:
      DevTrInSetKind(gen, index, TK_FALLINGEDGE);
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
    fprintf(stderr, "No generator available with triggererd burst support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
