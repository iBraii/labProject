#include "Config.h"
#include "DisplayUI.h"
#include "GameLogic.h"
#include "RFIDUtils.h"

// ── Sound helpers ─────────────────────────────────────────────────
// WIN  → ascending jingle on BUZZER1
// LOSE → descending tone  on BUZZER2

void playWinSound() {
  tone(BUZZER1, 523, 150); delay(160);   // C5
  tone(BUZZER1, 659, 150); delay(160);   // E5
  tone(BUZZER1, 784, 250); delay(260);   // G5
  noTone(BUZZER1);
}

void playLoseSound() {
  tone(BUZZER2, 392, 200); delay(210);   // G4
  tone(BUZZER2, 330, 200); delay(210);   // E4
  tone(BUZZER2, 262, 350); delay(360);   // C4
  noTone(BUZZER2);
}

// ── LED blink helper ──────────────────────────────────────────────

void blinkLED(int pin, int times, int ms) {
    digitalWrite(pin, HIGH); delay(ms);
    digitalWrite(pin, LOW);  //delay(ms);
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
  pinMode(BUZZER1,   OUTPUT);
  pinMode(BUZZER2,   OUTPUT);

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

  // Initial face: evil (robot is the enemy)
  // ── To change the starting face swap FACE_EVIL for another FaceExpression ──
  currentExpression = FACE_EVIL;
  currentEyes       = EYES_NORMAL;

  screenDirty = true;
  renderCurrentScreen();
  screenDirty = false;
}

// ── Main loop ─────────────────────────────────────────────────────

void loop() {

  // ── Game state machine ─────────────────────────────────────────

  switch (state) {

    case WAIT_OPERATION:
      waitOperation();
      break;

    // showOperation() shows the selected op name for 1.5 s, then
    // sets state = WAIT_INPUT automatically
    case SHOW_OPERATION:
      showOperation();
      break;

    case WAIT_INPUT:
      handleInput();
      break;

    case CHECK_RESULT:
      if (playerAnswer == correctAnswer) {
        currentExpression = FACE_WIN;   // defeated robot face
        state = WIN;
      } else {
        // Keep FACE_EVIL — robot is still taunting
        state = LOSE;
      }
      screenDirty = true;
      break;

    case WIN:
      // Screen renders "You Win!" + X eyes at end of this iteration
      // (screenDirty was set by CHECK_RESULT above).
      // Sound and LEDs play after the render.
      renderCurrentScreen();            // show win face now
      playWinSound();
      blinkLED(LED_RIGHT, 4, 3000);
      // Reset for a fresh round
      currentExpression = FACE_EVIL;
      currentEyes       = EYES_NORMAL;
      inputCount        = 0;
      state             = WAIT_OPERATION;
      screenDirty       = true;
      break;

    case LOSE:
      renderCurrentScreen();            // show "Try Again!" + evil face
      playLoseSound();
      blinkLED(LED_WRONG, 4, 3000);
      // Retry the same problem
      inputCount  = 0;
      state       = WAIT_INPUT;
      screenDirty = true;
      break;
  }

  // ── Blink animation ────────────────────────────────────────────
  // Comment out the next line if the RFID reader stops responding —
  // the millis() calls inside are harmless but the extra loop overhead
  // can occasionally race with SPI during card reads on some setups.
  updateAnimations();

  // ── Render only when something changed ────────────────────────
  if (screenDirty) {
    renderCurrentScreen();
    screenDirty = false;
  }
}
