#include "ADS131M08.h"
#include "esp_timer.h"

// Variables to find the peak-to-peak amplitude of AUD output
const int sampleTime = 50; 

//previous VU value
int preValue = 0; 

//Joshua Brewster, Jacob Tinkhauser

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

int CLKOUT  =                 8192000; //XTAL speed (50% duty cycle PWM)

ADS131M08 adc(CS_PIN,XTAL_PIN,DRDY_PIN,8192000);

unsigned long long sampleNum = 0;
unsigned long long currentMicros = 0;
char outputarr[128];
int32_t channelArr[8];


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Setting SPI");
  adc.spi = new SPIClass(VSPI);
  Serial.println("Booting SPI");
  adc.init(CLKOUT);

  delay(100);


  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);

  adc.writeReg(ADS131_CLOCK,0b1111111100011010); //Clock register (page 55 in datasheet)
  /*CLOCK REG SETTINGS
   * Bytes 15-8: ADC Channel enable/disable
   * Byte 7: Crystal disable 
   * Byte 6: External Reference Enable
   * Byte 5: Reserved
   * Bytes 4-2: Modulator Oversampling 000 = 128 OSR (32ksps), 111 = 16256 OSR (250sps)
   * Bytes 1-0: Power mode selections 11 or 10 = high resolution, 01 = low power, 00 = very low power
   */

  //adc.writeReg(ADS131_CFG,0b0000000000000000);

  //DC Block Filter settings:
  //adc.writeReg(ADS131_THRSHLD_LSB,0b0000000000001010);

  //Channel settings
  //adc.writeReg(ADS131_CH0_CFG,0b0000000000000000);

  //Gain settings, 1-128 (increasing by factor of 2)
  adc.setGain(32);

   
  uint16_t clkreg = adc.readReg(ADS131_CLOCK);
  Serial.print("CLOCK: ");
  Serial.println(clkreg,BIN);
  

  
  uint16_t gainreg = adc.readReg(ADS131_GAIN1);
  Serial.print("GAIN1: ");
  Serial.println(gainreg, BIN);
  
  //adc.globalChop(true,2);
  
  uint16_t id = adc.readReg(ADS131_ID);
  Serial.print("ID: ");
  Serial.println(id, BIN);
  
  uint16_t stat = adc.readReg(ADS131_STATUS);
  Serial.print("Status: ");
  Serial.println(stat, BIN);

  uint16_t Mode = adc.readReg(ADS131_MODE);
  Serial.print("Mode: ");
  Serial.println(Mode, BIN);

  Serial.println("Starting in 3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);


  currentMicros = esp_timer_get_time();

}


void loop() {
  
    findPTPAmp();
    
    sprintf(outputarr, "%d,%d,%d,%d,%d,%d,%d,%d\r\n", 
              channelArr[0], 
              channelArr[1], 
              channelArr[2],
              channelArr[3],
              channelArr[4],
              channelArr[5],
              channelArr[6],
              channelArr[7]
              );
              
    Serial.print(outputarr);
//    VUMeter(micOutput);
}

// Find the Peak-to-Peak Amplitude Function
void findPTPAmp(void){
// Time variables to find the peak-to-peak amplitude
   unsigned long startTime= millis();  // Start of sample window
   unsigned int PTPAmp[8] = {0};
   int micOut[8]; 

// Signal variables to find the peak-to-peak amplitude
   int maxAmp[8];
   std::fill_n(maxAmp, 8, 0);
   int minAmp[8] ;
   std::fill_n(minAmp, 8, 0);

// Find the max and min of the mic output within the 50 ms timeframe
   while(millis() - startTime < sampleTime) 
   {
    adc.readAllChannels(channelArr);
    for (uint8_t i = 0; i < 8; i++)
    {
      micOut[i] = channelArr[i];
      if( micOut[i] < 16777215) //prevent erroneous readings
      {
        if (micOut[i] > maxAmp[i])
        {
          maxAmp[i] = micOut[i]; //save only the max reading
        }
        else if (micOut[i] < minAmp[i])
        {
          minAmp[i] = micOut[i]; //save only the min reading
        }
      }
    }     
   }

  for (uint8_t i = 0; i < 8; i++){
    PTPAmp[i] = maxAmp[i] - minAmp[i]; // (max amp) - (min amp) = peak-to-peak amplitude 
  }
//  double micOut_Volts = (PTPAmp * 3.3) / 4096; // Convert ADC into voltage

  //Uncomment this line for help debugging (be sure to also comment out the VUMeter function)
  //Serial.println(PTPAmp); 

  //Return the PTP amplitude to use in the soundLevel function. 
  // You can also return the micOut_Volts if you prefer to use the voltage level.
  for (uint8_t i = 0; i < 8; i++){
    channelArr[i] = PTPAmp[i];
  }    
}

//// Volume Unit Meter function: map the PTP amplitude to a volume unit between 0 and 10.
//int VUMeter(int micAmp){
//
//  // Map the mic peak-to-peak amplitude to a volume unit between 0 and 10.
//   // Amplitude is used instead of voltage to give a larger (and more accurate) range for the map function.
//   // This is just one way to do this -- test out different approaches!
//  int fill = map(micAmp, 23, 400000, 0, 10); 
//
//  // Only print the volume unit value if it changes from previous value
//  while(fill != preValue)
//  {
//    Serial.println(fill);
//    preValue = fill;
//  }
//}
