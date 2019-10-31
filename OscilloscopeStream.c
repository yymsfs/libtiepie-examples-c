/**
 * OscilloscopeStream.c
 *
 * This example performs a stream mode measurement and writes the data to OscilloscopeStream.csv.
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

  // Try to open an oscilloscope with with stream measurement support:
  LibTiePieHandle_t scp = LIBTIEPIE_HANDLE_INVALID;

  for(uint32_t index = 0; index < LstGetCount(); index++)
  {
    if(LstDevCanOpen(IDKIND_INDEX, index, DEVICETYPE_OSCILLOSCOPE))
    {
      scp = LstOpenOscilloscope(IDKIND_INDEX, index);
      CHECK_LAST_STATUS();

      // Check for valid handle and stream measurement support:
      if(scp != LIBTIEPIE_HANDLE_INVALID && (ScpGetMeasureModes(scp) & MM_STREAM))
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

    // Set measure mode:
    ScpSetMeasureMode(scp, MM_STREAM);

    // Set sample frequency:
    ScpSetSampleFrequency(scp, 1e3); // 1 kHz

    // Set record length:
    const uint64_t recordLength = ScpSetRecordLength(scp, 1000); // 1 kS

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

    // Print oscilloscope info:
    printDeviceInfo(scp);

    // Create data buffers:
    float** channelData = malloc(sizeof(float*) * channelCount);
    for(uint16_t ch = 0; ch < channelCount; ch++)
    {
      channelData[ch] = malloc(sizeof(float) * recordLength);
    }

    // Open file with write/update permissions:
    const char* filename = "OscilloscopeStream.csv";
    FILE* csv = fopen(filename, "w");
    if(csv)
    {
      // Start measurement:
      ScpStart(scp);

      // Write csv header:
      fprintf(csv, "Sample");
      for(uint16_t ch = 0; ch < channelCount; ch++)
      {
        fprintf(csv, ";Ch%" PRIu16, (ch + 1));
      }
      fprintf(csv, NEWLINE);

      uint64_t currentSample = 0;

      for(uint8_t chunk = 0; chunk < 10; chunk++) // Measure 10 chunks
      {
        // Print a message, to inform the user that we still do something:
        printf("Data chunk %" PRIu8 NEWLINE, chunk + 1);

        // Wait for measurement to complete:
        while(!(ScpIsDataReady(scp) || ScpIsDataOverflow(scp) || ObjIsRemoved(scp)))
        {
          sleepMiliSeconds(10); // 10 ms delay, to save CPU time.
        }

        // Print error on device remove:
        if(ObjIsRemoved(scp))
        {
          fprintf(stderr, "Device gone!" NEWLINE);
          status = EXIT_FAILURE;
          break;
        }

        // Print error on data overflow:
        if(ScpIsDataOverflow(scp))
        {
          fprintf(stderr, "Data overflow!" NEWLINE);
          status = EXIT_FAILURE;
          break;
        }

        // Get data:
        uint64_t samplesRead = ScpGetData(scp, channelData, channelCount, 0, recordLength);

        // Write the data to csv:
        for(uint64_t i = 0; i < recordLength; i++)
        {
          fprintf(csv, "%" PRIu64, currentSample + i);
          for(uint16_t ch = 0; ch < channelCount; ch++)
          {
            fprintf(csv, ";%f", channelData[ch][i]);
          }
          fprintf(csv, NEWLINE);
        }

        currentSample += samplesRead;
      }

      printf("Data written to: %s" NEWLINE, filename);

      // Close file:
      fclose(csv);

      // Stop measurement:
      ScpStop(scp);
    }
    else
    {
      fprintf(stderr, "Couldn't open file: %s" NEWLINE, filename);
      status = EXIT_FAILURE;
    }

    // Delete data buffers:
    for(uint16_t ch = 0; ch < channelCount; ch++)
      free(channelData[ch]);
    free(channelData);

    // Close oscilloscope:
    ObjClose(scp);
    CHECK_LAST_STATUS();
  }
  else
  {
    fprintf(stderr, "No oscilloscope available with stream measurement support!" NEWLINE);
    status = EXIT_FAILURE;
  }

  // Exit library:
  LibExit();

  return status;
}
