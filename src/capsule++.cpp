#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FS.h>
#include <SD.h>

namespace CapsuleSettings
{
    const int pinDHT = D0;
    const int pinMQAnalog = A0;
    const int pinMQDigital = D14;
    const int pinAcidAnalog = A1;
    const int pinHeater = D3;

    const int pinMotor1 = D1;
    const int pinMotor2 = D2;

    const int pwmChannel1 = 0;
    const int pwmChannel2 = 1;
    const int pwmFrequency = 5000;
    const int pwmResolution = 8;

    const float THRESHOLD_CRITICAL_TEMP = 45.0;
    const int THRESHOLD_ACID_LEAK = 500;
    const int THRESHOLD_POLLUTED_AIR = 600;

    const int SD_CS_PIN = D10;
}

struct CapsuleData
{
    float temperature = 0.0;
    float humidity = 0.0;
    int gasLevel = 0;
    bool gasDanger = false;
    int acidLevel = 0;
    bool acidLeakDetected = false;
    bool heaterStatus = false;
    String systemStatus = "NORMAL";
};

class PlanetCapsule
{
private:
    SimpleDHT11 dht11;
    LiquidCrystal_I2C lcd;
    CapsuleData data;
    unsigned long lastLogTime = 0;
    const unsigned long logInterval = 5000;

    void showBootAnimation()
    {
        Serial.println("=========================================");
        Serial.println("      Initializing Core Systems...       ");
        Serial.println("=========================================");
        delay(300);
        Serial.println("   [  ] Loading Kernel...                ");
        delay(300);
        Serial.println("   [==] Mounting SD Card (FAT32)...     ");
        delay(300);
        Serial.println("   [====] Calibrating Environment...     ");
        delay(400);
        
        Serial.println("");
        Serial.println("       ____                               _ ");
        Serial.println("      / ___|__ _ _ __  ___ _   _ | | ___  _|_| ");
        Serial.println("     | |   / _` | '_ \\/ __| | | || |/ _ \\_ _ ");
        Serial.println("     | |__| (_| | |_) \\__ \\ |_| || |  __/_|_| ");
        Serial.println("      \\____\\__,_| .__/|___/\\__,_||_|\\___|    ");
        Serial.println("                |_|                          ");
        Serial.println("=========================================");
        Serial.println(" SYSTEM ONLINE - READY FOR DEPLOYMENT   ");
        Serial.println("=========================================");
        Serial.println("");

        lcd.setCursor(4, 0);
        lcd.print("SYSTEM BOOT");
        lcd.setCursor(4, 1);
        lcd.print("Capsule++");
        lcd.setCursor(0, 3);
        lcd.print("Loading modules...  ");
        delay(1500);
        
        lcd.setCursor(0, 3);
        lcd.print("Checking hardware...");
        delay(1000);
        lcd.clear();
    }

public:
    PlanetCapsule() : dht11(CapsuleSettings::pinDHT), lcd(0x27, 20, 4)
    {
    }

    void begin()
    {
        Serial.begin(9600);
        
        Wire.begin(D8, D9);
        lcd.init();
        lcd.backlight();

        pinMode(CapsuleSettings::pinMQDigital, INPUT);
        pinMode(CapsuleSettings::pinHeater, OUTPUT);
        digitalWrite(CapsuleSettings::pinHeater, LOW);
        
        ledcSetup(CapsuleSettings::pwmChannel1, CapsuleSettings::pwmFrequency, CapsuleSettings::pwmResolution);
        ledcSetup(CapsuleSettings::pwmChannel2, CapsuleSettings::pwmFrequency, CapsuleSettings::pwmResolution);
        ledcAttachPin(CapsuleSettings::pinMotor1, CapsuleSettings::pwmChannel1);
        ledcAttachPin(CapsuleSettings::pinMotor2, CapsuleSettings::pwmChannel2);

        showBootAnimation();

        lcd.setCursor(0, 0);
        lcd.print("Capsule++ Storage");
        lcd.setCursor(0, 1);
        lcd.print("Checking SD Card...");
        
        if (!SD.begin())
        {
            Serial.println("SD ERROR!");
            lcd.setCursor(0, 2);
            lcd.print("SD ERROR! USE FAT32");
            delay(3000);
        }
        else
        {
            Serial.println("SD OK.");
            lcd.setCursor(0, 2);
            lcd.print("SD Card: OK        ");
            delay(1000);
        }
        
        lcd.clear();
    }

