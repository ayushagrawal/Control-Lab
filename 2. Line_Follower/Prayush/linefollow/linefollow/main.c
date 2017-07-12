#include "lcd.h"
#include "motion.h"
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>

unsigned char ADC_Value;
unsigned char leftLine,rightLine,centerLine = 0;

int error = 0;
int prev_error = 0;
int integral_error = 0;
int diff_error = 0;
float pid = 0;

int flag = 0;
int byepass = 0;

#define MAX_SPEED 127

float Kp = 7;
float Ki = 0;
float Kd = 10;

/*int min(int a, int b){
	return (a < b) ? a : b;
}

int max(int a, int b){
	return (a > b) ? a : b;
}*/
//Function to Initialize ADC
void adc_init()
{
	ADCSRA = 0x00;
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//ADC pin configuration
void adc_pin_config (void)
{
	DDRA = 0x00;   //set PORTF direction as input
	PORTA = 0x00;  //set PORTF pins floating
}

//This Function accepts the Channel Number and returns the corresponding Analog Value
unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10;      //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	return a;
}

// Used for velocity control
//TIMER1 initialize - prescale:64
// WGM: 5) PWM 8bit fast, TOP=0x00FF
// desired value: 450Hz
// actual value: 450.000Hz (0.0%)
void timer1_init(void)
{
	TCCR1B = 0x00; //stop
	TCNT1H = 0xFF; //setup
	TCNT1L = 0x01;
	OCR1AH = 0x00;
	OCR1AL = 0xFF;
	OCR1BH = 0x00;
	OCR1BL = 0xFF;
	ICR1H  = 0x00;
	ICR1L  = 0xFF;
	TCCR1A = 0xA1;
	TCCR1B = 0x0D; //start Timer
}

void init()
{
	cli();					// Clears the global interrupts
	lcd_port_config();
	adc_init();
	adc_pin_config();
	timer1_init();
	motion_pin_config();	// Refer the function
	sei();					// Enables the global interrupts
	lcd_set_4bit();			// Setting the LCD to 4 bit mode
	lcd_init();
}


// Used for defining the error function
//	Uses global values of the ADC values and outputs in the global "error" variable
void compute_error(void)
{
	error = rightLine - leftLine;
	byepass = 0;
	if(centerLine >100 && rightLine < 20 && leftLine < 20)		// If center is the only one on black line
	{
		flag = 1;
	}
	
	if((centerLine < 15 && rightLine< 15 && leftLine< 15))			// If everyone is on white line
	{
		velocity(MAX_SPEED,MAX_SPEED);
		back();
		byepass = 1;
	}
	if((centerLine > 80 && rightLine > 80 && leftLine > 80))		// If everything is on black line
	{	
		for(int i =0; i<5;i++)
		{
			velocity(MAX_SPEED,MAX_SPEED);
			forward();
			byepass = 1;
		}
	}
	if(rightLine > 60 && leftLine > 60 && centerLine < 15)			// If right and left are on black line and center is not
	{
		velocity(2*MAX_SPEED,2*MAX_SPEED);
		forward();
		byepass = 1;
	}
}

// Used for controlling the motors depending on the value of the "pid" variable
void motor_control(void)
{
	if (flag==1)
	{
		velocity(MAX_SPEED*2,MAX_SPEED*2);
		forward();
		flag = 0;
	}
	else if((leftLine > rightLine) && byepass == 0)
	{
		velocity(MAX_SPEED/4 + pid,MAX_SPEED/4 + pid);
		left();
	}
	else if((rightLine > leftLine) && byepass == 0)
	{
		velocity(MAX_SPEED/4 + pid,MAX_SPEED/4 + pid);
		right();
	}
	/*lcd_print(2,1,MAX_SPEED+pid,3);
	lcd_print(2,5,MAX_SPEED+pid,3);*/

}

int main(void)
{
	init();
	lcd_string("Starting SPARK V");
	_delay_ms(1000);
	lcd_init();
	forward();
	velocity((char)MAX_SPEED*2,(char)MAX_SPEED*2);
	while(1)
	{
		leftLine = ADC_Conversion(3);
		centerLine = ADC_Conversion(4);
		rightLine = ADC_Conversion(5);
		compute_error();
		/*lcd_print(1,1,leftLine,3);
		lcd_print(1,5,centerLine,3);
		lcd_print(1,9,rightLine,3);*/
		
		// Till now we would have the value of the error

		/****************************** P.I.D. ******************************/
		integral_error = integral_error + error;
		diff_error = error - prev_error;
		pid = Kp*error + Ki*integral_error + Kd*diff_error;
		prev_error = error;
		/****************************** P.I.D. ******************************/
		motor_control();
		//_delay_ms(10);
	}

	return 0;
}