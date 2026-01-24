// UI ENGINE for Arduino Zero - ILI9341 TFT + XPT2046 Touch
// Feature-rich UI framework with screen management, buttons, navigation, and more

// TFT Pin definitions
#include <string.h>


#define TFT_CS   9
#define TFT_RST  8
#define TFT_DC   7
#define TFT_MOSI 6
#define TFT_SCK  5
#define TFT_MISO 3
#define TFT_LED  4

// Touch Pin definitions
#define T_IRQ  A5
#define T_DO   A4
#define T_DIN  A3
#define T_CS   A2
#define T_CLK  A1

// Screen dimensions
#define WIDTH  240
#define HEIGHT 320

// UI Layout constants
#define NAVBAR_HEIGHT 50
#define NAVBAR_Y (HEIGHT - NAVBAR_HEIGHT)
#define HEADER_HEIGHT 30
#define STATUS_HEIGHT 20
#define CONTENT_Y (HEADER_HEIGHT + STATUS_HEIGHT)
#define CONTENT_HEIGHT (NAVBAR_Y - CONTENT_Y)

// ILI9341 Commands
#define ILI9341_SWRESET   0x01
#define ILI9341_SLPOUT    0x11
#define ILI9341_DISPON    0x29
#define ILI9341_CASET     0x2A
#define ILI9341_PASET     0x2B
#define ILI9341_RAMWR     0x2C
#define ILI9341_MADCTL    0x36
#define ILI9341_PIXFMT    0x3A
#define ILI9341_FRMCTR1   0xB1
#define ILI9341_PWCTR1    0xC0
#define ILI9341_PWCTR2    0xC1
#define ILI9341_VMCTR1    0xC5
#define ILI9341_VMCTR2    0xC7

// XPT2046 Commands
#define XPT2046_CMD_X  0xD0
#define XPT2046_CMD_Y  0x90
#define XPT2046_CMD_Z1 0xB0
#define XPT2046_CMD_Z2 0xC0

// Colors (RGB565)
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFD20
#define GRAY    0x8410
#define DARKGRAY 0x4208
#define LIGHTGRAY 0xC618
#define DARKGREEN 0x03E0

// Touch calibration
#define TS_MINX 414
#define TS_MINY 311
#define TS_MAXX 3583
#define TS_MAXY 3713
#define PRESSURE_THRESHOLD 400

// ===================================
// UI ENGINE - TYPES & STRUCTURES
// ===================================

// Screen IDs
typedef enum {
  SCREEN_HOME,
  SCREEN_FILES,
  SCREEN_AI,
  SCREEN_SETTINGS,
  SCREEN_INPUT,
  SCREEN_COUNT
} ScreenID;

// Alert types
typedef enum {
  UI_ALERT_NONE,
  UI_ALERT_INFO,
  UI_ALERT_WARN,
  UI_ALERT_ERROR
} AlertType;

// Label IDs for bilingual support
typedef enum {
  LABEL_MOISTURE,
  LABEL_NITROGEN,
  LABEL_PHOSPHORUS,
  LABEL_POTASSIUM,
  LABEL_TEMPERATURE,
  LABEL_HUMIDITY,
  LABEL_PH,
  LABEL_COUNT
} LabelID;

// Button structure
typedef struct {
  int16_t x, y, w, h;
  const char* label;
  uint16_t color;
  void (*callback)(void);
  bool visible;
} UIButton;

// Data binding structure
typedef struct {
  LabelID id;
  int16_t x, y;
  int16_t value;
  int16_t lastValue;
  bool needsRedraw;
} UIValue;

// UI State
typedef struct {
  ScreenID currentScreen;
  ScreenID lastScreen;
  AlertType alertType;
  char alertMsg[32];
  uint32_t alertTime;
  bool needsFullRedraw;
  int16_t lastTouchX;
  int16_t lastTouchY;
  uint32_t lastTouchTime;
  uint8_t gsmSignal;
  uint8_t batteryLevel;
  bool gpsLock;
} UIState;

// Global UI state
UIState uiState = {SCREEN_HOME, SCREEN_HOME, UI_ALERT_NONE, "", 0, true, -1, -1, 0, 0, 0, false};

