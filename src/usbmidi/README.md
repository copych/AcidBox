# Arduino USB-MIDI Transport 
[![arduino-library-badge](https://www.ardu-badge.com/badge/USB-MIDI.svg?)](https://www.ardu-badge.com/USB-MIDI) 

This library implements the USB-MIDI transport layer for the [FortySevenEffects Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) and uses the underlying [Arduino MIDIUSB](https://github.com/arduino-libraries/MIDIUSB) library (so only devices working with MIDIUSB, will work here).

Alternative library: see also [Adafruit's TinyUSB Library for Arduino](https://github.com/adafruit/Adafruit_TinyUSB_Arduino) for ESP32, mbed_rp2040, ArduinoCore-samd and Pico

## Installation

<img width="800" alt="Screenshot 2020-04-25 at 09 42 19" src="https://user-images.githubusercontent.com/4082369/80274232-42810b80-86d9-11ea-94f6-643bf5ade5be.png">

This library depends on the [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) and [Arduino's MIDIUSB](https://github.com/arduino-libraries/MIDIUSB).

When installing this library from the Arduino IDE, both will be downloaded and installed in the same directory as this library. (Thanks to the `depends` clause in `library.properties`)

When manually installing this library, you have to manually download [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) and [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB) from github and install it in the same directory as this library - without these additional installs, this library will not be able to compile. 

## Usage
### Basic / Default
```cpp
#include <USB-MIDI.h>
...
USBMIDI_CREATE_DEFAULT_INSTANCE();
...
void setup()
{
   MIDI.begin(1);
...
void loop()
{
   MIDI.read();
```
will create a instance named `MIDI` (transport instance named `__usbMIDI`) and is by default connected to cable number 0 - and listens to incoming MIDI on channel 1.

### Modified
```cpp
#include <USB-MIDI.h>
...
USBMIDI_CREATE_INSTANCE(4);
```
will create a instance named `MIDI` (transport instance named `__usbMIDI`) and is connected to cable number 4.

### Advanced
```cpp
#include <USB-MIDI.h>
...
USBMIDI_NAMESPACE::usbMidiTransport usbMIDI2(5);
MIDI_NAMESPACE::MidiInterface<USBMIDI_NAMESPACE::usbMidiTransport> MIDI2((USBMIDI_NAMESPACE::usbMidiTransport&)usbMIDI2);
```
will create a instance named `usbMIDI2` (and underlaying MIDI object `MIDI2`) and is by default connected to cable number 5.

## Tested boards / modules
- Arduino Leonardo
- Teensy 4.1 (incl MIDI, MIDIx4 and MIDIx16)

### Boards / modules in development (help needed)
- Arduino NANO 33 BLE
- nRF52832 Bluefruit Feather

## Memory usage
The library does not add additional buffers and is extremely efficient and has a small memory footprint.

## Other Transport protocols:
The libraries below  the same calling mechanism (API), making it easy to interchange the transport layer.
- [Arduino AppleMIDI Transport](https://github.com/lathoub/Arduino-AppleMIDI-Library)
- [Arduino ipMIDI  Transport](https://github.com/lathoub/Arduino-ipMIDI)
- [Arduino BLE-MIDI  Transport](https://github.com/lathoub/Arduino-BLE-MIDI)
