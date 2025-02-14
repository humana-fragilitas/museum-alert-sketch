#ifndef MACROS_H
#define MACROS_H

  /**
   * Uncomment/comment the following line to enable debugging mode
   * Alternatively, enable debugging via Arduino CLI: arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DDEBUG
   */
  #define DEBUG 

  #ifdef DEBUG
    #define DEBUG_PRINT(x)    do { Serial.print("[debug] "); Serial.print(x); } while(0)
    #define DEBUG_PRINTLN(x)  do { Serial.print("[debug] "); Serial.println(x); } while(0)
    #define DEBUG_PRINTF(...) do { Serial.print("[debug] "); Serial.printf(__VA_ARGS__); } while(0)
  #else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
  #endif

#endif  // MACROS_H