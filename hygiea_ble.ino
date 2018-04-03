#include <SimpleTimer.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


SimpleTimer timer; //Timer para ejecutar funciones en cierto tiempo

/* The service information */
//For the CUSTOM Service
int32_t customServiceID;
int32_t customRXCharID;
int32_t customTXCharID;

//For Generic Access 0x1800
int32_t gaServiceID, dnCharID;
//For User Data (0x180F)
int32_t battServiceId;
int32_t battCharId;

//Hear rate service (0x180D)
int32_t heartRateServiceID;
int32_t measurementCharID; //Measurement ID (0x2A37)
int32_t bodysensorlocationCharID;

//For Enviromental Sensing (0x181A)
int32_t envSensing_ServiceID;
int32_t temp_CharID;

int data_in[4]; //battery_level, alert_level, elevation, temperature
int prev_data[] = {0,0,0,0};
int id = 0;
int update_id;
bool current_version, last_version;

//Catch the errors
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void repeatMe()
{    //Confirm name change
  //ble.sendCommandCheckOK("AT+GAPDEVNAME");
  ble.sendCommandCheckOK("AT+GATTLIST");
  //ble.sendCommandCheckOK("AT+GATTCHAR=3");
  //ble.reset();
  //ble.println(F("AT+GAPDEVNAME"));     // Request string from BLE module
  //ble.readline();     
}

bool success;

void setup()
{
  Serial.begin(115200);
  Wire.begin(5);
  Wire.onReceive(receiveEvent);
  timer.setInterval(5000,repeatMe);
  ble_setup();
  /* Wait for connection */
  /*while (! ble.isConnected()) {
    delay(500);
  }*/
  }

//int32_t respuesta = 0;
  void loop()
  {
    timer.run();
  //ble.isConnected();
  //ble.atcommandIntReply(F("AT+GATTCHAR=2"), &respuesta);
  //Serial.print("Index 2:  "); Serial.println(respuesta);    

  //ble.print(F("AT+GATTCHAR=6,")); ble.println(measurementCharID);
  }

  void receiveEvent(int howMany)
  {
    int x = Wire.available();
    char test;
    Serial.print("Wire available:  ");
    Serial.println(x);

    for(int i=0; i<=x; i++){
      test = Wire.read();
    data_in[i] = (int32_t)test; //Cast para que los datos recibidos en char se convieran en int32_t
    //data_in[i] = test;
  }

  Serial.println("Los datos que llegaron:  ");
  for (int i = 0; i <= 3; i++)
  {
    Serial.println(data_in[i]);
  }
//battery_level, alert_level, vacio, temperature
  while(id != 6){
    id =diff_array(prev_data, data_in);    
    if(id != 6){
      Serial.print("id es: "); Serial.println(id);
      current_version = !current_version;
      switch (id) {
        case 0:
        Serial.println("Bateria actualizada.");
        update_id = battCharId;
        break;
        case 1:
        Serial.println("Heart Rate Measurement.");
        update_id = measurementCharID;
        break;
        case 2:
        Serial.println("Dato vacio.");
          //update_id = elevation_CharID;        
        break;
        case 3:
        Serial.println("temperatura actualizada.");
        update_id = temp_CharID;        
        break;
        case 5:
        Serial.println("Error: Diferente longitud.");
        break;
      }
      prev_data[id] = data_in[id];
    }
    if(current_version != last_version){
      last_version = current_version;
      if( id == 1){
        update_ble(update_id, data_in[id]);        
      }else {
        update_test(update_id, data_in[id]);
      }      
    }
  }
  id = 0;
}

  /* Return 5 if the arrays aren't the same lenght; 6 if are equal; return a[n] when found a diff btw the 2 arrays. */
int diff_array(int *a, int *b){
  int n;
  if( sizeof(a) != sizeof(b) ){
    Serial.println(F("Not same length."));
    return 5;
  } 
  for(n= 0; n<=3;n++){
    Serial.print(a[n]); Serial.print("\t"); Serial.println(b[n]);
    if(a[n] != b[n]){
      return n;
    }
  }
  return 6;
}

void update_test(int32_t char_id, int value){
  ble.print( F("AT+GATTCHAR=") );
  ble.print(char_id);    
  ble.print(F(","));
  ble.println(value, HEX);
  if ( !ble.waitForOK() )
  {
    Serial.println(F("Failed to get response!"));
  }    
}

