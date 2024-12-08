#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Fonts/FreeSans9pt7b.h>
#include <ArduinoJson.h>

// Display-Konfiguration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WLAN-Konfiguration
const char* ssid = "WLAN-NAME HERE";
const char* password = "WLAN PASSWORD HERE";

// Spotify API URL (link zur spotifyAPI.php)
const char* spotifyAPIUrl = "https://example.com/spotify/spotifyAPI.php"; 

// Icon
const uint8_t faMusic_data[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x1F, 0xF0, 0x00, 0x07, 0xFF, 0xF0, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x1F, 0xFF, 0xF0, 0x00, 0x1F, 0xFC, 0x30, 0x00, 0x1F, 0x80, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0x30, 0x00, 0x18, 0x00, 0xF0, 0x00, 0x18, 0x07, 0xF0, 0x00, 0x18, 0x0F, 0xF0, 0x00, 0xF8, 0x1F, 0xF0, 0x07, 0xF8, 0x1F, 0xF0, 0x0F, 0xF8, 0x0F, 0xE0, 0x0F, 0xF8, 0x0F, 0xC0, 0x0F, 0xF0, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Datenstruktur für Track-Infos
struct TrackInfo {
  bool playing;
  char text[256]; // "Artist - Track"
};

// Globale Variablen für den Display-Task
static int scrollX = SCREEN_WIDTH;
static unsigned long previousMillisScroll = 0;
static int scrollInterval = 50;      // alle 50ms ein Scrollschritt
static int speed = 2;                // Scroll-Geschwindigkeit
static int iconWidth = 32;
static int iconHeight = 32;

// Aktueller angezeigter Text
static char currentTrackText[256] = "";
static int textWidth = 0;

// FreeRTOS-Handles
static TaskHandle_t httpTaskHandle = NULL;
static TaskHandle_t displayTaskHandle = NULL;

// Queue für Track-Updates
static QueueHandle_t trackQueue = NULL;

// Intervalle fürs HTTP-Fetching
static const unsigned long fetchIntervalMs = 1000; // refetch jede Sekunde

// Funktion zum Neuberechnen der Textbreite
void recalculateTextWidth() {
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(currentTrackText, 0, 0, &x1, &y1, &w, &h);
  textWidth = w;

  Serial.print("[DisplayTask] Textbreite berechnet: ");
  Serial.println(textWidth);
}



// Display-Task: Scrollt kontinuierlich den aktuellen Tracktext
void displayTask(void *pvParameters) {
  Serial.println("[DisplayTask] Gestartet");
  for (;;) {
    unsigned long currentMillis = millis();
    // Alle 50ms scrollen
    if (currentMillis - previousMillisScroll >= (unsigned long)scrollInterval) {
      previousMillisScroll = currentMillis;

      // TrackInfo in der Queue?
      TrackInfo newInfo;
        if (xQueueReceive(trackQueue, &newInfo, 0) == pdTRUE) {
            // neuer Text?
            if (strcmp(currentTrackText, newInfo.text) != 0) {
                Serial.println("[DisplayTask] Neuer Tracktext empfangen und er unterscheidet sich vom alten.");
                strcpy(currentTrackText, newInfo.text);
                scrollX = SCREEN_WIDTH; // nur reset, wenn neuer Text anders ist
                recalculateTextWidth();
            } else {
                Serial.println("[DisplayTask] Neuer Tracktext empfangen, aber er ist identisch mit dem alten. Kein Reset.");
            }
        }

      if (strlen(currentTrackText) > 0) {
        // Scroll-Animation
        display.clearDisplay();
        display.setFont(&FreeSans9pt7b);
        display.setTextSize(1);
        int y = 22; 
        Serial.print("[DisplayTask] ScrollX: ");
        Serial.println(scrollX);

        display.setCursor(scrollX, y);
        display.print(currentTrackText);

        // Bereich unter Icon schwärzen
        display.fillRect(0, 0, iconWidth + 4, display.height(), SSD1306_BLACK);

        // Icon zeichnen
        display.drawBitmap(0, 0, faMusic_data, iconWidth, iconHeight, SSD1306_WHITE);

        display.display();

        scrollX -= speed;
        if (scrollX < -textWidth - SCREEN_WIDTH) { // Zusätzliche Pufferzone (-10 Pixel), um sicherzustellen, dass der gesamte Text verschwindet
            scrollX = SCREEN_WIDTH; // reset
            Serial.println("[DisplayTask] Text ist einmal komplett durchgelaufen, Reset auf Startposition.");
        }

      } else {
        Serial.println("[DisplayTask] Kein TrackText gesetzt, nichts zu scrollen.");
      }
    }

    // Kurze Pause
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// HTTP-Task: Holt alle X Sekunden den aktuellen Track von der Spotify-API
void httpTask(void *pvParameters) {
  Serial.println("[HTTPTask] Gestartet");
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[HTTPTask] Starte HTTP-Request...");
      HTTPClient http;
      http.begin(spotifyAPIUrl);
      http.setTimeout(2000); // 2 Sekunden Timeout
      int httpResponseCode = http.GET();

      Serial.print("[HTTPTask] HTTP Response Code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.print("[HTTPTask] Server Antwort: ");
        Serial.println(payload);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          bool playing = doc["playing"] | false;

          String trackText;
          if (playing) {
            const char* track = doc["track"];
            JsonArray artists = doc["artists"].as<JsonArray>();
            String artistList;

            // Nur den ersten Artist aus dem Array verwenden (cries in 128x32 display)
            if (artists.size() > 0) {
              artistList = (const char*)artists[0];
            } else {
              artistList = "Unknown Artist";
            }

            trackText = artistList + " - " + String(track);
          } else {
            trackText = "Nichts spielt...";
          }

          Serial.print("[HTTPTask] Neuer TrackText: ");
          Serial.println(trackText);

          // TrackInfo füllen
          TrackInfo info;
          info.playing = playing;
          strncpy(info.text, trackText.c_str(), sizeof(info.text)-1);
          info.text[sizeof(info.text)-1] = '\0';

          // In Queue schreiben
          if (xQueueOverwrite(trackQueue, &info) == pdPASS) {
            Serial.println("[HTTPTask] TrackInfo erfolgreich in Queue geschrieben.");
          } else {
            Serial.println("[HTTPTask] Fehler beim Schreiben in die Queue!");
          }
        } else {
          Serial.println("[HTTPTask] JSON Parsing Error!");
        }
      } else {
        Serial.print("[HTTPTask] HTTP Error: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("[HTTPTask] WLAN getrennt. Versuche erneut...");
      WiFi.begin(ssid, password);
    }

    Serial.println("[HTTPTask] Warte bis zum nächsten Fetch...");
    vTaskDelay(pdMS_TO_TICKS(fetchIntervalMs));
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("[Main] Starte Setup...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[Main] Fehler: Display konnte nicht initialisiert werden!");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  Serial.println("[Main] Stelle WLAN-Verbindung her...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("[Main] Verbinde mit WLAN...");
  }
  Serial.println("[Main] WLAN verbunden!");
  Serial.print("[Main] IP-Adresse: ");
  Serial.println(WiFi.localIP());

  // Queue erstellen
  trackQueue = xQueueCreate(1, sizeof(TrackInfo));
  if (trackQueue == NULL) {
    Serial.println("[Main] Fehler: Konnte Queue nicht erstellen!");
    while (true);
  } else {
    Serial.println("[Main] Queue erfolgreich erstellt.");
  }

  // Display-Task erstellen (Core 1)
  BaseType_t dispRes = xTaskCreatePinnedToCore(
    displayTask,
    "DisplayTask",
    4096,
    NULL,
    1,
    &displayTaskHandle,
    1
  );
  if(dispRes == pdPASS) {
    Serial.println("[Main] DisplayTask erfolgreich erstellt.");
  } else {
    Serial.println("[Main] Fehler beim Erstellen des DisplayTasks!");
  }

  // HTTP-Task erstellen (Core 0)
  BaseType_t httpRes = xTaskCreatePinnedToCore(
    httpTask,
    "HTTPTask",
    8192,
    NULL,
    1,
    &httpTaskHandle,
    0
  );
  if(httpRes == pdPASS) {
    Serial.println("[Main] HTTPTask erfolgreich erstellt.");
  } else {
    Serial.println("[Main] Fehler beim Erstellen des HTTPTasks!");
  }

  Serial.println("[Main] Setup abgeschlossen.");
}

void loop() {
  // Hier nichts, da alles in Tasks
}
