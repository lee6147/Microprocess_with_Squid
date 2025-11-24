//1121_Timer Counter LED
#include <iom128v.h>
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);
void NumbertoTwoDigit(int a);
void NumbertoFourDigit(int a);
void write_string(char *pt);
void NumbertoBinary(char a);
void NumbertoHex(char a);
void Hex_convert(char a);
void delay_m(unsigned int m);

unsigned int ADC_Converter(char channel);
unsigned int ADC_result, OCR;
void DectoFloat(int a);

char dummy_1000, dummy_100, dummy_10, dummy_1;
unsigned int sec, min, count;
unsigned int volt_digital;

unsigned char duty_OCR[5] = {0, 64, 127, 191, 255};
unsigned char duty_percent[5] = {0, 25, 50, 75, 100};
int state = 0;

int main(void)
{
    DDRG = 0x07;
    DDRC = 0xff;
    DDRB = 0x10;
    EICRB = 0x0a;
    EIMSK = 0x30;
    lcd_init();

    ADMUX = ( (1 << REFS1) | (1 << REFS1) | (0 << ADLAR) | (1 << MUX0) );
    ADCSRA = ((1 << ADEN) | (0 << ADSC) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));

    TCCR0 = ((0 << WGM01) | (1 << WGM00) |(1 << COM01) | (0 << COM00) | (0 << CS02) | (1 << CS01) | (0 << CS00));
    SREG |= 0x80;

    while(1)
    {
        OCR0 = duty_OCR[state];
        write_instruction(0x80);
        NumbertoTwoDigit(state);
        write_string(" F: 3.92kHz"); // 주파수 작성 
        write_instruction(0xC0);
        write_string("Duty:");
        NumbertoFourDigit(duty_percent[state]);
        write_string("%");
    }
}

#pragma interrupt_handler int4_isr:iv_INT4
void int4_isr(void)
{
    delay_m(200);
    if(state < 4)
    {
        state++;
    }
    EIFR = 0x10;
}

#pragma interrupt_handler int5_isr:iv_INT5
void int5_isr(void)
{
    delay_m(200);
    if(state > 0)
    {
        state--;
    }
    EIFR = 0x20;
}