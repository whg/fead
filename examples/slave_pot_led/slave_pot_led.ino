#include "fead.hpp"

enum things {
  ledBrightness,
  pot,
};

CREATE_FEAD_TYPES(Things, things);

ThingsSlave slave;

class ThingHandler : public ThingsSlave::RequestHandler {
public:
  ThingHandler(): mLedPin(5), mPotPin(A3){}
  
  void setupPins() {
    pinMode(mLedPin, OUTPUT);
    pinMode(mPotPin, INPUT);
    pinMode(A1, OUTPUT);
    digitalWrite(A1, LOW);
    pinMode(A5, OUTPUT);
    digitalWrite(A5, HIGH);
  }
  
  ThingsResponse get(const ThingsRequest &request) {
    switch (request.getParam()) {
      case pot:
        return ThingsResponse(pot, mPotValue);
      case ledBrightness:
        return ThingsResponse(ledBrightness, mLedBrightness);
    }
  }

  bool set(const ThingsRequest &request) {
    switch (request.getParam()) {
      case ledBrightness:
        mLedBrightness = request.asInt();
        return ;
    }
    return true;
  }

  void update() {
    analogWrite(mLedPin, mLedBrightness);
    mPotValue = analogRead(mPotPin);
  }

  int mLedPin, mPotPin;
  int mLedBrightness, mPotValue;
};

ThingHandler handler;

void setup() {
  slave.open(0);
  slave.setAddress(3);
  slave.setHandler(&handler);
  slave.setDePin(2);

  handler.setupPins();
}

void loop() {
  slave.update();
  handler.update();
}