// Language strings (English)
const char* labels_en[LABEL_COUNT] = {
  "Moisture", "Nitrogen", "Phosphorus", "Potassium", "Temperature", "Humidity", "pH"
};

// ===================================
// TFT LOW LEVEL SPI
// ===================================

void tftSpiWrite(uint8_t data) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(TFT_SCK, LOW);
    digitalWrite(TFT_MOSI, (data >> i) & 0x01);
    digitalWrite(TFT_SCK, HIGH);
  }
}

void writeCommand(uint8_t cmd) {
  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_CS, LOW);
  tftSpiWrite(cmd);
  digitalWrite(TFT_CS, HIGH);
}

void writeData(uint8_t data) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  tftSpiWrite(data);
  digitalWrite(TFT_CS, HIGH);
}

void writeData16(uint16_t data) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  tftSpiWrite(data >> 8);
  tftSpiWrite(data & 0xFF);
  digitalWrite(TFT_CS, HIGH);
}

// ===================================
// TOUCH LOW LEVEL SPI
// ===================================

uint8_t touchSpiTransfer(uint8_t data) {
  uint8_t reply = 0;
  for (int i = 7; i >= 0; i--) {
    digitalWrite(T_CLK, LOW);
    digitalWrite(T_DIN, (data >> i) & 0x01);
    digitalWrite(T_CLK, HIGH);
    reply <<= 1;
    if (digitalRead(T_DO)) reply |= 1;
  }
  return reply;
}

uint16_t touchRead(uint8_t command) {
  digitalWrite(T_CS, LOW);
  touchSpiTransfer(command);
  uint8_t high = touchSpiTransfer(0x00);
  uint8_t low = touchSpiTransfer(0x00);
  digitalWrite(T_CS, HIGH);
  return ((high << 8) | low) >> 3;
}

bool getTouchRaw(uint16_t *x, uint16_t *y, uint16_t *z) {
  if (digitalRead(T_IRQ) != LOW) return false;
  
  uint32_t sumX = 0, sumY = 0, sumZ1 = 0, sumZ2 = 0;
  int samples = 4;
  
  for (int i = 0; i < samples; i++) {
    sumX += touchRead(XPT2046_CMD_X);
    sumY += touchRead(XPT2046_CMD_Y);
    sumZ1 += touchRead(XPT2046_CMD_Z1);
    sumZ2 += touchRead(XPT2046_CMD_Z2);
  }
  
  *x = sumX / samples;
  *y = sumY / samples;
  
  uint16_t z1 = sumZ1 / samples;
  uint16_t z2 = sumZ2 / samples;
  *z = (z1 == 0) ? 0 : (*x * (z2 - z1)) / z1;
  
  return (*z > PRESSURE_THRESHOLD);
}

bool getTouch(int16_t *x, int16_t *y) {
  uint16_t rawX, rawY, pressure;
  if (!getTouchRaw(&rawX, &rawY, &pressure)) return false;
  
  *x = map(rawX, TS_MAXX, TS_MINX, 0, WIDTH);
  *y = map(rawY, TS_MAXY, TS_MINY, 0, HEIGHT);
  *x = constrain(*x, 0, WIDTH - 1);
  *y = constrain(*y, 0, HEIGHT - 1);
  
  return true;
}

// ===================================
// DISPLAY INITIALIZATION
// ===================================

