#pragma once

#include <U8g2lib.h>

#define DISPLAY_CONTROLLER SH1106
// #define DISPLAY_CONTROLLER SSD1306
#define DISPLAY_SDA 8 // SDA GPIO
#define DISPLAY_SCL 9 // SCL GPIO

#define DISPLAY_W 128
#define DISPLAY_H 64
#define DISPLAY_ROTATE 0 // can be 0, 90, 180 or 270

typedef enum {PG_PATTERN, PG_TRACK, NUM_PAGES} UIpage_t;

// DO NOT CHANGE
#if (DISPLAY_ROTATE==180)
  #define U8_ROTATE U8G2_R2
#elseif (DISPLAY_ROTATE==90)
  #define U8_ROTATE U8G2_R1
#elseif (DISPLAY_ROTATE==270)
  #define U8_ROTATE U8G2_R3
#else
  #define U8_ROTATE U8G2_R0
#endif

#define W_H_DIV X
#define _U8_CONCAT( ctrl, w, div, h) U8G2_ ## ctrl ## _ ## w ## div ## h ## _NONAME_F_HW_I2C
#define U8_CONCAT( ctrl, w, div, h) _U8_CONCAT( ctrl, w, div, h)
#define U8_OBJECT U8_CONCAT( DISPLAY_CONTROLLER, DISPLAY_W, W_H_DIV, DISPLAY_H)

class OledGUI {
  public:
    OledGUI();
    ~OledGUI() {;};
    void draw();
  
  private:
    const int     _send_tiles = 4;
    int           _block_h = 0;
    int           _block_w = 0;
    int           _cur_xt = 0;
    int           _cur_yt = 0;
    UIpage_t      _cur_page = PG_TRACK;
    int           updateBlock();
    U8_OBJECT*    u8g2;

    // page drawing routines
    void pagePattern();
    void pageTrack();

    // misc info drawing routines
    void drawHeader();
    void drawFooter();

};


