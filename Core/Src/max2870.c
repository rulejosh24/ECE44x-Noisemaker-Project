#include "max2870.h" // MAX2870 PLL Synthesizer control for STM32

// Static internal register state (So that we can keep track of settings without reading back)
static uint32_t max2870_regs[6] = {
    0x00400000,  // REG0
    0x08008011,  // REG1
    0x00004E42,  // REG2
    0x000004B3,  // REG3
    0x0080003C,  // REG4
    0x00580005   // REG5
};

// DWT (Data Watchpoint and Trace unit) delay
static void DWT_Init(void)
{
    // Enable DWT and CYCCNT for microsecond delays
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    //DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us)
{
    // Calculate the number of cycles to wait based on the system clock
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

// Low-level write function (sends 4 bytes for one register)
static void MAX2870_Write(uint32_t reg)
{
    uint8_t data[4];
    // Convert 32-bit register value to 4 bytes (MSB first)
    data[0] = (reg >> 24) & 0xFF;
    data[1] = (reg >> 16) & 0xFF;
    data[2] = (reg >> 8) & 0xFF;
    data[3] = reg & 0xFF;

    HAL_GPIO_WritePin(MAX2870_LE_PORT, MAX2870_LE_PIN, GPIO_PIN_RESET); // LE low
    HAL_SPI_Transmit(&hspi2, data, 4, HAL_MAX_DELAY); // Send 4 bytes
    delay_us(1); // Short delay to ensure data is latched
    HAL_GPIO_WritePin(MAX2870_LE_PORT, MAX2870_LE_PIN, GPIO_PIN_SET); // LE high to latch
    delay_us(1); // Short delay after latching
}

// Write all registers (5 -> 0, REG0 last to trigger VCO load)
static void MAX2870_WriteAll(void)
{
    for (int i = 5; i >= 0; i--)
    {
        MAX2870_Write(max2870_regs[i]); // Write each register in order (REG5 to REG0)
    }
}

// RF divider helper
static uint64_t get_rf_div(uint64_t freq, uint8_t *sel)
{
    uint8_t div = 0;
    uint64_t vco = freq;

    while (vco < 3000000000ULL) // VCO must be between 3 GHz and 6 GHz, so we shift up until we reach that range (from chip hardware limits)
    {
        vco <<= 1;
        div++;
    }
    // MAX2870 supports dividers of 1, 2, 4, 8, 16, 32, 64 (div = 0 to 6)
    if (div > 6) div = 6;
    *sel = div;
    return (1ULL << div);
}

// Frequency setting
// MAX2870 PLL Synthesizer control for STM32 - Set frequency with noise tuning and jitter
// Formula (Fundamental output): fout = (fvco) / RF_DIV where fvco = (PFD_FREQ * (INT + FRAC/MOD))

void MAX2870_SetFrequency(uint64_t fout)
{
    // Calculate RF divider and VCO frequency
    uint8_t rf_div_sel;
    uint64_t rf_div = get_rf_div(fout, &rf_div_sel);
    // Calculate VCO frequency and PLL parameters
    uint64_t fvco = fout * rf_div;
    // Calculate INT and FRAC values
    uint64_t N = fvco / PFD_FREQ;
    uint64_t remainder = fvco % PFD_FREQ;
    // Calculate INT and FRAC values
    uint32_t INT = (uint32_t)N;
    uint32_t FRAC = (uint32_t)((remainder * MOD_VALUE) / PFD_FREQ);

    // Force noisy fractional mode
    if (FRAC == 0) FRAC = 1;

    // Add jitter
    FRAC += (HAL_GetTick() & 0x3);

    uint32_t MOD = MOD_VALUE;

    // Update register values based on calculated parameters
    max2870_regs[0] = (INT << 15) | (FRAC << 3) | 0;

    max2870_regs[1] &= ~(0xFFF << 3);
    max2870_regs[1] |= (MOD << 3);

    max2870_regs[4] &= ~(0x7 << 20);
    max2870_regs[4] |= (rf_div_sel << 20);

    MAX2870_WriteAll(); // Write all registers to apply changes
}

// Power
void MAX2870_SetPower(MAX2870_PowerLevel level)
{
    max2870_regs[4] &= ~(0x3 << 3); // Clear existing power bits
    max2870_regs[4] |= ((uint32_t)level << 3); // Set new power level

    MAX2870_WriteAll(); // Write all registers to apply changes
}

// Lock detect
uint8_t MAX2870_IsLocked(void)
{
    return (HAL_GPIO_ReadPin(MAX2870_LD_PORT, MAX2870_LD_PIN) == GPIO_PIN_SET); // LD high means locked
}

uint8_t MAX2870_WaitLock(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick(); // Wait until lock or timeout

    while ((HAL_GetTick() - start) < timeout_ms) // Check lock status every 10 ms
    {
        if (MAX2870_IsLocked())
            return 1;
    }
    return 0;
}

// Initialization
void MAX2870_Init(void)
{
    DWT_Init(); // Initialize DWT for microsecond delays
    MAX2870_WriteAll(); // Write initial register values
    HAL_Delay(10); // Wait for PLL to stabilize
}
