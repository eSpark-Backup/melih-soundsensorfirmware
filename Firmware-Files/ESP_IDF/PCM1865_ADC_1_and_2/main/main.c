#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/i2s.h"
#include "freertos/queue.h"
static const char *TAG = "i2c-example";

#define ACK_VAL    0x0         /*!< I2C ack value */
#define NACK_VAL   0x1         /*!< I2C nack value */
#define WRITE_BIT  I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT   I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN   0x1     /*!< I2C master will check ack from slave*/

#define MTCK 13
#define MTMS 14

#define SDA MTCK
#define SCL MTMS

#define TIMEOUT_MS		1000
#define DELAY_MS		1000

#define devAddressPCM2 0x4A
#define devAddressPCM1 0x4B
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

#define sampleTime 1

#define PCM1 0
#define PCM2 1
static esp_err_t i2c_master_read_slave_reg(i2c_port_t i2c_num, uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_rd, size_t size);
static esp_err_t i2c_master_write_slave_reg(i2c_port_t i2c_num, uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_wr, size_t size);
uint8_t readRegister(uint8_t deviceAddr, uint8_t registerAddr);
void writeRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t val);
void updateRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t val);
void micSelect(uint8_t micNo,uint8_t devAddress);
int findPTPAmp(uint8_t i2s_num);

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

void app_main() {

    i2s_driver_install((i2s_port_t)i2s_num_0, &i2s_config_1, 0, NULL);
    i2s_set_pin((i2s_port_t)i2s_num_0, &pin_config_1);

    i2s_driver_install((i2s_port_t)i2s_num_1, &i2s_config_2, 0, NULL);
    i2s_set_pin((i2s_port_t)i2s_num_1, &pin_config_2);

	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA,
		.scl_io_num = SCL,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 100000,
	};

    i2c_param_config(I2C_NUM_0, &conf);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    vTaskDelay(DELAY_MS/portTICK_RATE_MS);
    
    updateRegister(devAddressPCM1, pg0reg1, 0x40);
    micSelect(Mic_01,devAddressPCM1);
    updateRegister(devAddressPCM1, pg0reg11, 0x40);

    updateRegister(devAddressPCM2, pg0reg1, 0x40);
    micSelect(Mic_04,devAddressPCM2);
    updateRegister(devAddressPCM2, pg0reg11, 0x40);
    
	while (1) {
		
        // int samples =0;
        // uint8_t bytesRead;
        // i2s_read((i2s_port_t)i2s_num, &samples, 4, (size_t*)&bytesRead, 100);
        // printf("%d\r\n",samples);
        printf("%d,%d\r\n",findPTPAmp(PCM1),findPTPAmp(PCM2));
        vTaskDelay(10/portTICK_RATE_MS);

	}
}

static esp_err_t i2c_master_read_slave_reg(i2c_port_t i2c_num, uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_rd, size_t size)
{
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // first, send device address (indicating write) & register to be read
    i2c_master_write_byte(cmd, ( i2c_addr << 1 ), ACK_CHECK_EN);
    // send register we want
    i2c_master_write_byte(cmd, i2c_reg, ACK_CHECK_EN);
    // Send repeated start
    i2c_master_start(cmd);
    // now send device address (indicating read) & read data
    i2c_master_write_byte(cmd, ( i2c_addr << 1 ) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
static esp_err_t i2c_master_write_slave_reg(i2c_port_t i2c_num, uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // first, send device address (indicating write) & register to be written
    i2c_master_write_byte(cmd, ( i2c_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // send register we want
    i2c_master_write_byte(cmd, i2c_reg, ACK_CHECK_EN);
    // write the data
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

uint8_t readRegister(uint8_t deviceAddr, uint8_t registerAddr)
{
    uint8_t regVal=0;
   i2c_master_read_slave_reg(I2C_NUM_0,deviceAddr,registerAddr,&regVal,1); 
   return regVal;
}

void writeRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t val)
{  
    i2c_master_write_slave_reg(I2C_NUM_0,deviceAddr,registerAddr,&val,1);
}

void updateRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t val)
{
    uint8_t tempVal=0;
   ESP_LOG_BUFFER_HEX("Register Address ",&registerAddr,1);
   tempVal = readRegister(deviceAddr, registerAddr);

   ESP_LOG_BUFFER_HEX("Value Before Updation ",&tempVal,1);
   vTaskDelay(50/portTICK_RATE_MS);

   writeRegister(deviceAddr, registerAddr, val);
   vTaskDelay(50/portTICK_RATE_MS);

    tempVal = readRegister(deviceAddr, registerAddr);
   ESP_LOG_BUFFER_HEX("Value After Updation : ",&tempVal,1);
   vTaskDelay(50/portTICK_RATE_MS);
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
   unsigned long startTime= esp_timer_get_time()/1000;  // Start of sample window
   unsigned int PTPAmp = 0; 

// Signal variables to find the peak-to-peak amplitude
   int maxAmp = 0;
   int minAmp = 0;

// Find the max and min of the mic output within the 50 ms timeframe
   while((esp_timer_get_time()/1000) - startTime < sampleTime) 
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

  return PTPAmp;   
}