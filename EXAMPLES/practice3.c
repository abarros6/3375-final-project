/////////////////////////////////////////////////////
// PRACTICE DESIGN QUESTION 3
//
// Count up to 100 on the ARM in 1 s intervals
// Current count displayed on seven-segment display
//
// As a design choice (this wasn't completely specified
// in problem statement) after count reaches 99 it wraps
// around back to 0 and repeats
// (technically 0 to 99 is a 100-count)
//
// Written by John McLeod, 2021 04 18
/////////////////////////////////////////////////////

#define SSEG_BASE 0xFF200020
#define TIMER1_BASE 0xFF202000

typedef struct interval_timer
{
    int status;
    int control;
    int low_period;
    int high_period;
    int low_count;
    int high_count;
} interval_timer;

volatile interval_timer *const timer = (interval_timer *)TIMER1_BASE;
volatile int *const sseg = (int *)SSEG_BASE;

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

int main()
{
    int count = 0;
    int timer_interval = 100e6;

    // set up timer for 1 s counts
    timer->low_period = timer_interval;
    timer->high_period = (timer_interval >> 16);
    // start the timer for countdown repeat
    timer->control = 6;

    // main loop
    while (1)
    {
        // write to seven-segment
        // note that % is the modulo operator: a%b means "divide by b and keep remainder"
        *sseg = digit_code(count % 10) + (digit_code(count / 10) << 8);

        // check if timer is counting and had a timeout
        if ((timer->status & 0b01) == 0b01)
        {
            // increment count
            count += 1;
            // clear timeout flag
            timer->status = 0b01;
        }

        // check if count is reaches 100
        if (count == 100)
            count = 0;
    }
}