void update_ble(int32_t char_id, int value){
  ble.print( F("AT+GATTCHAR=") );
  ble.print(char_id);
  ble.print( F(",00-") );
  ble.println(value, HEX);
  if ( !ble.waitForOK() )
  {
    Serial.println(F("Failed to get response!"));
  }    
}

void ble_setup(){
  /*
  For properties field.
    0x02 - Read
    0x04 - Write Without Response
    0x08 - Write
    0x10 - Notify
    0x20 - Indicate
*/
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
  //ble.info();
  ble.update(250);
  
  /* Change the device name to make it easier to find */
  if (! ble.sendCommandCheckOK(F("AT+GAPDEVNAME=Pixky_SS18")) ) {
    error(F("Could not set device name?"));
  }
  
  /* Add Generic Access service definition */
  /*Serial.println(F("Adding the Generic Access Service definition(UUID=0x1800): "));
  success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID=0x1800"), &gaServiceID); 
  if (! success){
    error(F("Could not add Generic Access service"));
  }*/

    /* Add Device Name characteristic */
  /*Serial.println(F("Adding the Device Name characteristic(UUID=0x2A00): "));
  success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x2A00, PROPERTIES=0x02,MIN_LEN=8,VALUE=TEST, DATATYPE=1"), &dnCharID);
  if (! success) {
    error(F("Could not add Device Name characteristic"));
  }*/

  /* Add User Battery level service definition */
    Serial.println(F("Adding the Battery Level Service definition(UUID=0x180F): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID=0x180F"), &battServiceId); 
    if (! success){
      error(F("Could not add User Data service"));
    }
    /* Add battery level characteristic */
    Serial.println(F("Adding the Battery Level characteristic(UUID=0x2A19): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x2A19, PROPERTIES=0x10, MIN_LEN=1"), &battCharId);
    if (! success) {
      error(F("Could not add Battery Level characteristic"));
    }

  /* Add User Heart rate service definition */
    Serial.println(F("Adding the Heart Rate Service definition(UUID=0x180D): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID=0x180D"), &heartRateServiceID); 
    if (! success){
      error(F("Could not add Heart Rate service"));
    }
    /* Add Heart Rate Measurement characteristic */
    Serial.println(F("Adding the Heart Rate Measurement characteristic(UUID=0x2A37): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x2A37, PROPERTIES=0x10, MIN_LEN=2, MAX_LEN=3, VALUE=00-40"), &measurementCharID);
    if (! success) {
      error(F("Could not add Heart Rate Measurement characteristic"));
    }
    /* Add Body Sensor Location characteristic */
    Serial.println(F("Adding the Body Sensor Location characteristic(UUID=0x2A38): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x2A38, PROPERTIES=0x02, MIN_LEN=1, VALUE=3"), &bodysensorlocationCharID);
    if (! success) {
      error(F("Could not add Body Sensor Location characteristic"));
    }

    /* Add the Enviromental Sensing Service definition */
    Serial.println(F("Adding the Enviromental Sensing Service definition(UUID=0x181A): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x181A"), &envSensing_ServiceID);
    if (! success) {
      error(F("Could not add Enviromental Sensing service"));
    }
    /* Add the Temperature characteristic */
    Serial.println(F("Adding the Temperature characteristic(UUID=0x2A6E): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A6E, PROPERTIES=0x02,MIN_LEN=1"), &temp_CharID);
    if (! success) {
      error(F("Could not add Temperature characteristic"));
    }

    


  /* Add CUSTOM service definition */
  Serial.println(F("Adding CUSTOM Service definition(UUID=0x0001): "));
  success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID=0x0001"), &customServiceID); 
  if (! success){
    error(F("Could not add CUSTOM service"));
  }
    /* Add CUSTOM TX characteristic */
    Serial.println(F("Adding the CUSTOM TX characteristic(UUID=0x0002): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x0002, PROPERTIES=0x02, MIN_LEN=1, VALUE=0"), &customTXCharID);
    if (! success) {
      error(F("Could not add CUSTOM TX characteristic"));
    }
    /* Add RX characteristic */
    Serial.println(F("Adding the RX characteristic(UUID=0x0003): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x0003, PROPERTIES=0x08, MIN_LEN=1, VALUE=0"), &customRXCharID);
    if (! success) {
      error(F("Could not add RX characteristic"));
    }





  /* Reset the device for the new service setting changes to take effect */
    ble.reset();
    Serial.println("Setup finished.");
  }