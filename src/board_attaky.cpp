#include "board_attaky.h"

// Backlight helpers, FactoryTest-authoritative recipe
// (FactoryTest/main/src/display_driver.cpp + include/config.h): one-time
// LEDC setup, then duty writes only. arduino analogWrite() re-runs the LEDC
// channel setup + pin re-attach on EVERY call; doing that mid-menu (during
// SPI redraw bursts) hangs/reboots the ATK board (seen on HW 2026-08-26).
// The old v2.20 port dodged this by leaving CONTRASTPIN at -1 (no
// brightness control at all). The whole file is ATK-only; every shared
// call site in the sketch is #ifdef-gated.
#ifdef ATK_COMBO_V1
#define ATK_BL_PWM_CHANNEL     0
#define ATK_BL_PWM_FREQ        5000
#define ATK_BL_PWM_PWM_BITS    8

void boardBacklightInit() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  // arduino-esp32 3.x dropped ledcSetup/ledcAttachPin
  ledcAttach(14, ATK_BL_PWM_FREQ, ATK_BL_PWM_PWM_BITS);
#else
  ledcSetup(ATK_BL_PWM_CHANNEL, ATK_BL_PWM_FREQ, ATK_BL_PWM_PWM_BITS);
  ledcAttachPin(14, ATK_BL_PWM_CHANNEL);  // TFT_BL (Core_ESP32_1.0 SSOT)
#endif
  boardBacklightSet(100);
}

void boardBacklightSet(uint8_t percent) {
  if (percent > 100) percent = 100;
  // Match the upstream map(15..255) floor: at 0-1 % brightness the panel
  // backlight goes so dim the screen reads as dead (a user-saved low value
  // then looked like a boot crash loop, HW 2026-08-26).
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(14, map(percent, 0, 100, 15, 255));
#else
  ledcWrite(ATK_BL_PWM_CHANNEL, map(percent, 0, 100, 15, 255));
#endif
}

// Spec E12 light standby: screen fully dark (duty 0) while audio keeps
// playing. Intentionally bypasses the 15/255 "screen looks dead" floor —
// callers only reach Sleep() through an explicit screen-off action.
void boardBacklightSleep() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(14, 0);
#else
  ledcWrite(ATK_BL_PWM_CHANNEL, 0);
#endif
}

void boardBacklightWake(uint8_t percent) {
  boardBacklightSet(percent);
}
// Main I2C bus: keys, touch, sensors, fuel gauges (Core_ESP32_1.0).
static bool g_i2cError = false;
bool boardI2cHadError() { return g_i2cError; }
void boardI2cClearError() { g_i2cError = false; }
void boardI2cFlagError() { g_i2cError = true; }

// Bus watchdog. The FT6636 occasionally glitches the main I2C bus during
// heavy SPI menu drawing; without a timeout Wire transactions can block the
// UI loop (screen stuck, no reset — seen on HW 2026-08-26). Wire.setTimeOut
// bounds each transaction; after an error the next loop pass re-initialises
// the bus (the old v2.20 port brute-forced the same flakiness by re-running
// Wire1.begin() before every touch read).
void boardMainBusWatchdog() {
  if (boardI2cHadError()) {
    Wire.begin(ATK_MAIN_I2C_SDA, ATK_MAIN_I2C_SCL, ATK_MAIN_I2C_FREQ_HZ);
    Wire.setTimeOut(10);
    boardI2cClearError();
  }
}

void boardMainBusInit() {
  Wire.begin(ATK_MAIN_I2C_SDA, ATK_MAIN_I2C_SCL, ATK_MAIN_I2C_FREQ_HZ);
  Wire.setTimeOut(10);  // ms; never block the UI loop on a hung bus
}

// Tuner I2C bus: TEF6687 @ 0x64, 400 kHz (the clock the delivered 2.20-A001
// build ran on the same wiring). Lazy-init discipline: called on first tuner
// use, not at boot.
void boardTunerBusInit() {
  static bool started = false;
  if (started) return;
  started = true;
  Wire1.begin(ATK_TUNER_I2C_SDA, ATK_TUNER_I2C_SCL, ATK_TUNER_I2C_FREQ_HZ);
  Wire1.setTimeOut(10);  // same no-block rule as the main bus
}

