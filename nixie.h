#pragma once
#include "ch32fun/ch32fun/ch32fun.h"
#include "stdbool.h"
#include "stdint.h"

typedef enum {
    NIXIE_SEG_0 = PC3,
    NIXIE_SEG_1 = PC4,
    NIXIE_SEG_2 = PC5,
    NIXIE_SEG_3 = PC6,
    NIXIE_SEG_4 = PC7,
    NIXIE_SEG_5 = PD0,
    NIXIE_SEG_6 = PD2,
    NIXIE_SEG_7 = PD3,
    NIXIE_SEG_8 = PD4,
    NIXIE_SEG_9 = PD5,
    NIXIE_SEG_PL = PC0,
    NIXIE_SEG_NO_SEG = 0,
} NixieSegment;

void Nixie_Init(void);
/// Should be called every 10ms for software PWM
void Nixie_PWM_RefreshEvery1ms(void);
void Nixie_DisplayRefresh(uint16_t delay_ms);

NixieSegment Nixie_GetCurrentSeg(void);
bool Nixie_IsCommaOn(void);
uint8_t Nixie_ToUINT8(NixieSegment seg);
NixieSegment Nixie_FromUINT8(uint8_t seg);

void Nixie_TurnOn(NixieSegment seg);
void Nixie_TurnOff(void);
void Nixie_CommaOn(void);
void Nixie_CommaOff(void);
