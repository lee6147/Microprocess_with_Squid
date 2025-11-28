#include <iom128v.h>
#include <macros.h>

void delay_m(unsigned int m);
void write_data(char d);
void write_instruction (char i);
void lcd_init(void);
void write_string(char *pt);    //문자열 합치기
void NumbertoTwoDigit(int a);   //숫자값을 2개의 digit으로 변환
void NumbertoFourDigit(int a);  //숫자값을 4자리로 변환
void NumbertoHex(char a);       //숫자값을 16진수로 변환 0x00~0xff
void NumbertoBinary(char a);    //숫자값을 binary로 변환
char Hex_convert(char a);       //16진수 a~f까지의 영문값 변환 HEX안에 넣기
void DectoFlaot(int a);
char ADC_Converter(char channel); //unsigned int인 이유 : ADC컨버터가 10비트라서.
char ADC_result;
void OCR0toFloat(unsigned char a);

unsigned char number[10] = {0x04, 0x2f, 0x18, 0x09, 0x23, 0x41, 0x40, 0x07, 0x00, 0x01};
int sec,min,count;
float F_oc;
void main(void)
{
    DDRG = 0x07;
    DDRC = 0xff;
    DDRB = 0x10; // OC0 pin (PORTB.4)
    lcd_init();

/*  ADMUX = ((1<<REFS1)|(1<<REFS0)|(0<<ADLAR)|(0<<MUX1)|(0<<MUX0)); //1100 0000, ADC0
    ADCSRA = ((1<<ADEN)|(0<<ADSC)|(0<<ADFR)|(0<<ADIF)|(0<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); // 1000 0111, 0x87

    TCCR0 = ((1<<WGM01)|(0<<WGM00)|(0<<COM01)|(1<<COM00)|(0<<CS02)|(0<<CS01)|(1<<CS00)); // %1, 16MHz
    TIMSK = ((1<<OCIE0)|(0<<TOIE0)); //
    //OCR0 = 128; //OC0 PIN의 주파수 = 31.25KHz = 16MHz / N(분주비) / 256 / 2(토글때문)
    OCR0 = 0;
    SREG |= 0x80;
    //주파수는 1.953kHz
*/
    ADMUX = ((1<<REFS1)|(1<<REFS0)|(1<<ADLAR)|(0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0));
    ADCSRA = ((1<<ADEN)|(0<<ADSC)|(0<<ADFR)|(0<<ADIF)|(0<<ADIE)|(1<<ADPS2)|(0<<ADPS1)|(1<<ADPS0)); //32분주
    SREG |= 0x80;
    TCCR0 = ((1<<WGM01)|(1<<WGM00)|(1<<COM01)|(0<<COM00)|(1<<CS02)|(1<<CS01)|(0<<CS00)); // %1, 16MHz
    TIMSK = ((1<<OCIE0)|(0<<TOIE0));


    while(1){
    ADC_result = ADC_Converter(0); // 가변저항 사용
    OCR0 = ADC_result;


    write_instruction(0xc3);
    write_string("Duty = ");
    NumbertoFourDigit(100-(OCR0*100/256));
    write_data('%');

    //두번째 줄 가운데에서 좌측으로 3칸 떨어진 곳에
    //Duty = _%라고 적어주기.
    }

}




#pragma interrupt_handler timer0_comp_isr:iv_TIM0_COMP
void timer0_comp_isr(void){

}

#pragma interrupt_handler timer0_ovf_isr:iv_TIM0_OVF
void timer0_ovf_isr(void)
{
    //PORTB++;
    //16M / 8 / 256 = 7812.5KHz
/*  TCNT0 = 131; // 16M / 128 / (256-131) = 1000
    count++;
    if(count == 1000){
        count=0;
        sec++;
    }
*/
}   // 16M / ?? / 256-??  = 1000인 ??찾기

#pragma interrupt_handler adc_isr:iv_ADC
void adc_isr(void){

}

void OCR0toFloat(unsigned char a){
    float b, dummy_all;
    int c, dummy_st, dummy_nd;

    b = 16.0*1000.0/1.0/(a+1.0)/2.0;
    c = 16.0*1000.0/1.0/(a+1.0)/2.0; // c = (int)b;
    dummy_all = b-c;
    dummy_st = dummy_all*10;
    dummy_nd = (dummy_all*100) - (dummy_st*10);
    NumbertoFourDigit(c);
    write_data('.');
    write_data(dummy_st+0x30);
    write_data(dummy_nd+0x30);

}

void DectoFlaot(int a){
//1023 -> 5.00, 0 -> 0.00이 나오는 함수 204.8 -> 1.00 // 0.0048828125
    float volt;
    int integer, st, nd;

    volt = (a*5.0)/1023.0;
    integer = (int)volt;
    st = (int)((volt - integer) * 10.0);
    nd = (int)((volt - integer - st / 10.0)*100.0);

    write_data('0' + integer);
    write_data('.');
    write_data('0' + st);
    write_data('0' + nd);
    write_data('V');

}

