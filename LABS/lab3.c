#define GPIO_BASE 0xFF200060
#define ADC_BASE 0xFF204000
#define SW_BASE 0xFF200040

#define LED_BASE 0xFF200000

volatile int *gpio_ptr = (int *)GPIO_BASE;
volatile int *adc_ptr = (int *)ADC_BASE;
volatile int *sw_ptr = (int *)SW_BASE;

volatile int *led_ptr = (int *)LED_BASE;

volatile int flag;

int main(void)
{

	*(adc_ptr) = 0b1;
	*(adc_ptr + 1) = 0b1;
	*(gpio_ptr + 1) = 0b1111111111;

	while (1)
	{
		int switchValue = *(sw_ptr);
		int rMask = 0b1000000000000000;

		int max = 4096; // 2^12 -> bits 0-11 are the 12 data bits of the adc channels
		// therefore the max data that the potwntiometer can output is 12 bits worth of information

		if (switchValue == 0x0)
		{
			flag = 0;
			*(led_ptr) = 0b1;
		}
		else if (switchValue == 0x1)
		{
			flag = 1;
			*(led_ptr) = 0b10;
		}

		if ((flag == 0) && (((*(adc_ptr)) && (rMask) >> 15) == 1))
		{

			if ((*(adc_ptr) == 0))
			{
				*(gpio_ptr) = 0b0000000000;
			}
			else if ((*(adc_ptr) > 0) && (*(adc_ptr) < (max * 1 / 10)))
			{
				*(gpio_ptr) = 0b0000000001;
			}
			else if ((*(adc_ptr) >= (max * 1 / 10)) && (*(adc_ptr) < (max * 2 / 10)))
			{
				*(gpio_ptr) = 0b0000000011;
			}
			else if ((*(adc_ptr) >= (max * 2 / 10)) && (*(adc_ptr) < (max * 3 / 10)))
			{
				*(gpio_ptr) = 0b0000000111;
			}
			else if ((*(adc_ptr) >= (max * 3 / 10)) && (*(adc_ptr) < (max * 4 / 10)))
			{
				*(gpio_ptr) = 0b0000001111;
			}
			else if ((*(adc_ptr) >= (max * 4 / 10)) && (*(adc_ptr) < (max * 5 / 10)))
			{
				*(gpio_ptr) = 0b0000011111;
			}
			else if ((*(adc_ptr) >= (max * 5 / 10)) && (*(adc_ptr) < (max * 6 / 10)))
			{
				*(gpio_ptr) = 0b0000111111;
			}
			else if ((*(adc_ptr) >= (max * 6 / 10)) && (*(adc_ptr) < (max * 7 / 10)))
			{
				*(gpio_ptr) = 0b0001111111;
			}
			else if ((*(adc_ptr) >= (max * 7 / 10)) && (*(adc_ptr) < (max * 8 / 10)))
			{
				*(gpio_ptr) = 0b0011111111;
			}
			else if ((*(adc_ptr) >= (max * 8 / 10)) && (*(adc_ptr) < (max * 9 / 10)))
			{
				*(gpio_ptr) = 0b0111111111;
			}
			else if ((*(adc_ptr) >= (max * 9 / 10)) && (*(adc_ptr) < (max)))
			{
				*(gpio_ptr) = 0b1111111111;
			}
		}
		else if ((flag == 1) && (((*(adc_ptr + 1)) && (rMask) >> 15) == 1))
		{

			if ((*(adc_ptr + 1) == 0))
			{
				*(gpio_ptr) = 0b0000000000;
			}
			else if ((*(adc_ptr + 1) > 0) && (*(adc_ptr + 1) < (max * 1 / 10)))
			{
				*(gpio_ptr) = 0b0000000001;
			}
			else if ((*(adc_ptr + 1) >= (max * 1 / 10)) && (*(adc_ptr + 1) < (max * 2 / 10)))
			{
				*(gpio_ptr) = 0b0000000011;
			}
			else if ((*(adc_ptr + 1) >= (max * 2 / 10)) && (*(adc_ptr + 1) < (max * 3 / 10)))
			{
				*(gpio_ptr) = 0b0000000111;
			}
			else if ((*(adc_ptr + 1) >= (max * 3 / 10)) && (*(adc_ptr + 1) < (max * 4 / 10)))
			{
				*(gpio_ptr) = 0b0000001111;
			}
			else if ((*(adc_ptr + 1) >= (max * 4 / 10)) && (*(adc_ptr + 1) < (max * 5 / 10)))
			{
				*(gpio_ptr) = 0b0000011111;
			}
			else if ((*(adc_ptr + 1) >= (max * 5 / 10)) && (*(adc_ptr + 1) < (max * 6 / 10)))
			{
				*(gpio_ptr) = 0b0000111111;
			}
			else if ((*(adc_ptr + 1) >= (max * 6 / 10)) && (*(adc_ptr + 1) < (max * 7 / 10)))
			{
				*(gpio_ptr) = 0b0001111111;
			}
			else if ((*(adc_ptr + 1) >= (max * 7 / 10)) && (*(adc_ptr + 1) < (max * 8 / 10)))
			{
				*(gpio_ptr) = 0b0011111111;
			}
			else if ((*(adc_ptr + 1) >= (max * 8 / 10)) && (*(adc_ptr + 1) < (max * 9 / 10)))
			{
				*(gpio_ptr) = 0b0111111111;
			}
			else if ((*(adc_ptr + 1) >= (max * 9 / 10)) && (*(adc_ptr + 1) < (max)))
			{
				*(gpio_ptr) = 0b1111111111;
			}
		}
	}
}