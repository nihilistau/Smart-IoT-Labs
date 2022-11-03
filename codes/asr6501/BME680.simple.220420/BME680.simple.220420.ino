
#include "Zanshin_BME680.h"  // Include the BME680 Sensor library
const uint32_t SERIAL_SPEED{115200};  ///< Set the baud rate for Serial I/O

BME680_Class BME680;  ///< Create an instance of the BME680 class
///< Forward function declaration with default value for sea level
float altitude(const int32_t press, const float seaLevel = 1009.10);
float altitude(const int32_t press, const float seaLevel) {
  /*
             otherwise the standard atmospheric pressure of 1013.25hPa is used (see
             https://en.wikipedia.org/wiki/Atmospheric_pressure) for details.
  */
  static float Altitude;
  Altitude =
      44330.0 * (1.0 - pow(((float)press / 100.0) / seaLevel, 0.1903));  // Convert into meters
  return (Altitude);
}  // of method altitude()

void setup() 
{
  Serial.begin(9600);
  Wire.begin(); //(29,28);     
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext,LOW);//set vext to high

  Serial.print("- Initializing BME680 sensor\n");
  while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start BME680 using I2C, use first device found
    Serial.print("-  Unable to find BME680. Trying again in 5 seconds.\n");
    delay(5000);
  }  // of loop until device is located
  
  Serial.print("- Setting 16x oversampling for all sensors\n");
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  Serial.print("- Setting IIR filter to a value of 4 samples\n");
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  Serial.print("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n");  // "�C" symbols
  BME680.setGas(320, 150);  // 320�c for 150 milliseconds
}  // of method setup()

void loop() 
{
  static int32_t  temp, humidity, pressure, gas;  // BME readings
  static char     buf[16];                        // sprintf text buffer
  static float    alt;                            // Temporary variable
  static uint16_t loopCounter = 0;                // Display iterations
  if (loopCounter % 25 == 0) {                    // Show header @25 loops
    Serial.print(F("\nLoop Temp\xC2\xB0\x43 Humid% Press hPa   Alt m   Air m"));
    Serial.print(F("\xE2\x84\xA6\n==== ====== ====== ========= =======   ======\n"));  // "�C" symbol
  }                                                     // if-then time to show headers
  BME680.getSensorData(temp, humidity, pressure, gas);  // Get readings
  Serial.println((int)temp/100);
  Serial.println((int)humidity/1000);
  Serial.println((int)pressure/100);
  if (loopCounter++ != 0) {                             // Ignore first reading, might be incorrect
    sprintf(buf, "%4d %3d.%02d", (loopCounter - 1) % 9999,  // Clamp to 9999,
            (int8_t)(temp / 100), (uint8_t)(temp % 100));   // Temp in decidegrees
    Serial.print(buf);
    sprintf(buf, "%3d.%03d", (int8_t)(humidity / 1000),
            (uint16_t)(humidity % 1000));  // Humidity milli-pct
    Serial.print(buf);
    sprintf(buf, "%7d.%02d", (int16_t)(pressure / 100),
            (uint8_t)(pressure % 100));  // Pressure Pascals
    Serial.print(buf);
    alt = altitude(pressure);                                                // temp altitude
    sprintf(buf, "%5d.%02d  ", (int16_t)(alt), ((uint8_t)(alt * 100) % 100));  // Altitude meters
    Serial.print(buf);
    sprintf(buf, "%4d.%02d\n", (int16_t)(gas / 100), (uint8_t)(gas % 100));  // Resistance milliohms
    Serial.print(buf);
    delay(1200);  // Wait 10s
  }                // of ignore first reading
}  // of method loop()
