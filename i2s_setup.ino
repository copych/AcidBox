
#if ESP_ARDUINO_VERSION_MAJOR < 3 
// versions prior to 3.0.0
#include "driver/i2s.h"

const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

#ifdef USE_INTERNAL_DAC
void i2sInit() {
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate =  SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    .dma_buf_count = DMA_NUM_BUF,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = false
  };

  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
  i2s_set_pin(i2s_num, NULL);
  i2s_zero_dma_buffer(i2s_num);
}
#else
void i2sInit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX ),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S ),
//   .communication_format =  (i2s_comm_format_t)(I2S_LSB_FORMAT), // VDA1334
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    .dma_buf_count = DMA_NUM_BUF,
    .dma_buf_len = DMA_BUF_LEN,
    .use_apll = true,
  };

  i2s_pin_config_t i2s_pin_config = {
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num =  I2S_WCLK_PIN,
    .data_out_num = I2S_DOUT_PIN
  };

  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);

  i2s_set_pin(i2s_num, &i2s_pin_config);
  i2s_zero_dma_buffer(i2s_num);

  DEBF("I2S is started: BCK %d, WCK %d, DAT %d\r\n", I2S_BCLK_PIN, I2S_WCLK_PIN, I2S_DOUT_PIN);
}
#endif

void i2sDeinit() {
  i2s_zero_dma_buffer(i2s_num);
  i2s_driver_uninstall(i2s_num);
}


inline void i2s_output () {
  // now out_buf is ready, output
//  if (processing) {
  #ifdef USE_INTERNAL_DAC
    for (int i=0; i < DMA_BUF_LEN; i++) {      
      out_buf[current_out_buf]._unsigned[i*2] = (uint16_t)(127.0f * ( fast_shape( mix_buf_l[current_out_buf][i]) + 1.0f)) << 8U; // 256 output levels is way to little
      out_buf[current_out_buf]._unsigned[i*2+1] = (uint16_t)(127.0f * ( fast_shape( mix_buf_r[current_out_buf][i]) + 1.0f)) << 8U ; // maybe you'll be lucky to fully use this range
    }
    i2s_write(i2s_num, out_buf[current_out_buf]._unsigned, sizeof(out_buf[current_out_buf]._unsigned), &bytes_written, portMAX_DELAY);
  #else
    for (int i=0; i < DMA_BUF_LEN; i++) {      
      out_buf[current_out_buf]._signed[i*2] = 0x7fff * (float)(( mix_buf_l[current_out_buf][i])) ; 
      out_buf[current_out_buf]._signed[i*2+1] = 0x7fff * (float)(( mix_buf_r[current_out_buf][i])) ;
    }
    i2s_write(i2s_num, out_buf[current_out_buf]._signed, sizeof(out_buf[current_out_buf]._signed), &bytes_written, portMAX_DELAY);
  #endif
//  }
}


#else
  // Arduino core 3.0.0 and up
#include <ESP_I2S.h>
//const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

I2SClass I2S;

void i2sInit() {
  pinMode(I2S_BCLK_PIN, OUTPUT);
  pinMode(I2S_DOUT_PIN, OUTPUT);
  pinMode(I2S_WCLK_PIN, OUTPUT);
  I2S.setPins(I2S_BCLK_PIN, I2S_WCLK_PIN, I2S_DOUT_PIN); //SCK, WS, SDOUT, SDIN, MCLK
  I2S.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);

  DEBF("I2S is started: BCK %d, WCK %d, DAT %d\r\n", I2S_BCLK_PIN, I2S_WCLK_PIN, I2S_DOUT_PIN);
}

void i2sDeinit() {
  I2S.end();
}

static inline void i2s_output () {
// now out_buf is ready, output
  for (int i=0; i < DMA_BUF_LEN; i++) {
      out_buf[current_out_buf]._signed[i*2] = 0x7fff * (float)(( mix_buf_l[current_out_buf][i])) ; 
      out_buf[current_out_buf]._signed[i*2+1] = 0x7fff * (float)(( mix_buf_r[current_out_buf][i])) ;
   // if (i%4==0) DEBUG(out_buf[out_buf_id][i*2]);
   
   //if (out_buf[out_buf_id][i*2]) DEBF(" %d\r\n ", out_buf[out_buf_id][i*2]);
  }
  I2S.write((uint8_t*)out_buf[current_out_buf]._signed, sizeof(out_buf[current_out_buf]._signed));
}


#endif