    void readSensors()
    {
        byte tempByte = 0;
        byte humidByte = 0;
        
        if (dht11.read(&tempByte, &humidByte, NULL) == SimpleDHTErrSuccess)
        {
            data.temperature = (float)tempByte;
            data.humidity = (float)humidByte;
        }

        data.gasLevel = analogRead(CapsuleSettings::pinMQAnalog);
        data.gasDanger = digitalRead(CapsuleSettings::pinMQDigital) == HIGH || (data.gasLevel > CapsuleSettings::THRESHOLD_POLLUTED_AIR);
        data.acidLevel = analogRead(CapsuleSettings::pinAcidAnalog);
        data.acidLeakDetected = data.acidLevel > CapsuleSettings::THRESHOLD_ACID_LEAK;

        if (data.acidLeakDetected)
        {
            data.systemStatus = "EMERGENCY: ACID";
        }
        else if (data.gasDanger)
        {
            data.systemStatus = "AIR FILTER MODE";
        }
        else if (data.temperature > CapsuleSettings::THRESHOLD_CRITICAL_TEMP)
        {
            data.systemStatus = "OVERHEATING!";
        }
        else
        {
            data.systemStatus = "SAFE / STABLE";
        }
    }

    void distillationManager()
    {
        if (data.temperature < CapsuleSettings::THRESHOLD_CRITICAL_TEMP && data.acidLevel > 100)
        {
            digitalWrite(CapsuleSettings::pinHeater, HIGH);
            data.heaterStatus = true;
        }
        else
        {
            digitalWrite(CapsuleSettings::pinHeater, LOW);
            data.heaterStatus = false;
        }
    }

    void coolingManager()
    {
        int fanSpeed = 0;
        
        if (data.temperature > 25.0)
        {
            fanSpeed = map(constrain(data.temperature, 25.0, 50.0), 25, 50, 50, 255);
        }

        if (data.acidLeakDetected || data.temperature > CapsuleSettings::THRESHOLD_CRITICAL_TEMP)
        {
            fanSpeed = 255;
        }

        ledcWrite(CapsuleSettings::pwmChannel1, fanSpeed);
        ledcWrite(CapsuleSettings::pwmChannel2, fanSpeed);
    }

    void updateDisplay()
    {
        lcd.setCursor(0, 0);
        lcd.print("CP++: " + data.systemStatus.substring(0, 14)); 
        
        lcd.setCursor(0, 1);
        lcd.print("T:" + String(data.temperature, 1) + "C  H:%" + String(data.humidity, 0) + "   ");

        lcd.setCursor(0, 2);
        lcd.print("Gas:" + String(data.gasLevel) + (data.heaterStatus ? " HT:ON " : " HT:OFF"));

        lcd.setCursor(0, 3);
        
        if (data.acidLeakDetected)
        {
            lcd.print("ACID LEAK DETECTED!! ");
        }
        else
        {
            lcd.print("Acid/Liq: " + String(data.acidLevel) + " [OK]  ");
        }
    }

    void logToSD()
    {
        if (millis() - lastLogTime >= logInterval)
        {
            lastLogTime = millis();

            File logFile = SD.open("/capsule_log.txt", FILE_APPEND);
            
            if (logFile)
            {
                logFile.print(String(millis()) + ",");
                logFile.print(String(data.temperature, 2) + ",");
                logFile.print(String(data.humidity, 2) + ",");
                logFile.print(String(data.gasLevel) + ",");
                logFile.print(String(data.acidLevel) + ",");
                logFile.print(String(data.heaterStatus) + ",");
                logFile.println(data.systemStatus);
                logFile.close();
                Serial.println("Log OK.");
            }
            else
            {
                Serial.println("Log ERROR!");
            }
        }
    }
};

PlanetCapsule capsule;

void setup()
{
    capsule.begin();
}

void loop()
{
    capsule.readSensors();
    capsule.distillationManager();
    capsule.coolingManager();
    capsule.updateDisplay();
    capsule.logToSD();
    delay(200);
}