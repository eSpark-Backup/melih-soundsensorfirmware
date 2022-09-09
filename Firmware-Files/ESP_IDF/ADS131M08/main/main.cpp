#include <stdio.h>
#include <esp_log.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "ADS131M08.h"

const int sampleTime = 50; 

#define MISO_PIN              19
#define MOSI_PIN              23
#define SCK_PIN               18
#define CS_PIN                27

#define VSPI_MISO             MISO_PIN
#define VSPI_MOSI             MOSI_PIN
#define VSPI_SCLK             SCK_PIN
#define VSPI_SS               CS_PIN

#define DRDY_PIN              -1//21
#define XTAL_PIN              -1//22
#define RESET                -1 //17

int micOut[8];
int32_t channelArr[8];
void findPTPAmp(ADS131M08* adc);
long map(long x, long in_min, long in_max, long out_min, long out_max);

extern "C" void app_main(void)
{
	ADS131M08 adc((gpio_num_t)VSPI_MOSI,
                (gpio_num_t)VSPI_MISO,
                (gpio_num_t)VSPI_SCLK,
                VSPI_SS,
                XTAL_PIN,
                DRDY_PIN,
                8192000);

	adc.init();
	adc.writeReg(ADS131_CLOCK,0b1111111100011010); //Clock register (page 55 in datasheet)
  adc.setGain(32);

	uint16_t dataR;
  dataR = adc.readReg(ADS131_ID);
	ESP_LOGI("Chip ID "," %x \n",dataR);

	dataR = adc.readReg(ADS131_CLOCK);
	ESP_LOGI("Clock "," %x \n",dataR);

  dataR = 0;
  dataR = adc.readReg(ADS131_GAIN1);
	ESP_LOGI("Gain 1 "," %x \n",dataR);

  dataR = 0;
  dataR = adc.readReg(ADS131_GAIN2);
	ESP_LOGI("Gain 2 "," %x \n",dataR);

	while (1) {
		
  findPTPAmp(&adc);
  printf("%d,%d,%d,%d,%d,%d,%d,%d\r\n", 
            channelArr[0], 
            channelArr[1], 
            channelArr[2],
            channelArr[3],
            channelArr[4],
            channelArr[5],
            channelArr[6],
            channelArr[7]
            );

		vTaskDelay(10 / portTICK_RATE_MS);
	}
    
}

// Find the Peak-to-Peak Amplitude Function
void findPTPAmp(ADS131M08* adc){
// Time variables to find the peak-to-peak amplitude
   unsigned long startTime= esp_timer_get_time()/1000;  // Start of sample window
   unsigned int PTPAmp[8] = {0}; 

// Signal variables to find the peak-to-peak amplitude
   int maxAmp[8];
   std::fill_n(maxAmp, 8, 200000);
   int minAmp[8] ;
   std::fill_n(minAmp, 8, -200000);

// Find the max and min of the mic output within the 50 ms timeframe
   while((esp_timer_get_time()/1000) - startTime < sampleTime) 
   {
    adc->readAllChannels(channelArr);
    for (uint8_t i = 0; i < 8; i++)
    {
      micOut[i] = channelArr[i];
      if( micOut[i] < 1023) //prevent erroneous readings
      {
        if (micOut > maxAmp)
        {
          maxAmp[i] = micOut[i]; //save only the max reading
        }
        else if (micOut < minAmp)
        {
          minAmp[i] = micOut[i]; //save only the min reading
        }
      }
    }     
   }

  for (uint8_t i = 0; i < 8; i++){
  PTPAmp[i] = maxAmp[i] - minAmp[i]; // (max amp) - (min amp) = peak-to-peak amplitude
  }
//   double micOut_Volts = (PTPAmp * 3.3) / 4096; // Convert ADC into voltage

  //Uncomment this line for help debugging (be sure to also comment out the VUMeter function)
  //Serial.println(PTPAmp); 

  //Return the PTP amplitude to use in the soundLevel function. 
  // You can also return the micOut_Volts if you prefer to use the voltage level.
  for (uint8_t i = 0; i < 8; i++){
    channelArr[i] = PTPAmp[i];
  }   
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



