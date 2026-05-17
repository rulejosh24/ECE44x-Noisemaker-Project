#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"
#include "max2870.h"
#include <stdint.h>

// ==== USER CONFIG ====

#define ENCODER_A_PORT    GPIOB
#define ENCODER_A_PIN     GPIO_PIN_0

#define ENCODER_B_PORT    GPIOB
#define ENCODER_B_PIN     GPIO_PIN_1

#define ENCODER_SW_PORT   GPIOB
#define ENCODER_SW_PIN    GPIO_PIN_2

#define FREQ_MIN          2400000000ULL   // 2.4 GHz
#define FREQ_MAX          2500000000ULL   // 2.5 GHz
#define FREQ_STEP         1000000ULL      // 1 MHz per detent

#define DEBOUNCE_MS       50              // button debounce time in ms

// =====================

void              ENCODER_Init(void);
void              ENCODER_Update(void);
uint64_t          ENCODER_GetFrequency(void);
MAX2870_PowerLevel ENCODER_GetPower(void);

#endif
