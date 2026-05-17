#include "encoder.h"

static uint64_t           current_freq  = FREQ_MIN;
static MAX2870_PowerLevel current_power = MAX2870_PWR_NEG4;
static uint8_t            last_a        = 1;
static uint8_t            last_sw       = 1;
static uint32_t           last_press_ms = 0;

void ENCODER_Init(void)
{
    last_a        = (HAL_GPIO_ReadPin(ENCODER_A_PORT,  ENCODER_A_PIN)  == GPIO_PIN_SET) ? 1 : 0;
    last_sw       = (HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN) == GPIO_PIN_SET) ? 1 : 0;
    current_freq  = FREQ_MIN;
    current_power = MAX2870_PWR_NEG4;
}

void ENCODER_Update(void)
{
    uint8_t a  = (HAL_GPIO_ReadPin(ENCODER_A_PORT,  ENCODER_A_PIN)  == GPIO_PIN_SET) ? 1 : 0;
    uint8_t b  = (HAL_GPIO_ReadPin(ENCODER_B_PORT,  ENCODER_B_PIN)  == GPIO_PIN_SET) ? 1 : 0;
    uint8_t sw = (HAL_GPIO_ReadPin(ENCODER_SW_PORT, ENCODER_SW_PIN) == GPIO_PIN_SET) ? 1 : 0;

    // --- Rotation ---
    if (last_a == 1 && a == 0)
    {
        if (b == 1)
        {
            // Rotating right (CW) - increment
            if (current_freq < FREQ_MAX)
                current_freq += FREQ_STEP;
            else
                current_freq = FREQ_MAX;
        }
        else
        {
            // Rotating left (CCW) - decrement
            if (current_freq > FREQ_MIN)
                current_freq -= FREQ_STEP;
            else
                current_freq = FREQ_MIN;
        }
    }

    // --- Button press (falling edge with debounce) ---
    if (last_sw == 1 && sw == 0)
    {
        uint32_t now = HAL_GetTick();

        if ((now - last_press_ms) > DEBOUNCE_MS)
        {
            // Cycle through power levels: -4 -> -1 -> +2 -> +5 -> -4 ...
            current_power = (MAX2870_PowerLevel)((current_power + 1) % 4);
            last_press_ms = now;
        }
    }

    last_a  = a;
    last_sw = sw;
}

uint64_t ENCODER_GetFrequency(void)
{
    return current_freq;
}

MAX2870_PowerLevel ENCODER_GetPower(void)
{
    return current_power;
}
