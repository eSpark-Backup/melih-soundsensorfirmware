#include "Wire.h"
//#include <I2S.h>
#include "driver/i2s.h"
#include "freertos/queue.h"
#define sampleTime 1

#define devAddressPCM2 0x4A
#define devAddressPCM1 0x4B

// I2S ADC_1 Pins
#define _32K_XP 32
#define LRCKA _32K_XP
#define PIN_I2S_FS_1 LRCKA

#define BCKA 4
#define PIN_I2S_SCK_1 BCKA

#define DOUT1 25
#define PIN_I2S_SD_1 DOUT1

#define I2S_BCK_IO_1  BCKA
#define I2S_WS_IO_1   LRCKA
#define I2S_DO_IO_1   DOUT1
#define I2S_DI_IO_1   DOUT1

// I2S ADC_2 Pins
#define _32K_XN 33
#define LRCKB _32K_XN
#define PIN_I2S_FS_2 LRCKB

#define BCKB 22
#define PIN_I2S_SCK_2 BCKB

#define DOUT2 26
#define PIN_I2S_SD_2 DOUT2

#define I2S_BCK_IO_2  BCKB
#define I2S_WS_IO_2   LRCKB
#define I2S_DO_IO_2   DOUT2
#define I2S_DI_IO_2   DOUT2

#define MTCK 13
#define MTMS 14

#define SDA MTCK
#define SCL MTMS

#define pg0reg1 0x01
#define pg0reg6 0x06
#define pg0reg7 0x07
#define pg0reg8 0x08
#define pg0reg9 0x09
#define pg0reg11 0x0B
#define bclkdivreg 0x26

#define Mic_01 1
#define Mic_02 2
#define Mic_03 3
#define Mic_04 4

#define PCM1 0
#define PCM2 1

static const int i2s_num_0 = 0; // i2s port number
static const int i2s_num_1 = 1; // i2s port number

static const i2s_config_t i2s_config_1 = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
     .sample_rate = 48000,
     .bits_per_sample = (i2s_bits_per_sample_t)32,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
     .intr_alloc_flags = 0, // default interrupt priority
     .dma_buf_count = 8,
     .dma_buf_len = 64,
     .use_apll = false
};

static const i2s_pin_config_t pin_config_1 = {
    .bck_io_num = I2S_BCK_IO_1,
    .ws_io_num = I2S_WS_IO_1,
    .data_out_num = I2S_DO_IO_1,
    .data_in_num = I2S_DI_IO_1
};  

static const i2s_config_t i2s_config_2 = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
     .sample_rate = 48000,
     .bits_per_sample = (i2s_bits_per_sample_t)32,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
     .intr_alloc_flags = 0, // default interrupt priority
     .dma_buf_count = 8,
     .dma_buf_len = 64,
     .use_apll = false
};

static const i2s_pin_config_t pin_config_2 = {
    .bck_io_num = I2S_BCK_IO_2,
    .ws_io_num = I2S_WS_IO_2,
    .data_out_num = I2S_DO_IO_2,
    .data_in_num = I2S_DI_IO_2
};  
    
void setup() {
  
    Serial.begin(115200);
    Wire.begin(SDA,SCL);
    Serial.println("I2C Initialized");

    i2s_driver_install((i2s_port_t)i2s_num_0, &i2s_config_1, 0, NULL);
    i2s_set_pin((i2s_port_t)i2s_num_0, &pin_config_1);

    i2s_driver_install((i2s_port_t)i2s_num_1, &i2s_config_2, 0, NULL);
    i2s_set_pin((i2s_port_t)i2s_num_1, &pin_config_2);

    updateRegister(devAddressPCM1, pg0reg1, 0x40);
    micSelect(Mic_01,devAddressPCM1);
    updateRegister(devAddressPCM1, pg0reg11, 0x40);

    updateRegister(devAddressPCM2, pg0reg1, 0x40);
    micSelect(Mic_04,devAddressPCM2);
    updateRegister(devAddressPCM2, pg0reg11, 0x40);

}

