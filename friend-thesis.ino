/*
Description: Automated Vault Monitoring Management System
- 16x2 LCD I2C
- DHT11 Temp Humidity Sensor
- Keypad
- GSM

System Used: VB NET

*/
// variables
#include <MFRC522.h>
#define SS_PIN 53 // set pin 10
#define RST_PIN 5 // set pin 9 as Reset
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include "DHT.h"
#define DHTPIN 48 // what pin we're connected to
//Uncomment whatever the type of sensor we are using.
#define DHTTYPE DHT11 // DHT 11
// Create instances
SoftwareSerial mySerial(3, 4);    // SoftwareSerial SIM900(Rx, Tx)
MFRC522 mfrc522(SS_PIN, RST_PIN); // MFRC522 mfrc522(SS_PIN, RST_PIN)
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
// Initialize Pins for led's, servo and buzzer
// Blue LED is connected to 5V
constexpr uint8_t greenLed = 12;
constexpr uint8_t redLed = 13;
constexpr uint8_t relay = 28;
constexpr uint8_t buzzerPin = 46;
char initial_password[4] = {'1', '2', '3', '4'}; // Variable to store initial password
String tagUID = "57 CA 31 63";                   // String to store UID of tag. Change it with your tag's UID
char password[4];                                // Variable to store users password
boolean RFIDMode = true;                         // boolean to change modes
boolean NormalMode = true;                       // boolean to change modes
char key_pressed = 0;                            // Variable to store incoming keys
uint8_t i = 0;                                   // Variable used for counter
//dht setup
String TextForSms;
String humidity = " Humidity: %";
String temperature = "   Temperature";
String sign = " *C";
// defining how many rows and columns our keypad have
const byte rows = 4;
const byte columns = 4;
// Keypad pin map
char hexaKeys[rows][columns] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
// Initializing pins for keypad
byte row_pins[rows] = {9, 8, 7, 6};
byte column_pins[columns] = {A4, A5, A6, A7};
// Create instance for keypad
Keypad keypad_key = Keypad(makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);

//end variables

//new variables
String rfid;
String manager = "";
String bfp = "";

int AA = 22;
int BB = 23;
int CC = 24;
int DD = 25;
int EE = 26;

//end new variables

int signals = 0; // read ascii of incoming data

void setup()
{
    Serial.begin(9600);
    pinMode(buzzerPin, OUTPUT);
    pinMode(redLed, OUTPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(relay, OUTPUT);

    pinMode(AA, OUTPUT);
    pinMode(BB, OUTPUT);
    pinMode(CC, OUTPUT);
    pinMode(DD, OUTPUT);
    pinMode(EE, OUTPUT);

    //  for rfid
    SPI.begin(); // Init SPI bus
    mfrc522.PCD_Init();
    //end rfid

    //  for LCD
    lcd.begin(16, 2); // LCD screen
    lcd.backlight();
    //end for LCD

    delay(200);

    //  dht
    dht.begin();

    delay(200);

    //  for GSM
    mySerial.begin(9600);
    delay(2000);

    mySerial.println("AT");
    updateSerial();
}

void loop()
{
    // send data only when you receive data:
    if (Serial.available() > 0)
    {
        signals = Serial.read();
        //    a == 97 ASCII; meaning prepare for register employee
        if (signals == 97)
        {
            registerEmployee();
        }
        //    b == 98 ASCII; meaning prepare for register employee
        if (signals == 98)
        {
            attendances();
        }
        //    g
        if (signals == 103)
        {
            setGSM();
        }

        //    v
        if (signals == 118)
        {
            vaults();
        }

        //    l
        if (signals == 108)
        {
            ledControl();
        }
    }
    else
    {
        //DHT 11
        dhtSensors();

        delay(200);
    }
}

void dhtSensors()
{
    lcd.clear();
    int h = dht.readHumidity();
    // Read temperature as Celsius
    int t = dht.readTemperature();
    // Read temperature as Fahrenheit
    int f = dht.readTemperature(true);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Current value:");
    lcd.setCursor(0, 1);
    lcd.print(t);
    /*
    Serial.print("Humidity: ");
    Serial.println(h);
    Serial.print("Temp °C : ");
    Serial.println(t);
    Serial.print("Temp °F : ");
    Serial.println(f);*/
    if (t >= 32)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Fire detected..");
        lcd.setCursor(0, 1);
        lcd.print("Sending SMS");
        sendSMSFireAndManager();
    }
}

