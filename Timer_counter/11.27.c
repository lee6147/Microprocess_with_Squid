#include <iom128v.h>
#include <macros.h>

void delay_m(unsigned int m);
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);

void write_string(char *pt);
void NumbertoTwoDigit(int a);
void NumbertoFourDigit(int a);
void NumbertoHex(char a);
void NumbertoBinary(char a);
char Hex_convert(char a);
//void DectoFloat(int a);
char ADC_Converter(char channel);
int Get_OCR(int a);
void fOC_Float(int OCR);
unsigned int fan_level(int OCR);
void PWM_duty(int OCR);
void PWM_frequency(void);

unsigned int count, min, sec;
int OCR = 0;

void main(void)
{
    DDRG = 0x07; // 0000 0111, G2, G1, G0
    DDRC = 0xFF; // 1111 1111
    DDRB = 0x10;  //OCOpin (PORTB.4)
    EICRB = 0x0B; // 0000 1011, INT4 Rising, 5 Falling
    EIMSK = 0x30; // 0011 0000, INT4, 5 Enable

    ADMUX = (0<<REFS1) | (0<<REFS0) | (1<<ADLAR) | (0<<MUX4) |  (0<<MUX3) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0)
    ADCSRA = (0<<ADEN) | ADSC ADFR ADIF ADIE ADPS2 ADPS1 ADPS0
   
    lcd_init();

    TCCR0 = (1<<WGM01) | (1<<WGM00) | (1<<COM01) | (0<<COM00) | (0<<CS02) | (1<<CS01) | (1<<CS00) ;
    TIMSK = (1<<OCIE0) | (0<<TOIE0);
    //OC0 pin의 주파수는 31.25kHz = 16M / N(8) / 256 / 2(toggle)
    SREG |= 0X80;

    while(1){
        write_instruction(0xc3);
        write_string("DUTY = %");
        

        //write_instruction(0xC0);
        //PWM_frequency();

        //write_instruction(0x88);
        //PWM_duty(OCR0);

        delay_m(10);
    }//end of while loop
}//end of main loop

#pragma interrupt_handler int4_isr:iv_INT4
void int4_isr(void)
{
 //external interupt on INT4
    delay_m(200);
    if (OCR + 64 >= 256)
        OCR = 255;
    else if (OCR == 0)
        OCR += 63;
    else
        OCR += 64;
    OCR0 = OCR;
    EIFR = 0x10;
}

#pragma interrupt_handler int5_isr:iv_INT5
void int5_isr(void){
 //external interupt on INT5
    delay_m(200);
    if (OCR - 64 < 0)
        OCR = 0;
    else
        OCR -= 64;
    OCR0 = OCR;
    EIFR = 0x20;
}

#pragma interrupt_handler timer0_comp_isr:iv_TIM0_COMP
void timer0_comp_isr(void){
    count++;
    if(count == 1000){
        count = 0;
        sec++;
    }
}

#pragma interrupt_handler timer0_ovf_isr:iv_TIM0_OVF
void timer0_ovf_isr(void){
    // 16M / 128 / (256-131) = 1000
    count++;
    if(count == 1000) {
        sec++;
        count = 0;
    }
}

void PWM_frequency(void){
    long numerator = 1600000L;
    long denominator = (256 * 32);

    long result_int = numerator / denominator;
    int dec = result_int / 100;
    int flt = (int)result_int - ((long)dec * 100);

    char zpo = flt / 10;
    char zpzo = flt - (zpo * 10);
    NumbertoFourDigit(dec);
    write_data('.');
    write_data(0x30 + zpo);
    write_data(0x30 + zpzo);
    write_string("kHz");
}

void PWM_duty(int OCR) {
    int n = fan_level(OCR) * 25;
    NumbertoFourDigit(n);
    write_data('%');
}

unsigned int fan_level(int OCR){
    unsigned int n;
    if (OCR == 0)
        OCR -= 1;
    n = (OCR + 1) / 64;
    return n;
}

void fOC_Float(int OCR){
    long numerator = 1600000L;
    long denominator = (2 * 32) * (long)(1 + OCR);

    long result_int = numerator / denominator;
    int dec = result_int / 100;
    int flt = (int)result_int - ((long)dec * 100);

    char zpo = flt / 10;
    char zpzo = flt - (zpo * 10);
    NumbertoFourDigit(dec);
    write_data('.');
    write_data(0x30 + zpo);
    write_data(0x30 + zpzo);
    write_string("kHz");
}

int Get_OCR(int a) {
    long temp = ((long)a * 255) + 511;
    return (int)(temp / 1023);
}

/*void DectoFloat(int a) {
    float num = (a / 1023.0) * 5.0;
    int result = (int)(num * 100 + 0.005);
    char dummy_1 = result / 100;
    char dummy_0_1 = (result - (dummy_1 * 100)) / 10;
    char dummy_0_0_1 = (result - (dummy_1 * 100 + dummy_0_1 * 10)) / 1;
    write_data(0x30 + dummy_1);
    write_data('.');
    write_data(0x30 + dummy_0_1);
    write_data(0x30 + dummy_0_0_1);
}//1023 -> 5.00/ 0 -> 0.00*/

