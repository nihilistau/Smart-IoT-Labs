
#include <FPM.h>
#include <Wire.h>                
#include "SSD1306Wire.h"        

SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL  -  SDA 

/* Enroll fingerprints */

/*  pin #2 is IN from sensor (GREEN wire)
 *  pin #3 is OUT from arduino  (WHITE/YELLOW wire)
 */
HardwareSerial fserial(2);

FPM finger(&fserial);
FPM_System_Params params;

char dbuff[128];

void disp(char *text)
{
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "SmartComputerLab");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 16, text);
  display.display();
}


void setup_enroll()
{
  Serial.println("ENROLL test");
  fserial.begin(57600);
  disp("Press Button\n to Enroll");
  if(finger.begin()) {
    finger.readParams(&params);Serial.println();
    Serial.println("Found fingerprint sensor!");
    Serial.print("Capacity: "); Serial.println(params.capacity);
    Serial.print("Packet length: "); Serial.println(FPM::packet_lengths[params.packet_len]);
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) yield();
    }
    /* just to find out if your sensor supports the handshake command */
    if (finger.handshake()) {
        Serial.println("Handshake command is supported.");
    }
    else {
        Serial.println("Handshake command not supported.");
    }
}

void setup_search()
{
    Serial.begin(9600);
    Serial.println("1:N MATCH test");
    fserial.begin(57600);

    if (finger.begin()) {
        finger.readParams(&params);
        Serial.println("Found fingerprint sensor!");
        Serial.print("Capacity: "); Serial.println(params.capacity);
        Serial.print("Packet length: "); Serial.println(FPM::packet_lengths[params.packet_len]);
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) yield();
    }
}



void setup()
{
    Serial.begin(9600);
    pinMode(32,INPUT_PULLUP);
    if(digitalRead(32)==LOW)setup_enroll();
    else setup_search();
//    Serial.println("ENROLL test");
//    fserial.begin(57600);
//    disp("Press Button\n to Enroll");
//
//    if (finger.begin()) {
//        finger.readParams(&params);Serial.println();
//        Serial.println("Found fingerprint sensor!");
//        Serial.print("Capacity: "); Serial.println(params.capacity);
//        Serial.print("Packet length: "); Serial.println(FPM::packet_lengths[params.packet_len]);
//    } else {
//        Serial.println("Did not find fingerprint sensor :(");
//        while (1) yield();
//    }
//
//    /* just to find out if your sensor supports the handshake command */
//    if (finger.handshake()) {
//        Serial.println("Handshake command is supported.");
//    }
//    else {
//        Serial.println("Handshake command not supported.");
//    }
}

void loop()
{
    if(digitalRead(32)==LOW)loop_enroll();
    else loop_search();
}

void loop_enroll()
{    
    Serial.println("Send any character to enroll a finger...");
    disp("Press Button\n to Enroll");
    //while (Serial.available() == 0) yield();  // resets WDT timer
    while (touchRead(2)>30) yield();  // resets WDT timer
    Serial.println("Searching for a free slot to store the template...");
    int16_t fid;
    if (get_free_id(&fid))
        enroll_finger(fid);
    else
        Serial.println("No free slot in flash library!");
    while (Serial.read() != -1);  // clear buffer
}

bool get_free_id(int16_t * fid) {
    int16_t p = -1;
    for (int page = 0; page < (params.capacity / FPM_TEMPLATES_PER_PAGE) + 1; page++) {
        p = finger.getFreeIndex(page, fid);
        switch (p) {
            case FPM_OK:
                if (*fid != FPM_NOFREEINDEX) {
                    Serial.print("Free slot at ID ");
                    Serial.println(*fid);
                    return true;
                }
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error!");
                return false;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                return false;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                return false;
            default:
                Serial.println("Unknown error!");
                return false;
        }
        yield();
    }
    
    Serial.println("No free slots!");
    return false;
}

int16_t enroll_finger(int16_t fid) {
    int16_t p = -1;
    Serial.println("Waiting for valid finger to enroll");
    disp("Waiting for finger");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");disp("Image taken");
                break;
            case FPM_NOFINGER:
                Serial.println(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }
    // OK success!

    p = finger.image2Tz(1);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");disp("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    Serial.println("Remove finger");disp("Remove finger");
    delay(2000);
    p = 0;
    while (p != FPM_NOFINGER) {
        p = finger.getImage();
        yield();
    }

    p = -1;
    Serial.println("Place same finger again");disp("Place the same\nfinger again");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");disp("Image taken");
                break;
            case FPM_NOFINGER:
                Serial.print(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");disp("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return false;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return false;
        default:
            Serial.println("Unknown error");
            return p;
    }


    // OK converted!
    p = finger.createModel();
    if (p == FPM_OK) {
        Serial.println("Prints matched!");disp("Prints matched");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID "); Serial.println(fid);
    p = finger.storeModel(fid);
    if (p == FPM_OK) {
        Serial.println("Stored!");sprintf(dbuff,"ID %d \nStored\n",fid);disp(dbuff);delay(3000);
        return 0;
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    } else if (p == FPM_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }
}

void loop_search()
{
    Serial.println("Send any character to search for a print...");
    while (touchRead(2)>30) yield();
    //while (Serial.available() == 0) yield();
    search_database();
    while (Serial.read() != -1);
}

int search_database(void) {
    int16_t p = -1;

    /* first get the finger image */
    Serial.println("Waiting for valid finger");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                break;
            case FPM_NOFINGER:
                Serial.println(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }

    /* convert it */
    p = finger.image2Tz();
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    /* search the database for the converted print */
    uint16_t fid, score;
    p = finger.searchDatabase(&fid, &score);
    
    /* now wait to remove the finger, though not necessary; 
       this was moved here after the search because of the R503 sensor, 
       which seems to wipe its buffers after each scan */
    Serial.println("Remove finger");
    while (finger.getImage() != FPM_NOFINGER) {
        delay(500);
    }
    Serial.println();
    
    if (p == FPM_OK) {
        Serial.println("Found a print match!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_NOTFOUND) {
        Serial.println("Did not find a match");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    Serial.print("Found ID #"); Serial.print(fid);
    Serial.print(" with confidence of "); Serial.println(score);
}
