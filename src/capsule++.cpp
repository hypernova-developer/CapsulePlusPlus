/*
    Capsule++, TIG'nin Space Series başlığı altında geliştirdiği projelerden birisidir. 
    Kaynak kodu hypernova-developer tarafından yazılmış olup bu proje GNU General Public License v3.0 ile korunmaktadır. 
    Bu kaynak kodlardan yararlanmanız halinde yeni projenizin açık kaynaklı olarak yayınlanması bir zorunluluktur. 
    Projede kullanılan kart Deneyap Kart 1A (ESP32 işlemcili) olup Arduino IDE veya Deneyap Kart IDE gibi bir program kullanılarak kod yüklenmesinde sakınca yoktur. 
    Bize imkan tamıyan, her şekilde destek olan T3 Vakfı ekibine ve öğrenci arkadaşlarımıza teşekkür ederiz. 
    Rakiplerimize başarılar dileriz. 
    Açık kaynak topluluğuna ve yeni projelere destek olması ümidiyle...
    - Team: InterGalactic Üyeleri
*/

// Kütüphanelerin koda dahil edilmesi
#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pinlerin tanımlanması
#define pinDHT D0
#define pinMQAnalog A0
#define pinMQDigital D14
#define pinAcidAnalog A1

// Gerekli sınıf ve fonksiyonları içeren namespace yapısı
namespace CapsuleSystem
{
    // Sınır değişkenlerinin tanımlanması
    const float THRESHOLD_CRITICAL_TEMP = 45.0;
    const int THRESHOLD_LIQUID_CONTACT = 500;
    const int THRESHOLD_POLLUTED_AIR = 600;

    // Gerekli değişkenleri içeren veri yapısı
    struct CapsuleData
    {
        float temperature = 0.0;
        float humidity = 0.0;
        int gasLevel = 0;
        bool gasDanger = false;
        int liquidLevel = 0;
        bool liquidContactDetected = false;
        String systemStatus = "NORMAL";
    };

    // Gerekli metotları içeren sınıf veri yapısı
    class PlanetCapsule
    {
    private:
        // Özel değişkenler
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

    public:
        // Yapıcı fonksiyon
        PlanetCapsule() : dht11(pinDHT), lcd(0x27, 20, 4)
        {
        }

        // Başlatma metodu
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
            
            // DHT11 sensöründen verilerin başarıyla okunması durumunda, sıcaklık ve nemi float türüne dönüştürüp veri yapısına kaydedilmesi
            if (dht11.read(&tempByte, &humidByte, NULL) == SimpleDHTErrSuccess)
            {
                data.temperature = (float)tempByte;
                data.humidity = (float)humidByte;
            }

            // Alınan değerlerin değişkenlere kaydedilmesi
            data.gasLevel = analogRead(pinMQAnalog);
            data.gasDanger = digitalRead(pinMQDigital) == HIGH || (data.gasLevel > THRESHOLD_POLLUTED_AIR);
            data.liquidLevel = analogRead(pinAcidAnalog);
            data.liquidContactDetected = data.liquidLevel > THRESHOLD_LIQUID_CONTACT;

            // Durumlara göre modun ekrana yansıtılması
            if (data.liquidContactDetected)
            {
                data.systemStatus = "EMERGENCY: LIQ";
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
                lcd.print("Gas Level: " + String(data.gasLevel) + "       ");

                // LCD'de imlecin bir alt satıra geçirilmesi
                lcd.setCursor(0, 3);
                
                // Sıvı ile temas edildiyse...
                if (data.liquidContactDetected)
                {
                    // Sıvının tespit edildiğinin ekrana yazdırılması
                    lcd.print("LIQUID DETECTED!!!  ");
                }
                // Sıvı ile temas edilmediyse...
                else
                {
                    // Sıvı seviyesinin ekrana yazdırılması
                    lcd.print("Liquid/Liq: " + String(data.liquidLevel) + " [OK] ");
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
