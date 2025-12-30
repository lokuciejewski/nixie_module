#include "ch32fun/ch32fun/ch32fun.h"
#include "i2c_slave.h"
#include "nixie.h"
#include "stdio.h"

#define LED_PIN PA2

typedef enum : uint8_t {
    I2cReg_Address = 0,
    I2cReg_NixieValue,
    I2cReg_HvValueLowByte,
    I2cReg_HvValueHighByte,
    I2cReg_NixieDisplayRefresh,
    I2cReg_NixiePwmValue,
    I2cReg_NixieBrightnessCompensation,
} I2cRegisters_e;

#define I2C_DEFAULT_ADDRESS (uint8_t)(0x20)

#define SYSTEM_VOLTAGE_MV 3300U
#define HV_UPPER_LIMIT_MV 3300U
#define HV_LOWER_LIMIT_MV 2700U
#define HV_UPPER_LIMIT ((1000 * HV_UPPER_LIMIT_MV) / SYSTEM_VOLTAGE_MV)
#define HV_LOWER_LIMIT ((1000 * HV_LOWER_LIMIT_MV) / SYSTEM_VOLTAGE_MV)

#define DEFAULT_LOOP_TIME_US 450U

volatile uint8_t i2c_registers[4] = {0x00};
bool change_address = false;
uint8_t new_i2c_address = 0x00;
uint8_t* i2c_address = (uint8_t*)0x08003700;

bool unsafe_hv = true;

void onWrite(uint8_t reg, uint8_t length) {
    switch ((I2cRegisters_e)reg) {
    case I2cReg_Address:
        if (*i2c_address != i2c_registers[I2cReg_Address]) {
            change_address = true;
            new_i2c_address = i2c_registers[I2cReg_Address];
        }
        break;

    case I2cReg_NixieValue:
        if (!unsafe_hv) {
            if (i2c_registers[reg] != 0xff) {
                Nixie_TurnOn(Nixie_FromUINT8(i2c_registers[reg] & 0b1111));
                if ((i2c_registers[I2cReg_NixieValue] & 0b10000000) != 0) {
                    Nixie_CommaOn();
                } else {
                    Nixie_CommaOff();
                }
                break;
            } else {
                Nixie_TurnOff();
            }
            i2c_registers[I2cReg_NixieValue] = Nixie_GetCurrentSeg();
        }
        break;

    case I2cReg_NixieDisplayRefresh:
        if ((uint16_t)i2c_registers[reg] != 0) {
            Nixie_DisplayRefresh((uint16_t)i2c_registers[reg]);
        } else {
            Nixie_DisplayRefresh(2);
        }
        break;

    case I2cReg_NixiePwmValue:
        Nixie_PWM_SetDutyCycle(i2c_registers[reg]);
        break;
    case I2cReg_NixieBrightnessCompensation:
        break;

    default:
        break;
    }
}

void onRead(uint8_t reg) {
}

static void unlockFlash(void) {
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    while (FLASH->STATR & FLASH_STATR_BSY) {
    }
}

static void unlockProgramming(void) {
    FLASH->MODEKEYR = FLASH_KEY1;
    FLASH->MODEKEYR = FLASH_KEY2;
    while (FLASH->STATR & FLASH_STATR_BSY) {
    }
}

static void changeAddress(void) {
    unlockFlash();
    unlockProgramming();
    // Erase page
    FLASH->CTLR = CR_PAGE_ER;
    FLASH->ADDR = (intptr_t)i2c_address;
    FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
    // Page erase takes about 3ms
    while (FLASH->STATR & FLASH_STATR_BSY) {
    }

    if (*i2c_address != 0xff) {
        while (1) // Flash erase didn't work
        {
        }
    }
    // Clear buffer and prepare for flashing
    FLASH->CTLR = CR_PAGE_PG;
    FLASH->CTLR = CR_BUF_RST | CR_PAGE_PG;
    FLASH->ADDR = (intptr_t)i2c_address;
    while (FLASH->STATR & FLASH_STATR_BSY) // Not really needed
    {
    }
    // Write to the memory
    for (uint8_t i = 0; i < 16; i++) {
        ((uint32_t*)i2c_address)[i] = (uint32_t)new_i2c_address;
        FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
        while (FLASH->STATR &
               FLASH_STATR_BSY) // Only needed if running from RAM.
        {
        }
    }

    // Actually write the flash out. (Takes about 3ms)
    FLASH->CTLR = CR_PAGE_PG | CR_STRT_Set;

    while (FLASH->STATR & FLASH_STATR_BSY) {
    }

    if (*i2c_address != new_i2c_address) {
        while (1) // Flash write didn't work
        {
        }
    }
    // Perform software reset
    PFIC->SCTLR |= 0x80000000;
}

int main() {
    SystemInit();
    funGpioInitAll();
    funAnalogInit();

    // Initialise led
    funPinMode(LED_PIN, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    // Initialise adc - high voltage monitoring
    funPinMode(PA1, GPIO_CFGLR_IN_ANALOG);

    funDigitalWrite(LED_PIN, FUN_HIGH);
    Nixie_Init();
    Nixie_DisplayRefresh(2);

    // Initialize I2C slave
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL

    if (*i2c_address == 0xff) {
        // Address not set yet, use I2C_DEFAULT_ADDRESS
        SetupI2CSlave(I2C_DEFAULT_ADDRESS, i2c_registers, sizeof(i2c_registers),
                      onWrite, onRead, false);
        i2c_registers[I2cReg_Address] = I2C_DEFAULT_ADDRESS;
    } else {
        SetupI2CSlave(*i2c_address, i2c_registers, sizeof(i2c_registers),
                      onWrite, onRead, false);
        i2c_registers[I2cReg_Address] = *i2c_address;
    }

    i2c_registers[I2cReg_NixieValue] = 0xff; // Nixie off

    funDigitalWrite(LED_PIN, FUN_LOW);

    uint16_t hv_value = 0;

    while (1) {
        if (change_address) {
            funDigitalWrite(LED_PIN, FUN_HIGH);
            changeAddress();
        }
        hv_value = (uint16_t)funAnalogRead(ANALOG_1);
        i2c_registers[I2cReg_HvValueLowByte] = (uint8_t)(hv_value & 0xff);
        i2c_registers[I2cReg_HvValueHighByte] = (uint8_t)(hv_value >> 8);
        // unsafe_hv = hv_value > HV_UPPER_LIMIT;
        unsafe_hv =
            false; // TODO: replace 20k resistor with 15k to measure voltage
        if (unsafe_hv) {
            Nixie_CommaOff();
            Nixie_TurnOff();
            funDigitalWrite(LED_PIN, FUN_HIGH);
        } else {
            funDigitalWrite(LED_PIN, FUN_LOW);
            Nixie_PWM_RefreshEvery500us();
        }
        Delay_Us(DEFAULT_LOOP_TIME_US);
    }
}
