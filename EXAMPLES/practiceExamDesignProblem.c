////////////////////////////////////////////////
// PRECOMPILER DEFINITIONS
#define TIMER_BASE 0x6400
#define ADC_BASE 0x7000
#define GPIOA_BASE 0x6000
#define UART_BASE 0x7400

////////////////////////////////////////////////
// STRUCTURES
// boy-oh-boy, do I love structures!
// seriously, I would rather write a 100-variable structure
// that was only used once than explicitly use a pointer
// who likes pointers? nobody, that's who!
typedef struct _timer_struct
{
    int data;
    int status;
    int control;
} timer_type;

typedef struct _gpio_struct
{
    int data;
    int control;
    int interrupt;
    int edge_capture;
} gpio_type;

typedef struct _adc_base
{
    int control;
    int status;
    int data;
} adc_type;

typedef struct _uart_base
{
    int status;
    int control;
    int rx_data;
    int tx_data;
} uart_type;

////////////////////////////////////////////////
// FUNCTION DECLARATIONS
// ISR for timer must have this name
void _isr_8(void);
// ISR for ADC must have this name
void _isr_13(void);
// this is explained in the appendix

////////////////////////////////////////////////
// GLOBAL VARIABLES
// make all hardware global
volatile timer_type *const timer = (timer_type *)TIMER_BASE;
volatile gpio_type *const gpio = (gpio_type *)GPIOA_BASE;
volatile adc_type *const adc = (adc_type *)ADC_BASE;
volatile uart_type *const uart = (uart_type *)UART_BASE;
// use to store the number of clock cycles
// for the on-time
// (off-time is then 1000 - high_period)
int high_period = 500;
// used to indicate whether output is
// currently 0 or 1
int output_level = 1;

////////////////////////////////////////////////
// MAIN PROGRAM
void main()
{
    // configure ADC for interrupts
    adc->control = (1 << 8);
    // configure timer for interrupts
    timer->control = 1;
    // configure UART for 8-0-1 and 9600 baud
    uart->control = 0x0122;
    // from appendix, 8-0-1, 9600 baud, and no interrupts
    // is 0b00xx 0x01 xx10 xx10
    // where x is don't care, I'll treat x = 0

    // use pin 0 of GPIO port A, set to output
    gpio->control = 1;
    // set output level to 1
    gpio->data = output_level;

    // assume this whole thing starts on a rising
    // edge with 50-50 duty cycle
    // so start timer accordingly
    timer->data = high_period;

    // do nothing and watch interrupt magic happen
    while (1)
        ;
}

////////////////////////////////////////////////
// INTERRUPT SERVICE ROUTINES
// the name and type of these routines is required to be
// void _isr_[irq]( void )
// where [irq] is the IRQ #
// this is explained in the appendix

// this is for the timer (IRQ #8)
void _irq_8(void)
{
    // check if current output=0
    if (output_level == 0)
    {
        // switch output to high
        output_level = 1;
        gpio->data = output_level;
        // start timer for high period
        timer->data = high_period;
        // divide high period by 4 because
        // high period is 0 to 1000, but UART
        // only writes one byte
        // and 0 to 250 fits in one byte
        uart->tx_data = high_period / 4;
    }
    else
    {
        // switch output to low
        output_level = 0;
        gpio->data = output_level;
        // start timer for low period
        timer->data = 1000 - high_period;
        // start sampling ADC on channel 0
        // while preserving interrupts
        adc->control = 0b00 + (1 << 8);
        // I know the above math is unnecessary but I
        // want to make it clear that it is channel # + interrupt in bit 8
    }
    // clear timer interrupts
    timer->status = 1;
}

// this is for ADC (IRQ #13)
void _irq_13(void)
{
    // read data from conversion on channel 0
    high_period = adc->data;
    // make sure this is capped at 250
    if (high_period > 250)
        high_period = 250;
    // multiply by 4 to make it between 0 and 1000
    high_period *= 4;
}