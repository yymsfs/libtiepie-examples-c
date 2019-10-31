/**
 * OscilloscopeBlock.c
 *
 * This example performs a block mode measurment and writes the data to OscilloscopeBlock.csv.
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

  // Try to open an oscilloscope with block measurement support:
  LibTiePieHandle_t scp = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_OSCILLOSCOPE))
    {
      scp = LstOpenOscilloscope(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and block measurement support:
      if(scp != LIBTIEPIE_HANDLE_INVALID && (ScpGetMeasureModes(scp) & MM_BLOCK))
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
    const uint16_t channelCount = ScpGetChannelCount(scp);
    CHECK_LAST_STATUS();

    // Set measure mode:
    ScpSetMeasureMode(scp, MM_BLOCK);

    // Set sample frequency:
    ScpSetSampleFrequency(scp, 1e6); // 1 MHz

    // Set record length:
    uint64_t recordLength = ScpSetRecordLength(scp, 10000); // 10 kS
    CHECK_LAST_STATUS();

    // Set pre sample ratio:
    ScpSetPreSampleRatio(scp, 0); // 0 %

    // For all channels:
    for(uint16_t ch = 0; ch < channelCount; ch++)
    {
      // Enable channel to measure it:
      ScpChSetEnabled(scp, ch, BOOL8_TRUE);
      CHECK_LAST_STATUS();

      // Set range:
      ScpChSetRange(scp, ch, 8); // 8 V
      CHECK_LAST_STATUS();

      // Set coupling:
      ScpChSetCoupling(scp, ch, CK_DCV); // DC Volt
      CHECK_LAST_STATUS();
    }

    // Set trigger timeout:
    ScpSetTriggerTimeOut(scp, 100e-3); // 100 ms
    CHECK_LAST_STATUS();

    // Disable all channel trigger sources:
    for(uint16_t ch = 0; ch < channelCount; ch++)
    {
      ScpChTrSetEnabled(scp, ch, BOOL8_FALSE);
      CHECK_LAST_STATUS();
    }

    // Setup channel trigger:
    const uint16_t ch = 0; // Ch 1

    // Enable trigger source:
    ScpChTrSetEnabled(scp, ch, BOOL8_TRUE);
    CHECK_LAST_STATUS();

    // Kind:
    ScpChTrSetKind(scp, ch, TK_RISINGEDGE); // Rising edge
    CHECK_LAST_STATUS();

    // Level:
    ScpChTrSetLevel(scp, ch, 0, 0.5); // 50 %
    CHECK_LAST_STATUS();

    // Hysteresis:
    ScpChTrSetHysteresis(scp, ch, 0, 0.05); // 5 %
    CHECK_LAST_STATUS();

    // Print oscilloscope info:
    printDeviceInfo(scp);

    // Start measurement:
    ScpStart(scp);
    CHECK_LAST_STATUS();

    // Wait for measurement to complete:
    while(!ScpIsDataReady(scp) && !ObjIsRemoved(scp))
    {
      sleepMiliSeconds(10); // 10 ms delay, to save CPU time.
    }

    if(ObjIsRemoved(scp))
    {
      fprintf(stderr, "Device gone!");
      status = EXIT_FAILURE;
    }
    else if(ScpIsDataReady(scp))
    {
      // Create data buffers:
      float** channelData = malloc(sizeof(float*) * channelCount);
      for(uint16_t ch = 0; ch < channelCount; ch++)
      {
        channelData[ch] = malloc(sizeof(float) * recordLength);
      }

      // Get the data from the scope:
      recordLength = ScpGetData(scp, channelData, channelCount, 0, recordLength);
      CHECK_LAST_STATUS();

      // Open file with write/update permissions:
      const char* filename = "OscilloscopeBlock.csv";
      FILE* csv = fopen(filename, "w");
      if(csv)
      {
        // Write csv header:
        fprintf(csv, "Sample");
        for(uint16_t ch = 0; ch < channelCount; ch++)
        {
          fprintf(csv, ";Ch%" PRIu16, ch + 1);
        }
        fprintf(csv, NEWLINE);

        // Write the data to csv:
        for(uint64_t i = 0; i < recordLength; i++)
        {
          fprintf(csv, "%" PRIu64, i);
          for(uint16_t ch = 0; ch < channelCount; ch++)
          {
            fprintf(csv, ";%f", channelData[ch][i]);
          }
          fprintf(csv, NEWLINE);
        }

        printf("Data written to: %s" NEWLINE, filename);

        // Close file:
        fclose(csv);
      }
      else
      {
        fprintf(stderr, "Couldn't open file: %s" NEWLINE, filename);
        status = EXIT_FAILURE;
      }

      // Free data buffers:
      for(uint16_t ch = 0; ch < channelCount; ch++)
      {
        free(channelData[ch]);
      }
      free(channelData);
    }

    // Close oscilloscope:
    ObjClose(scp);
    CHECK_LAST_STATUS();
  }
  else
  {
    fprintf(stderr, "No oscilloscope available with block measurement support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
