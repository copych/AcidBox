#include "gui.h"

OledGUI::OledGUI() {
  u8g2 = new U8_OBJECT(U8_ROTATE, U8X8_PIN_NONE, DISPLAY_SCL, DISPLAY_SDA);
  _block_h = u8g2->getBufferTileHeight();
  _block_w = u8g2->getBufferTileWidth();
}

void OledGUI::begin() {
  u8g2->begin();
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_ncenB12_tr);
  u8g2->drawStr(0, 40, "AcidBox");
  u8g2->sendBuffer();
  delay(200);
}

void OledGUI::draw() {
  if ( this->updateBlock() == 0 ) { // if transmitted then redraw page
    u8g2->clearBuffer();
    drawHeader();
    switch (_cur_page) {
      case PG_PATTERN:
        pagePattern();
        break;
      default:
      case PG_TRACK:
        pageTrack();
    }
    drawFooter();
  }
}

void OledGUI::drawHeader() {
  // header
}
void OledGUI::drawFooter() {
  // footer
}
void OledGUI::pagePattern() {
  // pattern page
}
void OledGUI::pageTrack() {
  // track page
 // u8g2->setDrawColor(1);
  u8g2->drawRFrame(0, 0, DISPLAY_W, DISPLAY_H, 5);
  u8g2->setFont(u8g2_font_ncenB12_tr);
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
