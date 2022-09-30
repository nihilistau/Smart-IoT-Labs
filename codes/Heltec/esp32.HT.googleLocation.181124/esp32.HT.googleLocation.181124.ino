
#include <WiFi.h>
#include <WifiLocation.h>

const char* googleApiKey = "AIzaSyB1WnGEVtlhWWIzIPrYcLl0pWxKiAVKKio";
const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";

WifiLocation location(googleApiKey);

void setup() {
    Serial.begin(9600);
       WiFi.disconnect(true);
   delay(1000);
   WiFi.begin(ssid, pass);    
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println("WiFi setup ok");
    location_t loc = location.getGeoFromWiFi();

    Serial.println("Location request data");
    Serial.println(location.getSurroundingWiFiJson());
    Serial.println("Latitude: " + String(loc.lat, 7));
    Serial.println("Longitude: " + String(loc.lon, 7));
    Serial.println("Accuracy: " + String(loc.accuracy));


}

void loop() {

}
