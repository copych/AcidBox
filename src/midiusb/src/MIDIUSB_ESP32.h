#pragma once

#include "sdkconfig.h"
#if CONFIG_TINYUSB_MIDI_ENABLED

#include <stdint.h>
#include <inttypes.h>
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "Stream.h"

#include "USB.h"

#include "src/midiusb/src/MIDIUSB_Defs.h"

#include "HardwareSerial.h"
#include "HWCDC.h"
#include "esp32-hal-tinyusb.h"

#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial0
#define USBSerial Serial
#else
#define HWSerial Serial
#endif

ESP_EVENT_DECLARE_BASE(ARDUINO_USB_MIDI_EVENTS);
#define USB_MIDI_NUM_CABLES 1
#if (USB_MIDI_NUM_CABLES <= 0) || (USB_MIDI_NUM_CABLES > 3)
#error "USB_MIDI_NUM_CABLES must be 1, 2 or 3"
#endif

typedef enum {
    ARDUINO_USB_MIDI_ANY_EVENT = ESP_EVENT_ANY_ID,
} arduino_usb_midi_event_t;

class MIDIUSB
{
   public:
    MIDIUSB(); 
    ~MIDIUSB();

    // MIDIUSB method
    void begin(); 
    midiEventPacket_t read(); 
    void flush(void); 
    void sendMIDI(midiEventPacket_t event); 

};

extern MIDIUSB MidiUSB;

#endif /* CONFIG_TINYUSB_MIDI_ENABLED */
