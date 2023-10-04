#include <SoftwareSerial.h> //GSM
SoftwareSerial SIM900(8, 7); // rx,tx GSM
#include <Adafruit_Fingerprint.h>


#if (defined(_AVR) || defined(ESP8266)) && !defined(__AVR_ATmega2560_)
// For UNO and others without hardware serial, we must use software serial...
// pin #4 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(4, 3);//Fingerprint sensor RX and TX

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{SIM900.begin(9600); /* Define baud rate for software serial communication */
 
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();
 
//Check fingerprint data
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop()                     // run over and over again
{
  int idt=getFingerprintID();
 
}




// Fingerprint Code starts

uint8_t getFingerprintID() {

  //step 1: get Finger image
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

//Step 2:Image Convert
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!

  //Step 3: Image search
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
 
 
    sendonlinedata(finger.fingerID);
    delay(5000);  
       
  return finger.fingerID;
}


//GSM through send data online
void sendonlinedata(uint8_t id )
{
  Serial.println("HTTP get method :");
  Serial.print("AT\\r\\n");
  SIM900.println("AT"); /* Check Communication */
  delay(5000);
  ShowSerialData(); /* Print response on the serial monitor */
  delay(5000);
  /* Configure bearer profile 1 */
  Serial.print("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\\r\\n");    
  SIM900.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  /* Connection type GPRS */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=3,1,\"APN\",\"internet\"\\r\\n");  
  SIM900.println("AT+SAPBR=3,1,\"APN\",\"internet\"");  /* APN of the provider */

  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=1,1\\r\\n");
  SIM900.println("AT+SAPBR=1,1"); /* Open GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=2,1\\r\\n");
  SIM900.println("AT+SAPBR=2,1"); /* Query the GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+HTTPINIT\\r\\n");
  SIM900.println("AT+HTTPINIT"); /* Initialize HTTP service */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+HTTPPARA=\"CID\",1\\r\\n");
  SIM900.println("AT+HTTPPARA=\"CID\",1");  /* Set parameters for HTTP session */
  delay(5000);
  ShowSerialData();


 
  delay(5000);
  Serial.print("AT+HTTPPARA=\"URL\",\"https://smartatmsystem.000webhostapp.com/iotdata.php?id="+String(id)+"\"\\r\\n");

  SIM900.println("AT+HTTPPARA=\"URL\",\"smartatmsystem.000webhostapp.com/iotdata.php?id="+String(id)+"\"");  /* Set parameters for HTTP session */
 
  ShowSerialData();
 

   
 
  delay(5000);
  Serial.print("AT+HTTPACTION=0\\r\\n");                                  
  SIM900.println("AT+HTTPACTION=0");  /* Start GET session */
  delay(10000);
  ShowSerialData();
  delay(10000);
  Serial.print("AT+HTTPREAD\\r\\n");
  SIM900.println("AT+HTTPREAD");  /* Read data from HTTP server */
  delay(8000);
  ShowSerialData();
  delay(8000);
  Serial.print("AT+HTTPTERM\\r\\n");  
  SIM900.println("AT+HTTPTERM");  /* Terminate HTTP service */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=0,1\\r\\n");
  SIM900.println("AT+SAPBR=0,1"); /* Close GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
}
void ShowSerialData()
{
  while(SIM900.available()!=0)  /* If data is available on serial port */
  Serial.write(char (SIM900.read())); /* Print character received on to the serial monitor */

}



// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

// Fingerprint code end