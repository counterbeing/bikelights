#include <NRFLite.h>
#include <SPI.h>
#include <stdint.h>
#include "config.h"

struct RadioPacket          // Any packet up to 32 bytes can be sent.
{                           // 0 - bit count (256 max)
  uint8_t SHARED_SECRET;    // 8
  uint16_t senderId;        // 24
  int32_t rotaryPosition;   // 56
  uint8_t animationId;      // 64
                            // uint32_t keyframe;
                            // ... 200
};

class Radio {
 private:
  RadioPacket _incomingRadioPacket;
  RadioPacket _outboundRadioPacket;
  NRFLite _radio;
  bool radioAlive = false;
  MyKnob &knob;
  int &animation_index;
  SwarmMap &swarmMap;
  const static uint8_t SHARED_RADIO_ID = 1;
  const static uint8_t PIN_RADIO_CE = 7;   // 7 on PCBs 1.3, was 6 on 1.1
  const static uint8_t PIN_RADIO_CSN = 6;  // 6 on PCBs 1.3, was 7 on 1.1
  // const static uint8_t PIN_RADIO_CE = 9; // mac protoboard
  // const static uint8_t PIN_RADIO_CSN = 10; // mac protoboard
  // const static uint8_t PIN_RADIO_CE = 10; // mac makefun
  // const static uint8_t PIN_RADIO_CSN = 9; // mac makefun
  const static uint8_t SHARED_SECRET = 42;
  uint16_t RADIO_ID = random(1024,65532);
  int previousAnimationIndex;
  int previousRotaryPosition = -1;
  long lastIntervalTime = millis();

  void checkRadioReceive() {
    while (_radio.hasData()) {
      _radio.readData(&_incomingRadioPacket);
      if (_incomingRadioPacket.SHARED_SECRET != SHARED_SECRET) {
        if (RADIO_DEBUG) {
          Serial.print("got a packet with incorrect SHARED_SECRET: ");
          Serial.println(_incomingRadioPacket.SHARED_SECRET);
        }
        return;
      }
      swarmMap.logPacketFrom(_incomingRadioPacket.senderId);
      if (RADIO_DEBUG) {
        Serial.println("------INCOMING---------");
        Serial.print("SHARED_SECRET: ");
        Serial.println(_incomingRadioPacket.SHARED_SECRET);
        Serial.print("rotaryPosition: ");
        Serial.println(_incomingRadioPacket.rotaryPosition);
        Serial.print("animationId: ");
        Serial.println(_incomingRadioPacket.animationId);
        Serial.print("senderId: ");
        Serial.println(_incomingRadioPacket.senderId);
      }
      if (_incomingRadioPacket.senderId == 255) { // medallion controller
        // disable rotary & play green pulse animation for a while
        return;
      }
      if (stateChanged()) {
        knob.set(_incomingRadioPacket.rotaryPosition);
        animation_index = _incomingRadioPacket.animationId;
        lastIntervalTime = millis();
      }
    }
  }

  void checkRadioSend() {
    // Stay silent for the first moment in order to try to join a group rather
    // than telling all others to change.
    if (millis() < 1500) {
      return;
    }
    if (RADIO_DEBUG) {
      Serial.println("--- Sending Data");
    }
    _outboundRadioPacket.rotaryPosition = knob.get();
    _outboundRadioPacket.animationId = animation_index;

    _radio.send(SHARED_RADIO_ID, &_outboundRadioPacket,
                sizeof(_outboundRadioPacket), NRFLite::NO_ACK);
  }

  bool stateChanged() {
    int currentRotaryPosition = knob.get();
    return !(_incomingRadioPacket.rotaryPosition == currentRotaryPosition &&
             _incomingRadioPacket.animationId == animation_index);
  }

  bool runAfterInterval(int interval) {
    long nextRunTime = lastIntervalTime + interval;
    long current = millis();
    if (current > nextRunTime) {
      lastIntervalTime = current;
      return true;
    }
    return false;
  }

  // constructor declaration vvvvvvvvvvvv : vvvvvvvvvvv -member initializer
  // list- vvvvvvvv
 public:  //                                             vvvvv <--init named
          //                                             identifier (knob) with
          //                                             these params (knob_)
  // Radio(MyKnob &knob_, int &animation_index_)
  //     : knob(knob_), animation_index(animation_index_) {}
  Radio(MyKnob &knob_, int &animation_index_, SwarmMap &swarmMap_)
      : knob(knob_), animation_index(animation_index_), swarmMap(swarmMap_) {}
  void setup() {
    _outboundRadioPacket.SHARED_SECRET = SHARED_SECRET;
    _outboundRadioPacket.senderId = RADIO_ID;
    if (RADIO_DEBUG) {
      Serial.print("Picking random radio id: ");
      Serial.println(RADIO_ID);
    }
    pinMode(14, INPUT_PULLUP);
    if (!_radio.init(SHARED_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN)) {
      radioAlive = false;
      if (RADIO_DEBUG) {
        Serial.println("radio fail");
      }
    } else {
      radioAlive = true;
      if (RADIO_DEBUG) {
        Serial.println("radio ok");
      }
    }
  }
  void check() {
    // int newRotaryPosition = knob.get();
    knob.get();
    if (radioAlive) {
      checkRadioReceive();
      if (knob.manuallyChanged()) {
        checkRadioSend();
      }
    }
  }
};
