#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 1, 101);

#define RST_PIN 9      // Reset pin for RFID reader
#define SS_PIN 10      // Slave Select pin for RFID reader

// Initialize RFID reader
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialize ethernet client
EthernetClient client;

int    HTTP_PORT   = 8080;
String HTTP_METHOD = "POST";
char   HOST_NAME[] = "localhost"; 
String PATH_NAME   = "/EarTag/AddUidToEartag";

// Variable to store the eartag ID
int eartagID;

void setup() {
  Serial.begin(9600);

  // Initialize Ethernet
  Ethernet.begin(mac, ip);
  Serial.println("Ethernet initialized.");

  // Initialize SPI communication
  SPI.begin();

  // Initialize RFID reader
  mfrc522.PCD_Init();
}

void loop() {
  // Read the RFID tag's UID
  byte uid[4];

  // Prompt user for eartag ID
  Serial.println("Enter the eartag ID: ");
  while (!Serial.available()) {
    ; // Wait for user input
  }

  // Read the eartag ID from the serial monitor
  eartagID = Serial.parseInt();

  Serial.println("Eartag ID received: " + String(eartagID));

  delay(5000); // Wait for 5 seconds before checking for another RFID tag

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String userid;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    userid += String(mfrc522.uid.uidByte[i], HEX);
  }

  Serial.println("Found brick...");
  Serial.println("Adding UID: " + String(userid) + " to ear tag: " + String(eartagID));

  delay(1000);

  Serial.println("Sending UID to API..");
  delay(2000);

  // Connect to the API host
  if (client.connect(HOST_NAME, HTTP_PORT)) {
    // If connected:
    Serial.println("Connected to server");

    // Prepare POST request
    String postData = "eartagID=" + String(eartagID) + "&uid=" + String(userid);

    // Send POST request
    client.println(HTTP_METHOD + " " + PATH_NAME + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + postData.length());
    client.println();
    client.println(postData);

    while(client.connected()) {
      if(client.available()){
        // Read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else {// if not connected:
    Serial.println("connection failed");
  }

  delay(5000); // Wait for 5 seconds before resetting
}