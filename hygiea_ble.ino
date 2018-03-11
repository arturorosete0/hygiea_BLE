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

/* The service information */
//For Heart Rate (0x180D)
int32_t hrmServiceId;
int32_t hrmMeasureCharId;
int32_t hrmLocationCharId;

//For User Data (0x180F)
int32_t battServiceId;
int32_t battCharId;

//For Immediate alert (0x1802)
int32_t iAServiceId;
int32_t alertLCharId;

//int heart_rate;
int alert_level = 200;

void setup(void)
{
  boolean success;
  Serial.begin(115200);
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

  /* Add the Heart Rate Service definition */
  /* Service ID should be 1 */
  //Serial.println(F("Adding the Heart Rate Service definition (UUID = 0x180D): "));
  //success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x180D"), &hrmServiceId);
  //if (! success) {
  //  error(F("Could not add HRM service"));
  //}
  /* Add the Heart Rate Measurement characteristic */
  /* Chars ID for Measurement should be 1 */
  //Serial.println(F("Adding the Heart Rate Measurement characteristic (UUID = 0x2A37): "));
  //success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A37, PROPERTIES=0x10, MIN_LEN=2, MAX_LEN=3, VALUE=00-70"), &hrmMeasureCharId);
  //    if (! success) {
    //error(F("Could not add HRM characteristic"));
  //}
  /* Add the Body Sensor Location characteristic */
  /* Chars ID for Body should be 2 */
  //Serial.println(F("Adding the Body Sensor Location characteristic (UUID = 0x2A38): "));
  //success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A38, PROPERTIES=0x02, MIN_LEN=1, VALUE=3"), &hrmLocationCharId);
  //    if (! success) {
    //error(F("Could not add BSL characteristic"));
  //}

  /* Add User Battery level service definition */
  Serial.println(F("Adding the Battery Level Service definition (UUID = 0x180F): "));
  success = ble.sendCommandWithIntReply(F("AT+GATTADDSERVICE=UUID=0x180F"), &battServiceId); 
  if (! success){
    error(F("Could not add User Data service"));
  }

    /* Add battery level characteristic */
    Serial.println(F("Adding the Battery Level characteristic (UUID = 0x2A19): "));
    success = ble.sendCommandWithIntReply(F("AT+GATTADDCHAR=UUID=0x2A19, PROPERTIES=0x10, MIN_LEN=1, VALUE=100"), &battCharId);
    if (! success) {
      error(F("Could not add Battery Level characteristic"));
    }


  /* Add the Immediate Alert Service definition */
  Serial.println(F("Adding the Immediate Alert Service definition (UUID = 0x1802): "));
  success = ble.sendCommandWithIntReply( F("AT+GATTADDSERVICE=UUID=0x1802"), &iAServiceId);
  if (! success) {
    error(F("Could not add Immediate Alert service"));
  }
    /* Add the Alert Level characteristic */
    Serial.println(F("Adding the Alert Level characteristic (UUID = 0x2A06): "));
    success = ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2A06, MIN_LEN=1,VALUE=0"), &alertLCharId);
    if (! success) {
      error(F("Could not add Alert Level characteristic"));
    }

  /* Reset the device for the new service setting changes to take effect */
  ble.reset();

  ble.print( F("AT+GATTCHAR=") );
  ble.print( hrmMeasureCharId);
  ble.print( F(",00-") );
  ble.println(alert_level, HEX);
  /* Check if command executed OK */
    if ( !ble.waitForOK() )
      {
        Serial.println(F("Failed to get response!"));
      }
    
    delay(100);
}

void loop(void)
{
  /** Send randomized heart rate data continuously **/
  //int battery_level = random(15, 100);

  /* Command is sent when \n (\r) or println is called */
  /* AT+GATTCHAR=CharacteristicID,value */
  /*ble.print( F("AT+GATTCHAR=") );
  ble.print( hrmMeasureCharId);
  ble.print( F(",00-") );
  ble.println(alert_level, HEX);*/
  
  /*ble.print(F("AT+GATTCHAR="));
  ble.print(battCharId);
  ble.print(F(",00-"));
  ble.println(battery_level, HEX);*/
   
  /*ble.print(F("AT+GATTCHAR="));
  ble.print(alertLCharId);
  ble.print(F(",00-"));
  ble.println(alert_level, HEX);*/
  
}

void receiveEvent(int howMany)
{
  alert_level = Wire.read();
  ble.print( F("AT+GATTCHAR=") );
  ble.print( hrmMeasureCharId);
  ble.print( F(",00-") );
  ble.println(alert_level, HEX);
  Serial.print("Valor: "); Serial.println(alert_level);
    
  if ( !ble.waitForOK() )
    {
      Serial.println(F("Failed to get response!"));
    }
}