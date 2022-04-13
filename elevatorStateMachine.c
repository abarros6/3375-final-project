/*
designing the elevator control system as an FSM

STATES: q0 = idle-state, q1-q7 = next-floor-state, q* = passcode-entered-state, qE = emergency-state

INPUTS: switches 3-9 map to floors 1-7. Buttons 0-3 are mapped to passcode input.
		potentiometer attached to adc is mapped to the elevator weight sensor.

TRANSITIONS: q0 (switches 3-9)-> q1-7 // q0 (buttons 0-3)-> q* // q0 (switch 2)-> qE
			 q1-7 (e)-> q0 // q1-7 (switch 2)-> qE
			 q* (switches 3-9)-> q1-7 // q* (switch 2)-> qE
			 qE (switch 2)-> q0


*/

// all the memory locations needed
#define LED_BASE 0xFF200000;
#define ADC_BASE 0xFF204000;
#define KEY_BASE 0xFF200050;
#define SW_BASE 0xFF200040;
#define GPIO_BASE 0xFF200060;
#define AUDIO_BASE 0xFF203040;

// define constants for audio
#define BUF_SIZE 80000	 // about 10 seconds of buffer (@ 8K samples/sec)
#define BUF_THRESHOLD 96 // 75% of 128 word buffer

// first need to create pointers to all of the parts of the board in use
volatile int *led_ptr = (int *)LED_BASE;	 // pointer for the LEDs
volatile int *sw_ptr = (int *)SW_BASE;		 // pointer for the switches
volatile int *btn_ptr = (int *)KEY_BASE;	 // pointer for the buttons
volatile int *adc_ptr = (int *)ADC_BASE;	 // pointer for the ADC
volatile int *gpio_ptr = (int *)GPIO_BASE;	 // pointer for the gpio pins
volatile int *audio_ptr = (int *)AUDIO_BASE; // pointer for audio

// gloabal values
volatile int flag;
volatile int state; // use this value to hold the current state
volatile int floor;

/////////////////HELPER FUNCTIONS

// want to write a function to turn on led n
int LedOn(int light)
{
	(*(led_ptr)) = light;
}

int ReadSwitches(void)
{
	return (*(sw_ptr));
}

int ReadButton(int btn)
{
	// Returns 1 if the given button is pressed, 0 otherwise due to & operator
	return ((*btn_ptr >> (btn)) & 1);
}

int HandlePassword(void)
{
	// this if statement controls the password values
	// in this case buttons 0,2,3 need to be pressed
	if (ReadButton(0) && ReadButton(2) && ReadButton(3))
	{
		return 1;
	}

	return 0;
}

int adcControl(void)
{

	if (ReadSwitches() == 0b10) // carriage position in terms of floor
	{
		flag = 0;
	}
	else if (ReadSwitches() == 0b1) // weight sensing attribute
	{
		flag = 1;
	}
}

void SoundAlarm()
{
	/*used for audio record/playback*/
	int fifospace;
	int buffer_index = 0;
	int left_buffer[BUF_SIZE];
	int right_buffer[BUF_SIZE]; /*read and echo audio data*/

	fifospace = *(audio_ptr + 1);				  // read the audio port fifospace register
	if ((fifospace & 0x00FF0000) > BUF_THRESHOLD) // check WSRC
	{
		// output data until the buffer is empty or the audio-out FIFO is full
		while ((fifospace & 0x00FF0000) && (buffer_index < BUF_SIZE))
		{
			*(audio_ptr + 2) = left_buffer[buffer_index];
			*(audio_ptr + 3) = right_buffer[buffer_index];
			++buffer_index;
			fifospace = *(audio_ptr + 1); // read the audio port fifospace register
		}
	}
}

