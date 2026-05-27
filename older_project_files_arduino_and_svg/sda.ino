/* * ESP32-C3 Air Purifier System
 * * FEATURES:
 * 1. VISUAL: Smooth RGB Gradient (Green -> Yellow -> Red -> Purple)
 * 2. FAN: Custom Speed Ranges (Silent -> Turbo)
 * 3. CLOUD: ThingSpeak Logging
 */

#include <WiFiManager.h> 
#include <HTTPClient.h>

// --- HARDWARE CONFIGURATION ---
#define RXD2 20            
#define TXD2 21            
#define FAN_PIN 5          

// --- THINGSPEAK SETTINGS ---
const char* server = "http://api.thingspeak.com/update";
String apiKey = "1KGLC25WWVX0P5PB"; 
const unsigned long uploadInterval = 30000; 

// --- GLOBAL VARIABLES ---
float currentLPO = 0.0;    
float currentMass = 0.0;   
int currentAQI = 0;        
int currentVOC = 0;        
unsigned long lastUploadTime = 0;

void setup() {
  Serial.begin(115200);
  
  // 1. Setup Sensor
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // 2. Setup Fan
  ledcAttach(FAN_PIN, 25000, 8); 
  ledcWrite(FAN_PIN, 255); // Startup "Whoosh"
  delay(500);
  ledcWrite(FAN_PIN, 0);

  // 3. Initialize RGB (Blue = Setup)
  #ifdef RGB_BUILTIN
    rgbLedWrite(RGB_BUILTIN, 0, 0, 50); 
  #endif

  // 4. WiFi
  WiFiManager wm;
  if (!wm.autoConnect("GlobalShapers_Purifier")) {
    ESP.restart(); 
  }
  
  Serial.println("WiFi Connected!");
  #ifdef RGB_BUILTIN
    rgbLedWrite(RGB_BUILTIN, 0, 50, 0); // Green Flash
    delay(1000);
  #endif
}

// --- HELPER: Set RGB Color ---
void setRawRGB(int r, int g, int b) {
  #ifdef RGB_BUILTIN
    // Divide by 5 for brightness control
    rgbLedWrite(RGB_BUILTIN, r/5, g/5, b/5); 
  #endif
}

// --- LOGIC 1: SMOOTH COLOR TRANSITION ---
void updateSmoothColor(float mass) {
  int r = 0, g = 0, b = 0;

  // We interpolate (fade) between colors based on Mass value
  
  // Phase 1: 0 to 50 (Green -> Yellow)
  if (mass <= 50.0) {
    // Green is (0, 255, 0), Yellow is (255, 255, 0)
    // Red increases from 0 to 255. Green stays max.
    r = map(mass, 0, 50, 0, 255);
    g = 255;
    b = 0;
  }
  // Phase 2: 50 to 100 (Yellow -> Red)
  else if (mass <= 100.0) {
    // Yellow is (255, 255, 0), Red is (255, 0, 0)
    // Green decreases from 255 to 0. Red stays max.
    r = 255;
    g = map(mass, 50, 100, 255, 0);
    b = 0;
  }
  // Phase 3: 100 to 200 (Red -> Purple)
  else if (mass <= 200.0) {
    // Red is (255, 0, 0), Purple is (128, 0, 128)
    // Red decreases slightly, Blue increases significantly
    r = map(mass, 100, 200, 255, 128);
    g = 0;
    b = map(mass, 100, 200, 0, 128);
  }
  // Phase 4: 200 to 300+ (Purple -> Maroon)
  else {
    // Purple is (128, 0, 128), Maroon is (128, 0, 0)
    // Blue fades out to 0
    float cappedMass = (mass > 300.0) ? 300.0 : mass;
    r = 128;
    g = 0;
    b = map(cappedMass, 200, 300, 128, 0);
  }

  setRawRGB(r, g, b);
}

// --- LOGIC 2: FAN SPEED CONTROL ---
void updateFanSpeed(float mass) {
  int fanSpeed = 0;
  
  if (mass <= 10.0) {
    fanSpeed = 12;   // Silent
  } 
  else if (mass <= 30.0) {
    fanSpeed = 80;   // Low
  }
  else if (mass <= 60.0) {
    fanSpeed = 190;  // High
  } 
  else {
    fanSpeed = 255;  // Turbo (Above 60)
  }
  ledcWrite(FAN_PIN, fanSpeed);
}

// --- LOGIC 3: EPA AQI CALC (For Cloud) ---
int calcEPA_AQI(float pm25) {
  float c = pm25;
  if (c <= 9.0)    return map(c*10, 0, 90, 0, 50);
  if (c <= 35.4)   return map(c*10, 91, 354, 51, 100);
  if (c <= 55.4)   return map(c*10, 355, 554, 101, 150);
  if (c <= 125.4)  return map(c*10, 555, 1254, 151, 200);
  if (c <= 225.4)  return map(c*10, 1255, 2254, 201, 300);
  if (c <= 325.4)  return map(c*10, 2255, 3254, 301, 400);
  if (c <= 500.4)  return map(c*10, 3255, 5004, 401, 500);
  return 500; 
}

void uploadData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey + 
                 "&field1=" + String(currentMass) + 
                 "&field2=" + String(currentAQI) + 
                 "&field3=" + String(currentVOC);
    
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) Serial.println("ThingSpeak Upload Success.");
    http.end();
  }
}

void loop() {
  if (Serial1.available() >= 9) {
    if (Serial1.read() == 0xFF) {
      byte buf[9];
      buf[0] = 0xFF;
      for (int i = 1; i < 9; i++) buf[i] = Serial1.read();

      byte checksum = 0;
      for (int i = 1; i < 8; i++) checksum += buf[i];
      checksum = (~checksum) + 1;

      if (checksum == buf[8]) {
        currentLPO = buf[3] + (buf[4] / 100.0);
        currentMass = currentLPO * 10.0;
        currentAQI = calcEPA_AQI(currentMass);
        currentVOC = buf[7];

        // 1. Update Smooth LED
        updateSmoothColor(currentMass);

        // 2. Update Fan Speed
        updateFanSpeed(currentMass);

        // Debug
        Serial.printf("Mass: %.2f | AQI: %d | VOC: %d\n", currentMass, currentAQI, currentVOC);
      }
    }
  }

  if (millis() - lastUploadTime > uploadInterval) {
    uploadData();
    lastUploadTime = millis();
  }
}