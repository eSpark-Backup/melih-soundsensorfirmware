#include "Wire.h"
//#include <I2S.h>
#include "driver/i2s.h"
#include "freertos/queue.h"
#define sampleTime 1

#define _32K_XN 33
#define LRCKB _32K_XN
#define PIN_I2S_FS LRCKB

#define BCKB 22
#define PIN_I2S_SCK BCKB

#define DOUT2 26
#define PIN_I2S_SD DOUT2

#define I2S_BCK_IO  BCKB
#define I2S_WS_IO   LRCKB
#define I2S_DO_IO   DOUT2
#define I2S_DI_IO   DOUT2

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

static const int i2s_num = 0; // i2s port number

static const i2s_config_t i2s_config = {
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

static const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_IO,
    .ws_io_num = I2S_WS_IO,
    .data_out_num = I2S_DO_IO,
    .data_in_num = I2S_DI_IO
};   
    
void setup() {
  
  Serial.begin(115200);
  Wire.begin(SDA,SCL);
  Serial.println("I2C Initialized");

  i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, &pin_config);
//  I2S.setSckPin(PIN_I2S_SCK);
//  I2S.setFsPin(PIN_I2S_FS);
//  I2S.setDataPin(PIN_I2S_SD);
//  //I2S.setDataInPin(PIN_I2S_SD);
//  
//  // start I2S at 8 kHz with 32-bits per sample
//  if (!I2S.begin(I2S_PHILIPS_MODE, 192000, 32)) {
//    Serial.println("Failed to initialize I2S!");
//    while (1); // do nothing
//  }
//    Serial.println("I2S Initialized");
//    Serial.println(I2S.getDataInPin());

  byte devAddress;
  devAddress = 0x4A;

  updateRegister(devAddress, pg0reg1, 0x40);
  updateRegister(devAddress, pg0reg6, 0x60);
  updateRegister(devAddress, pg0reg7, 0x40);
  updateRegister(devAddress, pg0reg8, 0x40);
  updateRegister(devAddress, pg0reg9, 0x40);
  updateRegister(devAddress, pg0reg11, 0x40);
  Serial.println(readRegister(devAddress, bclkdivreg),HEX);
}

void loop() {
//int samples =0;
//uint8_t bytesRead;
//i2s_read((i2s_port_t)i2s_num, &samples, 4, (size_t*)&bytesRead, 100);
//Serial.println(samples);
Serial.println(findPTPAmp());
//    int sample = I2S.read();
//    Serial.println(findPTPAmp(sample));
//    Serial.println(sample);
    //delay(10);  
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
int findPTPAmp(){
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