void initDisplay() {
  digitalWrite(TFT_RST, HIGH);
  delay(10);
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
  delay(150);
  
  writeCommand(ILI9341_SWRESET);
  delay(150);
  writeCommand(ILI9341_SLPOUT);
  delay(120);
  
  writeCommand(0xCB); writeData(0x39); writeData(0x2C); writeData(0x00); writeData(0x34); writeData(0x02);
  writeCommand(0xCF); writeData(0x00); writeData(0xC1); writeData(0x30);
  writeCommand(0xE8); writeData(0x85); writeData(0x00); writeData(0x78);
  writeCommand(0xEA); writeData(0x00); writeData(0x00);
  writeCommand(0xED); writeData(0x64); writeData(0x03); writeData(0x12); writeData(0x81);
  writeCommand(0xF7); writeData(0x20);
  writeCommand(ILI9341_PWCTR1); writeData(0x23);
  writeCommand(ILI9341_PWCTR2); writeData(0x10);
  writeCommand(ILI9341_VMCTR1); writeData(0x3E); writeData(0x28);
  writeCommand(ILI9341_VMCTR2); writeData(0x86);
  writeCommand(ILI9341_MADCTL); writeData(0x68);
  writeCommand(ILI9341_PIXFMT); writeData(0x55);
  writeCommand(ILI9341_FRMCTR1); writeData(0x00); writeData(0x10);
  writeCommand(0xB6); writeData(0x08); writeData(0x82); writeData(0x27);
  writeCommand(0xF2); writeData(0x00);
  writeCommand(0x26); writeData(0x01);
  
  writeCommand(0xE0);
  writeData(0x0F); writeData(0x31); writeData(0x2B); writeData(0x0C); writeData(0x0E); writeData(0x08);
  writeData(0x4E); writeData(0xF1); writeData(0x37); writeData(0x07); writeData(0x10); writeData(0x03);
  writeData(0x0E); writeData(0x09); writeData(0x00);
  
  writeCommand(0xE1);
  writeData(0x00); writeData(0x0E); writeData(0x14); writeData(0x03); writeData(0x11); writeData(0x07);
  writeData(0x31); writeData(0xC1); writeData(0x48); writeData(0x08); writeData(0x0F); writeData(0x0C);
  writeData(0x31); writeData(0x36); writeData(0x0F);
  
  writeCommand(ILI9341_DISPON);
  delay(100);
}

// ===================================
// DRAWING FUNCTIONS
// ===================================

void setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  writeCommand(ILI9341_CASET);
  writeData16(x0);
  writeData16(x1);
  writeCommand(ILI9341_PASET);
  writeData16(y0);
  writeData16(y1);
  writeCommand(ILI9341_RAMWR);
}

void fillScreen(uint16_t color) {
  setWindow(0, 0, WIDTH - 1, HEIGHT - 1);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  for (uint32_t i = 0; i < (uint32_t)WIDTH * HEIGHT; i++) {
    tftSpiWrite(color >> 8);
    tftSpiWrite(color & 0xFF);
  }
  digitalWrite(TFT_CS, HIGH);
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
  setWindow(x, y, x, y);
  writeData16(color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (x >= WIDTH || y >= HEIGHT) return;
  if (x + w > WIDTH) w = WIDTH - x;
  if (y + h > HEIGHT) h = HEIGHT - y;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (w <= 0 || h <= 0) return;
  
  setWindow(x, y, x + w - 1, y + h - 1);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  for (int32_t i = 0; i < (int32_t)w * h; i++) {
    tftSpiWrite(color >> 8);
    tftSpiWrite(color & 0xFF);
  }
  digitalWrite(TFT_CS, HIGH);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  int16_t dx = abs(x1 - x0), dy = abs(y1 - y0);
  int16_t sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
  int16_t err = dx - dy;
  
  while (true) {
    drawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    int16_t e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 < dx) { err += dx; y0 += sy; }
  }
}

// ===================================
// UI ENGINE - DRAWING ABSTRACTION
// ===================================

void ui_drawHeader(const char* title) {
  fillRect(0, 0, WIDTH, HEADER_HEIGHT, BLUE);
  // Title would go here (needs font rendering)
}

void ui_drawStatus() {
  fillRect(0, HEADER_HEIGHT, WIDTH, STATUS_HEIGHT, DARKGRAY);
  
  // GSM signal (right side)
  int sigBars = uiState.gsmSignal / 25;
  for (int i = 0; i < 4; i++) {
    uint16_t color = (i < sigBars) ? GREEN : GRAY;
    fillRect(WIDTH - 30 + i * 6, HEADER_HEIGHT + 15 - i * 3, 4, 5 + i * 3, color);
  }
  
  // Battery (left side)
  fillRect(5, HEADER_HEIGHT + 5, 20, 10, WHITE);
  fillRect(25, HEADER_HEIGHT + 8, 2, 4, WHITE);
  int batWidth = (uiState.batteryLevel * 18) / 100;
  uint16_t batColor = (uiState.batteryLevel > 20) ? GREEN : RED;
  fillRect(6, HEADER_HEIGHT + 6, batWidth, 8, batColor);
  
  // GPS indicator
  if (uiState.gpsLock) {
    fillRect(35, HEADER_HEIGHT + 5, 8, 8, GREEN);
  }
}

