////////////////////////////////////////////////////////////////////////
// PREPROCESSOR MACROS
////////////////////////////////////////////////////////////////////////
#define LED_BASE 0xFF200000
#define I2C0_BASE 0xFFC04000

////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS
////////////////////////////////////////////////////////////////////////
void init_I2C(void);
unsigned char read_over_I2C(unsigned char address);
void write_over_I2C(unsigned char address, unsigned char value);
void read_over_I2C_multiple(unsigned char address, unsigned char *values, unsigned char length);
void init_GS(void);

////////////////////////////////////////////////////////////////////////
// STRUCTURES and GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////
// structure of I2C device
// a whole lot of registers, most of them we don't need
typedef struct _I2Cn
{
    // uses variables called 'pad' to align
    // needed registers in memory
    int control;         // 0x00
    int target;          // 0x04
    int slave;           // 0x08
    int pad0;            // skip
    int data_cmd;        // 0x10
    int std_scl_hcnt;    // 0x14
    int std_scl_lcnt;    // 0x18
    int fast_scl_hcnt;   // 0x1C
    int fast_scl_lcnt;   // 0x20
    int pad1;            // skip
    int pad2;            // skip
    int intr_status;     // 0x2C
    int intr_mask;       // 0x30
    int raw_intr_status; // 0x34
    int rx_fifo_thr;     // 0x38
    int tx_fifo_thr;     // 0x3C
    int cmb_intr;        // 0x40
    int rx_under_intr;   // 0x44
    int rx_over_intr;    // 0x48
    int tx_over_intr;    // 0x4C
    int intr_read;       // 0x50
    int tx_abort_intr;   // 0x54
    int rx_done_intr;    // 0x58
    int activity_intr;   // 0x5C
    int stop_dtct_intr;  // 0x60
    int start_dtct_intr; // 0x64
    int gen_call_intr;   // 0x68
    int enable;          // 0x6C
    int status;          // 0x70
    int tx_fifo_lvl;     // 0x74
    int rx_fifo_lvl;     // 0x78
    int sda_hold;        // 0x7C
    int tx_abort_src;    // 0x80
    int gen_slave_nack;  // 0x84
    int dma_control;     // 0x88
    int dma_tx_lvl;      // 0x8C
    int rx_data_lvl;     // 0x90
    int sda_setup;       // 0x94
    int ack_gen_call;    // 0x98
    int enable_status;   // 0x9C
    int ss_fs_supp;      // 0xA0
} I2Cn;

// make I2C pointer global for easy access
volatile I2Cn *const I2C_ptr = (I2Cn *)I2C0_BASE;

////////////////////////////////////////////////////////////////////////
// MAIN PROGRAM
////////////////////////////////////////////////////////////////////////
int main(void)
{
    // pointer to LEDs
    volatile int *const LED_ptr = (int *)LED_BASE;
    // array to hold x0, x1, y0, y1, z0, z1 data
    // x0, x1 are LSB and MSB of 16-bit data for x-axis
    // similar for y0,y1 and z0,z1
    unsigned char GS_data[6];
    // any other variable declarations here

    // first initialize I2C interface
    init_I2C();

    // check if GS is available
    // ADXL345 manual states
    //   device ID register is at address 0x00
    //   it should always read 0xE5
    if (read_over_I2C(0x00) != 0xE5)
    {
        // if this isn't 0xE5, we are in trouble
        // turn on all LEDs as a warning
        *LED_ptr = 0x3FF;
        // dead loop
        while (1)
            ;
    }

    // assuming we are talking to GS
    // initialize it for sampling
    init_GS();

    // now loop and read data
    while (1)
    {
        // check if reading is done
        // ADXL345 manual states
        //   source register is 0x30
        //   bit 7 is 1 when data is ready
        if (read_over_I2C(0x30) > 0x80)
        {
            // sample data
            read_over_I2C_multiple(0x32, GS_data, 6);
            // ok, we have the data
            // now do something cool with it
        }

        // more code here if necessary
    }
}

////////////////////////////////////////////////////////////////////////
// FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////
// this initializes an I2C device to connect
// to the ADXL345 sensor
void init_I2C()
{
    // set I2C enable to abort any transmissions
    // and disable device
    I2C_ptr->enable = 2;
    // wait until this is done
    // enable status register goes to 0 when finished
    while (I2C_ptr->enable_status & 0x1)
        ;

    // set control bits as 0b0110 0101
    //      0 : (nothing, bit reserved)
    //      1 : disable slave
    //      1 : enable master restarts
    //      0 : 7-bit addressing mode master
    //      0 : 7-bit addressing mode slave
    //      1 : fast mode (with below)
    //      0 : fast mode (with above)
    //      1 : master enabled
    I2C_ptr->control = 0b01100101;

    // set target address
    // address 0x53 is defined in ADXL345 documentation
    I2C_ptr->target = 0x53;

    // set count for high-period of clock
    I2C_ptr->fast_scl_hcnt = 90;
    // set count for low-period of clock
    I2C_ptr->fast_scl_lcnt = 160;
    // uses fast clock because control register set to [xxxx x10x]
    // set to [xxxx x01x] for slow clock
    // note these high- and low-periods are determined by
    // ADXL345:	high-period has minimum 60	 (x10 ns)
    //          low-period has minimum 130 (x10 ns)
    // here we padded both to total 250 = 2.5 us, i.e. the period of
    // SCL fast clock (400 KHz)
    // other devices may have different requirements for high-
    // and low-periods, so check documentation

    // turn on IC20
    I2C_ptr->enable = 1;
    // wait until this is done
    while ((I2C_ptr->enable_status & 0x1) == 0)
        ;
}

