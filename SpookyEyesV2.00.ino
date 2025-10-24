// ============================================================
// Eye Drawing Code - v1.30 (NVRAM role, reboot, TX power control)
// ============================================================

#include <TFT_eSPI.h>  // TFT display library
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <Preferences.h>

TFT_eSPI actualTft = TFT_eSPI();           // Real TFT hardware
TFT_eSprite tft = TFT_eSprite(&actualTft); // Sprite object, acts like an offscreen canvas
TFT_eSprite tft2 = TFT_eSprite(&actualTft);

Preferences prefs;

// Display parameters (240x240 for GC9A01)
#define SPARKLE_RADIUS 3
#define MAX_FIELDS 10       // Maximum number of CSV fields in messages received
#define MAX_FIELD_LEN 16    // Max chars per field
#define ROTATION 0          // change for screen orientation 0-3

int whiteColor = TFT_WHITE;
int eyeX, eyeY;       // pos of eye
int pupilSize = 50;
int irisSize = 94;
int eyeSize = 150;
int deviceRole = 1;   // -1 = LEADER, 1 = FOLLOWER
int eyeColor;
int senderDelay = 100;  
int receiverDelay = 90; 
int noInterrupts = 0;

// Broadcast MAC for all devices
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Callback when data is sent
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *recvInfo, const uint8_t *incomingData, int len) {
  if(noInterrupts==1) return;
  noInterrupts=1;  
  delay(receiverDelay);
  String msg = "";
  for (int i = 0; i < len; i++) {
    msg += (char)incomingData[i];
  }

  Serial.print("📩 Received: ");
  Serial.println(msg);

  // Split message into fields
  String fields[MAX_FIELDS];
  int fieldCount = 0;
  int start = 0;
  while (true) {
    int commaIndex = msg.indexOf(',', start);
    if (commaIndex == -1) {
      fields[fieldCount++] = msg.substring(start);
      break;
    } else {
      fields[fieldCount++] = msg.substring(start, commaIndex);
      start = commaIndex + 1;
    }
    if (fieldCount >= MAX_FIELDS) break;
  }

  // Command handling
  if (fields[0] == "LookTo" && fieldCount >= 3) {
    int x = fields[1].toInt();
    int y = fields[2].toInt();
    int speed = (fieldCount >= 4) ? fields[3].toInt() : 10;
    lookTo(x, y, speed);
  }
  else if (fields[0] == "ChangePupil" && fieldCount >= 2) {
    int size = fields[1].toInt();
    changePupil(size);
  }
  else if (fields[0] == "FastBlink") {
      fastBlink();
  }
  else if (fields[0] == "ChangeColor") {
      changeEyeColor(fields[1].toInt());
  }
  else {
    Serial.println("⚠️  Unknown command received.");
  }
  noInterrupts=0;
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));  
  actualTft.init();
  actualTft.setRotation(ROTATION);
  actualTft.fillScreen(TFT_BLACK);
  actualTft.setTextColor(TFT_GREEN);
  actualTft.setTextSize(3);
  tft.createSprite(240, 240);       
  tft2.createSprite(240, 240);
  tft.fillScreen(whiteColor);  // sclera color

  pinMode(4, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);  // buttons for ESP32 with integrated GC9A01

  eyeX = 0;
  eyeY = 0;
  eyeColor = TFT_BLUE;

  // Open NVRAM namespace
  prefs.begin("myApp", false);

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(4);   // TX power control

  // Read stored values if they exist
  if (prefs.isKey("deviceRole")) {
    deviceRole = prefs.getInt("deviceRole", deviceRole);
    Serial.print("Loaded deviceRole: "); Serial.println(deviceRole);
  }

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (deviceRole == -1) esp_now_register_send_cb(OnDataSent);  // LEADER
  if (deviceRole == 1)  esp_now_register_recv_cb(OnDataRecv);  // FOLLOWER

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  drawEye(120, 120, eyeSize, irisSize, pupilSize, whiteColor);
  post();  
  lookTo(eyeX, eyeY, 0);
  
  if (deviceRole == 1) {
    actualTft.setCursor(70, 100);
    actualTft.fillScreen(TFT_BLACK);
    actualTft.print("READY");
  }
}

