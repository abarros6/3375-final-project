#define ADC_BASE 0xFF204000
#define Mask_12_bits 0x00000FFF
/* This program
 * 1. updates the ADC in single conversion mode and
 * 2. reads the corresponding  values from  channels 0, 2, and 3
 */
int main(void)
{

    volatile int *ADC_BASE_ptr = (int *)ADC_BASE;
    int mask = Mask_12_bits;
    volatile int channel0, channel2, channel3;

    *(ADC_BASE_ptr) = 0; // write anything to channel 0 to update ADC

    channel0 = (*(ADC_BASE_ptr)&mask);
    channel2 = (*(ADC_BASE_ptr + 2) & mask);
    channel3 = (*(ADC_BASE_ptr + 3) & mask);
}
