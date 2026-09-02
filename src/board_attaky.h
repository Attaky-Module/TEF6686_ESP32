#ifndef BOARD_ATTAKY_H
#define BOARD_ATTAKY_H

#include <Arduino.h>
#include <Wire.h>

// Pin and bus definitions for the Attaky ATK_COMBO_V1 board profile.
// Hardware deltas concentrate here (re-port spec E14); upstream files keep
// only minimal, documented #ifdef hooks.
//
// Pin assignments and bus parameters follow the Attaky hardware documentation
// for the Core and FM/AM Radio modules.

// Main I2C bus: keys (AW9523), touch (FT6636), sensors, fuel gauges.
// Core_ESP32_1.0: IO1 = SCL, IO2 = SDA, external pull-ups, 400 kHz.
#define ATK_MAIN_I2C_SDA          2
#define ATK_MAIN_I2C_SCL          1
#define ATK_MAIN_I2C_FREQ_HZ       400000UL

// Tuner bus: TEF6687 @ 0x64 on the FM-AM module (M-BTB44/45).
// 400 kHz — the clock the delivered 2.20-A001 build ran this bus on
// (radio_v2.2_github: Wire.begin(42, 41); Wire.setClock(400000)).
#define ATK_TUNER_I2C_SDA         42
#define ATK_TUNER_I2C_SCL         41
#define ATK_TUNER_I2C_FREQ_HZ     400000UL

// AW9523 keypad matrix @ 0x59 (Core_ESP32_1.0 SSOT port map).
#define ATK_AW9523_ADDR           0x59
#define ATK_AW9523_REG_INPUT      0x00
#define ATK_AW9523_REG_OUTPUT_P1  0x03
#define ATK_AW9523_REG_GCR        0x02
#define ATK_AW9523_REG_CMD        0x04
#define ATK_AW9523_REG_CONFIG_P1  0x05
#define ATK_AW9523_REG_LEDMODE    0x12

// Key bits in the AW9523 input register (LOW = pressed, Core SSOT).
#define ATK_KEY_DOWN               0
#define ATK_KEY_LEFT               1
#define ATK_KEY_SELECT             2
#define ATK_KEY_RIGHT              3
#define ATK_KEY_UP                 4
#define ATK_KEY_L1                 5
#define ATK_KEY_R1                 6
#define ATK_KEY_POWER              7

// Fuel gauges on the main bus (optional Power_Standard-Cell module).
// Dual-probe order per re-port spec E11: INA219 first, MAX17048 fallback.
#define ATK_INA219_ADDR            0x44
#define ATK_MAX17048_ADDR          0x36

// FT6636 capacitive touch controller @ 0x38 on the main bus.
// Register set and init sequence from the proven v2.20 port (spec D7).
#define ATK_FT6636_ADDR            0x38
#define ATK_FT6636_REG_STATUS      0x02
#define ATK_FT6636_REG_XH          0x03

// Dummy "always-HIGH" button pin. Upstream shared functions (ModeButtonPress,
// ButtonPress, KeyUp/KeyDown) contain `while (digitalRead(MODEBUTTON) == LOW)`
// style release-wait loops, and the scan-cancel loop reads the same macros.
// With the ATK defines at -1, digitalRead() reads the ESP32-S3 GPIO
// input-register bit 31 — the live octal-PSRAM bus line — whose level floats
// with memory traffic (verified 2026-08-26: the ModeButtonPress wait spun its
// full 1000 ms and the unbounded tail loop hung loop() forever). IO21
// (F-BTB37) was tried first and measured LOW with the FM-AM module stacked
// (2026-09-01 pull-up scan) — a phantom "button held" that latched boot
// combos and the BAND long-press STANDBY branch. IO47 (F-BTB36) measured
// HIGH on the same scan: not wired by the FM-AM module, not a strap pin, not
// used on the Core itself. Held HIGH by its internal pull-up so every
// release-wait loop falls straight through to the short-press branch.
#define ATK_DUMMY_HIGH_PIN         47

// Volume floor (spec D8): the TEF6687 audio chain is louder than upstream
// hardware, so the UI range widens from -10..+10 dB to -60..+10 dB.
#define ATK_VOL_MIN_DB            (-60)

// Battery segment thresholds for a 1S Li-ion pack read from the fuel gauge
// (spec E11). Millivolts, clamped into the upstream 4-segment icon.
#define ATK_BAT_EMPTY_MV          3450
#define ATK_BAT_FULL_MV           4150

void boardMainBusInit();
void boardMainBusWatchdog();
bool boardI2cHadError();
void boardI2cClearError();
void boardBacklightInit();   // FactoryTest recipe: one-time LEDC setup on IO14
void boardBacklightSet(uint8_t percent);  // ledcWrite only, no re-attach
void boardBacklightSleep();               // true duty-0 (intentional screen off)
void boardBacklightWake(uint8_t percent); // restore from sleep (duty floor applies)
void boardTunerBusInit();
void boardKeypadInit();
uint8_t boardKeypadRead();
bool boardKeyDown(uint8_t key);
bool boardFuelGaugePresent();
bool boardTouchInit();
bool boardTouchRead(uint16_t *x, uint16_t *y);
void boardTouchMapToScreen(uint16_t *x, uint16_t *y);

// E11 battery: runtime dual-probe fuel gauge (INA219 @0x44 first, MAX17048
// @0x36 fallback, sticky probe) ported from the proven WadaMesh v1.9 recipe.
// Returns 0 when no gauge answers (caller keeps batterydetect false).
uint16_t boardBatteryMilliVolts();

#endif
