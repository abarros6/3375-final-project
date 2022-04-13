/////////////////////////////////////////////////////
// PRACTICE DESIGN QUESTION 5
//
// Turns ARM into countdown timer
// Any key (except "enter") pressed in JTAG UART will
// be displayed in ASCII code in binary on LED
//
// Pressing "enter" will start countdown in
//  100 ms intervals
//
// As a design choice (this wasn't completely specified
// in problem statement) everything entered in JTAG UART
// while countdown is in progress is silently ignored
//
// Written by John McLeod, 2021 04 18
/////////////////////////////////////////////////////

#define LED_BASE 0xFF200000
#define JTAG_UART_BASE 0xFF201000
#define TIMER1_BASE 0xFF202000
#define ASCII_ENTER 0x0A

typedef struct jtag_uart
{
    int data;
    int control;
} jtag_uart;

typedef struct interval_timer
{
    int status;
    int control;
    int low_period;
    int high_period;
    int low_count;
    int high_count;
} interval_timer;

volatile jtag_uart *const uart = (jtag_uart *)JTAG_UART_BASE;
volatile interval_timer *const timer = (interval_timer *)TIMER1_BASE;
volatile int *const led = (int *)LED_BASE;

int main()
{
    int uart_buffer = 0;
    int count = 0;
    int timer_interval = 10e6;

    // set up timer for 100 ms counts
    timer->low_period = timer_interval;
    timer->high_period = (timer_interval >> 16);

    // main loop
    while (1)
    {
        // read from UART
        // always do this each cycle
        // to prevent UART queue from overflowing
        uart_buffer = uart->data;
        // check that this is valid data, bit 15 = 0x8000
        if ((uart_buffer & 0x8000) == 0x8000)
        {
            // mask everything except bottom 8 bits
            uart_buffer &= 0xFF;
        }

        // check if timer is counting and had a timeout
        if ((timer->status & 0b01) == 0b01)
        {
            // decrement LED count
            count -= 1;
            // clear timeout flag
            timer->status = 0b01;
        }

        // check if LED count is below zero
        if (count < 0)
        {
            // countdown is done
            // turn off timer
            timer->control = 8;
            // reset count to zero
            count = 0;
        }
        else
        {
            // countdown is still continuing
            // update LEDs
            *led = count;
        }

        // now if timer is done counting, we can process
        // next UART input
        if (timer->status == 0b00)
        {
            // if "enter" is pressed, restart timer
            // here I assume "count" was already set to next
            // value
            if (uart_buffer == ASCII_ENTER)
            {
                // restart timer
                timer->control = 6;
            }
            else
            {
                // if "enter" was not pressed, use character as
                // next count
                count = uart_buffer;
            }
        }
    }
}