char ADC_Converter(char channel){
    ADMUX  = ((ADMUX & 0xe0) | channel); // ADC 몇번쓸건지
    ADCSRA |= (1 << ADSC);  // ADC start
    while(ADCSRA & (1 << ADSC));  // 변환이 끝날 때까지 대기 (ADSC 비트가 0이 될 때까지) // ADCSRA & 0x40;
    return ADCH; // ADC = 16bit

}


void delay_m(unsigned int m){
    unsigned int i, j;
    for(i=0;i<m;i++){
        for(j=0;j<2100;j++){
            ;
        }
    }
}

void write_data(char d){
    PORTG = 0x06;
    delay_m(1);
    PORTC = d;
    delay_m(1);
    PORTG = 0x04;
    //CLCD에 글자가 찍히는 함수
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



void write_string(char *pt){

        while((*pt >= 0x20) && (*pt <= 0x7f)){ // 0x20 ~ 0x7e
            write_data(*pt);
            pt++;
        }
}


void NumbertoTwoDigit(int a){
    char value;
    char dummy_1;
    char dummy_10;


        value = a -(a / 100)*100; // a%100
        dummy_10 = value / 10;
        dummy_1 = (value - dummy_10 * 10) / 1;

        if(dummy_10 == 10) dummy_10 = 0;
        if(dummy_10 == 0){
        write_data(' ');
        }
        else{
        write_data(dummy_10 + 0x30);
        }
        if(dummy_1 == 0){
        write_data(' ');
        }
        else{
        write_data(dummy_1 + 0x30);
        }

}


void NumbertoFourDigit(int a){ //exp02 20221349 ±è¼ºÇö
    char dummy_1000;
    char dummy_100;
    char dummy_10;
    char dummy_1;

    dummy_1000=a/10000;
    a=(a-dummy_1000*10000);

    dummy_1000=a/1000;
    dummy_100=(a-dummy_1000*1000)/100;
    dummy_10=(a-dummy_1000*1000-dummy_100*100)/10;
    dummy_1=(a-dummy_1000*1000-dummy_100*100-dummy_10*10)/1;

    if(dummy_1000==0)
        write_data(' ');
    else
        write_data(dummy_1000+0x30);
    if((dummy_1000==0)&&(dummy_100==0))
        write_data(' ');
    else
        write_data(dummy_100+0x30);
    if((dummy_1000==0)&&(dummy_100==0)&& (dummy_10==0))
        write_data(' ');
    else
        write_data(dummy_10+0x30);
    write_data(dummy_1+0x30);
}

void NumbertoBinary(char a){
    char bin_value[8] = {'0','0','0','0','0','0','0','0'};

    bin_value[0] = a/128;
    bin_value[1] = (a-bin_value[0]*128)/64;
    bin_value[2] = (a-bin_value[0]*128-bin_value[1]*64)/32;
    bin_value[3] = (a-bin_value[0]*128-bin_value[1]*64-bin_value[2]*32)/16;
    bin_value[4] = (a-bin_value[0]*128-bin_value[1]*64-bin_value[2]*32-bin_value[3]*16)/8;
    bin_value[5] = (a-bin_value[0]*128-bin_value[1]*64-bin_value[2]*32-bin_value[3]*16-bin_value[4]*8)/4;
    bin_value[6] = (a-bin_value[0]*128-bin_value[1]*64-bin_value[2]*32-bin_value[3]*16-bin_value[4]*8-bin_value[5]*4)/2;
    bin_value[7] = (a-bin_value[0]*128-bin_value[1]*64-bin_value[2]*32-bin_value[3]*16-bin_value[4]*8-bin_value[5]*4-bin_value[6]*2)/1;

    write_instruction(0x80+3);
    write_data(bin_value[0]+0x30);
    write_data(bin_value[1]+0x30);
    write_data(bin_value[2]+0x30);
    write_data(bin_value[3]+0x30);
    write_data(' ');
    write_data(bin_value[4]+0x30);
    write_data(bin_value[5]+0x30);
    write_data(bin_value[6]+0x30);
    write_data(bin_value[7]+0x30);
}

char Hex_convert(char a){
    if(a<=9){
    return a+0x30;
    }else{
    return a+0x37;
    }

}

void NumbertoHex(char a){
    char hex_value[4] = {'0','x','0','0'};
    char hex_up;
    char hex_down;

    hex_up = a / 16;
    hex_down = a - hex_up*16;

    hex_value[2] = Hex_convert(hex_up);
    hex_value[3] = Hex_convert(hex_down);

    write_instruction(0xc0+8);
    write_data(hex_value[0]);
    write_data(hex_value[1]);
    write_data(hex_value[2]);
    write_data(hex_value[3]);

}