void ui_drawFooter() {
  fillRect(0, NAVBAR_Y, WIDTH, NAVBAR_HEIGHT, DARKGRAY);
  
  const char* navLabels[] = {"Home", "Files", "AI", "Set", "Input"};
  uint16_t navColors[] = {BLUE, GREEN, ORANGE, GRAY, CYAN};
  
  for (int i = 0; i < 5; i++) {
    int16_t btnX = i * 48;
    uint16_t color = (uiState.currentScreen == i) ? navColors[i] : LIGHTGRAY;
    fillRect(btnX, NAVBAR_Y, 48, NAVBAR_HEIGHT, color);
    fillRect(btnX + 2, NAVBAR_Y + 2, 44, NAVBAR_HEIGHT - 4, DARKGRAY);
    
    // Simple icon representation (colored square)
    fillRect(btnX + 15, NAVBAR_Y + 8, 18, 18, navColors[i]);
  }
}

void ui_drawCard(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, int16_t value, uint16_t color) {
  fillRect(x, y, w, h, color);
  fillRect(x + 2, y + 2, w - 4, h - 4, WHITE);
  fillRect(x + 5, y + 5, w - 10, 20, color);
  
  // Value area
  fillRect(x + 10, y + 30, w - 20, 25, LIGHTGRAY);
}

void ui_drawButton(UIButton* btn) {
  if (!btn->visible) return;
  fillRect(btn->x, btn->y, btn->w, btn->h, btn->color);
  fillRect(btn->x + 2, btn->y + 2, btn->w - 4, btn->h - 4, WHITE);
}

void ui_drawAlert(const char* text, AlertType type) {
  if (type == UI_ALERT_NONE) return;
  
  uint16_t color = (type == UI_ALERT_ERROR) ? RED : (type == UI_ALERT_WARN) ? YELLOW : CYAN;
  fillRect(0, CONTENT_Y, WIDTH, 30, color);
  fillRect(2, CONTENT_Y + 2, WIDTH - 4, 26, BLACK);
}

// ===================================
// UI ENGINE - BUTTON SYSTEM
// ===================================

#define MAX_BUTTONS 12
UIButton buttons[MAX_BUTTONS];
int buttonCount = 0;

void ui_addButton(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t color, void (*callback)(void)) {
  if (buttonCount >= MAX_BUTTONS) return;
  buttons[buttonCount].x = x;
  buttons[buttonCount].y = y;
  buttons[buttonCount].w = w;
  buttons[buttonCount].h = h;
  buttons[buttonCount].label = label;
  buttons[buttonCount].color = color;
  buttons[buttonCount].callback = callback;
  buttons[buttonCount].visible = true;
  buttonCount++;
}

bool ui_checkButton(UIButton* btn, int16_t x, int16_t y) {
  if (!btn->visible) return false;
  return (x >= btn->x && x < btn->x + btn->w && y >= btn->y && y < btn->y + btn->h);
}

// ===================================
// UI ENGINE - DATA BINDING
// ===================================

#define MAX_VALUES 8
UIValue dataValues[MAX_VALUES];
int valueCount = 0;

void ui_registerValue(LabelID id, int16_t x, int16_t y, int16_t initialValue) {
  if (valueCount >= MAX_VALUES) return;
  dataValues[valueCount].id = id;
  dataValues[valueCount].x = x;
  dataValues[valueCount].y = y;
  dataValues[valueCount].value = initialValue;
  dataValues[valueCount].needsRedraw = true;
  valueCount++;
}

void ui_updateValue(LabelID id, int16_t newValue) {
  for (int i = 0; i < valueCount; i++) {
    if (dataValues[i].id == id && dataValues[i].value != newValue) {
      dataValues[i].value = newValue;
      dataValues[i].needsRedraw = true;
    }
  }
}

void ui_redrawValues() {
  for (int i = 0; i < valueCount; i++) {
    if (dataValues[i].needsRedraw) {
      // Redraw only this value area
      fillRect(dataValues[i].x, dataValues[i].y, 80, 25, LIGHTGRAY);
      dataValues[i].needsRedraw = false;
    }
  }
}

