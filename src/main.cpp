#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <string>
#include <iostream>
#include <algorithm>

#define LFMotorPWM 26 
#define LBMotorPWM 27 
#define RFMotorPWM 33 
#define RBMotorPWM 25
#define ledPin 19
#define lineSensorL 32
#define lineSensorM 35
#define lineSensorR 34

clock_t start ;
clock_t temp ;
clock_t czas;
long lastMsgH = 0;
long lastMsg = 0;
long lastMsgD = 0;
using namespace std;

string MODE = "MANUAL";
//Local wifi ssid and password
const char* ssid ="Korbank-internet-7f71_2.4GHz";   
const char* password = "f1836b6f";
//Your MQTT server
const char* mqtt_server ="broker.mqttdashboard.com";//"broker.emqx.io";"broker.mqttdashboard.com";"broker.emqx.io";//"broker.mqttdashboard.com";
const char* mqtt_username = "ESPCTR0008";
const char* mqtt_password = "12345678";
const char* inTopic = "PUM2023v2/BETA/#";
const int mqtt_port =1883;

bool left_led = false;
bool start_v = false;
bool right_led = false;
int multiplier=100;

WiFiClient espClient;
PubSubClient client(espClient);
HTTPClient http1;

String website = "http://kvba1337.eu.pythonanywhere.com";
String db = "/vehicle/1";


void blink_led(unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW);  
    delay(200);
  }
}

void stop(){
//Hamowanie
  analogWrite(LFMotorPWM, 0);
  analogWrite(RFMotorPWM, 0);
  analogWrite(LBMotorPWM, 0);
  analogWrite(RBMotorPWM, 0);
}

void setRightMotor(int RightPWM, int RightDirection){
  // RightDirection 0 - forward, 1 - backward
  if (RightPWM<=100)
  {
    stop();
  }
  if (RightPWM>100){
    if ((RightPWM > 255) || (RightPWM <0))
      stop();

    if(RightDirection==HIGH){
      analogWrite(RBMotorPWM, 0);
      analogWrite(RFMotorPWM, RightPWM*multiplier/100);
    }
    if(RightDirection==LOW){
      analogWrite(RFMotorPWM, 0);
      analogWrite(RBMotorPWM, RightPWM*multiplier/100);
    }
  }
}

void setLeftMotor(int LeftPWM, int LeftDirection){
  // RightDirection 0 - forward, 1 - backward
  if (LeftPWM<=100)
  {
    stop();
  }
  if (LeftPWM>100){
    if ((LeftPWM > 255) || (LeftPWM <0))
      stop();

    if(LeftDirection==HIGH){
      analogWrite(LBMotorPWM, 0);
      analogWrite(LFMotorPWM, LeftPWM*multiplier/100);
    }
    if(LeftDirection==LOW){
      analogWrite(LFMotorPWM, 0);
      analogWrite(LBMotorPWM, LeftPWM*multiplier/100);
    }
  }
}

void receive_http(){
long now = millis();
if (now - lastMsg > 500) {
    lastMsgH = now;
      int httpCode = http1.GET();
  if(httpCode > 0) {
    if(httpCode == HTTP_CODE_OK) {
      String payload = http1.getString();
      StaticJsonDocument<200> doc;
      deserializeJson(doc, payload);
      start_v = doc["start"].as<long>();
      left_led = doc["left_lamp"].as<long>();
      right_led = doc["right_lamp"].as<long>();
      multiplier = doc["speed"].as<long>();

    } else {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http1.errorToString(httpCode).c_str());
  }
}
}

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    blink_led(2,200); 
    Serial.print(".");
    c=c+1;
    if(c>10){
        ESP.restart(); //restart ESP after 10 seconds
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "PUM2023v2/BETA/MODE") {
      if(messageTemp == "AUTO"){
        start = clock();
        MODE = "AUTO";
        client.publish("PUM2023v2/BETA/AnswerR","AUTO MODE");
        Serial.print("AUTO MODE!!!!! ");
      }
      if(messageTemp == "MANUAL"){
        start = clock();
        MODE= "MANUAL";
        client.publish("PUM2023v2/BETA/AnswerR","MANUAL MODE");
        Serial.print("MANUAL MODE!!!!! ");
      }
    }
  if ((String(topic) == "PUM2023v2/BETA/CTR")&&(MODE == "MANUAL")) {
      start = clock();
      int DirectionV2 = 1;
      int DirectionO2 = 1;
      int Vlong = 0;
      int RelOmega;
      int RelValue;
      string VVal;
      string VOmega;
      string tempStrV;
      for (int i = 0; (((char)message[i]!=';')&&(i < length)); i++) {
      tempStrV[i]=(char)message[i];
      messageTemp += (char)message[i];
      Vlong++;
      }
      if(tempStrV[0]=='-'){
        DirectionV2 = -1;
        for (int i = 1; (i < Vlong); i++){
            VVal[i-1]=tempStrV[i];
        }
        Serial.print( "Kierunkek V - ");
      }
      if(tempStrV[0]=='+'){
        DirectionV2 = 1;
        for (int i = 1; (i < Vlong); i++){
            VVal[i-1]=tempStrV[i];
        }
        Serial.print( "Kierunkek V + ");
      }
      if((tempStrV[0]>=48)&&(tempStrV[0]<=57)){ 
        for (int i = 0; (i < Vlong); i++){
            VVal[i]=tempStrV[i];
        }
        Serial.print( "Kierunkek V domyslny (+) ");
      }
      for (int i = 0; (i < Vlong-1); i++){
            if((VVal[i]<48)&&(VVal[i]>57)){
              VVal="0";
            }
      }
      
      RelValue = stoi(VVal);
      Serial.print( "\nWartosc V: \n");
      Serial.print( RelValue);
      Serial.print( "\n");
      
      string tempStrO;
      for (int i = Vlong+1; ((i < length)); i++) {
      tempStrO[i-(Vlong+1)]=(char)message[i];
      messageTemp += (char)message[i];
      }

      if(tempStrO[0]=='-'){
        DirectionO2 = -1;
        for (int i = 1; (i < (length-Vlong+1)); i++){
            VOmega[i-1]=tempStrO[i];
        }
       Serial.print( "Kierunkek Omega - ");
      }
      if(tempStrO[0]=='+'){
        DirectionO2 = 1;
        for (int i = 1; (i < (length-Vlong+1)); i++){
            VOmega[i-1]=tempStrO[i];
        }
        Serial.print( "Kierunkek Omega + ");
      }

      if((tempStrO[0]>=48)&&(tempStrO[0]<=57)){ 
        for (int i = 0; (i < (length-Vlong+1)); i++){
            VOmega[i]=tempStrO[i];
        }
        Serial.print( "Kierunkek Omega domyslny (+) ");
      }  

      for (int i = 0; (i < (length-Vlong-2)); i++){
            if((VOmega[i]<48)&&(VOmega[i]>57)){
              VOmega="0";
            }
      }
      RelOmega = stoi(VOmega);
    int _RMotorD, _LMotorD;
    int _RMotorPWM, _LMotorPWM;

    if (RelOmega <= 20) { 
        _RMotorD = DirectionV2 == 1 ? HIGH : LOW;  
        _LMotorD = DirectionV2 == 1 ? HIGH : LOW;
        
        if (DirectionO2>0) {
        _RMotorPWM = map(abs(RelValue - RelValue * 0.01* RelOmega), 0, 100, 0, 255);
        _LMotorPWM = map(abs(RelValue), 0, 100, 0, 255);
        }
        if (DirectionO2<0) {
        _RMotorPWM = map(abs(RelValue), 0, 100, 0, 255);
        _LMotorPWM = map(abs(RelValue - RelValue * 0.01* RelOmega), 0, 100, 0, 255);

        }
    } else { 
        _RMotorD = DirectionO2 == 1 ? HIGH : LOW;  
        _LMotorD = DirectionO2 == 1 ? LOW : HIGH; 
        _RMotorPWM = map(RelOmega, 20, 100, 130, 255);
        _LMotorPWM = map(RelOmega, 20, 100, 130, 255);
    }
    setLeftMotor(_LMotorPWM,_LMotorD);
    setRightMotor(_RMotorPWM,_RMotorD);
    Serial.print( "\nL Value -  ");
    Serial.print(_LMotorPWM);
    Serial.print( "\nR Valiue -  ");
    Serial.print(_RMotorPWM);
    }
  }

