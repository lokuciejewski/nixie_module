#include "ch32fun/ch32fun/ch32fun.h"
#include "nixie.h"
#include "stdio.h"
#include "i2c_slave.h"
// #include "adc.h"

#define I2C_ADDRESS_REGISTER 0
#define I2C_NIXIE_VALUE_REGISTER 1
#define I2C_HV_VALUE_REGISTER_LB 2
#define I2C_HV_VALUE_REGISTER_HB 3

#define I2C_DEFAULT_ADDRESS (uint8_t)(0x20)

#define SYSTEM_VOLTAGE_MV 3300
#define HV_UPPER_LIMIT_MV 3300
#define HV_LOWER_LIMIT_MV 2700
#define HV_UPPER_LIMIT ((1000 * HV_UPPER_LIMIT_MV) / SYSTEM_VOLTAGE_MV)
#define HV_LOWER_LIMIT ((1000 * HV_LOWER_LIMIT_MV) / SYSTEM_VOLTAGE_MV)

volatile uint8_t i2c_registers[4] = {0x00};
bool change_address = false;
uint8_t new_i2c_address = 0x00;
uint8_t *i2c_address = (uint8_t *)0x08003700;

bool unsafe_hv = true;

void onWrite(uint8_t reg, uint8_t length)
{
	switch (reg)
	{
	case I2C_ADDRESS_REGISTER:
		if (*i2c_address != i2c_registers[I2C_ADDRESS_REGISTER])
		{
			change_address = true;
			new_i2c_address = i2c_registers[I2C_ADDRESS_REGISTER];
		}
		break;

	case I2C_NIXIE_VALUE_REGISTER:
		if (!unsafe_hv)
		{
			if (i2c_registers[reg] != 0xff)
			{
				nixieTurnOn(nixieFromUINT8(i2c_registers[reg] & 0b1111));
				if (i2c_registers[I2C_NIXIE_VALUE_REGISTER] >> 7 == 0b1)
				{
					nixieCommaOn();
				}
				else
				{
					nixieCommaOff();
				}
				break;
			}
			else
			{
				nixieTurnOff();
			}
			i2c_registers[I2C_NIXIE_VALUE_REGISTER] = nixieGetCurrent();
		}
		break;
	}
}

void onRead(uint8_t reg)
{
	switch (reg)
	{
	case I2C_ADDRESS_REGISTER:
		// i2c_registers[reg] = *i2c_address;
		break;
	case I2C_NIXIE_VALUE_REGISTER:
		// i2c_registers[reg] = nixieToUINT8(nixieGetCurrent());
		break;
	}
}

void unlockFlash(void)
{
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	while (FLASH->STATR & FLASH_STATR_BSY)
	{
	}
}

void unlockProgramming(void)
{
	FLASH->MODEKEYR = FLASH_KEY1;
	FLASH->MODEKEYR = FLASH_KEY2;
	while (FLASH->STATR & FLASH_STATR_BSY)
	{
	}
}

int main()
{
	SystemInit();
	funGpioInitAll();
	funAnalogInit();

	// Initialise led
	funPinMode(PA2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
	// Initialise adc - high voltage monitoring
	funPinMode(PA1, GPIO_CFGLR_IN_ANALOG);

	funDigitalWrite(PA2, FUN_HIGH);
	nixieInit();
	nixieRefresh();

	uint16_t blink_delay_ms = 0;
	// Initialize I2C slave
	funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
	funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL

	if (*i2c_address == 0xff)
	{
		// Address not set yet, use I2C_DEFAULT_ADDRESS
		SetupI2CSlave(I2C_DEFAULT_ADDRESS, i2c_registers, sizeof(i2c_registers), onWrite, onRead, false);
		i2c_registers[I2C_ADDRESS_REGISTER] = I2C_DEFAULT_ADDRESS;
		blink_delay_ms = 100;
	}
	else
	{
		SetupI2CSlave(*i2c_address, i2c_registers, sizeof(i2c_registers), onWrite, onRead, false);
		i2c_registers[I2C_ADDRESS_REGISTER] = *i2c_address;
		blink_delay_ms = 250;
	}

	i2c_registers[I2C_NIXIE_VALUE_REGISTER] = 0xff; // Nixie off

	funDigitalWrite(PA2, FUN_LOW);

	uint16_t hv_value = 0;

	while (1)
	{
		// if (i2c_registers[I2C_ADDRESS_REGISTER] != I2C_DEFAULT_ADDRESS)
		// {
			// __WFI();
		// }
		if (change_address)
		{
			funDigitalWrite(PA2, FUN_HIGH);
			unlockFlash();
			unlockProgramming();
			// Erase page
			FLASH->CTLR = CR_PAGE_ER;
			FLASH->ADDR = (intptr_t)i2c_address;
			FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
			// Page erase takes about 3ms
			while (FLASH->STATR & FLASH_STATR_BSY)
			{
			}

			if (*i2c_address != 0xff)
			{
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
			for (uint8_t i = 0; i < 16; i++)
			{
				((uint32_t *)i2c_address)[i] = (uint32_t)new_i2c_address;
				FLASH->CTLR = CR_PAGE_PG | FLASH_CTLR_BUF_LOAD; // Load the buffer.
				while (FLASH->STATR & FLASH_STATR_BSY)			// Only needed if running from RAM.
				{
				}
			}

			// Actually write the flash out. (Takes about 3ms)
			FLASH->CTLR = CR_PAGE_PG | CR_STRT_Set;

			while (FLASH->STATR & FLASH_STATR_BSY)
			{
			}

			if (*i2c_address != new_i2c_address)
			{
				while (1) // Flash write didn't work
				{
				}
			}
			// Perform software reset
			PFIC->SCTLR |= 0x80000000;
		}

		// TODO: add systick to make sure the unsafe_hv is being updated every X ms
		hv_value = (uint16_t)funAnalogRead(ANALOG_1);
		i2c_registers[I2C_HV_VALUE_REGISTER_LB] = (uint8_t)(hv_value & 0xff);
		i2c_registers[I2C_HV_VALUE_REGISTER_HB] = (uint8_t)(hv_value >> 8);
		// unsafe_hv = hv_value > HV_UPPER_LIMIT;
		unsafe_hv = false; // TODO: replace 20k resistor with 15k to measure voltage
		if (unsafe_hv)
		{
			nixieCommaOff();
			nixieTurnOff();
			funDigitalWrite(PA2, FUN_HIGH);
			Delay_Ms(blink_delay_ms);
		}
		else
		{
			// funDigitalWrite(PA2, FUN_HIGH);
			// Delay_Ms(blink_delay_ms);
			funDigitalWrite(PA2, FUN_LOW);
			// Delay_Ms(blink_delay_ms);
		}
	}
}
