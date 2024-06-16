/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include <stdio.h>
#include <stdlib.h>
#include <stm32f10x.h>
#define time 300000

#define ST_IDLE         (uint16_t)1
#define ST_OPEN         (uint16_t)3

#define BLINK_COUNT     (uint16_t) 50     // 500 ms LED ON and 500 ms LED OFF
#define OPEN_COUNT      (uint16_t)500     // 5 sec OPEN state 
//#define CYCLE_COUNT     (uint16_t)10     // number of timer cycles for program cycle = 10 ms
#define BTN_DEB_COUNT   (uint16_t)3      // 30 ms button debouncer delay

#define BTN_PRESSED     (uint16_t)1
#define BTN_RELEASED    (uint16_t)0

#define LEDS_PORT   GPIOC
#define LED1_ON     GPIOC->BSRR |= GPIO_BSRR_BR13 // LEDS turn-ON by ZERO
#define LED2_ON     GPIOC->BSRR |= GPIO_BSRR_BR14 // LEDS turn-ON by ZERO

#define LED1_OFF     GPIOC->BSRR |= GPIO_BSRR_BS13 // LEDS turn-OFF by ONE
#define LED2_OFF     GPIOC->BSRR |= GPIO_BSRR_BS14 // LEDS turn-OFF by ONE


#define BTN_PORT    GPIOB
#define BTN_PIN     1
#define BTN_IDR_PIN GPIO_IDR_IDR1


#define LOCK_PORT   GPIOB
#define LOCK_PIN    0
#define LOCK_OPEN   LOCK_PORT->BSRR |= GPIO_BSRR_BS0
#define LOCK_CLOSE  LOCK_PORT->BSRR |= GPIO_BSRR_BR0


/****  BLE-homephone LED BLINKER PINS PC13 PC14 ********************************************************
*
*       main()
*
*  Function description
*   Application entry point.

мигает светодиодами на PC13 и PC14 по таймету TIM1


*/



void GPIO_Init(void){
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // enable GPIOC

    // LEDS pins configurtion
    LEDS_PORT->CRH &= ~(GPIO_CRH_CNF13);
    LEDS_PORT->CRH |= GPIO_CRH_MODE13_0;    
    LEDS_PORT->BSRR |= GPIO_BSRR_BS13;  
    
    LEDS_PORT->CRH &= ~(GPIO_CRH_CNF14);
    LEDS_PORT->CRH |= GPIO_CRH_MODE14_0;    
    LEDS_PORT->BSRR |= GPIO_BSRR_BS14;

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // enable GPIOB - BTN_PORT enable
    LOCK_PORT->CRL &= ~(GPIO_CRL_CNF0); // output push-pull
    LOCK_PORT->CRL |= GPIO_CRL_MODE0_0; // LOCK PIN = PB0 output mode
    
    
}


void TIM1_Init(void){   // Timer1 period 10 ms
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;      // Вкл. тактирования TIM1 (APB2-72 MHz)
    //Настройка таймера TIM1//
    TIM1->PSC = 719;                      //Частота тактирования - 100 kHz = (72MHz/(PSC=719 + 1))
    TIM1->ARR = 999;                        // 1000 steps to the timer end
    TIM1->CR1 |= TIM_CR1_CEN;               // Timer1 enabled
}


uint16_t BTN_Checker(void){
    static uint16_t btn_counter;
    static uint16_t btn_status;

    if ((BTN_PORT->IDR & BTN_IDR_PIN) == 0){ // if BTN pressed
        if (btn_status != 1) {
            if (btn_counter < BTN_DEB_COUNT){
                btn_counter++;
                btn_status = BTN_RELEASED;
            }
            else{
                btn_status = BTN_PRESSED;
                btn_counter = 0;
            }
        }

    }
    else{      // if BTN released
         btn_status = BTN_RELEASED;
    }

    return btn_status;
}



void LED_Blinker(void){
    static uint16_t led_counter;
    
    if (led_counter < BLINK_COUNT){
        LED1_ON; // on led 1
        LED2_OFF; // OFF led 2
        led_counter++;
    }
    else{
        if (led_counter < (BLINK_COUNT*2)){
            LED1_OFF; // off led 1
            LED2_ON; // ON led 2
            led_counter++;
        }
        else{
            led_counter = 0;
        }

    }
    
    
}

void LEDS_ON(void){
    LED1_ON;
    LED2_ON;
}


void DoorLocked(void){
    LOCK_CLOSE;
}

void DoorOpened(void){
    LOCK_OPEN;
}


int main (void){

uint32_t i = 0 ;
uint16_t current_state = ST_IDLE;
uint16_t btn_state = BTN_RELEASED;
uint16_t open_time = 0;

SystemInit();   // main frequency = 72 MHz 
TIM1_Init();

GPIO_Init();


while (1) {
    btn_state = BTN_Checker();

    switch(current_state){    // FSM 
    case(ST_IDLE):
        open_time = 0;
        LED_Blinker();
        DoorLocked();
        if (btn_state != BTN_RELEASED) current_state = ST_OPEN; 
        break;

    case(ST_OPEN):
        DoorOpened();
        LEDS_ON();
        open_time++;
        if(open_time == OPEN_COUNT) current_state = ST_IDLE;
        break;
    }


    while((TIM1->SR & TIM_SR_UIF) == 0){};      //  wait for TIM1 end 10 ms
    TIM1->SR &= ~(TIM_SR_UIF);      // clear UIF-flag

}






//====================================================================================

    //GPIOB->BSRR |= GPIO_BSRR_BR10; // on led 1
    //for (i=0; i<time; i++);
    //GPIOB->BSRR |= GPIO_BSRR_BS10; // off led 1
    //for (i=0; i<time; i++);
    //GPIOB->BSRR |= GPIO_BSRR_BR11; // on led 2
    //for (i=0; i<time; i++);
    //GPIOB->BSRR |= GPIO_BSRR_BS11; // off led 2
    //for (i=0; i<time; i++);

    //GPIOB->BSRR |= GPIO_BSRR_BS12; // DOOR LOCKED 







return 0;
}
/*************************** End of file ****************************/