void connect_mqttServer() {
  while (!client.connected()) {

    if(WiFi.status() != WL_CONNECTED){
      setup_wifi();
    }
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESPCTR0008";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe(inTopic);   // subscribe the topics here
    } else {
      temp = clock();
      czas = (double)(temp - start) / CLOCKS_PER_SEC;
        if(czas>2){
         analogWrite(LFMotorPWM, 0);
         analogWrite(LBMotorPWM, 0);
         analogWrite(RFMotorPWM, 0);
         analogWrite(RBMotorPWM, 0);
        }
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");  
      blink_led(3,200);
      delay(500);
    }
  }
}

void setup() {
  Serial.begin(115200); 
  Serial.println("");
  pinMode(LFMotorPWM, OUTPUT);
  pinMode(LBMotorPWM, OUTPUT);
  pinMode(RFMotorPWM, OUTPUT);
  pinMode(RBMotorPWM, OUTPUT);
  pinMode(lineSensorL, INPUT);
  pinMode(lineSensorM, INPUT);
  pinMode(lineSensorR, INPUT);
  stop();
  http1.useHTTP10(true);
  http1.begin(website+db);
  pinMode(ledPin, OUTPUT);
  blink_led(1, 3000);
  setup_wifi();
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);
  blink_led(2, 500);
  digitalWrite(ledPin,HIGH);
}

void loop() {
  receive_http();
  if (start_v==false) stop();
  else if (start_v==true){
  int SensorL = analogRead(lineSensorL);
  int SensorM = analogRead(lineSensorM);
  int SensorR = analogRead(lineSensorR);
  if (left_led == true) digitalWrite(ledPin,HIGH);
  else digitalWrite(ledPin,LOW);
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    String MQTTL = (String)SensorL;
    client.publish("PUM2023v2/BETA/Answer",MQTTL.c_str()); 
    String MQTTM = (String)SensorM;
    client.publish("PUM2023v2/BETA/Answer",MQTTM.c_str()); 
    String MQTTR = (String)SensorR;
    client.publish("PUM2023v2/BETA/AnswerR",MQTTR.c_str()); 
  }
  if (MODE =="AUTO"){

    if((SensorL > 4000)&&(SensorM > 4000)&&(SensorR > 4000)){
      setLeftMotor(255,1);
      setRightMotor(255,1);
    }
    if((SensorL > 4000)&&(SensorR < 4000)){
      setLeftMotor(255,0);
      setRightMotor(255,1);
    }
    if((SensorL < 4000)&&(SensorR > 4000)){
      setLeftMotor(255,1);
      setRightMotor(255,0);
    }
  }
  Serial.print("Start: ");
  Serial.println(start_v);
  Serial.print("Left led: ");
  Serial.println(left_led);
  Serial.print("Right led: ");
  Serial.println(right_led);
  Serial.print("Speed: ");
  Serial.println(multiplier);
  if (!client.connected()) {
    connect_mqttServer();
    delay(50);
  }
  client.loop();
  http1.end();
  delay(50);
}
}