// this reads a byte from the G-sensor internal register
// at address 'address'
unsigned char read_over_I2C(unsigned char address)
{
    // the data_cmd register is used to load the transmit queue
    // data_cmd uses 10 bits: [0ccc xxxx xxxx]
    //   x's are for data sent to device
    //   c's are command bits
    // command bits 0100 (i.e. 0x4) issues a restart before sending data
    I2C_ptr->data_cmd = address + 0x400;

    // this is the general way to read from I2C
    // command bits 0001 (i.e. 0x1) switches to read mode
    I2C_ptr->data_cmd = 0x100;
    // wait until read data is obtained
    while (I2C_ptr->rx_fifo_lvl == 0)
        ;
    // read the data from read queue
    return I2C_ptr->data_cmd;
    // the data_cmd register is used for both recieve/transmit
    // write to the register to add data to transmit queue
    // read from this register to remove data from recieve queue
    // as indicated, use rx_fifo_lvl to determine whether recieve queue
    // has anything in it
}

// this reads multiple bytes from the peripheral's internal register
// at address 'address'
// note that the ADXL345 treats multiple reads as the instruction to increment
// internal address by 1 each read, so really this reads a set of sequential registers
void read_over_I2C_multiple(unsigned char address, unsigned char *values, unsigned char length)
{
    // counter
    int i;

    // the data_cmd register is used to load the transmit queue
    // data_cmd uses 10 bits: [0ccc xxxx xxxx]
    //   x's are for data sent to device
    //   c's are command bits
    // command bits 0100 (i.e. 0x4) issues a restart before sending data
    I2C_ptr->data_cmd = address + 0x400;

    for (i = 0; i < length; i++)
        // this is the general way to read from I2C
        // command bits 0001 (i.e. 0x1) switches to read mode
        I2C_ptr->data_cmd = 0x100;

    // loop until all data is read
    for (i = 0; i < length; i++)
    {
        // hold until data received
        while (I2C_ptr->rx_fifo_lvl == 0)
            ;
        // read data into array
        values[i] = I2C_ptr->data_cmd;
    }

    // this subroutine is type void because arrays are passed by reference in C
    // so original values[] array is modified by this subroutine
}

// this writes a byte to the peripheral's internal register
// at address 'address'
void write_over_I2C(unsigned char address, unsigned char value)
{
    // the data_cmd register is used to load the transmit queue
    // data_cmd uses 10 bits: [0ccc xxxx xxxx]
    //   x's are for data sent to device
    //   c's are command bits
    // command bits 0100 (i.e. 0x4) issues a restart before sending data
    I2C_ptr->data_cmd = address + 0x400;
    // write again to add value to transmit queue to send to device
    I2C_ptr->data_cmd = value;
    // this is a pretty general way of writing to I2C
}

// this writes multiple bytes to the peripheral's internal register
// at address 'address'
// this routine doesn't make sense for the ADXL345 so we don't use it
// but it is here for reference
void write_over_I2C_multiple(unsigned char address, unsigned char *values, unsigned char length)
{
    // counter
    int i;

    // the data_cmd register is used to load the transmit queue
    // data_cmd uses 10 bits: [0ccc xxxx xxxx]
    //   x's are for data sent to device
    //   c's are command bits
    // command bits 0100 (i.e. 0x4) issues a restart before sending data
    I2C_ptr->data_cmd = address + 0x400;
    for (i = 0; i < length; i++)
        // write each byte to transmit queue to send to device
        I2C_ptr->data_cmd = values[i];
    // this is a pretty general way of writing to I2C
}

// initialize GS device over I2C
void init_GS(void)
{
    // configure device for +/- 2g resolution
    // ADXL345 manual states:
    //   data format register is at address 0x31
    //   bit pattern is  0b00001000
    //      0 : no self-test
    //      0 : 3-wire SPI mode (irrelevant, we use I2C)
    //      0 : interrupts active high (irrelevant, interrupts not used)
    //      0 : unused
    //      1 : full resolution
    //      0 : right justification of bits
    //      0 : +/- 2 g resolution (with below)
    //      0 : +/- 2 g resolution (with above)
    write_over_I2C(0x31, 0x08);

    // configure device for 200 Hz sampling
    // ADXL345 manual states:
    //   sample register is at address 0x2C
    //   sample rate 200 Hz = code 0b1011 = 0x0B
    write_over_I2C(0x2C, 0x0B);

    // configure device to start measuring
    // ADXL345 manual states:
    //   power control register is 0x2D
    //   set bit 3 to start measuring
    //   other bits are for configuring sleep mode, not used here
    write_over_I2C(0x2D, 0x08);
}