#pragma once

#include <U8g2lib.h>

#define DISPLAY_CONTROLLER SH1106
// #define DISPLAY_CONTROLLER SSD1306
#define DISPLAY_SDA 8 // SDA GPIO
#define DISPLAY_SCL 9 // SCL GPIO

#define DISPLAY_W 128
#define DISPLAY_H 64
#define DISPLAY_ROTATE 0 // can be 0, 90, 180 or 270

#define FONT_TINY     u8g2_font_squeezed_r6_tr 
//#define FONT_TINY     u8g2_font_04b_03_tr
#define FONT_SMALL    u8g2_font_6x13_tr
//#define FONT_MID      u8g2_font_ncenB12_tr
#define FONT_MID      u8g2_font_7x14_tr
#define FONT_BIG      u8g2_font_balthasar_titling_nbp_tr
#define FONT_HUGE     u8g2_font_fub42_tr

typedef enum {PG_ABOUT, PG_PATTERN, PG_TRACK, PG_PARAM, PG_LOADCFG, PG_SAVECFG, PG_SETUP, NUM_PAGES} UIpage_t;

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
#define _U8_CONCAT(ctrl, w, div, h) U8G2_ ## ctrl ## _ ## w ## div ## h ## _NONAME_F_HW_I2C
#define U8_CONCAT(ctrl, w, div, h) _U8_CONCAT(ctrl, w, div, h)
#define U8_OBJECT U8_CONCAT(DISPLAY_CONTROLLER, DISPLAY_W, W_H_DIV, DISPLAY_H)

class OledGUI {
  public:
    OledGUI();
    ~OledGUI() {;};
    void begin();
    void draw();
  
  private:
    const int     _send_tiles = 4;
    int           _block_h = 0;
    int           _block_w = 0;
    int           _cur_xt = 0;
    int           _cur_yt = 0;
    int           _header_h = 12;  
    int           _footer_w = 12;  
    UIpage_t      _cur_page = PG_ABOUT;
    int           updateBlock();
    U8_OBJECT*    u8g2;

    // page drawing routines
    void pageAbout();
    void pagePattern();
    void pageTrack();
    void pageParam();
    void pageLoadCfg();
    void pageSaveCfg();
    void pageSetup();

    // misc info drawing routines
    void drawHeader();
    void drawFooter();

};


