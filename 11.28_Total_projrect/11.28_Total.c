/*

Changing C_Lcd's value according to Cds's value output accordingly.
245p => ATmel ADLAR =0,1 구별하는 법(상황에 따라) => 기말고사 시험
가변저항 => Cds로 컨셉 변경 
과제 단계
1.Threshhold 값을 적어라 
2.OCR값 찾는 법 -> 시험 문제
3.그림 던져주고 프로그래밍하는 과정을 서술하는것 
코딩을 하는게 시험 문제
*/


#include <iom128v.h>
#include <macros.h>

// --- 함수 시그니처 선언 ---
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);
void delay_m(unsigned int m);
void write_string(char *pt);
void NumbertoTwoDigit(int a);
void NumbertoFourDigit(char a); // 요청대로 char 입력으로 변경
void NumbertoBinary (char a);
void NumberToHex(char a);
//char ADC_Converter(char channel); // 요청대로 char 반환으로 변경
void DectoFloat(int a);
char Hex_convert(char a);


char ADC_result;        // ADC 상위 8비트
int sec     = 5;       // 5,4,3,2,1,0 카운트
unsigned char start   = 0;  // 카운트 동작 중인지
unsigned char fan_on  = 0;  // 선풍기 ON 상태인지
unsigned int tick = 0; // 4ms 단위 타이머 틱 (250개 = 1초)
unsigned int fan_sec  = 0; // 선풍기 ON 상태로 지난 초 단위 카운트


char number[10]={0x04,0x2F,0x18,0x09,0x23,0x41,0x40,0x07,0x00,0x01};
char thresold = 100;

void main(void)
{
    PORTB |= (1<<PB4);
    // 포트 설정
    DDRG = 0x07;     // LCD 제어용
    DDRC = 0xff;     // LCD 데이터용
    DDRB = 0x10;     // OC0 핀(PB4) 출력 설정
    DDRD = 0x7f;     // 7-segment
    SREG |= 0x80;    // 전역 인터럽트 허용

    lcd_init();

    //--- ADC 설정 ----------------------------------------------------
    // ADMUX : REFS1=1, REFS0=1 → 내부 2.56V 기준전압 사용
    // ADLAR=1 → 왼쪽 정렬 (상위 8비트를 ADCH에서 읽기 위함) **요청 반영**
    ADMUX = (1<<REFS1)|(1<<REFS0)|(1<<ADLAR)|(0<<MUX4)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(1<<MUX0);

    // ADCSRA : ADEN=1, ADPS=111 (128분주) 또는 적절한 분주비 설정
    ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADFR) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    //--- Timer0 설정 ------------------------------------------------
    TCCR0 = (0<<FOC0)|(1<<WGM01) | (0<<WGM00) | (0<<COM01) | (0<<COM00) | (1<<CS02) | (0<<CS01) | (0<<CS00);
    TIMSK = (1<<OCIE0) | (0<<TOIE0);

    OCR0 = 249;
    while (1) {

        write_instruction(0x80);
        NumberToHex(ADC_result);

        PORTD = number[sec];

    }
}

// --- ADC 인터럽트: "카운트 시작" 트리거만 담당 ---
#pragma interrupt_handler adc_isr:iv_ADC
void adc_isr(void)
{
    // ADC 결과 상위 8비트 읽기 (ADLAR=1이므로 ADCH만 사용)
    ADC_result = ADCH;

    // 아직 카운트 안 하고, 선풍기도 꺼져 있고, ADC가 기준 이상이면 → 5초 카운트 시작
    if (!start && !fan_on && ADC_result >= thresold) {
        start    = 1;   // 카운트 시작
        sec      = 5;   // 5부터 시작
        tick = 0;   // 1ms 카운터 리셋
        fan_sec  = 0;   // 나중에 선풍기 ON 시간 카운트 리셋

        // 카운트 중에는 선풍기 꺼져 있어야 함 (HIGH = OFF 가정)
        PORTB |= (1<<PB4);
    }
}

