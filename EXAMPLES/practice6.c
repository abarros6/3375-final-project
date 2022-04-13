/////////////////////////////////////////////////////
// PRACTICE DESIGN QUESTION 6
//
// Count up to 100 on the ARM in 1 s intervals
// Current count displayed on seven-segment display
//
// This uses an interrupt-enabled timer
// Interrupt code adapted from Altera
// example code provided in "Using the ARM Generic
// Interrupt Controller" tutorial, at
// https://software.intel.com/content/www/us/en/develop/topics/fpga-academic/materials-tutorials.html
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

// definitions of subroutines coded below
// this lets compiler know what to expect
// these subroutines are from Altera
void disable_A9_interrupts(void);
void set_A9_IRQ_stack(void);
void config_GIC(void);
void enable_A9_interrupts(void);
void timer_ISR(void);
void config_interrupt(int, int);

// this subroutine is mine
int digit_code(int);

// global variables for hardware
volatile interval_timer *const timer = (interval_timer *)TIMER1_BASE;
volatile int *const sseg = (int *)SSEG_BASE;

// global variable for count to simplify using it in a ISR
int count = 0;

int main()
{
    // disable interrupts in the A9 processor
    disable_A9_interrupts();
    // initialize the stack pointer for IRQ mode
    set_A9_IRQ_stack();
    // configure the general interrupt controller
    config_GIC();
    // enable interrupts in the A9 processor
    enable_A9_interrupts();

    // set up timer for 1 s counts
    int timer_interval = 100e6;
    timer->low_period = timer_interval;
    timer->high_period = (timer_interval >> 16);
    // start the timer for countdown repeat and enable interrupts
    timer->control = 7;

    // main loop
    // do nothing, everything is handled in by interrupts
    while (1)
        ;
}

// Define the IRQ exception handler
// this subroutine modified from one by from Altera
void __attribute__((interrupt)) __cs3_isr_irq(void)
{
    // Read the ICCIAR from the CPU Interface in the GIC
    int interrupt_ID = *((int *)0xFFFEC10C);

    // check if interrupt is from the timer
    // timer is IRQ 72 on the DE1-SoC board
    // if any other IRQ sent, just hang because that is
    // not supposed to happen, something went wrong
    if (interrupt_ID == 72)
        timer_ISR();
    else
        while (1)
            ;

    // Write to the End of Interrupt Register (ICCEOIR)
    *((int *)0xFFFEC110) = interrupt_ID;
}

// Define the remaining exception handlers
// these subroutines are from Altera
// these do nothing but hang because the other
// exceptions aren't implemented in this example,
// and should never be triggered
void __attribute__((interrupt)) __cs3_reset(void)
{
    while (1)
        ;
}
void __attribute__((interrupt)) __cs3_isr_undef(void)
{
    while (1)
        ;
}
void __attribute__((interrupt)) __cs3_isr_swi(void)
{
    while (1)
        ;
}
void __attribute__((interrupt)) __cs3_isr_pabort(void)
{
    while (1)
        ;
}
void __attribute__((interrupt)) __cs3_isr_dabort(void)
{
    while (1)
        ;
}
void __attribute__((interrupt)) __cs3_isr_fiq(void)
{
    while (1)
        ;
}

// turn off interrupts in the ARM processor
// this subroutine is from Altera
// interrupts must be disabled prior to setting up the GIC
void disable_A9_interrupts(void)
{
    int status = 0b11010011;
    asm("msr cpsr, %[ps]"
        :
        : [ps] "r"(status));
}

// initialize the banked stack pointer register for IRQ mode
// this subroutine is from Altera
void set_A9_IRQ_stack(void)
{
    int stack, mode;
    // top of A9 onchip memory, aligned to 8 bytes
    stack = 0xFFFFFFFF - 7;
    // change processor to IRQ mode with interrupts disabled
    mode = 0b11010010;
    asm("msr cpsr, %[ps]"
        :
        : [ps] "r"(mode));
    // set banked stack pointer
    asm("mov sp, %[ps]"
        :
        : [ps] "r"(stack));

    // go back to SVC mode before executing subroutine return
    mode = 0b11010011;
    asm("msr cpsr, %[ps]"
        :
        : [ps] "r"(mode));
}

// turn on interrupts in the ARM processor
// this subroutine is from Altera
// interrupts must be enabled after setting up the GIC
void enable_A9_interrupts(void)
{
    int status = 0b01010011;
    asm("msr cpsr, %[ps]"
        :
        : [ps] "r"(status));
}

// configure the Generic Interrupt Controller (GIC)
// this subroutine modified from one by from Altera
void config_GIC(void)
{
    // configure the interval timer 1 interrupt (IRQ 72)
    config_interrupt(72, 1);

    // Set Interrupt Priority Mask Register (ICCPMR). Enable all priorities
    *((int *)0xFFFEC104) = 0xFFFF;

    // Set the enable in the CPU Interface Control Register (ICCICR)
    *((int *)0xFFFEC100) = 1;

    // Set the enable in the Distributor Control Register (ICDDCR)
    *((int *)0xFFFED000) = 1;
}

// configure registers in the GIC for an individual Interrupt ID.  We
// configure only the Interrupt Set Enable Registers (ICDISERn) and
// Interrupt Processor Target Registers (ICDIPTRn). The default (reset)
// values are used for other registers in the GIC
// this subroutine is from Altera
void config_interrupt(int N, int CPU_target)
{
    int reg_offset, index, value, address;

    // configure the Interrupt Set-Enable Registers (ICDISERn).
    // reg_offset = (integer_div(N / 32)*4; value = 1 << (N mod 32)
    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    // Using the address and value, set the appropriate bit
    *(int *)address |= value;

    // configure the Interrupt Processor Targets Register (ICDIPTRn)
    // reg_offset = integer_div(N / 4)*4; index = N mod 4
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;

    // Using the address and value, write to (only) the appropriate byte
    *(char *)address = (char)CPU_target;
}

// ISR for the timer
// this was written by me
void timer_ISR(void)
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

    return;
}

// subroutine to return the seven-segment code for a decimal value
// this was written by me
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