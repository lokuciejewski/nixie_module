all: clean build flash

TARGET:=main
ADDITIONAL_C_FILES=nixie.c

TARGET_MCU?=CH32V003

include ch32fun/ch32fun/ch32fun.mk

flash: cv_flash
clean: cv_clean
