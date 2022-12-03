#if defined SSD1306 || defined SH1106
void oledInit() {
  oled.begin(SCREEN_ADDRESS, true);
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(1);
  oled.setCursor(0, 0);
  oled.println(PROG_NAME);
  oled.println(VERSION);
  oled.display();
}
#endif
