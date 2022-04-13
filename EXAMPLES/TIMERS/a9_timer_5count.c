#define LED_BASE 0xFF200000
#define TIMER_A9_BASE 0xFFFEC600

typedef struct _a9_timer
{
    int load;
    int count;
    int control;
    int status;
} a9_timer;

void main()
{
    // pointers to hardware
    volatile a9_timer *const timer_1 = (a9_timer *)TIMER_A9_BASE;
    volatile int *const led = (int *)LED_BASE;

    // interval for counting 5 s
    int interval = 500000000;
    // keeping track of the count
    int current_count = 0;
    int last_count = interval;
    // pattern for the LEDs
    int led_pattern = 0;

    // initialize timer for 5 s interval
    // assumes 100 MHz rate
    timer_1->load = interval;

    // start timer for continuous counting
    // 3 is (0b011) for control
    // (1 << 8) is for prescaler
    timer_1->control = 3 + (1 << 8);
    // blank out LEDs
    *led = led_pattern;

    // main loop
    while (1)
    {
        // get count
        current_count = timer_1->count;
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