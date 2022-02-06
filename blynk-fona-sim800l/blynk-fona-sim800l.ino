/*
 * ------------------------------------------------------------------------
 * Created by: Tauseef Ahmad
 * Created on: 27 January 2022
 *  
 * ------------------------------------------------------------------------
 * Download Resources
 * ------------------------------------------------------------------------
 * Preferences--> Aditional boards Manager URLs : 
 * For ESP8266 and NodeMCU:
 * http://arduino.esp8266.com/stable/package_esp8266com_index.json
 * ------------------------------------------------------------------------
 * Download latest Blynk Library:
 * https://github.com/blynkkk/blynk-library/releases/latest
 * ------------------------------------------------------------------------
 * Adafruit FONA Library
 * https://github.com/adafruit/Adafruit_FONA
 * ------------------------------------------------------------------------*/


//------------------------------------------------------------------
#define BLYNK_PRINT Serial
#define TINY_GSM_MODEM_SIM800
//------------------------------------------------------------------

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
//GPS Module RX pin to NodeMCU D3
//GPS Module TX pin to NodeMCU D4
#define rxPin D4
#define txPin D3
#define FONA_RST 13
SoftwareSerial Sim800L(rxPin,txPin);
SoftwareSerial *fonaSerial = &Sim800L;
//Hardware serial is also possible! for ESP32
//HardwareSerial *fonaSerial = &Serial2;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


//------------------------------------------------------------------
//#define BLYNK_TEMPLATE_ID "ENTER_TEMPLATE_ID"
//#define BLYNK_DEVICE_NAME "ENTER_DEVICE_NAME"
//#define BLYNK_AUTH_TOKEN "ENTER_AUTH_TOKEN"
//------------------------------------------------------------------

#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
//------------------------------------------------------------------
char auth[] = BLYNK_AUTH_TOKEN;
// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "";
char user[] = "";
char pass[] = "";
//------------------------------------------------------------------
TinyGsm modem(fona);
BlynkTimer timer;
//------------------------------------------------------------------
//NOTE: Enter the phone number that you want to register with the project
//You can only control the project, with the phone number you entered here
//Must enter your personal phone number with country code.
//Make Sure: never enter the gsm module's phone number here.

const String PHONE = "ENTER_YOUR_PERSONAL_PHONE_NUMBER";
//------------------------------------------------------------------
#define pin_relay1 D5
#define pin_relay2 D6
#define pin_relay3 D7
#define pin_relay4 D8
//------------------------------------------------------------------
int state_relay1 = 0;
int state_relay2 = 0;
int state_relay3 = 0;
int state_relay4 = 0;
//------------------------------------------------------------------
//Change the virtual pins, as you have set in the blynk account.
#define virtual_pin1    V1
#define virtual_pin2    V2
#define virtual_pin3    V3
#define virtual_pin4    V4
//------------------------------------------------------------------
String gsm_buff ="";
char sendsms[15];
char caller_id[32];
char sms_buffer[255];
int len=0;


//------------------------------------------------------------------
BLYNK_WRITE(virtual_pin1) {
  state_relay1 = param.asInt();
  digitalWrite(pin_relay1, state_relay1);
   Serial.print("Relay 1 is ");
   if(state_relay1==0)
   Serial.println("OFF");
   else
   Serial.println("ON");
}
//------------------------------------------------------------------
BLYNK_WRITE(virtual_pin2) {
  state_relay2 = param.asInt();
  digitalWrite(pin_relay2, state_relay2);
   Serial.print("Relay 2 is ");
   if(state_relay2==0)
   Serial.println("OFF");
   else
   Serial.println("ON");
}
//------------------------------------------------------------------
BLYNK_WRITE(virtual_pin3) {
  state_relay3 = param.asInt();
  digitalWrite(pin_relay3, state_relay3);
   Serial.print("Relay 3 is ");
   if(state_relay3==0)
   Serial.println("OFF");
   else
   Serial.println("ON");
}
//------------------------------------------------------------------
BLYNK_WRITE(virtual_pin4) {
  state_relay4 = param.asInt();
  digitalWrite(pin_relay4, state_relay4);
   Serial.print("Relay 4 is ");
   if(state_relay4==0)
   Serial.println("OFF");
   else
   Serial.println("ON");
}
//------------------------------------------------------------------

