#include "pin_setup.h"

void pinSetup() noexcept {

  // Configure output pins
  constexpr int outputPins[] = {
   Pins::Trigger,
   Pins::Alarm,
   Pins::WiFi,
   Pins::Status
  };

  for (const auto pin : outputPins) {
   pinMode(pin, OUTPUT);
  }

  // Configure input pins
  pinMode(Pins::Echo, INPUT);
  pinMode(Pins::ResetButton, INPUT_PULLDOWN);

  // Initialize output pins to LOW state
  for (const auto pin : outputPins) {
   digitalWrite(pin, LOW);
  }

}
