#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define pinDHT D0
#define pinMQAnalog A0
#define pinMQDigital D14
#define pinAcidAnalog A1
#define pinHeater D3

#define pinMotor1 D1
#define pinMotor2 D2

#define pwmChannel1 0
#define pwmChannel2 1
#define pwmFrequency 5000
#define pwmResolution 8

#define THRESHOLD_CRITICAL_TEMP 45.0
#define THRESHOLD_ACID_LEAK 500
#define THRESHOLD_POLLUTED_AIR 600

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
    unsigned long lastDisplayTime = 0;
    const unsigned long displayInterval = 1500;

    void showBootAnimation()
    {
        Serial.println("=========================================");
        Serial.println("      Initializing Core Systems...        ");
        Serial.println("=========================================");
        delay(300);
        Serial.println("   [  ] Loading Kernel...                 ");
        delay(300);
        Serial.println("   [====] Calibrating Environment...     ");
        delay(400);
        
        Serial.println("");
        Serial.println("        ____                                _ ");
        Serial.println("       / ___|__ _ _ __  ___ _   _ | | ___  _|_| ");
        Serial.println("      | |   / _` | '_ \\/ __| | | || |/ _ \\_ _ ");
        Serial.println("      | |__| (_| | |_) \\__ \\ |_| || |  __/_|_| ");
        Serial.println("       \\____\\__,_| .__/|___/\\__,_||_|\\___|    ");
        Serial.println("                 |_|                          ");
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
    PlanetCapsule() : dht11(pinDHT), lcd(0x27, 20, 4)
    {
    }

    void begin()
    {
        Serial.begin(9600);
        
        Wire.begin(D8, D9);
        lcd.init();
        lcd.backlight();

        pinMode(pinMQDigital, INPUT);
        pinMode(pinHeater, OUTPUT);
        digitalWrite(pinHeater, LOW);
        
        ledcSetup(pwmChannel1, pwmFrequency, pwmResolution);
        ledcSetup(pwmChannel2, pwmFrequency, pwmResolution);
        ledcAttachPin(pinMotor1, pwmChannel1);
        ledcAttachPin(pinMotor2, pwmChannel2);

        showBootAnimation();
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

        data.gasLevel = analogRead(pinMQAnalog);
        data.gasDanger = digitalRead(pinMQDigital) == HIGH || (data.gasLevel > THRESHOLD_POLLUTED_AIR);
        data.acidLevel = analogRead(pinAcidAnalog);
        data.acidLeakDetected = data.acidLevel > THRESHOLD_ACID_LEAK;

        if (data.acidLeakDetected)
        {
            data.systemStatus = "EMERGENCY: ACID";
        }
        else if (data.gasDanger)
        {
            data.systemStatus = "AIR FILTER MODE";
        }
        else if (data.temperature > THRESHOLD_CRITICAL_TEMP)
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
        if (data.temperature < THRESHOLD_CRITICAL_TEMP && data.acidLevel > 100)
        {
            digitalWrite(pinHeater, HIGH);
            data.heaterStatus = true;
        }
        else
        {
            digitalWrite(pinHeater, LOW);
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

        if (data.acidLeakDetected || data.temperature > THRESHOLD_CRITICAL_TEMP)
        {
            fanSpeed = 255;
        }

        ledcWrite(pwmChannel1, fanSpeed);
        ledcWrite(pwmChannel2, fanSpeed);
    }

    void updateDisplay()
    {
        if (millis() - lastDisplayTime >= displayInterval)
        {
            lastDisplayTime = millis();

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
}
