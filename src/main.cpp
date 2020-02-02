#include <Arduino.h>

#include "IRLibAll.h"

const unsigned long HDMI_SWITCH_ONE = 0x1FE58A7;
const unsigned long HDMI_SWITCH_TWO = 0x1FE807F;
const unsigned long HDMI_SWITCH_THREE = 0x1FE40BF;
const unsigned long HDMI_SWITCH_FOUR = 0x1FEC03F;
const unsigned long HDMI_SWITCH_FIVE = 0x1FEA05F;

const unsigned long SONY_BLUETOOTH_INPUT = 581908;
const unsigned long SONY_BD_DVD_INPUT = 426260;
const unsigned long SONY_MEDIA_BOX_INPUT = 8470;
const unsigned long SONY_SAT_CATV_INPUT = 24589;
const unsigned long SONY_GAME_INPUT = 7948;
const unsigned long SONY_CD_INPUT = 21004;
const unsigned long SONY_TV_INPUT = 11020;
const unsigned long SONY_FM_INPUT = 3084;

// if we receive two of the 
const int DOUBLE_TAP_DELAY_MILLIS = 500;
const int SONY_DEBOUNCE_MILLIS = 200;

unsigned long lastSignalReceived;
unsigned long lastSignalProtocolNum;
unsigned long lastSignalReceivedAtMillis = 0;

bool hasReceivedFirstDoubleTapSignal = false;

//Create a receiver object to listen on pin 2
IRrecvPCI myReceiver(2);
 
//Create a decoder object 
IRdecode myDecoder;   
 
void blinkLed(long durationMillis);
boolean isSonyIrSignalWeCareAbout(const unsigned long irSignal);

void setup() {
  Serial.begin(9600);
  delay(2000); while (!Serial); //delay for Leonardo
  myReceiver.enableIRIn(); // Start the receiver
  lastSignalReceivedAtMillis = millis();

  // set up LED pins
  pinMode(12, OUTPUT);
  
  Serial.println(("Ready to receive IR signals"));
}
 
void loop() {
  unsigned long now = millis();
  if (now - lastSignalReceivedAtMillis > DOUBLE_TAP_DELAY_MILLIS
      && isSonyIrSignalWeCareAbout(lastSignalReceived)
      && hasReceivedFirstDoubleTapSignal) {
    lastSignalReceived = 0;
    lastSignalReceivedAtMillis = now;
    Serial.println("\n\ncaught a single tap\n\n");
    myReceiver.enableIRIn();
    hasReceivedFirstDoubleTapSignal = false;
    blinkLed(100);
    return;
  }

  //Continue looping until you get a complete signal received
  if (myReceiver.getResults()) {

    // delay 50ms because of sony triple codes -- this way we'll receive the first code
    // and drop the other two on the ground.
    delay(50);
    myDecoder.decode();           //Decode it
    hasReceivedFirstDoubleTapSignal = true;

    // throw away "0" codes that the library can't parse
    if (myDecoder.value == 0) {
      myReceiver.enableIRIn();
      return;
    }
  
    // see if we're getting one of the three copies of the message that sony
    // remotes send for each button press.
    //
    // if the current code is same as the last received code, and it's a sony code,
    // and it came within the SONY_DEBOUNCE_MILLIS timeframe, return.
    if (isSonyIrSignalWeCareAbout(myDecoder.value)
        && myDecoder.value == lastSignalReceived
        && now - lastSignalReceivedAtMillis < SONY_DEBOUNCE_MILLIS) {
      myReceiver.enableIRIn();
      return;
    }
    if (isSonyIrSignalWeCareAbout(myDecoder.value)
        && myDecoder.value == lastSignalReceived
        && now - lastSignalReceivedAtMillis < DOUBLE_TAP_DELAY_MILLIS
        && hasReceivedFirstDoubleTapSignal) {
      Serial.println("\n\ncaught a double tap! " + String(myDecoder.value) + "\n\n");
      hasReceivedFirstDoubleTapSignal = false;
      
      // delay 100ms because of sony triple codes -- this way we'll receive the first code
      // and drop the other two on the ground.
      // without this, the next call to "loop" will hear the next sony triple code repeat and
      // consider it a single-tap
      blinkLed(100);
      delay(100);
      blinkLed(100);
    }
    lastSignalReceivedAtMillis = now;
    lastSignalReceived = myDecoder.value;
    myReceiver.enableIRIn();
  }

}

void blinkLed(long durationMillis) {
  digitalWrite(12, HIGH);
  delay(durationMillis);
  digitalWrite(12, LOW);
}

boolean isSonyIrSignalWeCareAbout(const unsigned long irSignal) {
  switch (irSignal) {
    case SONY_BLUETOOTH_INPUT:
    case SONY_BD_DVD_INPUT:
    case SONY_MEDIA_BOX_INPUT:
    case SONY_SAT_CATV_INPUT:
    case SONY_GAME_INPUT:
    case SONY_CD_INPUT:
    case SONY_TV_INPUT:
    case SONY_FM_INPUT:
      return true;
    default:
      return false; 
  }
}