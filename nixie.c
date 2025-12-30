#include "nixie.h"
#include "assert.h"
#include "stdbool.h"

static NixieSegment current_segment = NIXIE_SEG_NO_SEG;
static bool comma_on = false;

void nixieInit(void) {
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

static void __refreshSegment(NixieSegment seg, uint16_t delay_ms) {
  bool current_state = funDigitalRead(seg);
  funDigitalWrite(seg, !current_state);
  Delay_Ms(delay_ms);
  funDigitalWrite(seg, current_state);
  Delay_Ms(delay_ms);
}

void nixieRefresh(void) {
  __refreshSegment(NIXIE_SEG_0, 2);
  __refreshSegment(NIXIE_SEG_1, 2);
  __refreshSegment(NIXIE_SEG_2, 2);
  __refreshSegment(NIXIE_SEG_3, 2);
  __refreshSegment(NIXIE_SEG_4, 2);
  __refreshSegment(NIXIE_SEG_5, 2);
  __refreshSegment(NIXIE_SEG_6, 2);
  __refreshSegment(NIXIE_SEG_7, 2);
  __refreshSegment(NIXIE_SEG_8, 2);
  __refreshSegment(NIXIE_SEG_9, 2);
  __refreshSegment(NIXIE_SEG_PL, 2);
}

void nixieTurnOn(NixieSegment seg) {
  assert(seg != NIXIE_SEG_PL);
  if (seg == NIXIE_SEG_NO_SEG) {
    funDigitalWrite(current_segment, FUN_LOW);
    current_segment = NIXIE_SEG_NO_SEG;
  } else if (seg != current_segment) {
    if (current_segment != NIXIE_SEG_NO_SEG)
      funDigitalWrite(current_segment, FUN_LOW);
    funDigitalWrite(seg, FUN_HIGH);
    current_segment = seg;
  }
}

void nixieTurnOff(void) {
  funDigitalWrite(current_segment, FUN_LOW);
  current_segment = NIXIE_SEG_NO_SEG;
  nixieCommaOff();
}

void nixieCommaOn(void) {
  if (!comma_on) {
    funDigitalWrite(NIXIE_SEG_PL, FUN_LOW);
    comma_on = true;
  }
}

void nixieCommaOff(void) {
  if (comma_on) {
    funDigitalWrite(NIXIE_SEG_PL, FUN_HIGH);
    comma_on = false;
  }
}

bool nixieIsCommaOn(void) { return comma_on; }

NixieSegment nixieGetCurrent(void) { return current_segment; }

uint8_t nixieToUINT8(NixieSegment seg) {
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

NixieSegment nixieFromUINT8(uint8_t seg) {
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
