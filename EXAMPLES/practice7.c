/////////////////////////////////////////////////////
// PRACTICE DESIGN QUESTION 7
//
// Watch pin 16 on GPIO 1
// On rising edge start timer, and time how long it takes
// until it goes low again
//
// show time in 1/100ths second on 7-segment
//
// Written by John McLeod, 2022 04 06
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// DEFINITIONS
#define SSEG_03_BASE 0xFF200020
#define SSEG_45_BASE 0xFF200030
#define TIMER1_BASE 0xFF202000
#define GPIOA_BASE 0xFF200060

/////////////////////////////////////////////////////
// STRUCTURES
typedef struct interval_timer
{
    int status;
    int control;
    int low_period;
    int high_period;
    int low_count;
    int high_count;
} interval_timer;

typedef struct gpio_port
{
    int data;
    int control;
    int interrupt;
    int edge_capture;
} gpio_port;

/////////////////////////////////////////////////////
// GLOBAL VARIABLES
volatile interval_timer *const timer = (interval_timer *)TIMER1_BASE;
volatile gpio_port *const gpio = (gpio_port *)GPIOA_BASE;
volatile int *const sseg_1 = (int *)SSEG_03_BASE;
volatile int *const sseg_2 = (int *)SSEG_45_BASE;

/////////////////////////////////////////////////////
// FUNCTION DECLARATIONS
int digit_code(int digit);
void write_time(int time);

/////////////////////////////////////////////////////
// MAIN PROGRAM
int main()
{
    // I will use two variables to check when gpio goes 0 -> 1 or 1 -> 0
    // GPIO has an 'edge-capture' register, that may record a transistion as well
    // someone can experiment with it!
    int new_gpio = 0;
    int old_gpio = 0;
    int gpio_time = 0;
    int timer_interval = 1e6;

    // set up timer for 10 ms counts
    timer->low_period = timer_interval;
    timer->high_period = (timer_interval >> 16);
    // don't start timer yet

    // configure GPIO pin 16 for input
    gpio->control &= ~(1 << 16);

    // get initial value
    old_gpio = gpio->data & (1 << 16);

    // main loop
    while (1)
    {
        // get GPIO value
        new_gpio = gpio->data & (1 << 16);

        // check if it went low-to-high
        if ((new_gpio != old_gpio) && (new_gpio > 0))
        {
            // reset clock
            gpio_time = 0;
            // start timer for count-down and repeat
            timer->control = 6;
        }
        // check if it wen high-to-low
        if ((new_gpio != old_gpio) && (new_gpio == 0))
        {
            // stop clock
            timer->control = 8;
            // clear timeout, just in case
            timer->status = 0b01;
        }

        // check if timer is counting and had a timeout
        if ((timer->status & 0b01) == 0b01)
        {
            // increment count
            gpio_time += 1;
            // clear timeout flag
            timer->status = 0b01;
        }

        // check if count passes 1 hour
        // original problem was vague, so this is my design choice
        // 7-segment can only display up to 59:59:99
        if (gpio_time >= 60 * 60 * 100)
            gpio_time = 0;

        // record new state
        old_gpio = new_gpio;

        // write to seven-segment
        write_time(gpio_time);
    }
}

/////////////////////////////////////////////////////
// SUBROUTINES

// subroutine to return the seven-segment code for a decimal value
int digit_code(int digit)
{
    // an array to hold all the codes for 0 to 9
    // you could also do this with a switch...case construction
    int sseg_codes[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

    // a valid digit is between 0 and 9
    if ((digit >= 0) && (digit <= 9))
        return sseg_codes[digit];
    // if a negative number or a value > 9 is given,
    // just return 0
    return 0;
}

// subroutine to write time in min:sec:hsec on 7-segment
void write_time(int time)
{
    int csec = time % 10;
    int dsec = (time / 10) % 10;
    int sec = (time / 100) % 10;
    int Dsec = (time / 1000) % 6;
    int min = (time / 6000) % 10;
    int Dmin = (time / 60000) % 6;

    *sseg_1 = digit_code(csec) + (digit_code(dsec) << 8) + (digit_code(sec) << 16) + (digit_code(Dsec) << 24);
    *sseg_2 = digit_code(min) + (digit_code(Dmin) << 8);
}