/**
 * GeneratorArbitrary.c
 *
 * This example generates an arbitrary waveform.
 *
 * Find more information on http://www.tiepie.com/LibTiePie .
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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

  // Try to open a generator with arbitrary suppport:
  LibTiePieHandle_t gen = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_GENERATOR))
    {
      gen = LstOpenGenerator(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and arbitrary support:
      if(gen != LIBTIEPIE_HANDLE_INVALID && (GenGetSignalTypes(gen) & ST_ARBITRARY))
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
    GenSetSignalType(gen, ST_ARBITRARY);
    CHECK_LAST_STATUS();

    // Select frequency mode:
    GenSetFrequencyMode(gen, FM_SAMPLEFREQUENCY);
    CHECK_LAST_STATUS();

    // Set frequency:
    GenSetFrequency(gen, 100e3); // 100 kHz
    CHECK_LAST_STATUS();

    // Set amplitude:
    GenSetAmplitude(gen, 2); // 2 V
    CHECK_LAST_STATUS();

    // Set offset:
    GenSetOffset(gen, 0); // 0 V
    CHECK_LAST_STATUS();

    // Enable output:
    GenSetOutputOn(gen, BOOL8_TRUE);
    CHECK_LAST_STATUS();

    // Create signal array:
    float data[8192];
    for(unsigned int i = 0; i < 8192; i++)
    {
      data[i] = sinf((float)(i / 100)) * (1 - ((float)(i / 8192)));
    }

    // Load the signal array into the generator:
    GenSetData(gen, data, 8192);
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
    fprintf(stderr, "No generator available with arbitrary support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
