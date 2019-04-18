
class BubbleTree : public fead::Slave<vocab>::RequestHandler {
  public:
    BubbleTree(int pin, int pin2, int tempPin) {
      mLedPin = pin;
      mLedState = LOW;
      mLedPin2 = pin2;
      mLedState2 = LOW;
      mTempPin = tempPin;
      mTempVal = 0.0;
      mAddress = EEPROM.read(0);

      pinMode(mLedPin, OUTPUT);
      pinMode(mLedPin2, OUTPUT);
      pinMode(mTempPin, INPUT);
    }

    void update() {
      digitalWrite(mLedPin, mLedState);
      digitalWrite(mLedPin2, mLedState2);
      mTempVal = analogRead(mTempPin) / 10.23;
    }

    fead::Response<vocab> get(const fead::Request<vocab> &request) {
      if (request.getParam() == LED) {
        return fead::Response<vocab>(LED, mLedState);
      }
      else if (request.getParam() == LED2) {
        return fead::Response<vocab>(LED2, mLedState2);
      }
      else if (request.getParam() == TEMP) {
        return fead::Response<vocab>(TEMP, mTempVal);
      }
      else if (request.getParam() == setAddress) {
        return fead::Response<vocab>(setAddress, mAddress);
      }
    }

    bool set(const fead::Request<vocab> &request) {
      vocab param = request.getParam();
      if (param == LED) {
        mLedState = request.asBool();
      }
      else if (param == LED2) {
        mLedState2 = request.asBool();
      }
      else if (param == setAddress) {
        mAddress = request.asInt();
        EEPROM.update(0, mAddress);
      }
      return true;
    }

    int mLedPin , mLedPin2, mTempPin, mAddress, mReboot;
    float mTempVal;
    bool mLedState, mLedState2;
};
