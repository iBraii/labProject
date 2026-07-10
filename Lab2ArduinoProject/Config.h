#pragma once

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>

// INPUT
#define CONFIRM_BTN 11

// OUTPUT
// Separate buzzers: positive feedback and negative feedback.
#define BUZZER1 13
#define BUZZER2 7

// Set to 1 if your buzzer is an ACTIVE buzzer module that only needs HIGH/LOW.
// Keep 0 for a passive piezo buzzer that uses tone().
#define USE_ACTIVE_BUZZER 0

#define LED_RIGHT 9
#define LED_WRONG 10

// SCREEN
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// RFID
#define RST_PIN 5
#define SS_PIN  53

// GLOBAL OBJECTS
extern MFRC522 mfrc522;
extern U8G2_SSD1327_MIDAS_128X128_1_HW_I2C u8g2;
