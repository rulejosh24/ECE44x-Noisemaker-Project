#ifndef MAX2870_H
#define MAX2870_H


// MAX2870 PLL Synthesizer Driver
// For STM32 microcontrollers using HAL library
#include "main.h"
#include <stdint.h>

// SPI handle (defined in main.c)
extern SPI_HandleTypeDef hspi2;

// Pins and ports for MAX2870 control
#define MAX2870_LE_PORT   GPIOA
#define MAX2870_LE_PIN    GPIO_PIN_4

#define MAX2870_LD_PORT   GPIOA
#define MAX2870_LD_PIN    GPIO_PIN_6

// Reference (25 MHz TCXO)
#define REF_FREQ   25000000ULL
#define PFD_FREQ   25000000ULL

// Noise tuning
#define MOD_VALUE  1000   // lower = more noise

typedef enum {
    MAX2870_PWR_NEG4 = 0,
    MAX2870_PWR_NEG1 = 1,
    MAX2870_PWR_POS2 = 2,
    MAX2870_PWR_POS5 = 3
} MAX2870_PowerLevel;

// Core functions
void MAX2870_Init(void); // Initialize SPI and MAX2870
void MAX2870_SetFrequency(uint64_t freq); // Set output frequency (fundamental mode)

// Features
void MAX2870_SetPower(MAX2870_PowerLevel level); // Set power level
uint8_t MAX2870_IsLocked(void); // Check if PLL is locked
uint8_t MAX2870_WaitLock(uint32_t timeout_ms); // Wait for PLL lock with timeout

#endif