void loop() {
    //int samples =0;
    //uint8_t bytesRead;
    //i2s_read((i2s_port_t)i2s_num, &samples, 4, (size_t*)&bytesRead, 100);
    //Serial.println(samples);
    Serial.print(findPTPAmp(PCM1));
    Serial.print(",");
    Serial.println(findPTPAmp(PCM2));

}

uint8_t readRegister(uint8_t deviceAddr, uint8_t registerAddr)
{
   Wire.beginTransmission(deviceAddr); 
   Wire.write(registerAddr); 
   Wire.endTransmission(false); 
   // Returns '0' on succesful write- but in this case it keeps returning a '2' meaning it cannot locate the address I am trying to point to. 
   Wire.requestFrom(deviceAddr,byte(1)); 
   return Wire.read(); 
}

void writeRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t val)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(registerAddr);
    Wire.write(val);
    Wire.endTransmission();
}

void updateRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t val)
{
   Serial.print("Register Address : ");
   Serial.println(registerAddr);
   Serial.print("Value Before Updation : ");
   Serial.println(readRegister(deviceAddr, registerAddr),HEX);
   delay(10);
   writeRegister(deviceAddr, registerAddr, val);
   Serial.print("Value After Updation : ");
   Serial.println(readRegister(deviceAddr, registerAddr),HEX);
   delay(10);
}
void micSelect(uint8_t micNo,uint8_t devAddress)
{
  switch (micNo)
  {
    case 1:
      updateRegister(devAddress, pg0reg6, 0x50);   
      updateRegister(devAddress, pg0reg7, 0x40);
      updateRegister(devAddress, pg0reg8, 0x40);
      updateRegister(devAddress, pg0reg9, 0x40);
      break;

    case 2:
      updateRegister(devAddress, pg0reg6, 0x40);   
      updateRegister(devAddress, pg0reg7, 0x50);
      updateRegister(devAddress, pg0reg8, 0x40);
      updateRegister(devAddress, pg0reg9, 0x40);
      break;
      
    case 3:
      updateRegister(devAddress, pg0reg6, 0x40);   
      updateRegister(devAddress, pg0reg7, 0x60);
      updateRegister(devAddress, pg0reg8, 0x40);
      updateRegister(devAddress, pg0reg9, 0x40);
      break;
    case 4:
      updateRegister(devAddress, pg0reg6, 0x60);   
      updateRegister(devAddress, pg0reg7, 0x40);
      updateRegister(devAddress, pg0reg8, 0x40);
      updateRegister(devAddress, pg0reg9, 0x40);
      break;   

    default:
      updateRegister(devAddress, pg0reg6, 0x40);   
      updateRegister(devAddress, pg0reg7, 0x40);
      updateRegister(devAddress, pg0reg8, 0x40);
      updateRegister(devAddress, pg0reg9, 0x40);
  }

}

int findPTPAmp(uint8_t i2s_num){
// Time variables to find the peak-to-peak amplitude
   unsigned long startTime= millis();  // Start of sample window
   unsigned int PTPAmp = 0; 

// Signal variables to find the peak-to-peak amplitude
   int maxAmp = 0;
   int minAmp = 0;

// Find the max and min of the mic output within the 50 ms timeframe
   while(millis() - startTime < sampleTime) 
   {
      int micOut;
      uint8_t bytesRead;
      i2s_read((i2s_port_t)i2s_num, &micOut, 4, (size_t*)&bytesRead, 100);
      if( micOut < 8000000) //prevent erroneous readings
      {
        if (micOut > maxAmp)
        {
          maxAmp = micOut; //save only the max reading
        }
        else if (micOut < minAmp)
        {
          minAmp = micOut; //save only the min reading
        }
      }
   }

  PTPAmp = maxAmp - minAmp; // (max amp) - (min amp) = peak-to-peak amplitude
  double micOut_Volts = (PTPAmp * 3.3) / 4096; // Convert ADC into voltage

  //Uncomment this line for help debugging (be sure to also comment out the VUMeter function)
  //Serial.println(PTPAmp); 

  //Return the PTP amplitude to use in the soundLevel function. 
  // You can also return the micOut_Volts if you prefer to use the voltage level.
  return PTPAmp;   
}
