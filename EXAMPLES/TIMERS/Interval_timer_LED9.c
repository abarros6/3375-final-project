#define LED_BASE 0xFF200000
#define INTER_TIMER_BASE 0xFF202000
#define bit_9 0x00000200
/* This program provides a simple example of code for the ARM A9. The
 * program performs the following:
 * 1. starts the interval timer
 * 2. loops indefinitely, toggling the bit 9 red light LEDs when the timer expires
 */
int main(void)
{
    /* Declare volatile pointers to I/O registers (volatile means that the
     * locations will not be cached, even in registers) */
    volatile int *LED_ptr = (int *)LED_BASE;
    volatile int *INTER_TIMER_BASE_ptr = (int *)INTER_TIMER_BASE;
    int Bit9_LEDs = bit_9;                       // value to turn on the bit 9 red light LEDs
    int counter = 100000000;                     // timeout = 1/(100 MHz) x 100x10^6 = 1 sec
    *(LED_ptr) = bit_9;                          // write to the the LEDs register to turn on bit 9
    *(INTER_TIMER_BASE_ptr) = 0;                 //  write 0 to status register to initialize TO=0
    *(INTER_TIMER_BASE_ptr + 2) = counter;       //  write to timer counter start value (low)
    *(INTER_TIMER_BASE_ptr + 3) = counter >> 16; //  write to timer counter start value (high)
    *(INTER_TIMER_BASE_ptr + 1) = 0b0110;        // set bits: STOP=0, START=1, CONT=1, ITO=0
    while (1)
    {
        *LED_ptr = Bit9_LEDs; // turn on/off the bit 9 of the red LEDs
        while ((*(INTER_TIMER_BASE_ptr)&1) == 0)
            ;                        // wait for timer to expire
        *(INTER_TIMER_BASE_ptr) = 0; //  write 0 to status register to initialize TO=0
        Bit9_LEDs ^= bit_9;          // toggle bit that controls the bit 9 of LEDs
    }
}
