// button_interrupt.c
// Josh Brake
// jbrake@hmc.edu
// 10/31/22

#include "main.h"

int main(void) {
    // Enable LED as output
    gpioEnable(GPIO_PORT_B);
    pinMode(LED_PIN, GPIO_OUTPUT);

    // Enable button as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD7, 0b01); // Set PA7 as pull-up

    // Initialize timer
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);

    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input button interrupt
    // EXTI7 is in EXTICR2 (EXTICR[1] in CMSIS). Port A is 0b000, so clearing the field selects PA7.
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI7;
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI7, 0b000);

    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for falling edge of GPIO pin for button
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN));   // 1. Configure mask bit
    EXTI->RTSR1 &= ~(1 << gpioPinOffset(BUTTON_PIN)); // 2. Disable rising edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON_PIN));  // 3. Enable falling edge trigger
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);             // 4. Turn on EXTI interrupt in NVIC_ISER (EXTI9_5 is IRQ 23)

    while(1){
        delay_millis(DELAY_TIM, 200);
    }

}

// EXTI lines 5-9 share this handler
void EXTI9_5_IRQHandler(void){
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(BUTTON_PIN))){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 = (1 << gpioPinOffset(BUTTON_PIN));

        // Then toggle the LED
        togglePin(LED_PIN);

    }
}
