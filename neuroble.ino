#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <SPI.h>
#include <MFRC522.h>
#include <NimBLEDevice.h>
BLECharacteristic *pCharacteristic;
int num=1;
byte countread=1;
byte uidsold;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


#define RST_PIN         16           // 21Configurable, see typical pin layout above
#define SS_PIN          5          // Configurable, see typical pin layout above



String webPagec ="";


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;


const char* host = "esp32";
const char* ssid = "ST12.";
const char* password = "28282828";

WebServer server(80);

/*
 * Login page
 */

const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";








const char* sdata =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>Data page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * setup function
 */
void setup(void) {




BLEDevice::init("Neurology 2.1");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID,
                                  /***** Enum Type NIMBLE_PROPERTY now *****      
                                        BLECharacteristic::PROPERTY_READ   |
                                        BLECharacteristic::PROPERTY_WRITE  
                                        );
                                  *****************************************/
                                        NIMBLE_PROPERTY::READ |
                                        NIMBLE_PROPERTY::WRITE 
                                       );

  pCharacteristic->setValue("No data");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);

  BLEDevice::startAdvertising();




  








  
  Serial.begin(9600);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
   int connectnum=connectnum+1;
   if (connectnum > 20)  {
    break;
    WiFi.disconnect();
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/sdata", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", webPagec);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
    pinMode(LED_BUILTIN, OUTPUT);


Serial.begin(9600);  // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();    // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("Scan a MIFARE Classic PICC to demonstrate Value Block mode."));
    Serial.print(F("Using key (for A and B):"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
    
    Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));

    Serial.print(F("Ver: 0x"));
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  byte vers = readReg;
  Serial.println(readReg, HEX);
 Serial.print(F("Test: 0x"));

  byte regVal = 0x77; //01110111
  mfrc522.PCD_WriteRegister(mfrc522.GsNReg, regVal);
  
  byte   readReg2 = mfrc522.PCD_ReadRegister(mfrc522.GsNReg);
 Serial.println(readReg2, HEX);

regVal = 0xA0; //rx irq
mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);

 // pCharacteristic->setValue(String(readReg));
 // pCharacteristic->notify();

webPagec = "";
webPagec += "<meta http-equiv=\"refresh\" content=\"3\">";
webPagec += "<p>";
webPagec += String(readReg);
webPagec += "</p>";
webPagec += "<p>";
webPagec += String(readReg2);
webPagec += "</p>";

 
}

