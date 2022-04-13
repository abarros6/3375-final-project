// for displaying on seven segment
// assumes value is at most 3 decimal digits
void display_hex(int value)
{
    // pointer to hardware
    volatile int *const hex03_ptr = (int *)0xFF200020;
    // look-up table of hexadecimal codes fo
    int hex_codes[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
    int value_code;

    // get hex code for 1s place
    value_code = hex_codes[(value % 10)];
    // get hex code for 10s place, shift over by a byte
    value_code += hex_codes[((value / 10) % 10)] << 8;
    // get hex code for 100s place, shift over by two bytes
    value_code += hex_codes[((value / 100) % 10)] << 16;
    // display on seven-segment
    *hex03_ptr = value_code;
}

// structure for JTAG UART
typedef struct _jtag_uart
{
    int data;
    int control;
} jtag_uart;

void main()
{
    // define pointer to hardware
    volatile jtag_uart *const uart_ptr = (jtag_uart *)0xFF201000;
    // other variables
    int i;
    char message[11] = {'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D'};
    int read_uart;

    // loop through characters
    for (i = 0; i < 11; i++)
    {
        // write to JTAG UART
        uart_ptr->data = message[i];
    }

    // loop endlessly
    while (1)
    {
        // read JTAG UART
        read_uart = uart_ptr->data;

        // check if bit 15 set
        // this means that there is some data in the receive queue
        // alternatively, we could read bits [31..16] to see exactly
        // how many bytes of data are in the queue
        if (read_uart & (1 << 15))
        {
            // clear everything except the least-significant byte
            // bit 15 is set to 1, and the top 16 bits contain the
            // amount of data in the queue
            // but this information is not related to the data recieved
            read_uart &= 0xFF;

            // send ASCII character code receieved to seven segment display
            display_hex(read_uart);
        }
    }
}