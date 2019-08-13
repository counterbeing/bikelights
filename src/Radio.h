#include <stdint.h>
#include <NRFLite.h>
#include <SPI.h>

struct RadioPacket // Any packet up to 32 bytes can be sent.
{
    uint8_t SHARED_SECRET;
    uint8_t senderId;
    uint32_t rotaryPosition;
    uint8_t animationId;
    // uint32_t keyframe;
};

class Radio
{
private:
    RadioPacket _incomingRadioPacket;
    RadioPacket _outboundRadioPacket;
    NRFLite _radio;
    bool radioAlive = false;
    MyKnob &knob;
    int &animation_index;
    const static uint8_t SHARED_RADIO_ID = 1;
    // const static uint8_t PIN_RADIO_CE = 6; // will be 6 on PCBs, was 9
    // const static uint8_t PIN_RADIO_CSN = 7; // will be 7 on PCBs, was 10
    const static uint8_t PIN_RADIO_CE = 9; // mac protoboard
    const static uint8_t PIN_RADIO_CSN = 10; // mac protoboard
    const static uint8_t SHARED_SECRET = 42;
    uint8_t RADIO_ID = random();
    int previousAnimationIndex;
    int previousRotaryPosition = -1;
    long lastIntervalTime = millis();

    void checkRadioReceive()
    {
        while (_radio.hasData())
        {
            _radio.readData(&_incomingRadioPacket);
            Serial.println("------INCOMING---------");
            // Serial.print("SHARED_SECRET: ");
            // Serial.println(_incomingRadioPacket.SHARED_SECRET);
            // Serial.print("rotaryPosition: ");
            // Serial.println(_incomingRadioPacket.rotaryPosition);
            // Serial.print("animationId: ");
            // Serial.println(_incomingRadioPacket.animationId);
            Serial.print("senderId: ");
            Serial.println(_incomingRadioPacket.senderId);
            if (stateChanged())
            {
                knob.set(_incomingRadioPacket.rotaryPosition);
                animation_index = _incomingRadioPacket.animationId;
                lastIntervalTime = millis();
            }
        }
    }

    void checkRadioSend()
    {
        // Serial.println("--- Sending Data");
        _outboundRadioPacket.rotaryPosition = knob.get();
        _outboundRadioPacket.animationId = animation_index;

        _radio.send(SHARED_RADIO_ID, &_outboundRadioPacket, sizeof(_outboundRadioPacket), NRFLite::NO_ACK);
    }

    bool stateChanged()
    {
        int currentRotaryPosition = knob.get();
        return !(_incomingRadioPacket.rotaryPosition == currentRotaryPosition &&
                 _incomingRadioPacket.animationId == animation_index);
    }

    bool runAfterInterval(int interval)
    {
        long nextRunTime = lastIntervalTime + interval;
        long current = millis();
        if (current > nextRunTime)
        {
            lastIntervalTime = current;
            return true;
        }
        return false;
    }

public:
    Radio(MyKnob &knob_, int &animation_index_) : knob(knob_), animation_index(animation_index_) {}
    void setup()
    {
        _outboundRadioPacket.SHARED_SECRET = SHARED_SECRET;
        _outboundRadioPacket.senderId = RADIO_ID;
        Serial.print("Picking random radio id: ");
        Serial.println(RADIO_ID);
        pinMode(14, INPUT_PULLUP);
        if (!_radio.init(SHARED_RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
        {
            radioAlive = false;
            Serial.println("radio fail");
        }
        else
        {
            radioAlive = true;
            Serial.println("radio ok");
        }
    }
    void check()
    {
        int newRotaryPosition = knob.get();
        if (radioAlive)
        {
            checkRadioReceive();
            if (knob.manuallyChanged() || runAfterInterval(1000))
            {
                checkRadioSend();
            }
        }
    }
};
