#pragma once

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>

// INPUT
#define CONFIRM_BTN 11

// OUTPUT
#define BUZZER1 12
#define BUZZER2 14
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
