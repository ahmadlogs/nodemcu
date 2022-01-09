/* ****************************************************************************
 * Created By: Tauseef Ahmad
 * Create On: 29 December, 2021
 * 
 * NodeMCU SIM800L GPS Tracker with Register Phone Number | ESP8266 Call based GPS Tracker
 * Tutorial: https://youtu.be/HrIBGJep-1o
 * 
 * ****************************************************************************
 * Download Resources
 * ****************************************************************************
 * Install ESP8266 Board
 * http://arduino.esp826at+cmgr=76.com/stable/package_esp8266com_index.json
 * 
 * Install TinyGPS++ library
 * https://github.com/mikalhart/TinyGPSPlus
 * http://arduiniana.org/libraries/tinygpsplus
 * 
 *****************************************************************************/

#include <SoftwareSerial.h>
#include <TinyGPS++.h>

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#include <EEPROM.h>
//sender phone number with country code
//const String PHONE = "ENTER_PHONE_HERE";
const int totalPhoneNo = 5;
String phoneNo[totalPhoneNo] = {"","","","",""};
int offsetPhone[totalPhoneNo] = {0,13,26,39,52};
String tempPhone = "";
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//GPS Module TX pin to NodeMCU D1
//GPS Module RX pin to NodeMCU D2
#define rxGPS D1
#define txGPS D2
SoftwareSerial neogps(rxGPS,txGPS);
TinyGPSPlus gps;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//GSM Module TX pin to NodeMCU D3
//GSM Module RX pin to NodeMCU D4
#define rxGSM D3
#define txGSM D4
SoftwareSerial sim800(rxGSM,txGSM);
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
String smsStatus;
String senderNumber;
String receivedDate;
String msg;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


boolean DEBUG_MODE = 1;



/*******************************************************************************
 * setup function
 ******************************************************************************/
