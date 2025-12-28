# NodeMCU OLED Kurulum ve Yükleme Rehberi

Bu rehber, hazırlanan `dedoy_oled.ino` kodunu NodeMCU kartınıza nasıl yükleyeceğinizi ve bağlantıları nasıl yapacağınızı anlatır.

## 1. Donanım Bağlantıları
Şemaya göre bağlantılarınızı tekrar kontrol ediniz:

## 1. Donanım Bağlantıları (3-Wire SPI)
**Mevcut Jumper Ayarı:** R19 ve R20 Kısa Devre (BS1=0, BS0=1).
Bu mod **3-Wire SPI** (9-bit) modudur. **DC** pini kullanılmaz.

| NodeMCU Pin | OLED Pin No | Fonksiyon |
| :--- | :--- | :--- |
| **D5** (GPIO14) | **4** | SCLK (Clock) |
| **D7** (GPIO13) | **5** | SDIN (Data) |
| **D8** (GPIO15) | **16** | CS (Chip Select) |
| **D1** (GPIO5) | **15** | RES (Reset) |
| **-** | **-** | DC (Bağlanmaz) |
| **3V3** | **-** | VCC (3.3V) |
| **GND** | **-** | GND |

> **Önemli:** Eğer ekran yine çalışmazsa, jumper ayarlarınızın gerçekten R19+R20 (3-Wire) olduğundan emin olun. Fabrika çıkışı genellikle R19+R21 (4-Wire) olur.

## 2. Yazılım Gereksinimleri (Arduino IDE)

1.  **Arduino IDE'yi İndirin:** Yüklü değilse [buradan](https://www.arduino.cc/en/software) indirin.
2.  **ESP8266 Kart Desteği:**
    *   `File` -> `Preferences` -> `Additional Boards Manager URLs` kısmına şunu ekleyin:
        `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
    *   `Tools` -> `Board` -> `Boards Manager` açın.
    *   "esp8266" aratın ve **ESP8266 by ESP8266 Community** paketini kurun.
3.  **U8g2 Kütüphanesi:**
    *   `Sketch` -> `Include Library` -> `Manage Libraries` açın.
    *   "U8g2" aratın.
    *   **U8g2 by oliver** kütüphanesini yükleyin (en son sürüm).

## 3. Kodu Yükleme

1.  `dedoy_oled.ino` dosyasını Arduino IDE ile açın.
2.  **Kart Seçimi:** `Tools` -> `Board` menüsünden **NodeMCU 1.0 (ESP-12E Module)** seçin.
3.  **Port Seçimi:** NodeMCU'yu USB ile bağlayın ve `Tools` -> `Port` menüsünden ilgili portu seçin (COMx veya /dev/cu.usbserial...).
4.  **Yükleme:** Sol üstteki "Upload" (Sağ ok) butonuna basın.
5.  Derleme ve yükleme tamamlandığında altta "Done uploading" yazısını görmelisiniz.

## 4. Sorun Giderme
*   **Ekran Karıncalı veya Boş:** Bağlantıları, özellikle `CS`, `DC` ve `RST` pinlerini kontrol edin.
*   **Yükleme Hatası:** USB kablosunu değiştirin veya "BOOT" butonuna basılı tutarak yüklemeyi deneyin (NodeMCU'da genelde gerekmez ama bazen gerekebilir).
