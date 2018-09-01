#ifndef protocentral_apa102_h
#define protocentral_apa102_h

#define MISO 19
#define MOSI 26
#define SCLK  25
#define CS   22

#define DATA_PIN   26
#define CLOCK_PIN   25

#define LOW 0
#define HIGH 1
#define NUM_LEDS 144

#define WREG 0x00

#define ANIMATION 0
#define INTENSITY 1

void protocentral_init_apa102(void);
void apa102_show(uint8_t numof_leds, uint32_t rbg_val);
void apa102_setcolours(uint8_t numof_leds, uint32_t rbg_val, uint8_t brightness);
void transfer(uint8_t b);
void init_apa102_spi(void);
void blink_apa102(void);
void blink_heart(uint8_t pattern);

inline uint32_t setrgb_val(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t c;

  c  = (uint32_t) r<<16;
  c |= (uint32_t)g<<8;
  c |= (uint32_t)b;

  return c;
}

#endif