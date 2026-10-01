// button_debounce.c
// Josh Brake
// jbrake@hmc.edu
// 10/1/26

/*
  Debounces the button by sampling it from a timer interrupt instead of
  using the EXTI interrupt. TIM6 interrupts every SAMPLE_MS, and the handler
  only accepts a new button state after STABLE_SAMPLES samples in a row agree.
  This rejects bounce on both press and release, at the cost of up to
  SAMPLE_MS * STABLE_SAMPLES (20 ms) of latency after the bouncing stops.
*/

#include "main.h"

#define SAMPLE_MS      5 // How often TIM6 samples the button
#define STABLE_SAMPLES 4 // Samples in a row that must agree (4 x 5 ms = 20 ms)

void initDebounceTimer(void){
    RCC->APB1ENR1 |= (1 << 4); // TIM6EN

    TIM6->PSC = (SystemCoreClock / 1000) - 1; // 1 ms per count
    TIM6->ARR = SAMPLE_MS - 1;                // Update event every SAMPLE_MS
    TIM6->EGR |= (1 << 0);                    // UG: load PSC and ARR
    TIM6->SR &= ~(1 << 0);                    // UG also sets UIF, so clear it
    TIM6->DIER |= (1 << 0);                   // UIE: interrupt on update event
    TIM6->CR1 |= (1 << 0);                    // CEN: start counting
}

int main(void) {
    // Enable LED as output
    gpioEnable(GPIO_PORT_B);
    pinMode(LED_PIN, GPIO_OUTPUT);

    // Enable button as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (0b01 << 2*gpioPinOffset(BUTTON_PIN)); // Set PA7 as pull-up (PUPD7 = 01)

    // Initialize timer
    RCC->APB1ENR1 |= (1 << 0); // TIM2EN
    initTIM(DELAY_TIM);

    // Start the sampling timer
    initDebounceTimer();

    // Enable interrupts globally
    __enable_irq();

    // Turn on TIM6 interrupt in NVIC_ISER. TIM6_DAC is IRQ 54, so it is bit 54 - 32 = 22 of ISER1.
    NVIC->ISER[1] |= (1 << (54 - 32));

    while(1){
        delay_millis(DELAY_TIM, 200);
    }

}

void TIM6_DAC_IRQHandler(void){
    static int stable_state = 1; // Debounced button state (1 = released, because of the pull-up)
    static int count = 0;        // Samples in a row that differ from stable_state

    if (TIM6->SR & (1 << 0)){ // UIF
        // Clear the update interrupt flag (NB: Write 0 to reset.)
        TIM6->SR &= ~(1 << 0);

        int raw = digitalRead(BUTTON_PIN);
        if (raw != stable_state){
            count++;
            if (count >= STABLE_SAMPLES){
                // The button has settled in a new state
                stable_state = raw;
                count = 0;

                // Toggle the LED on a press (high to low)
                if (stable_state == 0) togglePin(LED_PIN);
            }
        } else {
            // A bounce back to the old state restarts the count
            count = 0;
        }
    }
}