// --- Timer0 비교매치 인터럽트: 1ms마다 호출 → 여기서 1초/선풍기 시간 모두 처리 ---
#pragma interrupt_handler timer0_comp_isr:iv_TIM0_COMP
void timer0_comp_isr(void)
{
    tick++;

    
    if (tick >= 1000) {
        tick = 0;

        // 1) 카운트 중이라면(sec>0) 1초마다 sec 감소
        if (start && sec > 0) {
            sec--;
            if (sec == 0) {
                // 5→4→3→2→1→0 카운트 끝나는 순간
                start   = 0;
                fan_on  = 1;
                fan_sec = 0;

                // 선풍기 ON (Active-Low 가정: LOW = ON)
                PORTB &= ~(1<<PB4);
            }
        }
        // 2) 카운트는 끝났고, 선풍기가 켜져 있는 상태라면 ON 시간 카운트
        else if (fan_on) {
            fan_sec++;

            // 예: 선풍기를 5초 동안 켜두고 싶다면
            if (fan_sec >= 5) {
                fan_on  = 0;
                // 선풍기 OFF
                PORTB |= (1<<PB4);
            }
        }
    }
}


// --- ADC 변환 함수 (8비트 반환) ---
/*char ADC_Converter(char channel){
    ADMUX = (ADMUX & 0xe0) | channel;
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));

    // ADLAR=1 설정이므로 상위 8비트인 ADCH만 반환하면 됩니다.
    // char형으로 반환 (0~255)
    return ADCH;
}*/


// --- NumbertoFourDigit 함수 (char 입력) ---
// 입력 a는 0~100 사이의 값이 들어옵니다.
void NumbertoFourDigit(char a) {
    // char a를 숫자 값으로 취급 (0~255)
    // -'0' 연산은 입력이 '문자'일 때 쓰는 것이므로,
    // 여기서는 duty 값(숫자)이 들어오므로 제거했습니다.

    int b = (unsigned char)a; // 계산을 위해 int로 확장
    int divisor = 1000;
    int started = 0;
    int i;
    for (i = 0; i < 4; i++) {
        int digit = b / divisor;

        if (i == 3) {
            write_data('0' + (b % 10));
        }
        else if (digit != 0 || started) {
            write_data('0' + digit);
            started = 1;
        }
        else {
            write_data(' ');
        }

        b %= divisor;
        divisor /= 10;
    }
}


// --- 나머지 함수들 ---
void DectoFloat(int a){
    unsigned long b;
    b=a;
    write_data('0'+b%10);
    write_data('.');
    write_data('0'+(b*10)%10);
    write_data('0'+(b*100)%10);
}
void FloattoFloat(float a){
    int frac;
    frac = (int)((a - (int)a)*100+0.5);
    NumbertoFourDigit((char)a);
    write_data('.');
    write_data('0'+frac/10);
    write_data('0'+frac%10);
}
void write_data(char d){
    PORTG=0x06;
    delay_m(1);
    PORTC=d;
    delay_m(1);
    PORTG=0x04;
}
void write_instruction(char i){
    PORTG=0x02;
    delay_m(1);
    PORTC=i;
    delay_m(1);
    PORTG=0x00;
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
    delay_m(15);
}
void NumbertoTwoDigit(int a){
    unsigned int dummy_10;
    a=a%100;

    dummy_10=a/10;

    if(dummy_10==0) write_data(' ');
    else write_data('0'+dummy_10);
    write_data('0'+a%10);
}
char Hex_convert(char a){
    if(a>9) return 'a'+(a-10);
    return '0'+a;
}
void NumberToHex(char a){
    a=a%256;
    write_string("0x");
    write_data(Hex_convert(a/16));
    write_data(Hex_convert(a%16));
}
void NumbertoBinary(char a){
    a=a%256;
    write_data('0'+a/16/4/2);
    write_data('0'+a/16/4%2);
    write_data('0'+a/16%4/2);
    write_data('0'+a/16%4%2);
    write_data(' ');
    write_data('0'+a%16/4/2);
    write_data('0'+a%16/4%2);
    write_data('0'+a%16%4/2);
    write_data('0'+a%16%4%2);
}

void write_string(char *pt){
    while((*pt>=' ')&&(*pt<='~')){
        write_data(*pt);
        pt++;
    }
}
void delay_m(unsigned int m){
    unsigned int i, j;
    for(i=0; i<m; i++){
        for(j=0; j<2100;j++){
            ;
        }
    }
}



#pragma interrupt_handler timer0_ovf_isr:iv_TIM0_OVF
void timer0_ovf_isr(void){
}
