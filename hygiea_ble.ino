#include <SimpleTimer.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"
#include <Wire.h>

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//Catch the errors
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
SimpleTimer timer;

/* The service information */
//For Generic Access (0x1800);
int32_t gA_ServiceID;
int32_t appearance_CharID;

//For User Data (0x180F)
int32_t battServiceId;
int32_t battCharId;

//For Immediate alert (0x1802)
int32_t iAServiceId;
int32_t alertLCharId;

//For Enviromental Sensing (0x181A)
int32_t envSensing_ServiceID;
int32_t elevation_CharID;
int32_t temp_CharID;

//For Link Loss (0x1803)
int32_t linkLoss_ServiceID;
int32_t ll_AlertLevel_CharID;

int alert_level;
int battery_level = 0;
int data_in[3]; //Batt_level,
int elevation, temperature;
int id, update_id;
bool current_version, last_version;

void repeatMe(){
  /*battery_level ++;
  ble.print(F("AT+GATTCHAR="));
  ble.print(battCharId);
  ble.print(F(",00-"));
  ble.println(battery_level, HEX);
  if ( !ble.waitForOK() )
    {
      Serial.println(F("Failed to get response!"));
    }*/
    }

    void setup()
    {
      boolean success;
      Serial.begin(9600);

      timer.setInterval(5000,repeatMe);

      Wire.begin(5);
      Wire.onReceive(receiveEvent);
      randomSeed(micros());

  /* Initialise the module */
      if ( !ble.begin(VERBOSE_MODE) )
      {
        error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
      }
  /* Perform a factory reset to make sure everything is in a known state */
      if (! ble.factoryReset() ){
       error(F("Couldn't factory reset"));
     }
  /* Disable command echo from Bluefruit */
     ble.echo(false);
  /* Print Bluefruit information */
     ble.info();
  /* Change the device name to make it easier to find */
     if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Hygiea")) ) {
      error(F("Could not set device name?"));
    }
  /* Add User Battery level service definition */
    Serial.println(F("Adding the Battery Level Service definition(UUID=0x180F): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID=0x180F"), &battServiceId); 
    if (! success){
      error(F("Could not add User Data service"));
    }
    /* Add battery level characteristic */
    Serial.println(F("Adding the Battery Level characteristic(UUID=0x2A19): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x2A19, PROPERTIES=0x10, MIN_LEN=1, VALUE=100"), &battCharId);
    if (! success) {
      error(F("Could not add Battery Level characteristic"));
    }
  /* Add the Immediate Alert Service definition */
    Serial.println(F("Adding the Immediate Alert Service definition(UUID=0x1802): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x1802"), &iAServiceId);
    if (! success) {
      error(F("Could not add Immediate Alert service"));
    }
    /* Add the Alert Level characteristic */
    Serial.println(F("Adding the Alert Level characteristic(UUID=0x2A06): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A06,PROPERTIES=0x04,MIN_LEN=1,VALUE=0"), &alertLCharId);
    if (! success) {
      error(F("Could not add Alert Level characteristic"));
    }
  /* Add the Enviromental Sensing Service definition */
    Serial.println(F("Adding the Enviromental Sensing Service definition(UUID=0x181A): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x181A"), &envSensing_ServiceID);
    if (! success) {
      error(F("Could not add Enviromental Sensing service"));
    }
    /* Add the Elevation characteristic */
    Serial.println(F("Adding the Elevation characteristic(UUID=0x2A6C): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A6C,PROPERTIES=0x02,MIN_LEN=1"), &elevation_CharID);
    if (! success) {
      error(F("Could not add Elevation characteristic"));
    }
    /* Add the Temperature characteristic */
    Serial.println(F("Adding the Temperature characteristic(UUID=0x2A6E): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A6E, PROPERTIES=0x02,MIN_LEN=1"), &temp_CharID);
    if (! success) {
      error(F("Could not add Temperature characteristic"));
    }
  /* Add the Link Loss Service definition */
    Serial.println(F("Adding the Link Loss Service definition(UUID=0x1803): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x1803"), &linkLoss_ServiceID);
    if (! success) {
      error(F("Could not add Link Loss service"));
    }
    /* Add the Link Loss Alert Level characteristic */
    Serial.println(F("Adding the Link Loss Alert Level characteristic(UUID=0x2A06): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A06,PROPERTIES=0x04,MIN_LEN=1,VALUE=0"), &ll_AlertLevel_CharID);
    if (! success) {
      error(F("Could not add Link Loss Alert Level characteristic"));
    }
  /* Reset the device for new the service setting changes to take effect */
    ble.reset();
    Serial.println("Setup finished.");
    delay(3000);
  }

  void loop()
  {
    timer.run();      
    if(current_version != last_version){
      last_version = current_version;
      update_ble(update_id, data_in[id]);
    }
  }

  void receiveEvent(int howMany)
  {
    int x = Wire.available();
    Serial.print("Wire available:  ");
    Serial.println(x);
    int prev_data[] = {battery_level, alert_level, elevation, temperature};
    for(int i=0; i<x; i++){
      data_in[i] = Wire.read();    
    }
    id =diff_array(prev_data, data_in);
    if(id != 6){
      current_version = !current_version;
      switch (id) {
        case 0:
        Serial.println("Bateria actualizada.");
        update_id = battCharId;
        break;
        case 1:
        Serial.println("Alerta actualizada.");
        update_id = alertLCharId;
        break;
        case 2:
        Serial.println("elevacion actualizada.");
        update_id = elevation_CharID;
        break;
        case 3:
        Serial.println("temperatura actualizada.");
        update_id = temp_CharID;
        break;
        case 6:
        Serial.println("Error: Diferente longitud.");
        break;
      }
    }
  }

  /* Return 5 if the arrays aren't the same lenght; 6 if are equal; return a[n] when found a diff btw the 2 arrays. */
  int diff_array(int *a, int *b){
    int n;
    if(sizeof(a)!=sizeof(b)){
      Serial.println(F("Not same length."));
      return 5;
    } 
    for(n=sizeof(b); n>0;n--){
      if(a[n]!= b[n]){
        return n;
      }
    }
    return 6;
  }

  void update_ble(int char_id, int value){
    ble.print( F("AT+GATTCHAR=") );
    ble.print( char_id);
    ble.print( F(",00-") );
    ble.println(value, HEX);
    if ( !ble.waitForOK() )
    {
      Serial.println(F("Failed to get response!"));
    }
  }