#include "gui.h"

OledGUI::OledGUI() {
  u8g2 = new U8_OBJECT(U8_ROTATE, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);
  _block_h = u8g2->getBufferTileHeight();
  _block_w = u8g2->getBufferTileWidth();
}

void OledGUI::begin() {
  u8g2->begin();
  u8g2->clearBuffer();
  _cur_page = PG_ABOUT;
  this->pageAbout();
  u8g2->sendBuffer();
  delay(1000);
  
}

void OledGUI::draw() {
  if ( this->updateBlock() == 0 ) { // if transmitted then redraw page
    u8g2->clearBuffer();
    switch (_cur_page) {
      case PG_ABOUT:
        drawHeader();
        pagePattern();
        drawFooter();
        break;
      case PG_PATTERN:
        drawHeader();
        pagePattern();
        drawFooter();
        break;
      default:
      case PG_TRACK:
        drawHeader();
        pageTrack();
        drawFooter();
    }
  }
}

void OledGUI::drawHeader() {
  // header
}

void OledGUI::drawFooter() {
  // footer
}

void OledGUI::pageAbout() {
  // track page
 // u8g2->setDrawColor(1);
  u8g2->drawRFrame(0, 0, DISPLAY_W, DISPLAY_H, 5);
  u8g2->setFont(FONT_MID);
  u8g2->drawStr(10, 40, "AcidBox");
}

void OledGUI::pagePattern() {
  // pattern page
}

void OledGUI::pageTrack() {
  // track page
 // u8g2->setDrawColor(1);
  u8g2->drawRFrame(0, 0, DISPLAY_W, DISPLAY_H, 5);
  u8g2->setFont(FONT_MID);
  u8g2->drawStr(10, 40, "AcidBox");
}


// to call internally: transmits only a fraction of a buffer, so it shouldn't take long
int OledGUI::updateBlock() {
  u8g2->updateDisplayArea(_cur_xt, _cur_yt, _send_tiles, 1);
  _cur_xt += _send_tiles;
  if(_cur_xt >= _block_w) {
    _cur_xt = 0;
    _cur_yt++;
  }
  _cur_yt %= _block_h;
  return _cur_xt + _cur_yt;
}
