# TIG Space Series - Capsule++

**Capsule++**, TIG Space Series ekosisteminin ilk parçası olarak geliştirilmiş, zorlu uzay veya gezegen koşullarında konumlandırılan bir yaşam kapsülünün hayati parametrelerini izlemek için tasarlanmış nesne yönelimli (OOP) bir gömülü sistem yazılımıdır. 

Sistem; ortam sıcaklığını, nemini, zararlı gaz yoğunluğunu ve dış/iç çeperdeki olası sıvı temaslarını anlık olarak tarayarak akıllı bir durum analizi üretir ve entegre 20x4 LCD ekran üzerinden gerçek zamanlı bilgi akışı sağlar.

---

## 🚀 Özellikler

* **TIG Standartlarında Veri Yönetimi:** Tüm sensör girdileri tek bir dinamik veri yapısı (`CapsuleData`) altında güvenli bir şekilde toplanır.
* **Namespace Koruması:** Proje mimarisi, isim çakışmalarını önleyen ve modülerliği artıran `CapsuleSystem` namespace yapısı üzerine kurulmuştur.
* **Öncelikli Durum Algoritması:** Kritik tehdit önceliklerine göre dinamik olarak güncellenen akıllı durum göstergesi (`NORMAL`, `OVERHEATING!`, `AIR FILTER MODE`, `EMERGENCY: LIQ`).
* **Kararlı Ekran Yenileme:** Ana işlem döngüsünü bloke eden `delay()` fonksiyonları yerine, `millis()` zaman aşımı mekanizması kullanılarak akıcı bir LCD arayüzü sunulur.

---

## 🛠️ Donanım Bileşenleri ve Pin Mimarisi

Sistem, ESP tabanlı donanım mimarisi üzerinde aşağıdaki pin konfigürasyonu ile çalışmaktadır:

| Bileşen | Açıklama | Pin Tanımlaması |
| :--- | :--- | :--- |
| **DHT11** | Sıcaklık ve Nem Sensörü | `D0` |
| **MQ Gas Sensor** | Hava Kalitesi / Gaz Yoğunluğu (Analog) | `A0` |
| **MQ Gas Sensor** | Hava Kalitesi / Kritik Eşik Girişi (Dijital) | `D14` |
| **Sıvı Sensörü** | Sıvı Temas Seviyesi (Analog) | `A1` |
| **I2C LCD (20x4)** | Bilgi Ekranı | `D8 (SDA) / D9 (SCL)` |

---

## 📊 Sistem Eşik Değerleri (Thresholds)

Sistemin alarm ve mod geçişleri aşağıda belirtilen kritik sınır değerlerine göre kararlaştırılır:

* **Kritik Sıcaklık (`THRESHOLD_CRITICAL_TEMP`):** 45.0 °C
* **Sıvı Temas Sınırı (`THRESHOLD_LIQUID_CONTACT`):** 500 (Analog Eşik)
* **Hava Kirlilik Sınırı (`THRESHOLD_POLLUTED_AIR`):** 600 (Analog Eşik)

---

## 💻 Kurulum ve Bağımlılıklar

Kodun sorunsuz derlenebilmesi için Arduino IDE, Deneyap Kart IDE veya PlatformIO ortamına aşağıdaki harici kütüphanelerin eklenmesi gerekmektedir:

1. **SimpleDHT** - DHT11 sensöründen kararlı veri okumak için.
2. **LiquidCrystal_I2C** - I2C LCD ekran sürücüsü.
3. **Wire** - I2C haberleşme protokolü için (Mikrodenetleyici çekirdeği ile yerleşik gelir).

---

## ⚙️ Çalışma Mantığı

1. **Boot Aşaması:** Sistem ilk enerjiyi aldığında donanım modüllerini ve I2C hattını hazır hale getirerek LCD ekranda donanım kontrol animasyonunu (`showBootAnimation`) oynatır.
2. **Sensör Tarama Döngüsü (`readSensors`):** Sürekli olarak analog ve dijital sensör verileri süzülerek okunur ve `CapsuleData` yapısına işlenir.
3. **Tehdit Değerlendirmesi:** Okunan veriler en kritikten en hafife doğru (`Sıvı Teması > Gaz Tehlikesi > Aşırı Isınma`) bir mantık süzgecinden geçirilerek ana durum kodu atanır.
4. **Asenkron Ekran Güncelleme (`updateDisplay`):** Sistem işlemcisini yormamak adına her 1500 milisaniyede bir LCD ekranındaki veriler güncellenir ve olası sıvı temaslarında anlık uyarı verilir.