void post() {  // Power On Self Test
  crtOn(TFT_WHITE, 20);
  lookTo(-60, -60, 0);
  lookTo(60, -60, 0);
  lookTo(60, 60, 0);
  lookTo(-60, 60, 0);
  lookTo(-60, -60, 0);
  lookTo(0, 0, 0);
  delay(1000);
  crtOff(TFT_WHITE, 20);
}

void loop() {
  if (deviceRole == -1) {  // LEADER
    int action = random(1, 6); 

    if (action == 1) {
      int x = random(1, 141) - 70;
      int y = random(1, 141) - 70;
      broadcastMessage("LookTo," + String(x) + "," + String(y) + ",0");
      lookTo(x, y, 0);    
    }
    if (action == 2 && random(1, 7) <= 2) {
      broadcastMessage("FastBlink");
      fastBlink();
    }
    if (action == 3 && random(1, 8) <= 1) {
      broadcastMessage("ChangePupil,36");
      changePupil(36);
    }
    if (action == 4 && random(1, 8) <= 1) {
      broadcastMessage("ChangePupil,67");
      changePupil(67);
    }
    if (action == 5 && random(1, 50) <= 1) {
      int col = random(1, 6);
      broadcastMessage("ChangeColor," + String(col));
      changeEyeColor(col);
    }
  }

  // Buttons
  if (digitalRead(4) == 0) {
    // placeholder for other button
  }
  if (digitalRead(19) == 0) {
    Serial.print("\n\r Changing role to ...");
    deviceRole = deviceRole * -1;
    prefs.putInt("deviceRole", deviceRole);  // save
    Serial.print(deviceRole);
    actualTft.fillScreen(TFT_BLACK);
    actualTft.setCursor(55, 100);
    if (deviceRole == -1) actualTft.print(" LEADER");
    else actualTft.print("FOLLOWER");
    delay(2000);  
    ESP.restart();  // role stored, reboot
  }

  delay(400);
}

void changeEyeColor(int action) {
  switch(action) {
    case 1: eyeColor = TFT_BLUE; break;
    case 2: eyeColor = TFT_RED; break;
    case 3: eyeColor = TFT_ORANGE; break;
    case 4: eyeColor = TFT_GREEN; break;
    case 5: eyeColor = TFT_PURPLE; break;
    default: eyeColor = TFT_BLUE; break;
  }
}

void changePupil(int newSize) {
  int delta = (newSize < pupilSize) ? -1 : 1;
  while (pupilSize != newSize) {
    pupilSize += delta;
    drawEye(120, 120, eyeSize, irisSize, pupilSize, whiteColor);
    tft.pushSprite(eyeX, eyeY);
    delay(5);
  }
  pupilSize = newSize;
}

// Color conversion
void rgb565To888(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = ((color >> 11) & 0x1F) << 3;
  g = ((color >> 5) & 0x3F) << 2;
  b = (color & 0x1F) << 3;
}
uint16_t rgb888To565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// CRT On/Off effects
void crtOn(uint16_t lineColor, int frameDelay) {
  int cx = 120, cy = 120;
  actualTft.fillScreen(TFT_BLACK);
  actualTft.fillCircle(cx, cy, 3, lineColor);
  delay(150);
  for (int w = 2; w <= 240; w += 8) { actualTft.fillRect(cx - w / 2, cy - 1, w, 2, lineColor); delay(frameDelay); }
  delay(100);
  for (int i = 0; i < cy; i += 4) {
    actualTft.fillRect(0, cy - i - 4, 240, 4, lineColor);
    actualTft.fillRect(0, cy + i, 240, 4, lineColor);
    delay(frameDelay);
  }
  actualTft.fillScreen(TFT_WHITE);
}