void sendSMSVaultManager()
{
    String smsNumber[1] = {manager};

    for (int i = 0; i < 1; i++)
    {
        mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
        updateSerial();
        mySerial.print("AT+CMGS=");
        mySerial.println(smsNumber[i]);
        updateSerial();
        mySerial.print("Someone opened the vault, text to manager."); //text content
        updateSerial();
        mySerial.write(26);
        delay(4000);
    }
}

void sendSMSVaultManagerIncorrectKeypad()
{
    String smsNumber[1] = {manager};

    for (int i = 0; i < 1; i++)
    {
        mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
        updateSerial();
        mySerial.print("AT+CMGS=");
        mySerial.println(smsNumber[i]);
        updateSerial();
        mySerial.print("Someone opened the vault, text to manager, incorrect keypad password."); //text content
        updateSerial();
        mySerial.write(26);
        delay(4000);
    }
}

void sendSMSFireAndManager()
{
    String smsNumber[2] = {manager, bfp};

    //  buzzer
    digitalWrite(buzzerPin, HIGH);

    for (int i = 0; i < 2; i++)
    {
        mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
        updateSerial();
        mySerial.print("AT+CMGS=");
        mySerial.println(smsNumber[i]);
        updateSerial();
        mySerial.print("Vault is on fire, sms to manager and bfp."); //text content
        updateSerial();
        mySerial.write(26);
        delay(4000);
    }

    // off buzzer after send SMS
    digitalWrite(buzzerPin, LOW);
}

void updateSerial()
{
    delay(500);
    while (Serial.available())
    {
        mySerial.write(Serial.read()); //Forward what Serial received to Software Serial Port
    }
    while (mySerial.available())
    {
        Serial.write(mySerial.read()); //Forward what Software Serial received to Serial Port
    }
}

void ledControl()
{
    bool reply = false;
    bool loops = true;

    while (loops == true)
    {
        int receives = 0;

        if (reply == false)
        {
            Serial.println("r");
            reply = true;
        }

        if (Serial.available() > 0)
        {
            receives = Serial.read();
            //      a;
            if (receives == 97)
            {
                //        on vault room
                digitalWrite(BB, HIGH);
                Serial.println("a");
            }
            //      b
            if (receives == 98)
            {
                //        off vault room
                digitalWrite(BB, LOW);
                Serial.println("b");
            }
            //      c
            if (receives == 99)
            {
                //        on teller room
                digitalWrite(CC, HIGH);
                Serial.println("c");
            }
            //      d
            if (receives == 100)
            {
                //        off teller room
                digitalWrite(CC, LOW);
                Serial.println("d");
            }
            //      e
            if (receives == 101)
            {
                //        on wait area room
                digitalWrite(DD, HIGH);
                Serial.println("e");
            }
            //      f
            if (receives == 102)
            {
                //        off wait area room
                digitalWrite(DD, LOW);
                Serial.println("f");
            }
            //      g
            if (receives == 103)
            {
                //        on manager room
                digitalWrite(EE, HIGH);
                Serial.println("g");
            }
            //      h
            if (receives == 104)
            {
                //        off manager room
                digitalWrite(EE, LOW);
                Serial.println("h");
            }
            //      i
            if (receives == 105)
            {
                //        on employees room
                digitalWrite(AA, HIGH);
                Serial.println("i");
            }
            //      j
            if (receives == 106)
            {
                //        off employees room
                digitalWrite(AA, LOW);
                Serial.println("j");
            }
            //      k
            if (receives == 107)
            {
                //        off employees room
                digitalWrite(AA, HIGH);
                digitalWrite(BB, HIGH);
                digitalWrite(CC, HIGH);
                digitalWrite(DD, HIGH);
                digitalWrite(EE, HIGH);
                Serial.println("k");
            }
            //      l
            if (receives == 108)
            {
                //        off employees room
                digitalWrite(AA, LOW);
                digitalWrite(BB, LOW);
                digitalWrite(CC, LOW);
                digitalWrite(DD, LOW);
                digitalWrite(EE, LOW);
                Serial.println("l");
            }
            // z; to exit
            if (receives == 122)
            {
                loops = false;
            }
        }
    }
}