void loop(void) {
  
  server.handleClient();
  delay(1);
   // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
 // delay(1000);                       // wait for a second
 // digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//delay(1000); 









  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;
byte uids;
byte uids1;
byte uids2;
byte uids3;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    uids=mfrc522.uid.uidByte[1];



if (uids != uidsold)
    {
      countread = 1; 
      }

if (uids == uidsold)
    {
      countread = countread+1;
      }
     
  

 uidsold=uids;

























byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
byte str[] = {uids,countread,0x00};
byte rbyte = random(256);
  pCharacteristic->setValue({uids,countread,rbyte});
  pCharacteristic->notify();


 Serial.print("Connected to ");
  Serial.println((char*)str);

  
webPagec = "";
webPagec += "<meta http-equiv=\"refresh\" content=\"3\">";
webPagec += "<p>";
webPagec += String(uids);
webPagec += "</p>";
webPagec += "<p>";
webPagec += String(readReg);
webPagec += "</p>";










    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }

    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7
    byte sector         = 1;
    byte valueBlockA    = 5;
    byte valueBlockB    = 6;
    byte trailerBlock   = 7;
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);
    int32_t value;

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Show the whole sector as it currently is
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();
    
    // We need a sector trailer that defines blocks 5 and 6 as Value Blocks and enables key B
    // The last block in a sector (block #3 for Mifare Classic 1K) is the Sector Trailer.
    // See http://www.nxp.com/documents/data_sheet/MF1S503x.pdf sections 8.6 and 8.7:
    //      Bytes 0-5:   Key A
    //      Bytes 6-8:   Access Bits
    //      Bytes 9:     User data
    //      Bytes 10-15: Key B (or user data)
    byte trailerBuffer[] = {
        255, 255, 255, 255, 255, 255,       // Keep default key A
        0, 0, 0,
        0,
        255, 255, 255, 255, 255, 255};      // Keep default key B
    // The access bits are stored in a peculiar fashion.
    // There are four groups:
    //      g[0]    Access bits for block 0 (for sectors 0-31)
    //              or blocks 0-4 (for sectors 32-39)
    //      g[1]    Access bits for block 1 (for sectors 0-31)
    //              or blocks 5-9 (for sectors 32-39)
    //      g[2]    Access bits for block 2 (for sectors 0-31)
    //              or blocks 10-14 (for sectors 32-39)
    //      g[3]    Access bits for the Sector Trailer: block 3 (for sectors 0-31)
    //              or block 15 (for sectors 32-39)
    // Each group has access bits [C1 C2 C3], in this code C1 is MSB and C3 is LSB.
    // Determine the bit pattern needed using MIFARE_SetAccessBits:
    //      g0=0    access bits for block 0 (of this sector) using [0 0 0] = 000b = 0
    //              which means key A|B have r/w for block 0 of this sector
    //              which (in this example) translates to block #4 within sector #1;
    //              this is the transport configuration (at factory delivery).
    //      g1=6    access bits for block 1 (of this sector) using [1 1 0] = 110b = 6
    //              which means block 1 (of this sector) is used as a value block,
    //              which (in this example) translates to block #5 within sector #1;
    //              where key A|B have r, key B has w, key B can increment,
    //              and key A|B can decrement, transfer, and restore.
    //      g2=6    same thing for block 2 (of this sector): set it to a value block;
    //              which (in this example) translates to block #6 within sector #1;
    //      g3=3    access bits for block 3 (of this sector): the Sector Trailer here;
    //              using [0 1 1] = 011b = 3 which means only key B has r/w access
    //              to the Sector Trailer (block 3 of this sector) from now on
    //              which (in this example) translates to block #7 within sector #1;
    mfrc522.MIFARE_SetAccessBits(&trailerBuffer[6], 0, 6, 6, 3);

    // Read the sector trailer as it is currently stored on the PICC
    Serial.println(F("Reading sector trailer..."));
    status = mfrc522.MIFARE_Read(trailerBlock, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    // Check if it matches the desired access pattern already;
    // because if it does, we don't need to write it again...
    if (    buffer[6] != trailerBuffer[6]
        ||  buffer[7] != trailerBuffer[7]
        ||  buffer[8] != trailerBuffer[8]) {
        // They don't match (yet), so write it to the PICC
        Serial.println(F("Writing new sector trailer..."));
        status = mfrc522.MIFARE_Write(trailerBlock, trailerBuffer, 16);
        if (status != MFRC522::STATUS_OK) {
            Serial.print(F("MIFARE_Write() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }
    }

    // Authenticate using key B
    Serial.println(F("Authenticating again using key B..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // A value block has a 32 bit signed value stored three times
    // and an 8 bit address stored 4 times. Make sure that valueBlockA
    // and valueBlockB have that format (note that it will only format
    // the block when it doesn't comply to the expected format already).
    formatValueBlock(valueBlockA);
    formatValueBlock(valueBlockB);

    // Add 1 to the value of valueBlockA and store the result in valueBlockA.
    Serial.print("Adding 1 to value of block "); Serial.println(valueBlockA);
    status = mfrc522.MIFARE_Increment(valueBlockA, 1);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Increment() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    status = mfrc522.MIFARE_Transfer(valueBlockA);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Transfer() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    // Show the new value of valueBlockA
    status = mfrc522.MIFARE_GetValue(valueBlockA, &value);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("mifare_GetValue() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    Serial.print("New value of value block "); Serial.print(valueBlockA);
    Serial.print(" = "); Serial.println(value);

    // Decrement 10 from the value of valueBlockB and store the result in valueBlockB.
    Serial.print("Subtracting 10 from value of block "); Serial.println(valueBlockB);
    status = mfrc522.MIFARE_Decrement(valueBlockB, 10);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Decrement() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    status = mfrc522.MIFARE_Transfer(valueBlockB);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Transfer() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    // Show the new value of valueBlockB
    status = mfrc522.MIFARE_GetValue(valueBlockB, &value);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("mifare_GetValue() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    Serial.print(F("New value of value block ")); Serial.print(valueBlockB);
    Serial.print(F(" = ")); Serial.println(value);
    // Check some boundary...
    if (value <= -100) {
        Serial.println(F("Below -100, so resetting it to 255 = 0xFF just for fun..."));
        status = mfrc522.MIFARE_SetValue(valueBlockB, 255);
        if (status != MFRC522::STATUS_OK) {
            Serial.print(F("mifare_SetValue() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }
    }

    // Dump the sector data
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();

    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

/**
 * Ensure that a given block is formatted as a Value Block.
 */
void formatValueBlock(byte blockAddr) {
    byte buffer[18];
    byte size = sizeof(buffer);
    MFRC522::StatusCode status;

    Serial.print(F("Reading block ")); Serial.println(blockAddr);
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    if (    (buffer[0] == (byte)~buffer[4])
        &&  (buffer[1] == (byte)~buffer[5])
        &&  (buffer[2] == (byte)~buffer[6])
        &&  (buffer[3] == (byte)~buffer[7])

        &&  (buffer[0] == buffer[8])
        &&  (buffer[1] == buffer[9])
        &&  (buffer[2] == buffer[10])
        &&  (buffer[3] == buffer[11])

        &&  (buffer[12] == (byte)~buffer[13])
        &&  (buffer[12] ==        buffer[14])
        &&  (buffer[12] == (byte)~buffer[15])) {
        Serial.println(F("Block has correct Value Block format."));
    }
    else {
        Serial.println(F("Formatting as Value Block..."));
        byte valueBlock[] = {
            0, 0, 0, 0,
            255, 255, 255, 255,
            0, 0, 0, 0,
            blockAddr, ~blockAddr, blockAddr, ~blockAddr };
        status = mfrc522.MIFARE_Write(blockAddr, valueBlock, 16);
        if (status != MFRC522::STATUS_OK) {
            Serial.print(F("MIFARE_Write() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
        }
    }
}
