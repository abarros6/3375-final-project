/////////////////////////////////////////////////////
// PRACTICE DESIGN QUESTION 4
//
// Use GPIO to accept input and send to LEDs,
// provide output based on switches
// pins 0 9 are input, but only ODD pins display
// to LEDs
// pins 10 to 19 are output, but only EVEN pins
// write data from switches
//
// Written by John McLeod, 2021 04 18
/////////////////////////////////////////////////////

#define LED_BASE 0xFF200000
#define SWITCH_BASE 0xFF200040
#define GPIO_BASE 0xFF200060

// define hardware
volatile int *const led = (int *)LED_BASE;
volatile int *const switches = (int *)SWITCH_BASE;
volatile int *const gpio = (int *)GPIO_BASE;
// just for fun, I will use just an integer pointer with
// offsets for the GPIO, instead of defining a structure
// the way I usually do.

int main()
{
    // configure bottom 10 pins of GPIO for input
    // *(gpio + 1) is one integer (4 bytes) above *(gpio) address
    // so that is the control register for the GPIO port
    // define a bit mask as 1's everywhere except in bits 0 to 9
    *(gpio + 1) &= 0xFFFFFFFF - 0b1111111111;
    // configure pins 10 to 19 of GPIO for output
    *(gpio + 1) |= (0b1111111111 << 10);
    // the above code is different than the way I normally do it,
    // but it shows you the flexibility of mixing hex, binary, and
    // bit shifts
    // it is also very careful in that it does not alter pins 20 to 31
    // (although in this example they aren't used, so it would be fine
    //  to alter them)

    ///////////////////////////////////////////////////////////////
    // method 1: uses just two lines, reads from GPIO and writes to LEDs
    // in one line, reads from switches and writes to GPIO in another line
    //
    // main loop
    while (1)
    {
        // read from GPIO input pins and write only odd bits to LEDs
        *led = (*gpio & 0b1010101010);
        // read from switches and write only even switches to GPIO output pins
        *gpio = (*gpio & ~(0b1111111111 << 10)) | ((*switches & 0b0101010101) << 10);
        // that's it!
    }
    ///////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////
    // method 2: this is another example of how to implement the code
    // above in a few more steps, possibly that makes it clearer
    //
    // note that as written, this code will never run unless you delete
    // the entire main loop above
    //
    // first define some bit masks
    int even_bit_mask = 0b0101010101;
    int odd_bit_mask = 0b1010101010;
    int all_bit_mask = 0b1111111111;
    // variables for interacting with GPIO
    int gpio_input;
    int gpio_output;

    // main loop
    while (1)
    {
        // read from GPIO input pins
        gpio_input = *gpio;
        // this gets everything from all 32 pins, even output
        // mask to get only odd input pins
        gpio_input &= odd_bit_mask;
        // write to LEDs
        *led = gpio_input;

        // read from switches
        gpio_output = *switches;
        // this gets everything from all 10 switches
        // mask to get only even switches
        gpio_output &= even_bit_mask;
        // bit shift left by 10
        gpio_output <<= 10;
        // to make sure we only modify the 10 output pins,
        // take the current state of the GPIO port currently
        // stored in "gpio_input" and clear bits 10 to 19
        gpio_input &= all_bit_mask << 10;
        // now combine old state with output
        gpio_output |= gpio_input;
        // write to GPIO output
        *gpio = gpio_output;
    }
    ///////////////////////////////////////////////////////////////
}