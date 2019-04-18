#include "fead.hpp"

enum vocab {
  LED,
};

CREATE_FEAD_TYPES(Led, vocab);

LedSlave slave;
bool ledState = false;

class RequestHandler : public LedSlave::RequestHandler {
public:
    LedResponse get(const LedRequest &request) {
      switch (request.getParam()) {
        case LED:
          return LedResponse(LED, ledState);
      }
    }

    bool set(const LedRequest &request) {
      switch (request.getParam()) {
        case LED:
          ledState = request.asBool();
          break;
      }
      return true;
    }
};

RequestHandler requestHandler;



void setup() {
  slave.open(0);
  slave.setAddress(3);
  slave.setHandler(&requestHandler);
  slave.setDePin(2);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  slave.update();
  digitalWrite(LED_BUILTIN, ledState);
}