void vaults()
{
    bool reply = false;
    bool loops = true;

    // loop here
    while (loops == true)
    {
        if (reply == false)
        {
            Serial.println("r");
            reply = true;
        }

        int receives = 0;

        if (Serial.available() > 0)
        {
            receives = Serial.read();

            //      a
            if (receives == 97)
            {
                validateKeypad();
            }
            //      b
            if (receives == 98)
            {
                setLCDToFingerScan();
            }
            //      c; not allowed to scan; no template in db
            if (receives == 99)
            {
                scanNotAllowed();
            }
            // e; to exit
            if (receives == 101)
            {
                loops = false;
            }
        }
        else
        {
            //    when there is no serial data; do scan of ID
            lcd.clear();
            lcd.print("Please scan ID.");
            if (mfrc522.PICC_IsNewCardPresent())
            {
                if (mfrc522.PICC_ReadCardSerial())
                {
                    for (byte i = 0; i < mfrc522.uid.size; i++)
                    {
                        rfid += mfrc522.uid.uidByte[i], HEX;
                    }
                    Serial.println(rfid);
                    digitalWrite(buzzerPin, HIGH);
                    delay(20);
                    digitalWrite(buzzerPin, LOW);
                    mfrc522.PICC_HaltA();
                    rfid = "";
                }
            }
        }
    }
}

void scanNotAllowed()
{
    //  code to display to LCD; scan not allowed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan not ");
    lcd.setCursor(0, 1);
    lcd.print("allowed.");
    digitalWrite(buzzerPin, HIGH);
    delay(3000);
    digitalWrite(buzzerPin, LOW);
}

void scanSuccessOpenVault()
{
    //  code to display text success match of finger;
    lcd.clear();
    lcd.print("Scan confirmed.");
    delay(1500);
    // set HIGH relay
    digitalWrite(relay, HIGH);
    sendSMSVaultManager();
    //  delay(3000);
    digitalWrite(relay, LOW);
}

void setLCDToFingerScan()
{
    //     display LCD to scan;
    //  when receive data from system; change to please wait validating.
    bool loops = true;
    lcd.clear();
    // loop here
    while (loops == true)
    {
        int receives = 0;

        if (Serial.available() > 0)
        {
            receives = Serial.read();
            //      a; fingers matched;dipslay LCD; and open vault; relay to HIGH
            if (receives == 97)
            {
                scanSuccessOpenVault();
                delay(500);
                loops = false;
            }
            //      b; invalid finger scan; please try again
            if (receives == 98)
            {
                loops = false;
                scanFailed();
            }
        }
        else
        {
            //      code here to display scan finger please;
            lcd.setCursor(0, 0);
            lcd.print("Please do ");
            lcd.setCursor(0, 1);
            lcd.print("finger scan.");
            delay(200);
        }
    }
}

void scanFailed()
{
    Serial.println("w");
    //  code to display failed scan; please try again
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan invalid ");
    lcd.setCursor(0, 1);
    lcd.print("Try again.");
    delay(2500);
}

