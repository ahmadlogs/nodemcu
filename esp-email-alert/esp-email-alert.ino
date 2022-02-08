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
 * For ESP32
 * https://dl.espressif.com/dl/package_esp32_index.json
 * ------------------------------------------------------------------------
 * TinyGPS++ Library (Version 1.0.2b):
 * https://github.com/mikalhart/TinyGPSPlus
 * ------------------------------------------------------------------------
 * ESP Mail Client Library
 * https://github.com/mobizt/ESP-Mail-Client
 * ------------------------------------------------------------------------*/

//-------------------------------------------------
#include <Arduino.h>
#include <ESP_Mail_Client.h>
//-------------------------------------------------
#include <TinyGPS++.h>
TinyGPSPlus gps;
//-------------------------------------------------
//******************************************
#if defined(ESP32)
  #include <WiFi.h>
  //GPS Module RX pin to NodeMCU 17
  //GPS Module TX pin to NodeMCU 16
  #define RXD2 16
  #define TXD2 17
  HardwareSerial neo6m(2);

  #define pir_sensor 5
//******************************************
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <SoftwareSerial.h>
  //GPS Module RX pin to NodeMCU D3
  //GPS Module TX pin to NodeMCU D4
  const int rxPin = D4, txPin = D3;
  SoftwareSerial neo6m(rxPin, txPin);

  #define pir_sensor D5
#endif
//******************************************
//-------------------------------------------------
//The Login Credentials for your SMTP Server
#define AUTHOR_EMAIL "samsing@gmail.com"
#define AUTHOR_PASSWORD "somloe56u3@3"
//-------------------------------------------------
//Recipient's Email - Receiver's Email
#define RECIPIENT_EMAIL "ahmadlogs2021@gmail.com"
//-------------------------------------------------
//Gmail SMTP Server Settings
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465 //SMTP port (SSL)
//#define SMTP_PORT 587 //SMTP port (TLS)
//********************************************
//Outlook SMTP Server Settings
//#define SMTP_HOST "smtp.office365.com"
//#define SMTP_PORT 587
//********************************************
//Live or Hotmail SMTP Server Settings
//#define SMTP_HOST "smtp.live.com"
//#define SMTP_PORT 587
//-------------------------------------------------
#define WIFI_SSID "smarthome"
#define WIFI_PASSWORD "happynewyear"
//-------------------------------------------------
/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);
//-------------------------------------------------




/* **********************************************************************************
 * setup() function
 * **********************************************************************************/
void setup(){
  //-------------------------------------------------
  Serial.begin(115200);
  //-------------------------------------------------
  //initialize sensor as an input
  pinMode(pir_sensor, INPUT);
  //-------------------------------------------------
  #if defined(ESP32)
    neo6m.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //-------------------------------------------------
  #elif defined(ESP8266)
    neo6m.begin(9600);
  #endif
  //-------------------------------------------------
  //WiFi Connectivity
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //-------------------------------------------------
}




/* **********************************************************************************
 * loop() function
 * **********************************************************************************/
void loop(){

  //read sensor value
  int val = digitalRead(pir_sensor);
  //check if the Motion is Detected
  if (val == HIGH) {
    Serial.println("Motion detected!");
    send_email_alert();
  }

  delay(1000);
}




/* **********************************************************************************
 * send_email_alert() function
 * **********************************************************************************/
void send_email_alert(){
  //------------------------------------------------------------------
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;){
    while (neo6m.available()){
      if (gps.encode(neo6m.read()))
      {newData = true;break;}}
  }
  //------------------------------------------------------------------
  if(newData != true or gps.location.isValid() != 1)
  {Serial.println("No Valid GPS Data is Found.");return;}
  //------------------------------------------------------------------
  newData = false;
  String latitude = String(gps.location.lat(), 6);
  String longitude = String(gps.location.lng(), 6);
  Serial.println("Latitude: "+ latitude);
  Serial.println("Longitude: "+ longitude);
  return;
  //------------------------------------------------------------------
  //Enable the debug via Serial port | No debug = 0 | Start debug = 1
  smtp.debug(1);
  //-------------------------------------------------
  //Set the callback function to get the sending results
  smtp.callback(smtpCallback);
  //-------------------------------------------------
  //Declare the session config data
  ESP_Mail_Session session;
  //-------------------------------------------------
  //Set the session config
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  //-------------------------------------------------
  //Declare the message class
  SMTP_Message message;
  //-------------------------------------------------
  //Set the message headers
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "ESP Motion Alert";
  message.addRecipient("AhmadLogs", RECIPIENT_EMAIL);
  //-------------------------------------------------
  //Send HTML message
  String htmlMsg = "<div style=\"color:#2f4468;\">";
  htmlMsg += "<h1>Latitude: "+latitude+"</h1>";
  htmlMsg += "<h1>Longitude: "+longitude+"!</h1>";
  htmlMsg += "<h1><a href=\"http://maps.google.com/maps?q=loc:"+latitude+","+longitude+"\">Check Location in Google Maps</a></h1>";
  htmlMsg += "<p>- Sent from ESP board</p></div>";
  //Serial.println("htmlMsg: "+ htmlMsg);
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  //-------------------------------------------------
  /*
  //Send raw text message
  String textMsg = "Hello World! - Sent from ESP board";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;*/

  /* Set the custom message header */
  //message.addHeader("Message-ID: <abcde.fghij@gmail.com>");
  //-------------------------------------------------
  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;
  //-------------------------------------------------
  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
  //-------------------------------------------------
}

/* **********************************************************************************
 * smtpCallback() function
 * **********************************************************************************/
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
