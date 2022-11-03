

#include <SoftwareSerial.h>

#ifndef D5
#if defined(ESP8266)
#define D8 (15)
#define D5 (14)
#define D7 (13)
#define D6 (12)
#define RX (3)
#define TX (1)
#elif defined(ESP32)
#define D8 (5)
#define D5 (18)
#define D7 (23)
#define D6 (19)
#define RX (3)
#define TX (1)
#endif
#endif

#ifdef ESP32
#define BAUD_RATE 57600
#else
#define BAUD_RATE 57600
#endif

#undef SWAPSERIAL

#ifndef SWAPSERIAL
auto& usbSerial = Serial;
SoftwareSerial testSerial;
#else
SoftwareSerial usbSerial;
auto& testSerial = Serial;
#endif

void setup() {
#ifndef SWAPSERIAL
    usbSerial.begin(115200);
    // Important: the buffer size optimizations here, in particular the isrBufSize (11) that is only sufficiently
    // large to hold a single word (up to start - 8 data - parity - stop), are on the basis that any char written
    // to the loopback SoftwareSerial adapter gets read before another write is performed.
    // Block writes with a size greater than 1 would usually fail. Do not copy this into your own project without
    // reading the documentation.
    testSerial.begin(BAUD_RATE, SWSERIAL_8N1, 18, 19, false, 95, 11);
#else
    testSerial.begin(115200);
    testSerial.setDebugOutput(false);
    testSerial.swap();
    usbSerial.begin(BAUD_RATE, SWSERIAL_8N1, 18, 19, false, 95);
#endif

    usbSerial.println(PSTR("\nSoftware serial test started"));

    for (char ch = ' '; ch <= 'z'; ch++) {
        testSerial.write(ch);
    }
    testSerial.println();
}

void loop() {
    while (testSerial.available() > 0) {
        usbSerial.write(testSerial.read());
        yield();
    }
    while (usbSerial.available() > 0) {
        testSerial.write(usbSerial.read());
        yield();
    }
}

 
