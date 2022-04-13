#define LED_BASE 0xFF200000
#define MPCORE_PRIV_TIMER 0xFFFEC600
#define bit_9 0x00000200
/* This program provides a simple example of code for the ARM A9. The
 * program performs the following:
 * 1. starts the ARM A9 private timer
 * 2. loops indefinitely, toggling the bit 9 red light LEDs when the timer expires
 */
int main(void)
{
    /* Declare volatile pointers to I/O registers (volatile means that the
     * locations will not be cached, even in registers) */
    volatile int *LED_ptr = (int *)LED_BASE;
    volatile int *MPcore_private_timer_ptr = (int *)MPCORE_PRIV_TIMER;
    int Bit9_LEDs = bit_9;                   // value to turn on the bit 9 red light LEDs
    int counter = 200000000;                 // timeout = 1/(200 MHz) x 200x10^6 = 1 sec
    *(LED_ptr) = bit_9;                      // write to the the LEDs register to turn on bit 9
    *(MPcore_private_timer_ptr) = counter;   // write to timer load register
    *(MPcore_private_timer_ptr + 2) = 0b011; // mode = 1 (auto), enable = 1
    while (1)
    {
        *LED_ptr = Bit9_LEDs; // turn on/off the bit 9 of the red LEDs
        while (*(MPcore_private_timer_ptr + 3) == 0)
            ;                                // wait for timer to expire
        *(MPcore_private_timer_ptr + 3) = 1; // reset timer flag bit
        Bit9_LEDs ^= bit_9;                  // toggle bit that controls the bit 9 of LEDs
    }
}
