#include "nixie.h"
#include "assert.h"
#include "stdbool.h"

#define PWM_MAX_COUNTER 20U

static NixieSegment current_segment = NIXIE_SEG_NO_SEG;
static uint8_t pwm_counter = 0;
static uint32_t pwm_duty_cycle_setting =
    10; // x5, so 10 for 50% etc. 32-bit to ensure atomic operations
static bool comma_on = false;
static bool brigthness_compensation = true;

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

void Nixie_PWM_RefreshEvery500us(void) {
    if (current_segment != NIXIE_SEG_NO_SEG) {
        if (pwm_counter < pwm_duty_cycle_setting) {
            funDigitalWrite(current_segment, FUN_HIGH);
            if (comma_on) {
                funDigitalWrite(NIXIE_SEG_PL, FUN_HIGH);
            }
        } else {
            funDigitalWrite(current_segment, FUN_LOW);
            if (comma_on) {
                funDigitalWrite(NIXIE_SEG_PL, FUN_LOW);
            }
        }
        pwm_counter = (pwm_counter + 1) % PWM_MAX_COUNTER;
    }
}

void Nixie_PWM_SetDutyCycle(uint8_t new_duty_cycle) {
    if (new_duty_cycle <= PWM_MAX_COUNTER) {
        pwm_duty_cycle_setting = new_duty_cycle;
    }
}

static void refreshSegment(NixieSegment seg, uint16_t delay_ms) {
    funDigitalWrite(seg, true);
    Delay_Ms(delay_ms);
    funDigitalWrite(seg, false);
    Delay_Ms(delay_ms);
}

void Nixie_DisplayRefresh(uint16_t delay_ms) {
    funDigitalWrite(current_segment, FUN_LOW);
    if (comma_on) {
        funDigitalWrite(NIXIE_SEG_PL, FUN_LOW);
    }
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
        funDigitalWrite(current_segment, FUN_HIGH);
    }
    if (comma_on) {
        funDigitalWrite(NIXIE_SEG_PL, FUN_HIGH);
    }
}

void Nixie_EnableCommaBrightnessCompensation(void) {
    brigthness_compensation = true;
}

void Nixie_DisableCommaBrightnessCompensation(void) {
    brigthness_compensation = false;
}

static const NixieSegment SEGMENTS[] = {
    NIXIE_SEG_0, NIXIE_SEG_1, NIXIE_SEG_2, NIXIE_SEG_3, NIXIE_SEG_4,
    NIXIE_SEG_5, NIXIE_SEG_6, NIXIE_SEG_7, NIXIE_SEG_8, NIXIE_SEG_9};

inline static void Nixie_TurnOffOtherSegments(NixieSegment seg) {
    for (uint8_t i = 0; i < sizeof(SEGMENTS) / sizeof(NixieSegment); i++) {
        if (SEGMENTS[i] != seg) {
            funDigitalWrite(SEGMENTS[i], FUN_LOW);
        }
    }
}

void Nixie_TurnOn(NixieSegment seg) {
    if (seg == NIXIE_SEG_NO_SEG) {
        Nixie_TurnOff();
    } else {
        Nixie_TurnOffOtherSegments(seg);
        funDigitalWrite(seg, FUN_HIGH);
        current_segment = seg;
    }
}

void Nixie_TurnOff(void) {
    for (uint8_t i = 0; i < sizeof(SEGMENTS) / sizeof(NixieSegment); i++) {
        funDigitalWrite(SEGMENTS[i], FUN_LOW);
    }
    current_segment = NIXIE_SEG_NO_SEG;
    Nixie_CommaOff();
}

void Nixie_CommaOn(void) {
    if (!comma_on) {
        funDigitalWrite(NIXIE_SEG_PL, FUN_HIGH);
        comma_on = true;
        if (brigthness_compensation) {
            pwm_duty_cycle_setting =
                (pwm_duty_cycle_setting + 1) % PWM_MAX_COUNTER;
        }
    }
}

void Nixie_CommaOff(void) {
    if (comma_on) {
        funDigitalWrite(NIXIE_SEG_PL, FUN_LOW);
        comma_on = false;
        if (brigthness_compensation && pwm_duty_cycle_setting > 0) {
            pwm_duty_cycle_setting = (pwm_duty_cycle_setting - 1);
        }
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
