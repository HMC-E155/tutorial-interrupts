# Tutorial for Configuring Interrupts on STM32L432KC

This tutorial demonstrates how to configure interrupts on the STM32L432KC.
The example shows how to configure the EXTI controller to trigger an interrupt on the falling edge of a GPIO pin.
This GPIO pin is connected to a pullup resistor, thus, pressing it will ground the pin, generate a falling edge, and trigger the interrupt.

## Configuration Steps

A button press reaches the interrupt handler only if every stage passes it on. Leave one closed and nothing
happens, with no error message.

0.   **RCC**: turn on the clock to the `SYSCFG` peripheral.
1.   **SYSCFG**: set the `EXTI` mux in `SYSCFG_EXTICR` so the button's port drives its EXTI line.
2.   **EXTI**: unmask the line in the interrupt mask register (`EXTI_IMR1`) and select the falling edge
     (`EXTI_RTSR1` and `EXTI_FTSR1`).
3.   **NVIC**: turn on the interrupt in `NVIC_ISER`. The bits in the NVIC registers correspond to the interrupt
     position in the vector table, not the EXTI line.
4.   **CPU**: enable interrupts globally with `__enable_irq()`.
5.   **Vector table**: name the handler exactly as it appears in the vector table. Inside the handler, check and
     clear the pending bit in `EXTI_PR1`.

## Debouncing

The switch bounces for a few milliseconds on press and release. Each bounce is a new falling edge, so the EXTI
interrupt can toggle the LED several times per press.

`src/button_debounce.c` is an alternative that drops the EXTI interrupt and samples the button from a TIM6
interrupt every 5 ms. A new button state only counts once 4 samples in a row agree, so bounce on both press
and release is ignored, at the cost of up to 20 ms of latency. TIM6_DAC is IRQ 54, so it is enabled in
`NVIC_ISER1` (bit 22).

`button_polling.c`, `button_interrupt.c`, and `button_debounce.c` each define `main()`, so build only one of
them at a time (exclude the other two from the build in SEGGER Embedded Studio).
