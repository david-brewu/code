 #include "driver/gpio.h"
#include "driver/can.h"

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "BluetoothSerial.h"
#include <HTTPClient.h>

BluetoothSerial SerialBT; HTTPClient http; WiFiMulti WiFiMulti;

static const int RXPin = 16, TXPin = 17, GPSBaud = 9600;
int rpmSpeedID = 790, coolantTempID = 160;
//carSpeed, rpm;
//float coolantTemp;
unsigned long previousBLEblink = 0, previousRecieve=0, previousGPS = 0;

const long intervalBLE = 500, receiveInterval = 6000, receiveIntervalGPS = 4000;
TinyGPSPlus gps;
int CANSTATUS = 13, BLESTATUS = 14, WIFISTATUS = 12, BLEpinStatus = LOW;
SoftwareSerial ss(RXPin, TXPin);

void setup()
{ 
  Serial.begin(115200);
  ss.begin(GPSBaud);
  SerialBT.begin("CarMonitor");
  app_main();
  pinMode(CANSTATUS, OUTPUT);pinMode(WIFISTATUS, OUTPUT); pinMode(BLESTATUS, OUTPUT);
  WiFiMulti.addAP("BlackNetwork", "pi20222022");
  
}

void loop()
{
 
   unsigned long currentMillis = millis();
  checkCon(currentMillis);
  if (currentMillis - previousRecieve >= receiveInterval) {
    previousRecieve = currentMillis;
    receive_t();
  }

  if (currentMillis - previousGPS >= receiveIntervalGPS) {
    previousGPS = currentMillis;
    while (ss.available() > 0)
    if (gps.encode(ss.read())){
      gpsInfo();
   // }
    //else Serial.println("ss.read.unavailable");

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    //while(true);
 // }
  }
} 
}   }
    
void gpsInfo()
{
  Serial.print(F("Location: ")); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    //Serial.println();
    String gps_data = String(gps.location.lat(),6) + "," + String(gps.location.lng(),6);
    sendData("location",gps_data, " ");
}

void app_main()
{
    //Initialize configuration structures using macro initializers
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, CAN_MODE_NORMAL);
    can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    //Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    //Start CAN driver
    if (can_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }

}

void receive_t(){
  can_message_t message;
if (can_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
  digitalWrite(CANSTATUS, HIGH);
  Serial.printf("Message received");
  Serial.println();
} else {
   digitalWrite(CANSTATUS, LOW);
   Serial.printf("Failed to receive message");
  // Serial.println();
   return;
}
//printf("ID is %d\n", message.identifier);
if (!(message.flags & CAN_MSG_FLAG_RTR)) {
  if(message.identifier == rpmSpeedID){
    int carSpeed = message.data[6];
    int rpm = (message.data[2]*256 + message.data[3])/6.4;
    sendData("speed",String(carSpeed), "km/h");
    Serial.println("speed is: "); Serial.print(carSpeed);
    sendData("rpm",String(rpm),"rpm");
    Serial.println("rpm is: "); Serial.print(rpm);
    }
   if(message.identifier == coolantTempID){
    float coolantTemp = message.data[3]*0.75 - 48.373+70;
    float throttle = message.data[5]*(100/255);
    //float throttle = message.data[5];
    sendData2("coolantTemp",String(coolantTemp),"*C" );
    sendData2("throttle",String(throttle),"%" );
    Serial.println("Coolant Temp is: "); Serial.print(coolantTemp);
    Serial.println("Throttle Position is: "); Serial.print(throttle);
    
  }
}}

void checkCon(unsigned long currentMillis){
  if(WiFiMulti.run() == WL_CONNECTED){
       digitalWrite(WIFISTATUS , HIGH);
       if(SerialBT.available()){
       digitalWrite(BLESTATUS, HIGH);
       }else{digitalWrite(BLESTATUS, LOW);}
   }else{
     if(SerialBT.available()){
       digitalWrite(BLESTATUS, HIGH);
       }else{
      if (currentMillis - previousBLEblink >= intervalBLE) {
    previousBLEblink = currentMillis;
    digitalWrite(WIFISTATUS , LOW);
    if (BLEpinStatus == LOW) {
      BLEpinStatus = HIGH;
    } else {
      BLEpinStatus = LOW;
    }
    digitalWrite(BLESTATUS, BLEpinStatus);
  }}
}}

void sendData(String table, String _data, String unit){
   String completeURL = "http://davidbrewu.atwebpages.com/insert.php?table="+table+"&data="+String(_data);
   http.begin(completeURL.c_str());
  http.GET();
   String payload = http.getString();
   Serial.println(payload);
   http.end();
   String a = table+": "+String(_data)+" " + unit;
   uint8_t buf[a.length()];
   memcpy(buf, a.c_str(), a.length());
  SerialBT.write(buf,a.length());
  //SerialBT.println();
}


void sendData2(String table, String _data1, String unit){
  String completeURL = "http://20.117.122.111/Mobile/insert_carPara.php?table="+table+"&data="+ _data1;
   http.begin(completeURL.c_str());
  http.GET();
   String payload = http.getString();
   Serial.println(payload);
   http.end();
   String a = table+": "+String(_data1)+" " + unit;
   uint8_t buf[a.length()];
   memcpy(buf, a.c_str(), a.length());
  SerialBT.write(buf,a.length());
  SerialBT.println();
}