void handle_sms(){
  while(Serial.available())  {
    fona.println(Serial.readString());
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  gsm_buff ="";
  //+CMTI: "SM",1 (here! 1 is slot)
  int slot = 0;
  uint16_t smslen;
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //any data available from the fona?
  gsm_buff= fona.readString();
  //if(gsm_buff.length() > 0)
    //{Serial.println();Serial.println(gsm_buff);}
  len = gsm_buff.length();
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //if(gsm_buff.indexOf("+CMTI:")) {
  if (len >10) {
    gsm_buff =gsm_buff.substring(14,len);
    slot = gsm_buff.toInt();
    Serial.print("Slot: ");
    Serial.println(slot);
    if (slot == 0) {
      return;
    }
    //------------------------------------------------------------
    if (!fona.readSMS(slot, sms_buffer, 250, &smslen)) {  // pass in buffer and max len!
      Serial.println(F("Failed!"));
      return;
    }
    //------------------------------------------------------------
    if (! fona.getSMSSender(slot, caller_id, 31)) {
      Serial.println("Didn't find SMS message in slot!");
      return;
    }
    //------------------------------------------------------------
    if(PHONE != String(caller_id)){
      Serial.println("The phone number is not Registered.");
      Serial.println("Please use the Registered Phone Number.");
    }
    else 
    {
      //_________________________________________
      String sms_temp = sms_buffer;
      sms_temp.toLowerCase();
      String caller_temp = caller_id;
      Serial.println("SMS: "+sms_temp);
      Serial.println("From: "+caller_temp);
      //_________________________________________
      if(sms_temp == "1on"){
        state_relay1 =1;
        digitalWrite(pin_relay1, HIGH);
        Blynk.virtualWrite(virtual_pin1, state_relay1);
        fona.sendSMS(caller_id, "Relay 1 is ON");
        Serial.println("Response: Relay 1 is ON");
      }
      //_________________________________________
      if(sms_temp == "1off"){
        state_relay1 =0;
        digitalWrite(pin_relay1, LOW);
        Blynk.virtualWrite(virtual_pin1, state_relay1);
        fona.sendSMS(caller_id, "Relay 1 is OFF");
        Serial.println("Response: Relay 1 is OFF");
      }
      //_________________________________________
      if(sms_temp == "2on"){
        state_relay2 =1;
        digitalWrite(pin_relay2, HIGH);
        Blynk.virtualWrite(virtual_pin2, state_relay2);
        fona.sendSMS(caller_id, "Relay 2 is ON");
        Serial.println("Response: Relay 2 is ON");
      }
      //_________________________________________
      if(sms_temp == "2off"){
        state_relay2 =0;
        digitalWrite(pin_relay2, LOW);
        Blynk.virtualWrite(virtual_pin2, state_relay2);
        fona.sendSMS(caller_id, "Relay 2 is OFF");
        Serial.println("Response: Relay 2 is OFF");
      }
      //_________________________________________ 
      if(sms_temp == "3on"){
        state_relay3 =1;
        digitalWrite(pin_relay3, HIGH);
        fona.sendSMS(caller_id, "Relay 3 is ON");
        Blynk.virtualWrite(virtual_pin3, state_relay3);
        Serial.println("Response: Relay 3 is ON");
      }
      //_________________________________________
      if(sms_temp == "3off"){
        state_relay3 =0;
        digitalWrite(pin_relay3, LOW);
        fona.sendSMS(caller_id, "Relay 3 is OFF");
        Blynk.virtualWrite(virtual_pin3, state_relay3);
        Serial.println("Response: Relay 3 is OFF");
      }
      //_________________________________________
      if(sms_temp == "4on"){
        state_relay4 =1;
        digitalWrite(pin_relay4, HIGH);
        Blynk.virtualWrite(virtual_pin4, state_relay4);
        fona.sendSMS(caller_id, "Relay 4 is ON");
        Serial.println("Response: Relay 4 is ON");
      }
      //_________________________________________
      if(sms_temp == "4off"){
        state_relay4 =0;
        digitalWrite(pin_relay4, LOW);
        Blynk.virtualWrite(virtual_pin4, state_relay4);
        fona.sendSMS(caller_id, "Relay 4 is OFF");
        Serial.println("Response: Relay 4 is OFF");
      }
      //_________________________________________
      else
        {
          fona.sendSMS(caller_id, "ERROR: Send SMS with valid command");
        }
      //_________________________________________
    }
    //------------------------------------------------------------
    fona.deleteSMS(slot);
    fona.print(F("AT+CMGD=1,4\n\r"));
    fona.print(F("AT+CMGDA= \"DEL ALL\""));
    //------------------------------------------------------------
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}
 

 

void setup()
{
  Serial.begin(115200);
    //--------------------------------------------------------------------
  pinMode(pin_relay1, OUTPUT);
  pinMode(pin_relay2, OUTPUT);
  pinMode(pin_relay3, OUTPUT);
  pinMode(pin_relay4, OUTPUT);
  //--------------------------------------------------------------------
  //During Starting all Relays should TURN OFF
  digitalWrite(pin_relay1, LOW);
  digitalWrite(pin_relay2, LOW);
  digitalWrite(pin_relay3, LOW);
  digitalWrite(pin_relay4, LOW);
  //--------------------------------------------------------------------
  delay(2000);
   fonaSerial->begin(9600);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while(1);
  }
  //--------------------------------------------------------------------
  Serial.println(F("FONA is OK"));
  fona.println("AT+CMGF=1"); // Configuring TEXT mode
  delay(1000);
  fona.print ("AT+CSMP=17,167,0,0\r");// Configuring TEXT mode
  delay(1000);
  fona.print("AT+CNMI=2,1\r\n");  //set up the fona to send a +CMTI notification when an SMS is received
 
  //fona.println(F("AT+CMGDA=\"DEL ALL\""));
  //delay(5000);
  delay(1000);
  Serial.println("FONA Ready");
  //--------------------------------------------------------------------
  modem.restart();
  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  Blynk.begin(auth, modem, apn, user, pass);
  Blynk.virtualWrite(virtual_pin1, state_relay1);
  Blynk.virtualWrite(virtual_pin2, state_relay2);
  Blynk.virtualWrite(virtual_pin3, state_relay3);
  Blynk.virtualWrite(virtual_pin4, state_relay4);
  //--------------------------------------------------------------------
  timer.setInterval(1L, handle_sms);
}

void loop()
{
  Blynk.run();
  timer.run();
}
