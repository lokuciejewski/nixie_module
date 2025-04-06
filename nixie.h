#pragma once
#include "ch32fun/ch32fun/ch32fun.h"
#include "stdbool.h"
#include "stdint.h"

typedef enum
{
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

void nixieInit(void);
void nixieRefresh(void);

NixieSegment nixieGetCurrent(void);
bool nixieIsCommaOn(void);
uint8_t nixieToUINT8(NixieSegment seg);
NixieSegment nixieFromUINT8(uint8_t seg);

void nixieTurnOn(NixieSegment seg);
void nixieTurnOff(void);
void nixieCommaOn(void);
void nixieCommaOff(void);
