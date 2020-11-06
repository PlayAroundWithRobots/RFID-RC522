//code from https://github.com/rmorenojr/ElegooTutorial/tree/master/Lesson%2021%20-%20RC522%20RFID%20Module
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
Servo doorlock;
int red = 2;
int green = 3;
   // Other hardware pins
    const byte rstPin =  9;                // Reset pin
    const byte  ssPin = 10;                // Slave Select pin
    
    // Instantiate MFRC522 object class
    MFRC522 rfidReader(ssPin, rstPin);
 
    // Other Global Constants 
    const long timeout = 30000;      
    char* myTags[100] = {};
    int tagsCount = 0;
    String tagID = "";

bool readRFID(long _timeout=timeout, bool useTimeout=false){
   
    bool successRead = false;

    unsigned long startTime = millis();
    unsigned long currentTime = startTime;
    // S'U'+S'T'
    // T  = (currentTime-startTime) > timeout
    // T' = (currentTime-startTime) < timeout
    while (((successRead==false)&&(useTimeout==false)) || ((successRead==false)&&((currentTime - startTime) < _timeout))) {    
        if (isTagPresent() == true){ successRead = getTagID(); }
        currentTime = millis();
    }
    return successRead;
}


void setup() {
    // Initiating
    Serial.begin(9600);     
    pinMode(red,OUTPUT);
    pinMode(green,OUTPUT);
    doorlock.attach(4);
    // Start the serial monitor
    SPI.begin();                            // Start SPI bus
    rfidReader.PCD_Init();                  // Start MFRC522 object

    while (!Serial);                        // Do nothing if no serial port is opened
    
    // Obviously this is an over simplified sketch
    // Master tags would be save in flash storage and
    // retrieved here.  OR a special PIN entered to set
    // Master Tag.
    // But for the sake of simplicity, the sketch will 
    // obtain a new master tag when restarted.
    
    // Prints the initial message
    Serial.println(F("-No Master Tag!-"));
    Serial.println(F("    SCAN NOW"));

    // readRFID will wait until a master card is scanned
    if (readRFID() == true) {
        myTags[tagsCount] = strdup(tagID.c_str()); // Sets the master tag into position 0 in the array
        Serial.println(F("Master Tag is Set!"));
        tagsCount++;
    }
    printNormalModeMessage();
}


void loop() {
    if (isTagPresent()==true){
        getTagID();
        checkTagID();
    } else {
        delay(50);
        //return;
    }
    
}    


bool isTagPresent(){
   
    bool returnValue = true;
    
    // NOT a new PICC_IsNewCardPresent in RFID reader
    //OR
    // NOT a PICC_ReadCardSerial active in Serial
    if ( !rfidReader.PICC_IsNewCardPresent() || !rfidReader.PICC_ReadCardSerial() ) { 
      returnValue = false;
    }
    return returnValue;
}

byte checkMyTags(String tagID){
    
    byte tagIndex = 0;
    //Serial.println("checkMyTags Started");
    // Zero is reserved for master tag
    for (int i = 1; i < 100; i++) {
        if (tagID == myTags[i]) { tagIndex = i; }
    }
    //Serial.println("checkMyTags ended");
    return tagIndex;
}

void checkTagID(){
   
    // Checks for Master tag
    if (tagID == myTags[0]) {
        // Switch to program mode
        Serial.println(F(" Program mode:"));
        Serial.println(F(" Add/Remove Tag"));
        
        // Now with timeout
        // readRFID will skip if timeout exceeded
        if (readRFID(timeout,true)==true) {
            //Check for authorized tag
            byte tagIndex = checkMyTags(tagID);
            if (tagIndex!=0){
                //Remove existing tag
                myTags[tagIndex] = '\0';
                Serial.println(F("  Tag Removed!"));
            } else {
                //Not existing, add tag
                myTags[tagsCount] = strdup(tagID.c_str());
                Serial.println(F("  Tag Added!"));
                tagsCount++;
            }
        } else {
            Serial.println(F("    timeout"));
        }
    } else { 
        //Check for authorized tag
        byte tagIndex = checkMyTags(tagID);
        if (tagIndex!=0){
            //Authorized tag
            Serial.println(F(" Access Granted!"));
            digitalWrite(green,HIGH);
            digitalWrite(red,LOW);
            doorlock.write(90);
        } else {
            //Not authorized tag
            Serial.println(F(" Access Denied!"));
            digitalWrite(red,HIGH);
            digitalWrite(green,LOW);
        }
    }
    printNormalModeMessage();
}

bool getTagID() {
    /*  getTagID retrieves the tag ID.  Modifies global variable tagID
     *           
     *    Parameters: (none)    
     *    Returns: true
     */

    tagID = "";
    
    Serial.print(F(" UID tag: "));
    for (byte i = 0; i < rfidReader.uid.size; i++){
        // The MIFARE PICCs that we use have 4 byte UID
        Serial.print(rfidReader.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfidReader.uid.uidByte[i], HEX);
        // Adds the bytes in a single String variable
        tagID.concat(String(rfidReader.uid.uidByte[i] < 0x10 ? " 0" : " "));
        tagID.concat(String(rfidReader.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.println();
    tagID.toUpperCase();
    rfidReader.PICC_HaltA();                     // Stop reading
    return true;
}

void printNormalModeMessage() {
    /*  printNormalModeMessage sends the standard greeting
     *    to the serial monitor.
     *  Parameters: (none)
     *  Returns: (none)
     */
    delay(1500);
    Serial.println();
    digitalWrite(red,LOW);
    digitalWrite(green,LOW);
    doorlock.write(-90);
    Serial.println(F("-Access Control-"));
    Serial.println(F(" Scan Your Tag!"));
}
