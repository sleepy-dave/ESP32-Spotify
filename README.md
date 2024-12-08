> [!NOTE]
> Work in progress!

# Spotify Track Anzeige mit ESP32 und SSD1306 OLED
Dieses Projekt zeigt den aktuell abgespielten Spotify-Track auf einem SSD1306 OLED-Display, gesteuert von einem ESP32 (AZ-Delivery DevKit V4). Das System verbindet sich mit einem WLAN-Netzwerk, ruft regelmäßig die Spotify-API ab und zeigt den Track- und Künstlernamen in einer scrollenden Textanzeige an. Zusätzlich wird ein Musik-Icon zur visuellen Unterstützung angezeigt. Entwickelt mit PlatformIO und dem Arduino-Framework, nutzt das Projekt Bibliotheken wie Adafruit GFX, Adafruit SSD1306 und ArduinoJson für eine effiziente Darstellung und Datenverarbeitung.

## Features
+ **WLAN-Verbindung**: Stabile Verbindung zu Ihrem WLAN-Netzwerk.
+ **Spotify-Integration**: Ruft den aktuell abgespielten Track von der Spotify-API ab.
+ **OLED-Anzeige**: Anzeige des Tracknamens und des Künstlernamens auf einem SSD1306 OLED-Display.
+ **Scrollende Textanzeige**: Flüssiges Scrollen des Textes für eine bessere Lesbarkeit.
+ **Musik-Icon**: Anzeige eines Musik-Icons neben dem Tracktext.
+ **Echtzeit-Updates**: Aktualisiert die Anzeige jede Sekunde mit den neuesten Trackinformationen.
## Hardware
+ **ESP32 Entwicklungsboard**: AZ-Delivery DevKit V4
+ **SSD1306 OLED Display**: 128x32 Pixel
+ **Verbindungskabel**: Für I2C-Kommunikation zwischen ESP32 und OLED
+ **Stromquelle**: USB-Kabel oder geeignete Stromversorgung für das ESP32-Board