void DisplayResults()
{
	int rMask = 0b1000000000000000;
	int max = 4096; // 2^12 -> bits 0-11 are the 12 data bits of the adc channels
					// therefore the max data that the potwntiometer can output is 12 bits worth of information

	if ((flag == 0) && (((*(adc_ptr)) && (rMask) >> 15) == 1)) // floor sensing
	{
		if ((*(adc_ptr) == 0))
		{
			*(gpio_ptr) = 0b1000000000; // floor 1
			floor = 1;
		}
		else if ((*(adc_ptr) > 0) && (*(adc_ptr) < (max * 1 / 10)))
		{
			*(gpio_ptr) = 0b1100000000; // floor 2
			floor = 2;
		}
		else if ((*(adc_ptr) >= (max * 1 / 10)) && (*(adc_ptr) < (max * 2 / 10)))
		{
			*(gpio_ptr) = 0b1110000000; // floor 3
			floor = 3;
		}
		else if ((*(adc_ptr) >= (max * 2 / 10)) && (*(adc_ptr) < (max * 3 / 10)))
		{
			*(gpio_ptr) = 0b1111000000; // floor 4
			floor = 4;
		}
		else if ((*(adc_ptr) >= (max * 3 / 10)) && (*(adc_ptr) < (max * 4 / 10)))
		{
			*(gpio_ptr) = 0b1111100000; // floor 5
			floor = 5;
		}
		else if ((*(adc_ptr) >= (max * 4 / 10)) && (*(adc_ptr) < (max * 5 / 10)))
		{
			*(gpio_ptr) = 0b1111110000; // floor 6
			floor = 6;
		}
		else if ((*(adc_ptr) >= (max * 5 / 10)) && (*(adc_ptr) < (max * 6 / 10)))
		{
			*(gpio_ptr) = 0b1111111000; // floor 7
			floor = 7;
		}
	}
	else if ((flag == 1) && (((*(adc_ptr + 1)) && (rMask) >> 15) == 1)) // weight sensing
	{
		if ((*(adc_ptr + 1) == 0))
		{
			*(gpio_ptr) = 0b0000000000;
		}
		else if ((*(adc_ptr + 1) > 0) && (*(adc_ptr + 1) < (max * 1 / 10)))
		{
			*(gpio_ptr) = 0b1000000000;
		}
		else if ((*(adc_ptr + 1) >= (max * 1 / 10)) && (*(adc_ptr + 1) < (max * 2 / 10)))
		{
			*(gpio_ptr) = 0b1100000000;
		}
		else if ((*(adc_ptr + 1) >= (max * 2 / 10)) && (*(adc_ptr + 1) < (max * 3 / 10)))
		{
			*(gpio_ptr) = 0b1110000000;
		}
		else if ((*(adc_ptr + 1) >= (max * 3 / 10)) && (*(adc_ptr + 1) < (max * 4 / 10)))
		{
			*(gpio_ptr) = 0b1111000000;
		}
		else if ((*(adc_ptr + 1) >= (max * 4 / 10)) && (*(adc_ptr + 1) < (max * 5 / 10)))
		{
			*(gpio_ptr) = 0b1111100000;
		}
		else if ((*(adc_ptr + 1) >= (max * 5 / 10)) && (*(adc_ptr + 1) < (max * 6 / 10)))
		{
			*(gpio_ptr) = 0b1111110000;
		}
		else if ((*(adc_ptr + 1) >= (max * 6 / 10)) && (*(adc_ptr + 1) < (max * 7 / 10)))
		{
			*(gpio_ptr) = 0b1111111000;
		}
		else if ((*(adc_ptr + 1) >= (max * 7 / 10)) && (*(adc_ptr + 1) < (max * 8 / 10)))
		{
			*(gpio_ptr) = 0b1111111100;
		}
		else if ((*(adc_ptr + 1) >= (max * 8 / 10)) && (*(adc_ptr + 1) < (max * 9 / 10)))
		{
			*(gpio_ptr) = 0b1111111110;
		}
		else if ((*(adc_ptr + 1) >= (max * 9 / 10)) && (*(adc_ptr + 1) < (max))) // max weight limit imposed
		{
			*(gpio_ptr) = 0b1111111111;
			SoundAlarm();
		}
	}
}
///////////////////END OF HELPER FUNCTIONS AREA

/////////////////STATE FUNCTIONS
int EpsilonTransition(void)
{
	for (int i = 0; i < 5000; i++)
	{
		// do nothing for a little
	}
	state = 0;
}

int HandleTransition(void)
{
	if (ReadSwitches() == 0b100) // switch 2 is mapped to the emergency button
	{
		state = 3;
	}

	if (HandlePassword() & 1)
	{
		state = 2;
	}

	if (ReadSwitches() == 0b1000) // switch 3 == floor 1
	{
		LedOn(0b1000);
		state = 1;
	}
	else if (ReadSwitches() == 0b10000) // switch 4 == floor 2
	{
		LedOn(0b10000);
		state = 1;
	}
	else if (ReadSwitches() == 0b100000) // switch 5 == floor 3
	{
		LedOn(0b100000);
		state = 1;
	}
	else if (ReadSwitches() == 0b1000000) // switch 6 == floor 4
	{
		LedOn(0b1000000);
		state = 1;
	}
	else if (ReadSwitches() == 0b10000000) // switch 7 == floor 5
	{
		LedOn(0b10000000);
		state = 1;
	}
	else if (ReadSwitches() == 0b100000000) // switch 8 == floor 6
	{
		LedOn(0b100000000);
		state = 1;
	}
	else if (ReadSwitches() == 0b1000000000) // switch 9 == floor 7
	{
		LedOn(0b1000000000);
		state = 1;
	}
}

int IdleState()
{
	LedOn(0b1); // turn on led0 to indicate in idle state
	HandleTransition();
}

int NextFloorState()
{
	EpsilonTransition();
}

int PasscodeEnteredState()
{
	LedOn(0b10); // turn on led 1 to indicate password entered correctly
	HandleTransition();
}

int EmergencyState()
{
	LedOn(0b100);
	SoundAlarm();
	if (ReadSwitches() != 0b100)
	{
		EpsilonTransition();
	}
}
////////////////END OF STATE FUNCTIONS

////////////MAIN PROGRAM EXECUTION
int main(void)
{

	// Idle-state = 0
	// q1-7 = 1
	// q* = 2
	// qE = 3

	/////////init the gpio and adc values
	*(adc_ptr) = 0b1;
	*(adc_ptr + 1) = 0b1;
	*(gpio_ptr + 1) = 0b1111111111;

	while (1)
	{
		switch (state)
		{
		case 0:
			// idle state
			IdleState();
			break;

		case 1:
			// q1-7
			NextFloorState();
			break;

		case 2:
			// q*
			PasscodeEnteredState();
			break;

		case 3:
			// qE
			EmergencyState();
			break;
		}
		/*
		adcControl();	  // update the adc
		DisplayResults(); // display the results through the GPIO pins
		*/
	}
}
//////////////////END OF MAIN PROGRAM