char ADC_Converter(char channel){
    if(channel == 0)
        ADMUX = 0xc0;
    else if (channel == 1)
        ADMUX = 0xc1;
    else if (channel == 2)
        ADMUX = 0xc2;

    ADMUX = (ADMUX & 0xef) | channel;
    ADCSRA |= (1<<ADSC);
    while(ADCSRA & 0x40);
    if (ADC <= 5)
        return 0;
    else
        return ADC;
}

void delay_m(unsigned int m){
    unsigned int i, j;
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < 2100; j++)
            ;
    }
}

void write_data(char d){
    PORTG =0x06;
    delay_m(1);
    PORTC =d;
    delay_m(1);
    PORTG = 0x04;
}

void write_instruction(char i){
    PORTG =0x02;
    delay_m(1);
    PORTC =i;
    delay_m(1);
    PORTG = 0x00;
}

void lcd_init(void){
    write_instruction(0x30); // 8-bit, 1 line, 5x8 font
    delay_m(15);
    write_instruction(0x30);
    delay_m(1);
    write_instruction(0x30);
    delay_m(1);
    write_instruction(0x30);
    delay_m(1);
    write_instruction(0x38); //8-bit, 2lines, 5x8 font
    delay_m(1);
    write_instruction(0x0e); //8-bit, 2lines, 5x8 font
    delay_m(1);
    write_instruction(0x01); //8-bit, 2lines, 5x8 font
    delay_m(1);
}

void NumbertoTwoDigit(int a){
    char dummy_10;
    char dummy_1;

    dummy_10 = a / 100;
    a = a - dummy_10 * 100;

    dummy_10 = a / 10;
    dummy_1 = (a - dummy_10 * 10) / 1;


    if (dummy_10 >= 10) dummy_10 = 0;

    if (dummy_10 == 0)
        write_data(' ');
    else {
        write_data(dummy_10 + 0x30);
    }
    write_data(dummy_1 + 0x30);
}

void write_string(char *pt){
    while(*pt >= ' ' && *pt <= '~'){
        write_data(*pt);
        pt++;
    }
}

void NumbertoFourDigit(int a){
    char dummy_1000, dummy_100, dummy_10, dummy_1;

    dummy_1000 = a / 10000;
    a = a - dummy_1000 * 10000;

    dummy_1000 = a / 1000;
    dummy_100 = (a - dummy_1000 * 1000) / 100;
    dummy_10 = (a - (dummy_1000 * 1000 + dummy_100 * 100)) / 10;
    dummy_1 = (a - (dummy_1000 * 1000 + dummy_100 * 100 + dummy_10 * 10)) / 1;

    if (dummy_1000 >= 10){
        dummy_1000 = dummy_100 = dummy_10 = 0;
    }
    if (dummy_100 >= 10){
        dummy_100 = dummy_10 = 0;
    }
    if (dummy_10 >= 10){
        dummy_10 = 0;
    }

    if (dummy_1000 == 0)
        write_data(' ');
    else {
        write_data(dummy_1000 + 0x30);
    }

    if (dummy_1000 == 0 && dummy_100 == 0)
        write_data(' ');
    else {
        write_data(dummy_100 + 0x30);
    }

    if (dummy_1000 == 0 && dummy_100 == 0 && dummy_10 == 0)
        write_data(' ');
    else {
        write_data(dummy_10 + 0x30);
    }

    write_data(dummy_1 + 0x30);
}

void NumbertoHex(char a){
    char hex_value[4] = {'0', 'x', '0', '0'};
    hex_value[2] = a / 16;
    hex_value[3] = a - (hex_value[2] * 16);

    write_data(Hex_convert(hex_value[2]));
    write_data(Hex_convert(hex_value[3]));
}

void NumbertoBinary(char a){

    char bin_value[8] = {'0', '0', '0', '0', '0', '0', '0', '0'};

    bin_value[0] = a / 128;
    bin_value[1] = (a - bin_value[0] * 128) / 64;
    bin_value[2] = (a - bin_value[0] * 128 - bin_value[1] * 64) / 32;
    bin_value[3] = (a - bin_value[0] * 128 - bin_value[1] * 64 - bin_value[2] * 32) / 16;
    bin_value[4] = (a - bin_value[0] * 128 - bin_value[1] * 64 - bin_value[2] * 32 - bin_value[3] * 16) / 8;
    bin_value[5] = (a - bin_value[0] * 128 - bin_value[1] * 64 - bin_value[2] * 32 - bin_value[3] * 16 - bin_value[4] * 8) / 4;
    bin_value[6] = (a - bin_value[0] * 128 - bin_value[1] * 64 - bin_value[2] * 32 - bin_value[3] * 16 - bin_value[4] * 8- bin_value[5] * 4) / 2;
    bin_value[7] = (a - bin_value[0] * 128 - bin_value[1] * 64 - bin_value[2] * 32 - bin_value[3] * 16 - bin_value[4] * 8- bin_value[5] * 4- bin_value[6] * 2);

    write_data(bin_value[0] + 0x30);
    write_data(bin_value[1] + 0x30);
    write_data(bin_value[2] + 0x30);
    write_data(bin_value[3] + 0x30);
    write_data(bin_value[4] + 0x30);
    write_data(bin_value[5] + 0x30);
    write_data(bin_value[6] + 0x30);
    write_data(bin_value[7] + 0x30);
}

char Hex_convert(char a){
    if (a <= 9) return '0' + a;
    else return ('A' + a - 10);
}
