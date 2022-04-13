// base addresses of hardware
#define JTAG_UART_BASE 0xFF201000
#define TIMER1_BASE 0xFF202000

// data structure for JTAG UART
typedef struct _jtag_uart
{
    int data;
    int control;
} jtag_uart;

// data structure for interval timer
typedef struct _interval_timer
{
    int status;
    int control;
    int low_period;
    int high_period;
    int low_counter;
    int high_counter;
} interval_timer;

// set up hardware
volatile jtag_uart *const uart = (jtag_uart *)JTAG_UART_BASE;
volatile interval_timer *const timer = (interval_timer *)TIMER1_BASE;

int main()
{
    // buffer for reading data from JTAG UART
    int uart_buf;
    // character to terminate a literary word
    char char_space = ' ';

    // set up timer for 1s counts
    // at 100 MHz, this is 100 000 000
    // in hex, 0x05F5E100
    timer->low_period = 0xE100;
    timer->high_period = 0x05F5;
    // start timer for count-down and repeat
    timer->control = 0b0110;

    // main loop
    while (1)
    {
        // check for timeout
        if (timer->status == 0b0011)
        {
            // clear timeout
            timer->status = 1;

            // read from UART
            uart_buf = uart->data;

            // write entire literary word to UART if characters are stored
            // check if bit 15 is set and UART character is not a space
            // if so, write that character back to UART
            while ((uart_buf & 0x8000) && ((uart_buf & 0xFF) != char_space))
            {
                uart->data = (uart_buf & 0xFF);
                uart_buf = uart->data;
            }

            // write space between words
            if ((uart_buf & 0x8000) && ((uart_buf & 0xFF) == char_space))
                uart->data = char_space;
        }
    }
}