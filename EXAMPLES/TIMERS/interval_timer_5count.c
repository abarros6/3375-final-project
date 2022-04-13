#define LED_BASE 0xFF200000
#define TIMER_BASE 0xFF202000

typedef struct _interval_timer
{
    int status;
    int control;
    int low_period;
    int high_period;
    int low_counter;
    int high_counter;
} interval_timer;

void main()
{
    // pointers to hardware
    volatile interval_timer *const timer_1 = (interval_timer *)TIMER_BASE;
    volatile int *const led = (int *)LED_BASE;

    // interval for counting 5 s
    int interval = 500000000;
    // keeping track of the count
    int current_count = 0;
    int last_count = interval;
    // pattern for the LEDs
    int led_pattern = 0;

    // initialize timer for 5 s interval
    // write to low period
    // only lowest-16 bits will be written
    timer_1->low_period = interval;
    // write to high period
    timer_1->high_period = interval >> 16;

    // start timer for continuous counting
    // 6 is(0b0110)
    timer_1->control = 6;
    // blank out LEDs
    *led = led_pattern;

    // main loop
    while (1)
    {
        // write junk to get current count
        timer_1->low_counter = 1;
        // now get updated count
        current_count = timer_1->low_counter + (timer_1->high_counter << 16);
        // check if 1s has passed
        if (current_count <= last_count)
        {
            // increment led pattern
            led_pattern = (led_pattern << 1) + 1;
            *led = led_pattern;
            // check if we have passed 5 s
            // 63 is 0b111111
            if (led_pattern == 63)
            {
                // blank LEDs
                led_pattern = 0;
                *led = led_pattern;
                // reset last count to 5 s
                last_count = interval;
            }
            else
            {
                // subtract 1 s from interval
                last_count -= 100000000;
            }
        }
    }
}