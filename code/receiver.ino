// ===== Receiver =====
// Change AT+BAND=915000000 to match your region:
// Europe: 868000000
// US/Canada: 915000000
// Asia: 923000000

#include <SoftwareSerial.h>
#include <DFRobot_RGBLCD1602.h>

// LCD configuration
DFRobot_RGBLCD1602 lcd(0x60, 16, 2);

// LoRa configuration
#define LORA_RX 4  // LoRa RX -> Arduino TX (D4)
#define LORA_TX 5  // LoRa TX -> Arduino RX (D5)
#define LORA_NRST 6
#define BUTTON_PIN 3

SoftwareSerial ReyaxLoRa(LORA_TX, LORA_RX);  // (TX, RX)

// Display variables
int currentScreen = 0;  // 0: Celsius, 1: Fahrenheit, 2: GPS
const int totalScreens = 3;

bool lastButtonState = HIGH;
bool dataReceived = false;

// Weather data
float temperature = 0.0;
float humidity = 0.0;
float latitude = 0.0;
float longitude = 0.0;
String latDir = "N";
String lonDir = "W";

void setup() {
  Serial.begin(115200);
  ReyaxLoRa.begin(9600);
  
  pinMode(LORA_NRST, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Reset LoRa module
  digitalWrite(LORA_NRST, LOW);
  delay(100);
  digitalWrite(LORA_NRST, HIGH);
  delay(1000);
  
  // Initialize LCD
  lcd.init();
  lcd.setColorWhite();
  lcd.clear();
  
  Serial.println("--- Receiver Initializing ---");
  
  // Initialize LoRa module
  ReyaxLoRa.println("AT");
  delay(200);
  ReyaxLoRa.println("AT+OPMODE=1");  // Proprietary mode for point-to-point
  delay(200);
  ReyaxLoRa.println("AT+BAND=915000000");  // Same frequency as transmitter
  delay(200);
  ReyaxLoRa.println("AT+ADDRESS=2");  // Receiver address
  delay(200);
  
  Serial.println("LoRa initialized!");
  
  // Display waiting message
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("data...");
}

void loop() {
  // Check for button press
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Change screen
    currentScreen = (currentScreen + 1) % totalScreens;
    Serial.print("Screen changed to: ");
    Serial.println(currentScreen);
    
    if (dataReceived) {
      updateDisplay();
    }
    delay(200);  // Debounce delay
  }
  lastButtonState = buttonState;
  
  // Check for incoming LoRa messages
  if (ReyaxLoRa.available()) {
    String incoming = ReyaxLoRa.readStringUntil('\n');
    incoming.trim();
    
    if (incoming.startsWith("+RCV=")) {
      parseLoRaMessage(incoming);
    }
  }
}

void parseLoRaMessage(String incoming) {
  // Format: +RCV=sender,len,message,RSSI,SNR
  int firstComma = incoming.indexOf(',', 5);
  int secondComma = incoming.indexOf(',', firstComma + 1);
  int thirdComma = incoming.indexOf(',', secondComma + 1);
  
  if (firstComma > 0 && secondComma > 0 && thirdComma > 0) {
    String sender = incoming.substring(5, firstComma);
    String message = incoming.substring(secondComma + 1, thirdComma);
    
    Serial.print("Message received from ");
    Serial.print(sender);
    Serial.print(": ");
    Serial.println(message);
    
    // Parse the data: temp,hum,lat,lat_dir,lon,lon_dir
    parseWeatherData(message);
    
    dataReceived = true;
    updateDisplay();
  }
}

void parseWeatherData(String data) {
  int comma1 = data.indexOf(',');
  int comma2 = data.indexOf(',', comma1 + 1);
  int comma3 = data.indexOf(',', comma2 + 1);
  int comma4 = data.indexOf(',', comma3 + 1);
  int comma5 = data.indexOf(',', comma4 + 1);
  
  if (comma1 > 0 && comma2 > 0) {
    temperature = data.substring(0, comma1).toFloat();
    humidity = data.substring(comma1 + 1, comma2).toFloat();
    
    if (comma3 > 0 && comma4 > 0 && comma5 > 0) {
      latitude = data.substring(comma2 + 1, comma3).toFloat();
      latDir = data.substring(comma3 + 1, comma4);
      longitude = data.substring(comma4 + 1, comma5).toFloat();
      lonDir = data.substring(comma5 + 1);
      
      Serial.print("Parsed - Temp: ");
      Serial.print(temperature);
      Serial.print("°C, Hum: ");
      Serial.print(humidity);
      Serial.print("%, Lat: ");
      Serial.print(latitude, 6);
      Serial.print(latDir);
      Serial.print(", Lon: ");
      Serial.print(longitude, 6);
      Serial.println(lonDir);
    }
  }
}

void updateDisplay() {
  lcd.clear();
  
  switch(currentScreen) {
    case 0:  // Celsius screen
      displayCelsius();
      break;
    case 1:  // Fahrenheit screen
      displayFahrenheit();
      break;
    case 2:  // GPS screen
      displayGPS();
      break;
  }
}

void displayCelsius() {
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(temperature, 1);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(humidity, 1);
  lcd.print("%");
}

void displayFahrenheit() {
  float tempF = temperature * 9.0 / 5.0 + 32.0;
  
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(tempF, 1);
  lcd.print("F");
  
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(humidity, 1);
  lcd.print("%");
}

void displayGPS() {
  lcd.setCursor(0, 0);
  lcd.print("LAT: ");
  
  // Format latitude with 4 decimal places
  if (latitude > 0) {
    lcd.print(latitude, 4);
    lcd.print(" ");
    lcd.print(latDir);
  } else {
    lcd.print("---");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("LON: ");
  
  if (longitude > 0) {
    lcd.print(longitude, 4);
    lcd.print(" ");
    lcd.print(lonDir);
  } else {
    lcd.print("---");
  }
}
