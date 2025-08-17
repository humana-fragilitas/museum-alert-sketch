#include "pin_setup.h"

void pinSetup() {

  pinMode(Pins::Trigger, OUTPUT);
  pinMode(Pins::Alarm, OUTPUT);
  pinMode(Pins::WiFi, OUTPUT);
  pinMode(Pins::Status, OUTPUT);
  pinMode(Pins::Echo, INPUT);
  pinMode(Pins::ResetButton, INPUT_PULLDOWN);

  digitalWrite(Pins::Trigger, LOW);
  digitalWrite(Pins::Alarm, LOW);
  digitalWrite(Pins::WiFi, LOW);
  digitalWrite(Pins::Status, LOW);

}