// AW9523 keypad setup. LEDMODE (0x12) and CMD (0x04) both 0xFF: every P0 pin
// as a GPIO input. The proven v2.20 port wrote 0x7f here, which pushed P07
// (POWER_BTN) into LED constant-current mode driven as an output — the key
// could never be read (button-map audit, 2026-08-24). All-POR-default 0xFF
// keeps P07 readable; bench-check: P07 reads LOW on press, power path
// unaffected.
// Port 1: P13 is CTP_RESET for the FT6636 (LOW = held in reset) and must be
// driven HIGH or the touch controller never leaves reset (FactoryTest does
// the same). P10-P12 are the RGB LEDs (active low) — output-high keeps them
// off. P14-P17 float; left as inputs with interrupts unused.
void boardKeypadInit() {
  Wire.beginTransmission(ATK_AW9523_ADDR);
  Wire.write(ATK_AW9523_REG_LEDMODE);
  Wire.write(0xff);
  Wire.endTransmission();
  Wire.beginTransmission(ATK_AW9523_ADDR);
  Wire.write(ATK_AW9523_REG_CMD);
  Wire.write(0xff);
  Wire.endTransmission();

  // CONFIG_P1 (0x05): P10/P11/P12/P13 as outputs, P14-P17 inputs.
  Wire.beginTransmission(ATK_AW9523_ADDR);
  Wire.write(ATK_AW9523_REG_CONFIG_P1);
  Wire.write(0xF0);
  Wire.endTransmission();
  // OUTPUT_P1 (0x03): all high — LEDs off (active low), CTP out of reset.
  Wire.beginTransmission(ATK_AW9523_ADDR);
  Wire.write(ATK_AW9523_REG_OUTPUT_P1);
  Wire.write(0xff);
  Wire.endTransmission();
}

// One transaction reads the whole key port; bit is LOW when pressed. No
// settle delay: FactoryTest reads AW9523 registers back-to-back on the same
// 400 kHz bus without one; the v2.20 port's delay(20) only capped loop rate.
uint8_t boardKeypadRead() {
  uint8_t val = 0xFF;
  Wire.beginTransmission(ATK_AW9523_ADDR);
  Wire.write(ATK_AW9523_REG_INPUT);
  if (Wire.endTransmission() != 0) { boardI2cFlagError(); return 0xFF; }
  if (Wire.requestFrom(ATK_AW9523_ADDR, (int)1) != 1) { boardI2cFlagError(); return 0xFF; }
  while (Wire.available()) {
    val = Wire.read();
  }
  return val;
}

bool boardKeyDown(uint8_t key) {
  return !bitRead(boardKeypadRead(), key);
}

// Fuel-gauge presence probe (dual-probe battery, spec E11; the WadaMesh v1.9
// port with INA219-first / MAX17048-fallback sticky probing lands in its own
// bring-up step). No Power module on the bus -> both probes fail -> hidden.
bool boardFuelGaugePresent() {
  Wire.beginTransmission(ATK_INA219_ADDR);
  if (Wire.endTransmission() == 0) return true;
  Wire.beginTransmission(ATK_MAX17048_ADDR);
  return Wire.endTransmission() == 0;
}

