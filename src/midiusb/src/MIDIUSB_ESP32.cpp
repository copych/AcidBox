#include "USB.h"
#if CONFIG_TINYUSB_MIDI_ENABLED

#include "MIDIUSB_ESP32.h"

// Interface counter
enum interface_count {
#if CFG_TUD_MIDI
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
#endif
    ITF_COUNT
};

// USB Endpoint numbers
enum usb_endpoints {
    // Available USB Endpoints: 5 IN/OUT EPs and 1 IN EP
    EP_EMPTY = 0,
#if CFG_TUD_MIDI
    EPNUM_MIDI,
#endif
};

/** TinyUSB descriptors **/
#define TUSB_DESCRIPTOR_ITF_MIDI_LEN                                         \
    (TUD_MIDI_DESC_HEAD_LEN + TUD_MIDI_DESC_JACK_LEN * USB_MIDI_NUM_CABLES + \
     TUD_MIDI_DESC_EP_LEN(USB_MIDI_NUM_CABLES) * 2)
#define TUSB_DESCRIPTOR_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + CFG_TUD_MIDI * TUSB_DESCRIPTOR_ITF_MIDI_LEN)

ESP_EVENT_DEFINE_BASE(ARDUINO_USB_MIDI_EVENTS);

MIDIUSB MidiUSB;

static uint16_t load_midi_descriptor(uint8_t *dst, uint8_t *itf) {
    uint8_t descriptor[TUSB_DESCRIPTOR_ITF_MIDI_LEN] = {
        TUD_MIDI_DESC_HEAD(*itf, 4, USB_MIDI_NUM_CABLES),
        TUD_MIDI_DESC_JACK_DESC(1, 0),
#if USB_MIDI_NUM_CABLES >= 2
        TUD_MIDI_DESC_JACK_DESC(2, 0),
#endif
#if USB_MIDI_NUM_CABLES >= 3
        TUD_MIDI_DESC_JACK_DESC(3, 0),
#endif
        TUD_MIDI_DESC_EP(EPNUM_MIDI, 64, USB_MIDI_NUM_CABLES),
        TUD_MIDI_JACKID_IN_EMB(1),
#if USB_MIDI_NUM_CABLES >= 2
        TUD_MIDI_JACKID_IN_EMB(2),
#endif
#if USB_MIDI_NUM_CABLES >= 3
        TUD_MIDI_JACKID_IN_EMB(3),
#endif
        TUD_MIDI_DESC_EP(0x80 | EPNUM_MIDI, 64, USB_MIDI_NUM_CABLES),
        TUD_MIDI_JACKID_OUT_EMB(1),
#if USB_MIDI_NUM_CABLES >= 2
        TUD_MIDI_JACKID_OUT_EMB(2),
#endif
#if USB_MIDI_NUM_CABLES >= 3
        TUD_MIDI_JACKID_OUT_EMB(3)
#endif
    };
    *itf += 2;  // +2 interface count(Audio Control, Midi Streaming)
    memcpy(dst, descriptor, TUSB_DESCRIPTOR_ITF_MIDI_LEN);
    return TUSB_DESCRIPTOR_ITF_MIDI_LEN;
}

static void usbEventCallback(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
        arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
        switch (event_id) {
            case ARDUINO_USB_STARTED_EVENT:
                HWSerial.println("USB PLUGGED");
                break;
            case ARDUINO_USB_STOPPED_EVENT:
                HWSerial.println("USB UNPLUGGED");
                break;
            case ARDUINO_USB_SUSPEND_EVENT:
                HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n",
                                data->suspend.remote_wakeup_en);
                break;
            case ARDUINO_USB_RESUME_EVENT:
                HWSerial.println("USB RESUMED");
                break;
            default:
                break;
        }
    } else if (event_base == ARDUINO_USB_MIDI_EVENTS) {
        midiEventPacket_t *data = (midiEventPacket_t *)event_data;
        switch (event_id) {
            default:
                HWSerial.printf("MIDI EVENT:  ID=%d, DATA=%d\r\n", event_id,
                                (uint32_t)data);
                break;
        }
    }
}

MIDIUSB::MIDIUSB() {
    tinyusb_enable_interface(USB_INTERFACE_MIDI, TUSB_DESCRIPTOR_ITF_MIDI_LEN,
                             load_midi_descriptor);
    USB.onEvent(usbEventCallback);
}

MIDIUSB::~MIDIUSB(){};

void MIDIUSB::begin() {
    USB.begin();
}

midiEventPacket_t MIDIUSB::read() {
    // The MIDI interface always creates input and output port/jack descriptors
    // regardless of these being used or not. Therefore incoming traffic should
    // be read (possibly just discarded) to avoid the sender blocking in IO
    uint8_t packet[4] = {0};
    midiEventPacket_t data = {0, 0, 0, 0};
    if (tud_midi_packet_read(packet)) {
        memcpy(&data, packet, 4);
    }
    return data;
}
void MIDIUSB::flush(void) {}
void MIDIUSB::sendMIDI(midiEventPacket_t event) {
    tud_midi_packet_write((uint8_t *)&event);
}

#endif /* CONFIG_TINYUSB_MIDI_ENABLED */
