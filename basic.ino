// Inspired by https://lastminuteengineers.com/sim800l-gsm-module-arduino-tutorial/
#include <SoftwareSerial.h>

//Create software serial object to communicate with SIM800L
SoftwareSerial sim800(3, 2);                        //SIM800L Tx => 3 & Rx => 2

String dest_number_admin = "\"+ZZ123456789\"";      // Put your phone number here (with country code)
const int led = LED_BUILTIN;
String sms_from = String("");
String sms_content = String("");

void setup()
{
  Serial.begin(9600);                               //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  sim800.begin(9600);                               //Begin serial communication with Arduino and SIM800L

  Serial.println("Initializing...");
  delay(10000);                                     // Give some time to sim800 to connect network

  sim800.println("AT");                             // Once the handshake test is successful, it will back to OK
  updateSerial();
  sim800.println("AT+CSQ");                         // Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  sim800.println("AT+CCID");                        // Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  sim800.println("AT+CREG?");                       // Check whether it has registered in the network
  updateSerial();
  sim800.println("AT+CMGF=1");                      // Configuring TEXT mode
  updateSerial();
  sim800.println("AT+CNMI=1,2,0,0,0");              // Decides how newly arrived SMS messages should be handled
  updateSerial();
  sim800.println("AT+CMGDA=\"DEL ALL\"");           // Delete all messages
  updateSerial();

  send_sms("Setup Complete", dest_number_admin);
  Serial.println("===============================================");

}

void loop()
{
  read_sms();
  if (sms_content.length() >= 1)
  {
    if (sms_content == "on")
    {
      digitalWrite(led, HIGH);
      Serial.println("On received!");
      send_sms("Led has been turned on",sms_from);
      sms_content = String("");
    }
    else if (sms_content == "off")
    {
      digitalWrite(led, LOW);
      Serial.println("Off received!");
      send_sms("Led has been turned off",sms_from);
      sms_content = String("");
    }
    else
    {
      send_sms("Unknow Command", sms_from);
      Serial.println("Unknown command received!");
      sms_content = String("");
    }
  }
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    sim800.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(sim800.available()) 
  {
    Serial.write(sim800.read());//Forward what Software Serial received to Serial Port
  }
}

void send_sms(String text_to_send, String dest_number)
{
  sim800.print("AT+CMGS=");
  sim800.println(dest_number);
  updateSerial();
  sim800.print(text_to_send);
  updateSerial();
  sim800.write(26);
  Serial.println("");
}

void read_sms()
{
  if (sim800.available()>0)
  {
    String sms = sim800.readString();
    if (sms.startsWith("\r\n+CMT:"))                                        // Start to parse only if it's a received sms
    {
      sms.toLowerCase();
      int delimIndex = sms.indexOf('"');
      int secondDelimIndex = sms.indexOf('"', delimIndex+1);
      int lastDelimIndex = sms.lastIndexOf('"');
      int lastNLIndex = sms.lastIndexOf('\n');
      sms_from = sms.substring(delimIndex, secondDelimIndex+1);             // Get sms from between 2 first quotes 
                                                                            // Keep quotes ! (To avoid quotes delimIndex+1, secondDelimIndex)
      sms_content = sms.substring(lastDelimIndex+3, lastNLIndex-1);         // Get sms content between last quote and last \n
      Serial.println("===== SMS Received =====");
      Serial.println("sms_from:" + sms_from);
      Serial.println("sms_content:" + sms_content);
      Serial.println("========================");
    }
  }
}

