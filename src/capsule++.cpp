// Kütüphane eklenmesi
#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pinlerin tanımlanması
#define pinDHT D0
#define pinMQAnalog A0
#define pinMQDigital D14
#define pinAcidAnalog A1

// Gerekli sınıf ve fonksiyonları içeren yapı
namespace CapsuleSystem
{
    // Sınır değişkenlerinin tanımlanması
    const float THRESHOLD_CRITICAL_TEMP = 45.0;
    const int THRESHOLD_ACID_LEAK = 500;
    const int THRESHOLD_POLLUTED_AIR = 600;

    // Gerekli değişkenleri içeren veri yapısı
    struct CapsuleData
    {
        float temperature = 0.0;
        float humidity = 0.0;
        int gasLevel = 0;
        bool gasDanger = false;
        int acidLevel = 0;
        bool acidLeakDetected = false;
        String systemStatus = "NORMAL";
    };

    // Gerekli metotları içeren sınıf veri yapısı
    class PlanetCapsule
    {
    // Özel değişkenler
    private:
        SimpleDHT11 dht11;
        LiquidCrystal_I2C lcd;
        CapsuleData data;
        unsigned long lastDisplayTime = 0;
        const unsigned long displayInterval = 1500;

        // Açılış ekranındaki animasyon
        void showBootAnimation()
        {
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

    // Açık değişkenler
    public:
        // Yapıcı fonksiyon
        PlanetCapsule() : dht11(pinDHT), lcd(0x27, 20, 4)
        {
        }

        // Başlamak için metot
        void begin()
        {
            delay(2000);
            
            Wire.begin(D8, D9);
            lcd.init();
            lcd.backlight();

            pinMode(pinMQDigital, INPUT);

            showBootAnimation();
        }

        // Sensörlerden alınan verilerin okunmasını sağlayan metot
        void readSensors()
        {
            // Değişkenlerin tanımlanması
            byte tempByte = 0;
            byte humidByte = 0;
            
            // DHT11 sensöründen veriler başarıyla okunursa, sıcaklık ve nemi float türüne dönüştürüp veri yapısına kaydeder.
            if (dht11.read(&tempByte, &humidByte, NULL) == SimpleDHTErrSuccess)
            {
                data.temperature = (float)tempByte;
                data.humidity = (float)humidByte;
            }

            // Alınan değerlerin değişkenlere kaydedilmesi
            data.gasLevel = analogRead(pinMQAnalog);
            data.gasDanger = digitalRead(pinMQDigital) == HIGH || (data.gasLevel > THRESHOLD_POLLUTED_AIR);
            data.acidLevel = analogRead(pinAcidAnalog);
            data.acidLeakDetected = data.acidLevel > THRESHOLD_ACID_LEAK;

            // Durumlara göre modun ekrana yansıtılması
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

        // Ekranın yenilenmesi
        void updateDisplay()
        {
            // Zaman aşımının kontrol edilmesi
            if (millis() - lastDisplayTime >= displayInterval)
            {
                // Son görüntülenme süresine fonksiyondan döndürülen değerin kaydedilmesi
                lastDisplayTime = millis();

                // LCD'de birinci satıra dönülmesi
                lcd.setCursor(0, 0);
                lcd.print("CP++: " + data.systemStatus.substring(0, 14)); 
                
                // Sıcaklık ve nem değerlerinin ekrana girilmesi
                lcd.setCursor(0, 1);
                lcd.print("T:" + String(data.temperature, 1) + "C  H:%" + String(data.humidity, 0) + "   ");

                // Gaz seviyesinin gösterilmesi
                lcd.setCursor(0, 2);
                lcd.print("Gas Level: " + String(data.gasLevel) + "      ");

                // LCD'de imlecin bir alt satıra geçirilmesi
                lcd.setCursor(0, 3);
                
                // Asit tespit edildiyse...
                if (data.acidLeakDetected)
                {
                    // Asit sızıntısının tespit edildiği ekrana yazdırılır.
                    lcd.print("ACID LEAK DETECTED!! ");
                }
                // Asit tespit edilmediysa...
                else
                {
                    // Asit seviyesi ekrana yazdırılır.
                    lcd.print("Acid/Liq: " + String(data.acidLevel) + " [OK]  ");
                }
            }
        }
    };

    // PlanetCapsule sınıfından capsule nesnesinin türetilmesi
    PlanetCapsule capsule;
}

// capsule nesnesinin başlatma fonksiyonunun tetiklenmesi
void setup()
{
    CapsuleSystem::capsule.begin();
}

// capsule nesnesinin sensör okuma ve ekranı güncelleme ile ilgili metotlarını içeren döngü halinde sürekli yenilenecek olan fonksiyon kod bloğu
void loop()
{
    CapsuleSystem::capsule.readSensors();
    CapsuleSystem::capsule.updateDisplay();
}

/*
#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define pinDHT D0
#define pinMQAnalog A0
#define pinMQDigital D14
#define pinAcidAnalog A1

#define pinMotor1 D1
#define pinMotor2 D2

#define pwmChannel1 0
#define pwmChannel2 1
#define pwmFrequency 5000
#define pwmResolution 8

namespace CapsuleSystem
{
    const float THRESHOLD_CRITICAL_TEMP = 45.0;
    const int THRESHOLD_ACID_LEAK = 500;
    const int THRESHOLD_POLLUTED_AIR = 600;

    struct CapsuleData
    {
        float temperature = 0.0;
        float humidity = 0.0;
        int gasLevel = 0;
        bool gasDanger = false;
        int acidLevel = 0;
        bool acidLeakDetected = false;
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
            delay(2000);
            
            Wire.begin(D8, D9);
            lcd.init();
            lcd.backlight();

            pinMode(pinMQDigital, INPUT);
            
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
                lcd.print("Gas Level: " + String(data.gasLevel) + "     ");

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
}

void setup()
{
    CapsuleSystem::capsule.begin();
}

void loop()
{
    CapsuleSystem::capsule.readSensors();
    CapsuleSystem::capsule.coolingManager();
    CapsuleSystem::capsule.updateDisplay();
}
*/
