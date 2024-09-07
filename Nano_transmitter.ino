//scientist BENIELS LAB
/* CREATED BY: HOW TO ELECTRONICS | MODIFIED BY: SCIENTIST BENIELS  LAB */

// #include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <SoftwareSerial.h>

// Define RX and TX pins
#define RX 10  // Pin 10 as RX
#define TX 11  // Pin 11 as TX

// Create a SoftwareSerial object
SoftwareSerial mySerial(RX, TX); // RX, TX

#define REPORTING_PERIOD_MS     1000
String data;

PulseOximeter pox;
uint32_t tsLastReport = 0;
float bpm=72.0;
float spo2=95.0;
float hrv=80.00;
float gap=1000.0;
float intervals[10]={0};
int intervalindex = 0;
void onBeatDetected()
{

  // Serial.println("Beat!!!");

}

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  if (!pox.begin()) {
    // Serial.println("FAILED");
    for (;;);
  } else {
    // Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    bpm=pox.getHeartRate();
    spo2=pox.getSpO2();
    if(bpm!=0) gap=60000/bpm;

    intervals[intervalindex]= gap;
    intervalindex = int((intervalindex + 1) % 10);

    if (intervalindex == 0) {
      hrv = calculateHRV();
      if(hrv > 200)
          hrv=150.00;
    }
    tsLastReport = millis();
  }
    Serial.print(bpm);
    Serial.print(",");
    Serial.print(spo2);
    Serial.print(";");
    Serial.print(hrv);
    Serial.println("|");

    // if (Serial.available() > 0) {
    // // Read the incoming data
    // data = Serial.readStringUntil('\n');
    // }
    // if(data[0]=='0')
    //   mySerial.print("Stable");
    // if(data[0]=='1')
    //   mySerial.print("Person in motion");
    // if(data[0]=='2')
    //   mySerial.print("Fall detected");
    // mySerial.print(":");
    mySerial.print(spo2);
    mySerial.print(":");
    mySerial.print(hrv);
    mySerial.print(":");

}
float calculateHRV() {
  float mean = 0;
  float variance = 0;
  
  // Calculate mean
  for (int i = 0; i < 10; i++) {
    mean += intervals[i];
  }
  mean /= 10;
  
  // Calculate variance
  for (int i = 0; i < 10; i++) {
    float diff = intervals[i] - mean;
    variance += diff * diff;
  }
  variance /= 10;
  
  // HRV is the square root of variance
  return sqrt(variance);
}