void validateKeypad()
{
    bool loops = true;
    //  code here; display in LCD too;
    //  after validation; send serial data;
    lcd.clear();
    while (loops == true)
    {
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
        key_pressed = keypad_key.getKey(); // Storing keys
        if (key_pressed)
        {
            password[i++] = key_pressed; // Storing in password variable
            lcd.setCursor(i, 1);
            lcd.print("*");
            digitalWrite(buzzerPin, HIGH);
            delay(20);
            digitalWrite(buzzerPin, LOW);
        }
        //  press # to validate password
        if (key_pressed == '#')
        {
            if (!(strncmp(password, initial_password, 4)))
            {
                //        password matched
                Serial.println("k");
                memset(password, 0, sizeof(password)); // clear password
                i = 0;                                 // reset counter

                lcd.clear();
                lcd.print("Correct password.");
                delay(3000);
                loops = false;
            }
            else
            {
                //        password not matched
                Serial.println("v");
                memset(password, 0, sizeof(password)); // clear password
                i = 0;                                 // reset counter
                lcd.clear();
                lcd.print("Wrong Password.");
                digitalWrite(buzzerPin, HIGH);
                sendSMSVaultManagerIncorrectKeypad();
                digitalWrite(buzzerPin, LOW);
                loops = false;
            }
        }

        delay(10);
    }
}

void setGSM()
{
    bool reply = false;
    bool loops = true;

    // loop here
    while (loops == true)
    {
        if (reply == false)
        {
            Serial.println("r");
            reply = true;
        }

        int receives = 0;

        if (Serial.available() > 0)
        {
            receives = Serial.read();

            //      a
            if (receives == 97)
            {
                setManagerNumber();
            }
            //b
            if (receives == 98)
            {
                setBFPNumber();
            }
            // e
            if (receives == 101)
            {
                loops = false;
            }
        }
    }
}

void setManagerNumber()
{
    bool loops = true;
    // loop here

    while (loops == true)
    {
        if (Serial.available() > 0)
        {
            String signals = "";
            signals = Serial.readString();
            signals.trim();
            if (signals != "")
            {
                manager = signals;
                Serial.println("q");
                loops = false;
            }
        }
    }
}

void setBFPNumber()
{
    bool loops = true;
    // loop here

    while (loops == true)
    {
        if (Serial.available() > 0)
        {
            String signals = "";
            signals = Serial.readString();
            signals.trim();
            if (signals != "")
            {
                bfp = signals;
                Serial.println("w");
                loops = false;
            }
        }
    }
}

void attendances()
{
    bool reply = false;
    bool loops = true;

    // loop here
    while (loops == true)
    {
        if (reply == false)
        {
            Serial.println("r");
            reply = true;
        }

        int receive = "";

        if (Serial.available() > 0)
        {
            receive = Serial.read();
            //      exit return to void loop
            if (receive == 101)
            {
                loops = false;
            }
        }
        else
        {
            //        read RFID
            if (mfrc522.PICC_IsNewCardPresent())
            {
                if (mfrc522.PICC_ReadCardSerial())
                {
                    for (byte i = 0; i < mfrc522.uid.size; i++)
                    {
                        rfid += mfrc522.uid.uidByte[i], HEX;
                    }
                    Serial.println(rfid);
                    mfrc522.PICC_HaltA();
                    rfid = "";
                }
            }
        }
    }
}

void registerEmployee()
{
    bool reply = false;
    bool loops = true;

    // loop here
    while (loops == true)
    {
        if (reply == false)
        {
            Serial.println("r");
            reply = true;
        }

        int receive = "";

        if (Serial.available() > 0)
        {
            receive = Serial.read();
            //      exit return to void loop
            if (receive == 101)
            {
                loops = false;
            }
        }
        else
        {
            //        read RFID
            if (mfrc522.PICC_IsNewCardPresent())
            {
                if (mfrc522.PICC_ReadCardSerial())
                {
                    for (byte i = 0; i < mfrc522.uid.size; i++)
                    {
                        rfid += mfrc522.uid.uidByte[i], HEX;
                    }
                    Serial.println(rfid);
                    mfrc522.PICC_HaltA();
                    rfid = "";
                }
            }
        }
    }
}