void crtOff(uint16_t lineColor, int frameDelay) {
  int cx = 120, cy = 120;
  for (int i = 0; i < cy; i += 4) {
    actualTft.fillRect(0, i, 240, 4, TFT_BLACK);
    actualTft.fillRect(0, 240 - i - 4, 240, 4, TFT_BLACK);
    delay(frameDelay);
  }
  actualTft.fillRect(0, cy - 1, 240, 2, lineColor);
  delay(100);
  for (int w = 240; w > 2; w -= 8) {
    actualTft.fillRect(cx - w / 2, cy - 1, w, 2, TFT_BLACK);
    actualTft.fillRect(cx - w / 2 + 4, cy - 1, w - 8, 2, lineColor);
    delay(frameDelay);
  }
  actualTft.fillCircle(cx, cy, 3, lineColor);
  delay(150);
  for (int r = 3; r > 0; r--) { actualTft.fillCircle(cx, cy, r, TFT_BLACK); delay(80); }
  actualTft.fillScreen(TFT_BLACK);
}

void resetTft() {
  actualTft.setRotation(ROTATION);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(3);
  if (deviceRole == 1) {
    actualTft.fillScreen(TFT_BLACK);
    actualTft.setCursor(70, 100);
    actualTft.print("READY");
  }
}

void fastBlink() {
  tft2.fillSprite(TFT_BLACK);
  tft2.pushSprite(0,0);
  tft2.fillSprite(whiteColor);
  tft2.pushSprite(0,0);
  tft.pushSprite(eyeX, eyeY);
}

void lookTo(int endX, int endY, float moveDuration) {
  const int steps = 15.73;
  float dx = (endX - eyeX) / (float)steps;
  float dy = (endY - eyeY) / (float)steps;
  int delayPerStep = (int)((moveDuration * 1000.0f) / steps); if (delayPerStep < 1) delayPerStep = 1;
  float curX = eyeX, curY = eyeY;
  for (int i = 0; i <= steps; i++) {
    int newX = (int)(eyeX + dx * i);
    int newY = (int)(eyeY + dy * i);
    tft.pushSprite(newX, newY);
    curX = newX; curY = newY;
    delay(delayPerStep);
  }
  eyeX = curX; eyeY = curY;
}

void drawEye(int x, int y, int eyeRadius, int irisRadius, int pupilRadius, uint16_t scleraColor) {
  drawIris(x, y, irisRadius, eyeColor);
  drawIrisOutline(x, y, irisRadius);
  drawPupil(x, y, pupilRadius);
  drawSparkle(x, y);
}

void drawIris(int x, int y, int radius, uint16_t baseColor) {
  int r = ((baseColor >> 11) & 0x1F) * 255 / 31;
  int g = ((baseColor >> 5) & 0x3F) * 255 / 63;
  int b = (baseColor & 0x1F) * 255 / 31;
  for (int rad = radius; rad > 0; rad--) {
    uint8_t shade = map(rad, 0, radius, 80, 255);
    int rr = (r * shade) / 255;
    int gg = (g * shade) / 255;
    int bb = (b * shade) / 255;
    tft.fillCircle(x, y, rad, tft.color565(rr, gg, bb));
  }
}

void drawIrisOutline(int x, int y, int radius) { tft.drawCircle(x, y, radius, TFT_BLACK); }
void drawPupil(int x, int y, int radius) { tft.fillCircle(x, y, radius, TFT_BLACK); }
void drawSparkle(int x, int y) {
  int offset = (pupilSize / 2) - 4;
  tft.fillCircle(x - offset, y - offset, SPARKLE_RADIUS, TFT_LIGHTGREY);
}

void broadcastMessage(const String &msg) {
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)msg.c_str(), msg.length());
  if (result == ESP_OK) Serial.println("Broadcast sent successfully: " + msg);
  else Serial.println("Error sending broadcast: " + msg);
  delay(senderDelay);
}
