#define MPCORE_PRIV_TIMER 0xFFFEC600
#define LED_BASE 0xFF200000
#define SW_BASE 0xFF200040
#define KEY_BASE 0xFF200050
/* This program demonstrates use of parallel ports in the Computer System
 *
 * It performs the following:
    1. displays a rotating pattern on the green LED
    2. if a KEY is pressed, uses the SW switches as the pattern */
/* Declare volatile pointers to private timer registers */
volatile int *MPcore_private_timer_ptr = (int *)MPCORE_PRIV_TIMER; // timer address
int main(void)
{
    /* Declare volatile pointers to I/O registers (volatile means that IO load
     * and store instructions will be used to access these pointer locations,
     * instead of regular memory loads and stores)
     */
    volatile int *LED_ptr = (int *)LED_BASE;      // LED address
    volatile int *SW_switch_ptr = (int *)SW_BASE; // SW slider switch address
    volatile int *KEY_ptr = (int *)KEY_BASE;      // pushbutton KEY address
    void private_timer(int counter);              // subroutine to configure the private timer
    int LED_bits = 0x0F0F0F0F;                    // pattern for LED lights
    int SW_value, KEY_value;
    int counter = 200000000; // timeout = 1/(200 MHz) x 200x10^6 = 1 sec
    while (1)
    {
        SW_value = *(SW_switch_ptr); // read the SW slider (DIP) switch values
        KEY_value = *(KEY_ptr);      // read the pushbutton KEY values
        if (KEY_value != 0)          // check if any KEY was pressed
        {
            /* set pattern using SW values */
            LED_bits = (SW_value | (SW_value << 8) | (SW_value << 16) |
                        (SW_value << 24));
            while (*KEY_ptr)
                ; // wait for pushbutton KEY release
        }
        *(LED_ptr) = LED_bits; // light up the LEDs
                               /* rotate the pattern shown on the LEDs */
        if (LED_bits & 0x80000000)
            LED_bits = (LED_bits << 1) | 1;
        else
            LED_bits = LED_bits << 1;
        private_timer(counter); // passing initial private counter value (load)
    }
}
void private_timer(int counter)
{
    *(MPcore_private_timer_ptr) = counter;   // write to timer load register
    *(MPcore_private_timer_ptr + 2) = 0b011; // mode = 1 (auto), enable =
    while (*(MPcore_private_timer_ptr + 3) == 0)
        ;                                // wait for timer to expire
    *(MPcore_private_timer_ptr + 3) = 1; // reset timer flag bit
    return;
}