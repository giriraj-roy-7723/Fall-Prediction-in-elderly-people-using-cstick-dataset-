#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

HardwareSerial mySerial(1); // Use UART1 or UART2
String readString;
String bpm;
String spo2;
String hrv;
int ind1;
int ind2;
int ind3;
char c;

const char* ssid = "OnePlus 11R 5G";  // Replace with your network credentials
const char* password = "v4b8zmi3";
const char* serverUrl = "http://192.168.245.239:5000/predict";  // Flask server IP

//For display
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(128,64, &Wire, OLED_RESET); /* Object of class Adafruit_SSD1306 */


//For accelarometer
Adafruit_MPU6050 mpu;
const float ACCEL_THRESHOLD = 10.0; // m/s^2

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // RX, TX pins
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);
  display.begin(0x3C,true);/* Initialize display with address 0x3C */
  display.clearDisplay(); /* Clear display */
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2); 
  display.setCursor(5,5);
  display.print("BPM:");
  display.setCursor(5,30);
  display.print("SpO2:");
  display.display();

  Wire.begin(21, 22);

  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  //setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);	// Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
}



void loop() {
  digitalWrite(4,LOW);
  if (mySerial.available() > 0) {
    // Read the incoming data
    readString = mySerial.readStringUntil('\n');
    ind1 = readString.indexOf(','); //finds location of first ,
    bpm = readString.substring(0, ind1); //captures first data String
    ind2 = readString.indexOf(';'); //finds location of second ,
    spo2 = readString.substring(ind1+1, ind2); //captures second data String
    ind3 = readString.indexOf('|');
    hrv = readString.substring(ind2+1, ind3);
  }
    float acc = suddenAccelaration();
    Serial.print(bpm);
    Serial.print(",");
    Serial.print(hrv);
    Serial.print(",");
    Serial.print(spo2);
    Serial.print(",");
    Serial.println(acc);

    String x = sendDataToServer(hrv.toFloat(),spo2.toFloat(),acc);
    c=x[5];
    Serial.println(c);
    int z = (spo2.toFloat()==0.0)&&(acc==0.0);
  if(c=='0'){
    display.clearDisplay(); /* Clear display */
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2); /* Select font size of text. Increases with size of argument. */
    display.setCursor(5,5);
    display.print("BPM:");
    display.println(bpm);
    display.setCursor(5,30);
    display.print("SpO2:");
    display.println(spo2);
    display.display();
  }
  else if(c =='1'){
    display.clearDisplay(); /* Clear display */
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2); /* Select font size of text. Increases with size of argument. */
    display.setCursor(20,5);
    display.println("Motion");
    display.setCursor(20,30);
    display.println("Detetcted");
    display.display();
  }
  else if(c=='2' && !z){
    display.clearDisplay(); /* Clear display */
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2); /* Select font size of text. Increases with size of argument. */
    display.setCursor(20,5);
    display.println("Fall");
    display.setCursor(20,30);
    display.println("Detected");
    display.display();
    digitalWrite(4,HIGH);
    delay(500);
  }
  
}

float suddenAccelaration(){
   // Read acceleration values from the sensor
  sensors_event_t a,g,temp;
  mpu.getEvent(&a,&g,&temp);
  
  // Calculate the magnitude of the acceleration vector
  float accel_x = a.acceleration.x;
  float accel_y = a.acceleration.y;
  float accel_z = a.acceleration.z;

  // Check if acceleration exceeds the threshold on any axis
  if (abs(accel_x) > ACCEL_THRESHOLD || abs(accel_y) > ACCEL_THRESHOLD || abs(accel_z) > ACCEL_THRESHOLD) {
    return 1.0;
  }
  else{
    return 0.0;
  }
}
String sendDataToServer(float hrv,float spo2,float acc) {
  String response;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Specify the server URL
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");  // Specify content-type header

    // Create JSON object with an array
    StaticJsonDocument<200> jsonDoc;

    // Create the array to match the format {"input": [ 93.960, 82.700, 1.0]}
    JsonArray inputArray = jsonDoc.createNestedArray("input");
    inputArray.add(hrv);  // Second value
    inputArray.add(spo2);  // Third value
    inputArray.add(acc);     // Fourth value

    // Serialize JSON to string
    String jsonData;
    serializeJson(jsonDoc, jsonData);

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      response = http.getString();  // Get the response from the server

    } else {
      Serial.println("Error in sending POST request: " + String(httpResponseCode));
    }
    // End the HTTP connection
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
  return response;
}