// ===================================
// UI ENGINE - ALERT SYSTEM
// ===================================

void ui_showAlert(const char* msg, AlertType type) {
  strncpy(uiState.alertMsg, msg, 31);
  uiState.alertMsg[31] = '\0';
  uiState.alertType = type;
  uiState.alertTime = millis();
  ui_drawAlert(msg, type);
}

void ui_hideAlert() {
  if (uiState.alertType != UI_ALERT_NONE && millis() - uiState.alertTime > 3000) {
    uiState.alertType = UI_ALERT_NONE;
    fillRect(0, CONTENT_Y, WIDTH, 30, WHITE);
  }
}

// ===================================
// SCREEN IMPLEMENTATIONS
// ===================================

void screen_home_draw() {
  fillRect(0, CONTENT_Y, WIDTH, CONTENT_HEIGHT, WHITE);
  
  // Draw sensor cards
  ui_drawCard(10, CONTENT_Y + 40, 105, 80, "Moisture", 0, CYAN);
  ui_drawCard(125, CONTENT_Y + 40, 105, 80, "Nitrogen", 0, GREEN);
  ui_drawCard(10, CONTENT_Y + 130, 105, 80, "Phosphorus", 0, ORANGE);
  ui_drawCard(125, CONTENT_Y + 130, 105, 80, "Potassium", 0, MAGENTA);
  
  // Register values for updating
  ui_registerValue(LABEL_MOISTURE, 20, CONTENT_Y + 75, 0);
  ui_registerValue(LABEL_NITROGEN, 135, CONTENT_Y + 75, 0);
  ui_registerValue(LABEL_PHOSPHORUS, 20, CONTENT_Y + 165, 0);
  ui_registerValue(LABEL_POTASSIUM, 135, CONTENT_Y + 165, 0);
}

void screen_files_draw() {
  fillRect(0, CONTENT_Y, WIDTH, CONTENT_HEIGHT, WHITE);
  fillRect(10, CONTENT_Y + 10, WIDTH - 20, 40, LIGHTGRAY);
  fillRect(10, CONTENT_Y + 60, WIDTH - 20, 40, LIGHTGRAY);
  fillRect(10, CONTENT_Y + 110, WIDTH - 20, 40, LIGHTGRAY);
}

void screen_ai_draw() {
  fillRect(0, CONTENT_Y, WIDTH, CONTENT_HEIGHT, WHITE);
  fillRect(10, CONTENT_Y + 10, WIDTH - 20, 60, CYAN);
  fillRect(10, CONTENT_Y + 80, WIDTH - 20, 100, LIGHTGRAY);
}

void screen_settings_draw() {
  fillRect(0, CONTENT_Y, WIDTH, CONTENT_HEIGHT, WHITE);
  
  ui_clearButtons();
  ui_addButton(10, CONTENT_Y + 20, WIDTH - 20, 40, "WiFi", BLUE, NULL);
  ui_addButton(10, CONTENT_Y + 70, WIDTH - 20, 40, "Language", GREEN, NULL);
  ui_addButton(10, CONTENT_Y + 120, WIDTH - 20, 40, "About", ORANGE, NULL);
  
  for (int i = 0; i < buttonCount; i++) {
    ui_drawButton(&buttons[i]);
  }
}

