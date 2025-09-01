#include "pin_setup.h"

void pinSetup() noexcept {

  constexpr int outputPins[] = {
    Pins::Trigger,
    Pins::Alarm,
    Pins::WiFi,
    Pins::Status
  };

  for (const auto pin : outputPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  pinMode(Pins::Echo, INPUT);
  pinMode(Pins::ResetButton, INPUT_PULLDOWN);

}
