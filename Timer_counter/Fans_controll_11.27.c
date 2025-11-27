//11.27.c
#include <iom128v.h>
#include <macros.h>

// --- 함수 선언 ---
void delay_m(unsigned int m);
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);
void write_string(char *pt);
void Write_Dec_3Digit(unsigned int a);
unsigned char ADC_Converter(char channel);

// --- 변수 ---
unsigned char adc_value;   // 0~255 (OCR0에 들어갈 값)
unsigned int duty_percent; // 0~100 (LCD 표시용)

void main(void)
{
    // 1. 포트 설정
    DDRB = 0x10; // PB4 (OC0) 출력 설정 (모터 PWM 신호)
    DDRG = 0x07; // LCD 제어 신호
    DDRC = 0xFF; // LCD 데이터 신호
    DDRF = 0x00; // ADC 입력

    lcd_init();

    // 2. ADC 설정 (좌측 정렬 ADLAR=1 -> 8비트 0~255 값 읽기)
    // AREF 사용, ADC0 채널
    ADMUX = (0<<REFS1) | (0<<REFS0) | (1<<ADLAR) | (0<<MUX4) | (0<<MUX3) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0);
    // ADC 켜기(ADEN=1), 128분주(ADPS=111)
    ADCSRA = (1<<ADEN) | (0<<ADSC) | (0<<ADFR) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    // 3. 타이머 0 설정 (32분주, Fast PWM)
    // WGM01:00 = 1, 1 -> Fast PWM 모드
    // COM01:00 = 1, 0 -> 비반전 모드 (OCR0값 클수록 빨라짐)
    // CS02:00  = 0, 1, 1 -> 32분주 설정 (약 1.95kHz)
    TCCR0 = (1<<WGM01) | (1<<WGM00) | (1<<COM01) | (0<<COM00) | (0<<CS02) | (1<<CS01) | (1<<CS00);

    TIMSK = 0x00; // 인터럽트 끄기 (순수 하드웨어 PWM)

    while(1){
        // 1) 가변저항 값 읽기 (0~255)
        adc_value = ADC_Converter(0);

        // 2) 모터 속도 반영 (PWM 듀티 제어)
        OCR0 = adc_value;

        // 3) Duty 퍼센트 계산 (스케일링)
        // 0~255 범위를 0~100 범위로 변환
        duty_percent = (unsigned int)((adc_value * 100) / 255);

        // 4) LCD 출력 - 첫 번째 줄 (실제 값)
        write_instruction(0x84); // 1번째 줄 맨 앞
        write_string(" ");
        Write_Dec_3Digit(adc_value); // 0~255 출력
        write_string("   ");         // 잔상 제거용 공백

        // 5) LCD 출력 - 두 번째 줄 (퍼센트)
        write_instruction(0xC0); // 2번째 줄 맨 앞
        write_string("DUTY : ");
        Write_Dec_3Digit(duty_percent); // 0~100 출력
        write_string("%  ");            // 퍼센트 기호 및 잔상 제거

        delay_m(50); // 화면 갱신 딜레이
    }
}

// --- ADC 변환 (8비트 모드) ---
unsigned char ADC_Converter(char channel){
    ADMUX = (ADMUX & 0xE0) | (channel & 0x07);
    ADCSRA |= (1<<ADSC);  // 변환 시작
    while(ADCSRA & 0x40); // 변환 완료 대기
    return ADCH;          // 상위 8비트 반환
}

// --- 10진수 3자리 출력 함수 (공백 처리 포함) ---
void Write_Dec_3Digit(unsigned int a)
{
    // 백의 자리
    if(a >= 100) write_data((a/100) + '0');
    else write_data(' ');

    // 십의 자리
    if(a >= 10) write_data(((a%100)/10) + '0');
    else if(a >= 100) write_data('0');
    else write_data(' ');

    // 일의 자리 (항상 출력)
    write_data((a%10) + '0');
}

// --- LCD 기본 함수들 ---
void lcd_init(void){
    write_instruction(0x30); delay_m(15);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x38); delay_m(1);
    write_instruction(0x0C); delay_m(1);
    write_instruction(0x01); delay_m(2);
    write_instruction(0x06); delay_m(1);
}

void write_string(char *pt){
    while(*pt) { write_data(*pt); pt++; }
}

void write_data(char d){
    PORTG = 0x06; delay_m(1);
    PORTC = d;    delay_m(1);
    PORTG = 0x04;
}

void write_instruction(char i){
    PORTG = 0x02; delay_m(1);
    PORTC = i;    delay_m(1);
    PORTG = 0x00;
}

void delay_m(unsigned int m){
    unsigned int i, j;
    for(i = 0; i < m; i++){
        for(j = 0; j < 2100; j++);
    }
}
