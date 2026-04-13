// Transmitter

#include <SoftwareSerial.h>
#include <DHT.h>

// DHT11 configuration
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// GPS configuration
#define GPS_RX 2  // GPS TX -> Arduino RX (D2)
#define GPS_TX 3  // GPS RX -> Arduino TX (D3)
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);  // (RX, TX)

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
char lonDirection = 'W';
bool gpsFixed = false;

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
  
  // Initialize LoRa module
  ReyaxLoRa.println("AT");
  delay(200);
  ReyaxLoRa.println("AT+OPMODE=1");  // Proprietary mode for point-to-point
  delay(200);
  ReyaxLoRa.println("AT+BAND=915000000");  // Frequency (adjust for your region: 868MHz for EU, 915MHz for US)
  delay(200);
  ReyaxLoRa.println("AT+ADDRESS=1");  // Transmitter address
  delay(200);
  
  Serial.println("LoRa initialized!");
  Serial.println("Collecting data...");
}

void loop() {
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
      
      // Read GPS (continuously, store latest valid reading)
      readGPS();
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
  
  // Keep reading GPS data in background
  if (gpsSerial.available()) {
    parseGPSData();
  }
}

void readGPS() {
  // This function is called periodically, but actual parsing happens in parseGPSData()
  // Just make sure we're processing GPS data regularly
  while (gpsSerial.available()) {
    parseGPSData();
  }
}

void parseGPSData() {
  static String gpsBuffer = "";
  
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    gpsBuffer += c;
    
    if (c == '\n') {
      // Process complete line
      if (gpsBuffer.startsWith("$GPGGA")) {
        parseGPGGA(gpsBuffer);
      }
      gpsBuffer = "";
    }
  }
}

void parseGPGGA(String sentence) {
  // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
  int comma1 = sentence.indexOf(',');
  int comma2 = sentence.indexOf(',', comma1 + 1);
  int comma3 = sentence.indexOf(',', comma2 + 1);
  int comma4 = sentence.indexOf(',', comma3 + 1);
  int comma5 = sentence.indexOf(',', comma4 + 1);
  int comma6 = sentence.indexOf(',', comma5 + 1);
  
  if (comma2 > 0 && comma3 > 0 && comma4 > 0 && comma5 > 0) {
    String latStr = sentence.substring(comma2 + 1, comma3);
    String latDir = sentence.substring(comma3 + 1, comma4);
    String lonStr = sentence.substring(comma4 + 1, comma5);
    String lonDir = sentence.substring(comma5 + 1, comma6);
    
    if (latStr.length() > 0 && lonStr.length() > 0) {
      // Convert latitude (DDMM.MMMMM format to decimal degrees)
      float latDegrees = latStr.substring(0, 2).toFloat();
      float latMinutes = latStr.substring(2).toFloat();
      latitude = latDegrees + (latMinutes / 60.0);
      latDirection = latDir.charAt(0);
      
      // Convert longitude (DDDMM.MMMMM format to decimal degrees)
      float lonDegrees = lonStr.substring(0, 3).toFloat();
      float lonMinutes = lonStr.substring(3).toFloat();
      longitude = lonDegrees + (lonMinutes / 60.0);
      lonDirection = lonDir.charAt(0);
      
      gpsFixed = true;
      
      Serial.print("GPS Fix - Lat: ");
      Serial.print(latitude, 6);
      Serial.print(" ");
      Serial.print(latDirection);
      Serial.print(", Lon: ");
      Serial.print(longitude, 6);
      Serial.print(" ");
      Serial.println(lonDirection);
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
  
  if (gpsFixed) {
    dataString += String(latitude, 6) + "," +
                  String(latitude > 0 ? latDirection : (latDirection == 'S' ? 'S' : 'N')) + "," +
                  String(longitude, 6) + "," +
                  String(longitude > 0 ? lonDirection : (lonDirection == 'W' ? 'W' : 'E'));
  } else {
    dataString += "0.000000,N,0.000000,E";  // No GPS fix
  }
  
  Serial.println("\n--- Sending Data ---");
  Serial.print("Average Temperature: ");
  Serial.print(avgTemp);
  Serial.println("°C");
  Serial.print("Average Humidity: ");
  Serial.print(avgHum);
  Serial.println("%");
  Serial.print("GPS: ");
  Serial.println(gpsFixed ? "Valid" : "No Fix");
  Serial.print("Data: ");
  Serial.println(dataString);
  
  // Send via LoRa to receiver (address 2)
  String command = "AT+SEND=2," + String(dataString.length()) + "," + dataString;
  ReyaxLoRa.println(command);
  delay(100);
  
  Serial.println("Data sent!");
  Serial.println("---\n");
}