void screen_input_draw() {
  fillRect(0, CONTENT_Y, WIDTH, CONTENT_HEIGHT, WHITE);
  fillRect(20, CONTENT_Y + 20, WIDTH - 40, 40, LIGHTGRAY);
  
  // Simple keyboard layout
  const char* keys[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
  for (int i = 0; i < 10; i++) {
    int x = 10 + (i % 5) * 44;
    int y = CONTENT_Y + 80 + (i / 5) * 50;
    fillRect(x, y, 40, 45, BLUE);
    fillRect(x + 2, y + 2, 36, 41, WHITE);
  }
}

// ===================================
// UI ENGINE - SCREEN MANAGEMENT
// ===================================

void ui_setScreen(ScreenID screen) {
  if (screen == uiState.currentScreen) return;
  
  uiState.lastScreen = uiState.currentScreen;
  uiState.currentScreen = screen;
  uiState.needsFullRedraw = true;
  valueCount = 0;
}

void ui_drawScreen() {
  if (!uiState.needsFullRedraw) return;
  
  ui_drawHeader("Farm Monitor");
  ui_drawStatus();
  
  switch (uiState.currentScreen) {
    case SCREEN_HOME: screen_home_draw(); break;
    case SCREEN_FILES: screen_files_draw(); break;
    case SCREEN_AI: screen_ai_draw(); break;
    case SCREEN_SETTINGS: screen_settings_draw(); break;
    case SCREEN_INPUT: screen_input_draw(); break;
  }
  
  ui_drawFooter();
  uiState.needsFullRedraw = false;
}

// ===================================
// UI ENGINE - TOUCH ROUTING
// ===================================

void ui_handleNavbar(int16_t x, int16_t y) {
  int navIndex = x / 48;
  if (navIndex >= 0 && navIndex < 5) {
    ui_setScreen((ScreenID)navIndex);
  }
}

void ui_handleTouch(int16_t x, int16_t y) {
  // Debounce
  if (millis() - uiState.lastTouchTime < 200) return;
  uiState.lastTouchTime = millis();
  
  // Check navbar
  if (y >= NAVBAR_Y) {
    ui_handleNavbar(x, y);
    return;
  }
  
  // Check buttons
  for (int i = 0; i < buttonCount; i++) {
    if (ui_checkButton(&buttons[i], x, y)) {
      if (buttons[i].callback != NULL) {
        buttons[i].callback();
      }
      return;
    }
  }
  
  // Screen-specific touch handling would go here
}

// ===================================
// UI ENGINE - UPDATE LOOP
// ===================================

void ui_update() {
  // Handle touch input
  int16_t x, y;
  if (getTouch(&x, &y)) {
    ui_handleTouch(x, y);
  }
  
  // Update screen if needed
  ui_drawScreen();
  
  // Update dynamic values
  ui_redrawValues();
  
  // Hide alert after timeout
  ui_hideAlert();
}

// ===================================
// UI ENGINE - SYSTEM STATUS
// ===================================

void ui_setGSM(uint8_t signal) {
  if (uiState.gsmSignal != signal) {
    uiState.gsmSignal = signal;
    ui_drawStatus();
  }
}

void ui_setBattery(uint8_t level) {
  if (uiState.batteryLevel != level) {
    uiState.batteryLevel = level;
    ui_drawStatus();
  }
}

void ui_setGPS(bool locked) {
  if (uiState.gpsLock != locked) {
    uiState.gpsLock = locked;
    ui_drawStatus();
  }
}

// ===================================
// SETUP & LOOP
// ===================================

void setup() {
  SerialUSB.begin(115200);
  delay(2000);
  SerialUSB.println("\n=== UI Engine Starting ===");
  
  // Setup TFT
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_MOSI, OUTPUT);
  pinMode(TFT_SCK, OUTPUT);
  pinMode(TFT_MISO, INPUT);
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TFT_SCK, LOW);
  digitalWrite(TFT_LED, HIGH);
  
  // Setup Touch
  pinMode(T_CS, OUTPUT);
  pinMode(T_IRQ, INPUT);
  pinMode(T_DIN, OUTPUT);
  pinMode(T_DO, INPUT);
  pinMode(T_CLK, OUTPUT);
  digitalWrite(T_CS, HIGH);
  digitalWrite(T_CLK, LOW);
  
  SerialUSB.println("Initializing display...");
  initDisplay();
  SerialUSB.println("Display ready!");
  
  // Initialize UI
  ui_setScreen(SCREEN_HOME);
  ui_setBattery(75);
  ui_setGSM(80);
  ui_setGPS(true);
  
  SerialUSB.println("=== UI Engine Ready ===");
}

void loop() {
  // Main UI update loop
  ui_update();
  
  // Simulate sensor updates every 2 seconds
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    ui_updateValue(LABEL_MOISTURE, random(20, 80));
    ui_updateValue(LABEL_NITROGEN, random(30, 90));
    ui_updateValue(LABEL_PHOSPHORUS, random(25, 75));
    ui_updateValue(LABEL_POTASSIUM, random(35, 85));
  }
  
  delay(1);
}