#include <iom128v.h>
#include <macros.h>

void delay_m(unsigned int m);
void write_data(char d);
void write_instruction(char i);
void write_string(char *pt);
void lcd_init(void);
void NumbertoTwodigit(int a);
void NumbertoFourdigit(int a);
void NumbertoBinary(char a);
void NumbertoHex(int a);
void write_string(char *pt);

unsigned char ADC_result;
char number[10] = {0x04, 0x2f, 0x18, 0x09, 0x23, 0x41, 0x40, 0x07, 0x00, 0x01};

int num=0;

int main(void)
{
    DDRD = 0x7f;
    DDRG = 0x07; // 0000 0111
    DDRC = 0xff; //lcd연결부
    DDRB = 0x10; //OCR0출력 연결부

    lcd_init();

    OCR0 = 0;
    SREG |= 0x80;

    TCCR0 = (1<<WGM01)|(1<<WGM00)|(1<<COM01)|(1<<COM00)|(0<<CS02)|(1<<CS01)|(1<<CS00); //fast, 500kHz , 1/500k tcnt/sec
    TIMSK = (1<<OCIE0)|(0<<TOIE0);
    ADMUX = (1 << REFS1)|(1 << REFS0)|(1 << ADLAR)|(0 << MUX4)|(0 << MUX3)|(0 << MUX2)|(0 << MUX1)|(1 << MUX0);
    ADCSRA = (1 << ADEN)|(1 << ADSC)|(1 << ADFR)|(0 << ADIF)|(1 << ADIE)|(1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0);

    while(1){
        write_instruction(0x80);
        NumbertoFourdigit(ADC_result);
        PORTD = number[0];
        OCR0=0;
        if(ADC_result > 80)
        {
            for(num = 5; num >= 0; num--)
            {
                PORTD = number[num];
                delay_m(1000);
            }
            OCR0=255;
            for(num = 0; num <= 5; num++)
            {
                PORTD = number[num];
                delay_m(1000);
            }
            OCR0=0;
        }

        delay_m(100);
    }
    return 0;
}

#pragma interrupt_handler adc_isr:iv_ADC
void adc_isr(void)
{
    ADC_result = ADCH;
}

#pragma interrupt_handler timer0_ovf_isr:iv_TIM0_OVF
void timer0_ovf_isr(void)
{

}

#pragma interrupt_handler timer0_comp_isr:iv_TIM0_COMP
void timer0_comp_isr(void)
{

}

void NumbertoBinary(char a){
    char arr[8];
    int i;
    for(i = 7; i >= 0; i--)
        {
             arr[i] = (a - (a / 2) * 2);
             a = a / 2;
        }
    for(i = 0; i <= 3; i++)
        write_data(arr[i]+0x30);
    write_data(' ');
    for(i = 4; i <= 7; i++)
        write_data(arr[i]+0x30);
}

void NumbertoHex(int a){
    int dummy1, dummy2;
    dummy1 = a / 16;
    dummy2 = a - 16 * dummy1;
    write_data('0');
    write_data('x');
    if(dummy1 >= 10)
        write_data('a'+(dummy1 - 10));
    else
        write_data('0'+dummy1);
    if(dummy2 >= 10)
        write_data('a'+(dummy2 - 10));
    else
        write_data('0'+dummy2);
}



void NumbertoTwodigit(int a)
{
    char dummy_10, dummy_1;
    dummy_10 = a / 10;
    dummy_1 = (a - dummy_10 * 10) / 1;
    if(dummy_10 == 0)
        write_data('0');
    else
        write_data(dummy_10 + 0x30);
    write_data(dummy_1 + 0x30);
}

void NumbertoFourdigit(int a)
{
    char dummy_1000, dummy_100, dummy_10, dummy_1;
    dummy_1000 = a / 1000;
    dummy_100 = (a - dummy_1000 * 1000) / 100;
    dummy_10 = (a - dummy_1000 * 1000 - dummy_100 * 100) / 10;
    dummy_1 = (a - dummy_1000 * 1000 - dummy_100 * 100 - dummy_10 * 10) / 1;
    if(dummy_1000 == 0)
        write_data(' ');
    else
        write_data(dummy_1000 + 0x30);
    if(dummy_1000 == 0 && dummy_100 == 0)
        write_data(' ');
    else
        write_data(dummy_100 + 0x30);
    if(dummy_1000 == 0 && dummy_100 == 0 && dummy_10 == 0)
        write_data(' ');
    else
        write_data(dummy_10 + 0x30);
    write_data(dummy_1 + 0x30);
}

void write_string(char *pt)
{
    while(((*pt >= ' ')&&(*pt <= '~'))){
        write_data(*pt);
        pt++;
    }
}

void delay_m(unsigned int m){
    unsigned int i, j;
    for(i = 0; i < m; i++){
        for(j = 0; j < 2100; j++)
            ;
    }
}

void write_data(char d){
    PORTG = 0x06;
    delay_m(1);
    PORTC = d;
    delay_m(1);
    PORTG = 0x04;
}

void write_instruction(char i){
    PORTG = 0x02;
    delay_m(1);
    PORTC = i;
    delay_m(1);
    PORTG = 0x00;
}

void lcd_init(void){
    write_instruction(0x30);
    delay_m(15);
    write_instruction(0x30);
    delay_m(1);
    write_instruction(0x30);
    delay_m(1);
    write_instruction(0x30);
    delay_m(1);
    write_instruction(0x38);
    delay_m(1);
    write_instruction(0x0e);
    delay_m(1);
    write_instruction(0x01);
    delay_m(1);
}
