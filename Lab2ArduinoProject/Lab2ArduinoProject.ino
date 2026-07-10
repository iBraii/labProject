#include "Config.h"
#include "DisplayUI.h"
#include "GameLogic.h"
#include "RFIDUtils.h"

// ── Sound helpers ─────────────────────────────────────────────────
// If the buzzer is passive, this uses tone().
// If the buzzer is active, set USE_ACTIVE_BUZZER to 1 in Config.h.

void beep(int frequency, int ms) {
#if USE_ACTIVE_BUZZER
  digitalWrite(BUZZER_PIN, HIGH);
  delay(ms);
  digitalWrite(BUZZER_PIN, LOW);
  delay(25);
#else
  tone(BUZZER_PIN, frequency, ms);
  delay(ms + 25);
  noTone(BUZZER_PIN);
#endif
}

void playStartSound() {
  beep(880, 80);
  beep(1175, 80);
}

void playCorrectSound() {
  beep(784, 70);
  beep(1047, 90);
}

void playWrongSound() {
  beep(220, 120);
  beep(165, 160);
}

void playBossDefeatedSound() {
  beep(523, 100);
  beep(659, 100);
  beep(784, 120);
  beep(1047, 180);
}

void playGameOverSound() {
  beep(392, 150);
  beep(330, 150);
  beep(262, 250);
}

// ── LED blink helper ──────────────────────────────────────────────
void blinkLED(int pin, int times, int ms) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(ms);
    digitalWrite(pin, LOW);
    delay(ms);
  }
}

// ── Setup ─────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  // I2C + SPI
  Wire.begin();
  SPI.begin();
  pinMode(SS_PIN, OUTPUT);   // required for Mega SPI master mode

  // Inputs
  pinMode(CONFIRM_BTN, INPUT_PULLUP);

  // Outputs
  pinMode(LED_RIGHT, OUTPUT);
  pinMode(LED_WRONG, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // RFID
  mfrc522.PCD_Init();
  // mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  Serial.println("RFID READY");

  // OLED
  if (!u8g2.begin()) {
    Serial.println("OLED init failed");
    while (true);
  }
  Serial.println("OLED READY");
  u8g2.setContrast(60);

  // Random seed from floating analog pin
  randomSeed(analogRead(A0));

  setupGame();
  renderCurrentScreen();
  screenDirty = false;

  playStartSound();
}

// ── Main loop ─────────────────────────────────────────────────────
void loop() {
  switch (state) {
    case WAIT_OPERATION:
      waitOperation();
      break;

    case SHOW_OPERATION:
      showOperation();
      break;

    case WAIT_INPUT:
      handleInput();
      break;

    case CHECK_RESULT:
      resolveAnswer();
      break;

    case WIN:
      renderCurrentScreen();
      playCorrectSound();
      blinkLED(LED_RIGHT, 2, 90);
      prepareNextRound();
      break;

    case LOSE:
      renderCurrentScreen();
      playWrongSound();
      blinkLED(LED_WRONG, 2, 120);
      prepareNextRound();
      break;

    case BOSS_DEFEATED:
      renderCurrentScreen();
      playBossDefeatedSound();
      blinkLED(LED_RIGHT, 5, 80);
      delay(500);
      continueAfterBossDefeated();
      break;

    case GAME_OVER:
      renderCurrentScreen();
      playGameOverSound();
      blinkLED(LED_WRONG, 5, 120);
      delay(500);
      resetGame();
      break;
  }

  // Only blink during playable states so feedback screens are not overwritten.
  if (state == WAIT_OPERATION || state == SHOW_OPERATION || state == WAIT_INPUT) {
    updateAnimations();
  }

  if (screenDirty) {
    renderCurrentScreen();
    screenDirty = false;
  }
}
