#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

__attribute__((naked)) __attribute__((section(".start_section")))
void startup(void) {
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n"); /* call main */
    __asm__ volatile(".L1: B .L1\n"); /* never return */
}

volatile static unsigned char code[4] = {
    0x11,
    0x11,
    0x11,
    0x11
};
volatile unsigned char password_in[4];

void store_password() {
    for (int i = 0; i < 4; i++) {
        password_in[i] = keyboard();
    }
}
void check_password() {
    unsigned char size;
    for (int i = 0; i < 4; i++) {
        if (password_in[i] == code[i]) {
            size++;
        }
    }
    return (size == sizeof(code));
}

void app_init(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitTypeDef keypad_In;
    GPIO_InitTypeDef keypad_Out;

    GPIO_StructInit(&keypad_In); // COLUMN 0-3 pin as input
    GPIO_StructInit(&keypad_Out); //ROW 4-7 as output

    keypad_In.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    keypad_Out.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;

    keypad_In.GPIO_PuPd = GPIO_PuPd_DOWN; //PULLDOWN 

    keypad_In.GPIO_OType = GPIO_OType_PP; //push/pull
    keypad_Out.GPIO_OType = GPIO_OType_PP; //OPEN DRAIN

    keypad_In.GPIO_Mode = GPIO_Mode_IN;
    keypad_Out.GPIO_Mode = GPIO_Mode_OUT;

    GPIO_Init(GPIOE, &keypad_In);
    GPIO_Init(GPIOE, &keypad_Out);
}

void keyboardActivate(unsigned int row) {
    //Activate chosen row for keypad or deactivate all
    switch (row) {
    case 1:
        GPIO_SetBits(GPIOE, GPIO_Pin_4);
        break; //GPIO_SetBits(GPIOD,GPIO_Pin_4)
    case 2:
        GPIO_SetBits(GPIOE, GPIO_Pin_5);
        break; //GPIO_SetBits(GPIOD,GPIO_Pin_5)
    case 3:
        GPIO_SetBits(GPIOE, GPIO_Pin_6);
        break; //GPIO_SetBits(GPIOD,GPIO_Pin_6)
    case 4:
        GPIO_SetBits(GPIOE, GPIO_Pin_7);
        break; //GPIO_SetBits(GPIOD,GPIO_Pin_7)
    case 0:
        GPIO_Write(GPIOE, 0x0000);
        break;
    }
}
int keyboardGetColumn(void) {
    // If any key in the activated row is pressed
    // return its column number, otherwise 0
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)) {
        return 4;
    }
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)) {
        return 3;
    }
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1)) {
        return 2;
    }
    if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)) {
        return 1;
    }
    return 0;
}

void keyboard(void) {
    unsigned
    const char * key[] = {
        "1",
        "2",
        "3",
        "0xA",
        "4",
        "5",
        "6",
        "0xB",
        "7",
        "8",
        "9",
        "0xC",
        "0xE",
        "0",
        "0xF",
        "0xD"
    };
    int row, col;
    for (row = 1; row < 5; row++) {
        keyboardActivate(row);
        if (col = keyboardGetColumn()) {
            keyboardActivate(0);
            print(key[4 * (row - 1) + (col - 1)]);
            return (key[4 * (row - 1) + (col - 1)]);
        }
    }
    keyboardActivate(0);
    return 0xFF;
}

void main(void) {
    app_init();
    while (1) {
        keyboard();
    }
}