// ===== Transmitter with TinyGPS++ =====
// Change AT+BAND=915000000 to match your region:
// Europe: 868000000
// US/Canada: 915000000
// Asia: 923000000

#include <SoftwareSerial.h>
#include <DHT.h>
#include <TinyGPS++.h>

// DHT11 configuration
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// GPS configuration
#define GPS_RX 2  // GPS TX -> Arduino RX (D2)
#define GPS_TX 3  // GPS RX -> Arduino TX (D3)
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;  // Create TinyGPS++ object

// LoRa configuration
#define LORA_RX 4  // LoRa RX -> Arduino TX (D4)
#define LORA_TX 5  // LoRa TX -> Arduino RX (D5)
#define LORA_NRST 6
SoftwareSerial ReyaxLoRa(LORA_TX, LORA_RX);  // (TX, RX)

// Data collection
float tempSum = 0;
float humSum = 0;
int readingCount = 0;
const int readingsPerTransmission = 10;

// GPS data
float latitude = 0.0;
float longitude = 0.0;
char latDirection = 'N';
char lonDirection = 'E';
bool gpsFixed = false;
unsigned long lastGPSUpdate = 0;

// Timing
unsigned long lastReadingTime = 0;
const unsigned long readingInterval = 1000; // 1 second

void setup() {
  Serial.begin(115200);
  ReyaxLoRa.begin(9600);
  gpsSerial.begin(9600);
  dht.begin();
  
  pinMode(LORA_NRST, OUTPUT);
  
  // Reset LoRa module
  digitalWrite(LORA_NRST, LOW);
  delay(100);
  digitalWrite(LORA_NRST, HIGH);
  delay(1000);
  
  Serial.println("--- Transmitter Initializing ---");
  Serial.println("Waiting for GPS fix... (may take 30-60 seconds)");
  
  // Initialize LoRa module
  ReyaxLoRa.println("AT");
  delay(200);
  ReyaxLoRa.println("AT+OPMODE=1");  // Proprietary mode for point-to-point
  delay(200);
  ReyaxLoRa.println("AT+BAND=915000000");  // Frequency (adjust for your region)
  delay(200);
  ReyaxLoRa.println("AT+ADDRESS=1");  // Transmitter address
  delay(200);
  
  Serial.println("LoRa initialized!");
  Serial.println("Collecting data...");
}

void loop() {
  // Read and process GPS data continuously
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
  }
  
  // Update GPS status every second
  if (millis() - lastGPSUpdate >= 1000) {
    lastGPSUpdate = millis();
    updateGPSData();
  }
  
  // Read sensors every second
  if (millis() - lastReadingTime >= readingInterval) {
    lastReadingTime = millis();
    
    // Read DHT11
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      tempSum += t;
      humSum += h;
      readingCount++;
      
      Serial.print("Reading ");
      Serial.print(readingCount);
      Serial.print("/");
      Serial.print(readingsPerTransmission);
      Serial.print(" - Temp: ");
      Serial.print(t);
      Serial.print("°C, Hum: ");
      Serial.print(h);
      Serial.println("%");
    }
    
    // Check if we have enough readings to send
    if (readingCount >= readingsPerTransmission) {
      sendData();
      
      // Reset sums and counter
      tempSum = 0;
      humSum = 0;
      readingCount = 0;
    }
  }
}

void updateGPSData() {
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    
    // Determine direction letters
    latDirection = (latitude >= 0) ? 'N' : 'S';
    lonDirection = (longitude >= 0) ? 'E' : 'W';
    
    // Store absolute values for sending
    latitude = abs(latitude);
    longitude = abs(longitude);
    
    if (!gpsFixed) {
      gpsFixed = true;
      Serial.println("*** GPS FIX ACQUIRED! ***");
    }
    
    Serial.print("GPS Update - Lat: ");
    Serial.print(latitude, 6);
    Serial.print(" ");
    Serial.print(latDirection);
    Serial.print(", Lon: ");
    Serial.print(longitude, 6);
    Serial.print(" ");
    Serial.print(lonDirection);
    Serial.print(" | Satellites: ");
    Serial.println(gps.satellites.value());
  } else {
    if (gpsFixed) {
      Serial.println("GPS signal lost!");
      gpsFixed = false;
    } else {
      // Print dots to show we're trying
      static int dotCount = 0;
      if (dotCount++ % 10 == 0) {
        Serial.print(".");
      }
    }
  }
}

void sendData() {
  // Calculate averages
  float avgTemp = tempSum / readingsPerTransmission;
  float avgHum = humSum / readingsPerTransmission;
  
  // Format data: temp_c,humidity,latitude,longitude,lat_dir,lon_dir
  // Example: "24.5,65.2,40.7128,N,74.0060,W"
  String dataString = String(avgTemp, 1) + "," + 
                      String(avgHum, 1) + ",";
  
  if (gpsFixed && gps.location.isValid()) {
    dataString += String(latitude, 6) + "," +
                  String(latDirection) + "," +
                  String(longitude, 6) + "," +
                  String(lonDirection);
  } else {
    dataString += "0.000000,N,0.000000,E";  // No GPS fix
  }
  
  Serial.println("\n--- Sending Data Package ---");
  Serial.print("Average Temperature: ");
  Serial.print(avgTemp);
  Serial.println("°C");
  Serial.print("Average Humidity: ");
  Serial.print(avgHum);
  Serial.println("%");
  Serial.print("GPS: ");
  Serial.println(gpsFixed ? "Valid" : "No Fix");
  if (gpsFixed) {
    Serial.print("Coordinates: ");
    Serial.print(latitude, 6);
    Serial.print(" ");
    Serial.print(latDirection);
    Serial.print(", ");
    Serial.print(longitude, 6);
    Serial.print(" ");
    Serial.println(lonDirection);
  }
  Serial.print("Data String: ");
  Serial.println(dataString);
  
  // Send via LoRa to receiver (address 2)
  String command = "AT+SEND=2," + String(dataString.length()) + "," + dataString;
  ReyaxLoRa.println(command);
  delay(100);
  
  Serial.println("Data sent successfully!");
  Serial.println("---\n");
}