void setup() {
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  Serial.begin(115200);
  Serial.println("NodeMCU USB serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  sim800.begin(9600);
  Serial.println("SIM800L serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  neogps.begin(9600);
  Serial.println("NEO6M serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  sim800.listen();
  neogps.listen();
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //commit 512 bytes of ESP8266 flash (for "EEPROM" emulation)
  //this step actually loads the content (512 bytes) of flash
  //into a 512-byte-array cache in RAM
  EEPROM.begin(512);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  Serial.println("List of Registered Phone Numbers");
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    if(phoneNo[i].length() != 13)
      {phoneNo[i] = "";Serial.println(String(i+1)+": empty");}
    else
      {Serial.println(String(i+1)+": "+phoneNo[i]);}
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  delay(9000);
  sim800.println("AT+CMGF=1"); //SMS text mode
  delay(1000);
  sim800.println("AT+CLIP=1"); //Enable Caller ID
  delay(500);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}




/*******************************************************************************
 * loop function
 ******************************************************************************/
void loop() {
  //sim800.listen();
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
while(sim800.available()){
  parseData(sim800.readString());
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
while(Serial.available())  {
  sim800.println(Serial.readString());
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
} //main loop ends





/*******************************************************************************
 * parseData function:
 * this function parse the incomming command such as CMTI or CMGR etc.
 * if the sms is received. then this function read that sms and then pass 
 * that sms to "extractSms" function. Then "extractSms" function divide the
 * sms into parts. such as sender_phone, sms_body, received_date etc.
 ******************************************************************************/
void parseData(String buff){
  Serial.println(buff);

  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
  if(buff.indexOf("RING") > -1)
  {
    boolean flag = 0;
    String callerID = "";
    //----------------------------------------------------------------------
    //+CLIP: "03001234567",129,"",0,"",0
    if(buff.indexOf("+CLIP:")){
     unsigned int index, index1;
     
      index = buff.indexOf("\"");
      index1 = buff.indexOf("\"", index+1);
      //get phone like this format 3001234567
      callerID = buff.substring(index+2, index1);
      callerID.trim();
      //debugPrint("caller ID: "+callerID);
      //Serial.println(temp.length());
      if(callerID.length() == 13){
        //number formate xxx-yyy-zzzzzzz
        //+923001234567
        flag = comparePhone(callerID);
      }
      //--------------------------------------------------------------------
      else if(callerID.length() == 10){
        //number formate yyyy-zzzzzzz
        //3001234567
        flag = compareWithoutCountryCode(callerID);
        callerID = "0" + callerID;
      }
    }
    //----------------------------------------------------------------------
    if(flag == 1){
      sim800.println("ATH");
      delay(1000);
      sendLocation(callerID);
    }
    else{
      sim800.println("ATH");
      debugPrint("The phone number is not registered.");
    }
    //----------------------------------------------------------------------
    return;
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  
  unsigned int len, index;
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();
  //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    if(cmd == "+CMTI"){
      //get newly arrived memory location and store it in temp
      index = buff.indexOf(",");
      String temp = buff.substring(index+1, buff.length()); 
      temp = "AT+CMGR=" + temp + "\r"; 
      //get the message stored at memory location "temp"
      sim800.println(temp); 
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    else if(cmd == "+CMGR"){
      extractSms(buff);
      //-------------------------------------------------------
      if(msg.equals("r") && phoneNo[0].length() != 13) {
        writeToEEPROM(offsetPhone[0],senderNumber);
        phoneNo[0] = senderNumber;
        String text = "Number is Registered: ";
        text = text + senderNumber;
        debugPrint(text);
        Reply("Number is Registered", senderNumber);
      }
      //-------------------------------------------------------
      if(comparePhone(senderNumber)){
        doAction(senderNumber);
        //sendLocation();
      }
      //-------------------------------------------------------
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  }
  else{
  //The result of AT Command is "OK"
  }
}





/*******************************************************************************
 * doAction function:
 * Performs action according to the received sms
 ******************************************************************************/
void doAction(String phoneNumber){
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "send location"){  
      sendLocation(phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "r2="){  
      //mmmmmmm
      Serial.println(offsetPhone[1]);
      writeToEEPROM(offsetPhone[1],tempPhone);
      phoneNo[1] = tempPhone;
      String text = "Phone2 is Registered: ";
      text = text + tempPhone;
      debugPrint(text);
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r3="){  
      writeToEEPROM(offsetPhone[2],tempPhone);
      phoneNo[2] = tempPhone;
      String text = "Phone3 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r4="){  
      writeToEEPROM(offsetPhone[3],tempPhone);
      phoneNo[3] = tempPhone;
      String text = "Phone4 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r5="){  
      writeToEEPROM(offsetPhone[4],tempPhone);
      phoneNo[4] = tempPhone;
      String text = "Phone5 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "list"){  
      String text = "";
      if(phoneNo[0])
        text = text + phoneNo[0]+"\r\n";
      if(phoneNo[1])
        text = text + phoneNo[1]+"\r\n";
      if(phoneNo[2])
        text = text + phoneNo[2]+"\r\n";
      if(phoneNo[3])
        text = text + phoneNo[3]+"\r\n";
      if(phoneNo[4])
        text = text + phoneNo[4]+"\r\n";
        
      debugPrint("List of Registered Phone Numbers: \r\n"+text);
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=1"){  
      writeToEEPROM(offsetPhone[0],"");
      phoneNo[0] = "";
      Reply("Phone1 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=2"){  
      writeToEEPROM(offsetPhone[1],"");
      phoneNo[1] = "";
      debugPrint("Phone2 is deleted.");
      Reply("Phone2 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=3"){  
      writeToEEPROM(offsetPhone[2],"");
      phoneNo[2] = "";
      debugPrint("Phone3 is deleted.");
      Reply("Phone3 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=4"){  
      writeToEEPROM(offsetPhone[3],"");
      phoneNo[3] = "";
      debugPrint("Phone4 is deleted.");
      Reply("Phone4 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=5"){  
      writeToEEPROM(offsetPhone[4],"");
      phoneNo[4] = "";
      debugPrint("Phone5 is deleted.");
      Reply("Phone5 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  
  if(msg == "del=all"){  
      writeToEEPROM(offsetPhone[0],"");
      writeToEEPROM(offsetPhone[1],"");
      writeToEEPROM(offsetPhone[2],"");
      writeToEEPROM(offsetPhone[3],"");
      writeToEEPROM(offsetPhone[4],"");
      phoneNo[0] = "";
      phoneNo[1] = "";
      phoneNo[2] = "";
      phoneNo[3] = "";
      phoneNo[4] = "";
      offsetPhone[0] = NULL;
      offsetPhone[1] = NULL;
      offsetPhone[2] = NULL;
      offsetPhone[3] = NULL;
      offsetPhone[4] = NULL;
      debugPrint("All phone numbers are deleted.");
      Reply("All phone numbers are deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  tempPhone = "";
}





/*******************************************************************************
 * extractSms function:
 * This function divide the sms into parts. such as sender_phone, sms_body, 
 * received_date etc.
 ******************************************************************************/
void extractSms(String buff){
   unsigned int index;
   
    index = buff.indexOf(",");
    smsStatus = buff.substring(1, index-1); 
    buff.remove(0, index+2);
    
    senderNumber = buff.substring(0, 13);
    buff.remove(0,19);
   
    receivedDate = buff.substring(0, 20);
    buff.remove(0,buff.indexOf("\r"));
    buff.trim();
    
    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();
    msg = buff;
    buff = "";
    msg.toLowerCase();

    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    String tempcmd = msg.substring(0, 3);
    if(tempcmd.equals("r1=") || tempcmd.equals("r2=") ||
       tempcmd.equals("r3=") || tempcmd.equals("r4=") ||
       tempcmd.equals("r5=")){
        
        tempPhone = msg.substring(3, 16);
        msg = tempcmd;
        //debugPrint(msg);
        //debugPrint(tempPhone);
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
       
}




/*******************************************************************************
 * Reply function
 * Send an sms
 ******************************************************************************/
void Reply(String text, String Phone)
{
  //return;
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\""+Phone+"\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");

  /*************************************************************
   * only upto 70 messages can be stored in sim800l memory
   * after the memory is full, then no new sms will be received
   * until you free up the sms memory by deleting some sms.
   * that's why the below written command deletes all the sms
   * from the memory automatically. 
   *************************************************************/
  //sim800.print("AT+CMGD=1,4");


  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  tempPhone = "";  
}





/*******************************************************************************
 * writeToEEPROM function:
 * Store registered phone numbers in EEPROM
 ******************************************************************************/
void writeToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = 13; //strToWrite.length();
  //EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    //meee
    //Serial.print(addrOffset + i);
    //Serial.println(strToWrite[addrOffset + i]);
    EEPROM.write(addrOffset + i, strToWrite[i]);
  }

  // actually write the content of byte-array cache to
  // hardware flash.  flash write occurs if and only if one or more byte
  // in byte-array cache has been changed, but if so, ALL 512 bytes are 
  // written to flash
  EEPROM.commit();

}






/*******************************************************************************
 * readFromEEPROM function:
 * Store phone numbers in EEPROM
 ******************************************************************************/
String readFromEEPROM(int addrOffset)
{
  int len = 13;
  char data[len + 1];
  for (int i = 0; i < len; i++)
  {
    data[i] = EEPROM.read(addrOffset + i);
  }
  data[len] = '\0';
  return String(data);
}




/*******************************************************************************
 * comparePhone function:
 * compare phone numbers stored in EEPROM
 ******************************************************************************/
boolean comparePhone(String number)
{
  boolean flag = 0;
  //--------------------------------------------------
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    if(phoneNo[i].equals(number)){
      //Serial.println(phoneNo[i]);
      flag = 1;
      break;
    }
  }
  //--------------------------------------------------
  return flag;
}




/*******************************************************************************
 * compareWithoutCountryCode function:
 * compare phone numbers stored in EEPROM
 ******************************************************************************/
boolean compareWithoutCountryCode(String number)
{
  boolean flag = 0;
  //--------------------------------------------------
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    //remove first 3 digits (country code)
    phoneNo[i].remove(0,3);
    //Serial.println("meee1: "+phoneNo[i]);
    if(phoneNo[i].equals(number)){
      //Serial.println(phoneNo[i]);
      flag = 1;
      break;
    }
  }
  //--------------------------------------------------
  return flag;
}



/*******************************************************************************
 * debugPrint function:
 * compare phone numbers stored in EEPROM
 ******************************************************************************/
 void debugPrint(String text){
  if(DEBUG_MODE == 1)
    Serial.println(text);
}





/*******************************************************************************
 * setup function
 ******************************************************************************/
void sendLocation(String phoneNumber)
{
  //-----------------------------------------------------------------
  // Can take up to 60 seconds
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
        {newData = true;break;}
    }
  }
  //-----------------------------------------------------------------

  //-----------------------------------------------------------------
  //If newData is true
  if(newData)
  {
    newData = false;
    String latitude = String(gps.location.lat(), 6);
    String longitude = String(gps.location.lng(), 6);
    //String speed = String(gps.speed.kmph());
    
    String text = "Latitude= " + latitude;
    text += "\n\r";
    text += "Longitude= " + longitude;
    text += "\n\r";
    text += "Speed= " + String(gps.speed.kmph()) + " km/h";
    text += "\n\r";
    text += "Altitude= " + String(gps.altitude.meters()) + " meters";
    //text += "\n\r";
    //text += "Connected Satellites= " + String(gps.satellites.value());
    text += "\n\r";
    text += "http://maps.google.com/maps?q=loc:" + latitude + "," + longitude;
         
    debugPrint(text);
    //delay(300);
    ///*
    Reply(text, phoneNumber);
    //*/
  }
  //-----------------------------------------------------------------
}
