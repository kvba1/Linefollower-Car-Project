#include <Arduino.h>

#define left 36
#define middle 34
#define right 39
#define DC_1 27
#define DC_2 12

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

/*int IR_sensor_value(int analog_pin){
  int analog = analogRead(analog_pin);
  Serial.print(analog);
  Serial.print("\n");
  return analog;
}*/

void setup() {
  Serial.begin(115200);
  pinMode(left,INPUT);
  pinMode(middle,INPUT);
  pinMode(right,INPUT);
  Serial.print("HALO!");


  ledcAttachPin(12, 1); // assign RGB led pins to channels
  ledcAttachPin(14, 2);
  ledcAttachPin(27, 3);
  ledcAttachPin(13, 4);
  
  // Initialize channels 
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);
  ledcSetup(4, 12000, 8);
}

void loop() {
int analog_lewy = analogRead(left);
int analog_srodkowy = analogRead(middle);
int analog_prawy = analogRead(right);
Serial.print(analog_lewy);
Serial.print(" - lewy\n");
Serial.print(analog_srodkowy);
Serial.print(" - srodkowy\n");
Serial.print(analog_prawy);
Serial.print(" - prawy\n");
Serial.print("\n");
if (analog_lewy > analog_prawy) Serial.print("skrec w prawo");
if (analog_prawy > analog_lewy) Serial.print("skrec w lewo");
Serial.print("\n");
Serial.print("\n");
ledcWrite(1, 255);
  ledcWrite(2, 180);
  ledcWrite(3, 0);
  ledcWrite(4, 0);
delay(1000);
}