// E11 dual-probe read, WadaMesh v1.9 recipe (with its two inherited bugfixes:
// keep last-good on I2C failure instead of propagating garbage, and INA219
// bus voltage needs no >>8 shift here because the driver returns volts
// directly). Sticky probe order: INA219 (Power 1.9) first, MAX17048
// (Power 1.0) fallback. 2 s cache keeps the UI poll cheap.
uint16_t boardBatteryMilliVolts() {
  static bool probed = false;
  static bool ina219Present = false;
  static uint16_t cacheMv = 0;
  static uint32_t lastMs = 0;
  static uint16_t lastGoodMv = 0;

  uint32_t now = millis();
  if (cacheMv != 0 && now - lastMs < 2000) return cacheMv;

  if (!probed) {
    Wire.beginTransmission(ATK_INA219_ADDR);
    ina219Present = (Wire.endTransmission() == 0);
    probed = true;
  }

  bool ok = false;
  uint16_t mv = 0;
  if (ina219Present) {
    // INA219 bus-voltage register 0x02: 16-bit value, 4 uV/LSB, bit 0 = CNVR.
    Wire.beginTransmission(ATK_INA219_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom(ATK_INA219_ADDR, 2) == 2) {
      uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();
      raw >>= 3;                                 // drop CNVR + overflow bits -> 4 uV LSB
      mv = (uint16_t)(((uint32_t)raw * 4) / 1000);
      ok = true;
    }
  } else {
    // MAX17048 VCELL reg 0x02, 78.125 uV/LSB -> raw * 5 / 64 mV.
    Wire.beginTransmission(ATK_MAX17048_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom(ATK_MAX17048_ADDR, 2) == 2) {
      uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();
      mv = (uint16_t)(((uint32_t)raw * 5) / 64);
      ok = true;
    }
  }

  if (ok && mv >= 1000 && mv <= 5500) {          // plausible pack range only
    cacheMv = mv;
    lastGoodMv = mv;
    lastMs = now;
  }
  // On failure keep the last good value for the cache window; past that,
  // report it still (better a stale mV than a flickering icon).
  return cacheMv ? cacheMv : lastGoodMv;
}

// FT6636 init, mined from the proven v2.20 port (spec D7): power/scheme
// registers 0x88/0x89, gesture auto-switch off (0xA4). Main bus only.
bool boardTouchInit() {
  const uint8_t init[][2] = {
    {0x88, 0x50},
    {0x89, 0x50},
    {0xA4, 0x00},
  };
  for (auto &reg : init) {
    Wire.beginTransmission(ATK_FT6636_ADDR);
    Wire.write(reg[0]);
    Wire.write(reg[1]);
    if (Wire.endTransmission() != 0) return false;
    delay(50);
  }
  return true;
}

// Polling read. Returns false when no finger is down. Raw panel coordinates
// are already calibrated to the panel; rotation mapping is applied by the
// caller (the .ino owns rotation state).
bool boardTouchRead(uint16_t *x, uint16_t *y) {
  uint8_t status;
  Wire.beginTransmission(ATK_FT6636_ADDR);
  Wire.write(ATK_FT6636_REG_STATUS);
  if (Wire.endTransmission() != 0) { boardI2cFlagError(); return false; }
  if (Wire.requestFrom(ATK_FT6636_ADDR, 1) != 1) { boardI2cFlagError(); return false; }
  status = Wire.read();
  if (status < 1 || status > 5) return false;

  uint8_t pt[4];
  Wire.beginTransmission(ATK_FT6636_ADDR);
  Wire.write(ATK_FT6636_REG_XH);
  if (Wire.endTransmission() != 0) { boardI2cFlagError(); return false; }
  if (Wire.requestFrom(ATK_FT6636_ADDR, 4) != 4) { boardI2cFlagError(); return false; }
  for (auto &b : pt) b = Wire.read();
  if ((pt[0] & 0x40) != 0 || (pt[2] & 0xf0) == 0xf0) return false;  // released
  *x = ((pt[0] & 0x0f) << 8) | pt[1];
  *y = ((pt[2] & 0x0f) << 8) | pt[3];
  return true;
}

// Rotate the panel's native-portrait report into the tuner's landscape
// screen coordinates: screenX = 320 - rawY, screenY = rawX. Same mapping the
// shipped 2.20-A001 build verified in its vendored TFT_eSPI Touch.cpp
// (FT6X36 convertRawXY branch); re-verified by the panel-edge tap test that
// produced the screenX ~28-39 left-edge measurement in src/touch.cpp.
void boardTouchMapToScreen(uint16_t *x, uint16_t *y) {
  uint16_t rawX = *x, rawY = *y;
  uint16_t sx = (uint16_t)320 - rawY;
  uint16_t sy = rawX;
  if (sx > 319) sx = 0;
  if (sy > 239) sy = 239;
  *x = sx;
  *y = sy;
}

// See board_attaky.h. Superseded 2026-08-27 by the mirror-free
// settings_store (src/settings_store.*) — a begin()-reload proved
// insufficient: the heap mirror is stomped even inside the
// reload->write->commit window.
#endif  // ATK_COMBO_V1
