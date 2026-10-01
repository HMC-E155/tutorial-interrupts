# Tutorial for Configuring Interrupts on STM32L432KC

This tutorial demonstrates how to configure interrupts on the STM32L432KC.
The example shows how to configure the EXTI controller to trigger an interrupt on the falling edge of a GPIO pin.
This GPIO pin is connected to a pullup resistor, thus, pressing it will ground the pin, generate a falling edge, and trigger the interrupt.

## Configuration Steps

The steps to configure the interrupt are as follows:

1.   `EXTI` mux in the `SYSCFG` peripheral
2.   Configure interrupt generation settings in EXTI peripheral
3.   Globally enable interrupts
4.   Set Interrupt Mask Register (`IMR`)
5.   Select rising/falling edge trigger
6.   Turn on the interrupt in the `NVIC_ISER` (NB: The bits in the NVIC registers correspond to the interrupt position in the vector table).

## Debouncing

The switch bounces for a few milliseconds on press and release. Each bounce is a new falling edge, so the EXTI
interrupt can toggle the LED several times per press.

`src/button_debounce.c` is an alternative that drops the EXTI interrupt and samples the button from a TIM6
interrupt every 5 ms. A new button state only counts once 4 samples in a row agree, so bounce on both press
and release is ignored, at the cost of up to 20 ms of latency. TIM6_DAC is IRQ 54, so it is enabled in
`NVIC_ISER1` (bit 22).

`button_polling.c`, `button_interrupt.c`, and `button_debounce.c` each define `main()`, so build only one of
them at a time (exclude the other two from the build in SEGGER Embedded Studio).
