#include "nixie.h"
#include "assert.h"
#include "stdbool.h"

#define PWM_MAX_COUNTER 10U

static NixieSegment current_segment = NIXIE_SEG_NO_SEG;
static NixieSegment saved_segment = NIXIE_SEG_NO_SEG;
static bool saved_comma = false;
static uint8_t pwm_counter = 0;
static uint8_t pwm_duty_cycle_percent_x10 = 5;
static bool comma_on = false;

void Nixie_Init(void) {
    funPinMode(NIXIE_SEG_0, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_1, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_5, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_6, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_7, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_8, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_9, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(NIXIE_SEG_PL, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
}

void Nixie_PWM_RefreshEvery1ms(void) {
    if (saved_segment != NIXIE_SEG_NO_SEG) {
        if (pwm_counter < pwm_duty_cycle_percent_x10) {
            Nixie_TurnOn(saved_segment);
            if (saved_comma) {
                Nixie_CommaOn();
            }
        } else {
            saved_comma = comma_on;
            saved_segment = current_segment;
            Nixie_TurnOff();
        }
        pwm_counter = (pwm_counter + 1) % PWM_MAX_COUNTER;
    }
}

static void refreshSegment(NixieSegment seg, uint16_t delay_ms) {
    bool current_state = funDigitalRead(seg);
    funDigitalWrite(seg, !current_state);
    Delay_Ms(delay_ms);
    funDigitalWrite(seg, current_state);
    Delay_Ms(delay_ms);
}

void Nixie_DisplayRefresh(uint16_t delay_ms) {
    refreshSegment(NIXIE_SEG_0, delay_ms);
    refreshSegment(NIXIE_SEG_1, delay_ms);
    refreshSegment(NIXIE_SEG_2, delay_ms);
    refreshSegment(NIXIE_SEG_3, delay_ms);
    refreshSegment(NIXIE_SEG_4, delay_ms);
    refreshSegment(NIXIE_SEG_5, delay_ms);
    refreshSegment(NIXIE_SEG_6, delay_ms);
    refreshSegment(NIXIE_SEG_7, delay_ms);
    refreshSegment(NIXIE_SEG_8, delay_ms);
    refreshSegment(NIXIE_SEG_9, delay_ms);
    refreshSegment(NIXIE_SEG_PL, delay_ms);
    if (current_segment != NIXIE_SEG_NO_SEG) {
        Nixie_TurnOn(current_segment);
    }
}

void Nixie_TurnOn(NixieSegment seg) {
    if (seg != current_segment) {
        if (seg == NIXIE_SEG_NO_SEG) {
            Nixie_TurnOff();
        } else {
            funDigitalWrite(current_segment, FUN_LOW);
            funDigitalWrite(seg, FUN_HIGH);
            current_segment = seg;
        }
    } else {
        funDigitalWrite(seg, FUN_HIGH);
    }
    saved_segment = current_segment;
}

void Nixie_TurnOff(void) {
    funDigitalWrite(current_segment, FUN_LOW);
    current_segment = NIXIE_SEG_NO_SEG;
    Nixie_CommaOff();
}

void Nixie_CommaOn(void) {
    if (!comma_on) {
        funDigitalWrite(NIXIE_SEG_PL, FUN_LOW);
        comma_on = true;
        saved_comma = true;
    }
}

void Nixie_CommaOff(void) {
    if (comma_on) {
        funDigitalWrite(NIXIE_SEG_PL, FUN_HIGH);
        comma_on = false;
        saved_comma = false;
    }
}

bool Nixie_IsCommaOn(void) {
    return comma_on;
}

NixieSegment Nixie_GetCurrentSeg(void) {
    return current_segment;
}

uint8_t Nixie_ToUINT8(NixieSegment seg) {
    switch (seg) {
    case NIXIE_SEG_0:
        return 0;
    case NIXIE_SEG_1:
        return 1;
    case NIXIE_SEG_2:
        return 2;
    case NIXIE_SEG_3:
        return 3;
    case NIXIE_SEG_4:
        return 4;
    case NIXIE_SEG_5:
        return 5;
    case NIXIE_SEG_6:
        return 6;
    case NIXIE_SEG_7:
        return 7;
    case NIXIE_SEG_8:
        return 8;
    case NIXIE_SEG_9:
        return 9;
    case NIXIE_SEG_PL:
        return 0xfe;
    case NIXIE_SEG_NO_SEG:
    default:
        return 0xff;
    }
}

NixieSegment Nixie_FromUINT8(uint8_t seg) {
    switch (seg) {
    case 0:
        return NIXIE_SEG_0;
    case 1:
        return NIXIE_SEG_1;
    case 2:
        return NIXIE_SEG_2;
    case 3:
        return NIXIE_SEG_3;
    case 4:
        return NIXIE_SEG_4;
    case 5:
        return NIXIE_SEG_5;
    case 6:
        return NIXIE_SEG_6;
    case 7:
        return NIXIE_SEG_7;
    case 8:
        return NIXIE_SEG_8;
    case 9:
        return NIXIE_SEG_9;
    case 0xfe:
        return NIXIE_SEG_PL;
    case 0xff:
    default:
        return NIXIE_SEG_NO_SEG;
    }
}
