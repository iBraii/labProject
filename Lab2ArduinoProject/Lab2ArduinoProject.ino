#include "Config.h"
#include "DisplayUI.h"
#include "GameLogic.h"
#include "RFIDUtils.h"

// ── Sound helpers ─────────────────────────────────────────────────
// BUZZER1 (pin 13): start, correct answer, boss defeated.
// BUZZER2 (pin 7): wrong answer, game over.
// If the buzzers are active, set USE_ACTIVE_BUZZER to 1 in Config.h.

void beep(int buzzerPin, int frequency, int ms) {
#if USE_ACTIVE_BUZZER
  digitalWrite(buzzerPin, HIGH);
  delay(ms);
  digitalWrite(buzzerPin, LOW);
  delay(25);
#else
  tone(buzzerPin, frequency, ms);
  delay(ms + 25);
  noTone(buzzerPin);
#endif
}

void playStartSound() {
  beep(BUZZER1, 880, 80);
  beep(BUZZER1, 1175, 80);
}

void playCorrectSound() {
  beep(BUZZER1, 784, 70);
  beep(BUZZER1, 1047, 90);
}

void playWrongSound() {
  beep(BUZZER2, 220, 120);
  beep(BUZZER2, 165, 160);
}

void playBossDefeatedSound() {
  beep(BUZZER1, 523, 100);
  beep(BUZZER1, 659, 100);
  beep(BUZZER1, 784, 120);
  beep(BUZZER1, 1047, 180);
}

void playGameOverSound() {
  beep(BUZZER2, 392, 150);
  beep(BUZZER2, 330, 150);
  beep(BUZZER2, 262, 250);
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
  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

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

  // The boss only blinks while waiting for its operation card.
  updateAnimations();

  if (screenDirty) {
    renderCurrentScreen();
    screenDirty = false;
  }
}
