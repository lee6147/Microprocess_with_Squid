#include <iom128v.h>
#include <macros.h>

// --- 함수 선언 ---
void delay_m(unsigned int m);
char ADC_Converter(char channel);

void main(void)
{
    // 1. 포트 초기화
    // 가변저항 입력: PF0 (ADC0) -> 입력으로 설정
    DDRF = 0x00;
    // PWM 출력: PB4 (OC0 핀) -> 출력으로 설정해야 파형이 나감 [cite: 1689]
    DDRB = 0x10;

    // 2. ADC 초기화
    // 기준전압: AREF(00), 정렬: 좌측정렬(ADLAR=1), 채널: ADC0(00000)
    // 좌측 정렬을 쓰는 이유: 8비트(0~255) 값만 필요하므로 상위 레지스터(ADCH)만 읽기 위함
    ADMUX = (0<<REFS1) | (0<<REFS0) | (1<<ADLAR) | (0<<MUX4) | (0<<MUX3) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0);

    // ADC 허용(ADEN), 분주비 128 (안정적인 변환을 위해 느리게 설정)
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

    // 3. 타이머/카운터 0 설정 (핵심: 1.95kHz 만들기)
    // 모드: 고속 PWM (Fast PWM) -> WGM01=1, WGM00=1 [cite: 1188]
    // 출력: 비반전 모드 (Non-Inverting) -> COM01=1, COM00=0 [cite: 1200]
    //       (값이 클수록 High 구간이 길어져 모터가 세게 돔)
    // 분주비: 32분주 -> CS02=0, CS01=1, CS00=1 [cite: 1210]
    TCCR0 = (1<<WGM01) | (1<<WGM00) | (1<<COM01) | (0<<COM00) | (0<<CS02) | (1<<CS01) | (1<<CS00);

    while(1)
    {
        // 1. 가변저항 값 읽기 (PF0 핀 = 채널 0)
        // 결과값: 0(최소) ~ 255(최대)
        unsigned char duty_value = ADC_Converter(0);

        // 2. 읽은 값을 PWM 듀티비 통(OCR0)에 넣음
        // OCR0 값이 커질수록 듀티비(%)가 증가하여 모터가 빨라짐
        OCR0 = duty_value;

        // 3. 너무 빠른 갱신 방지를 위한 지연 (0.05초)
        delay_m(50);
    }
}

// --- 보조 함수 ---

// ADC 변환 함수 (짐님의 기존 로직 최적화)
char ADC_Converter(char channel){
    // 기존 설정(REFS, ADLAR)은 유지하고 하위 5비트(채널)만 변경
    ADMUX = (ADMUX & 0xE0) | (channel & 0x07);

    ADCSRA |= (1<<ADSC); // 변환 시작 (Start Conversion)
    while(ADCSRA & 0x40); // 변환 완료 대기 (ADSC가 0이 될 때까지)

    return ADCH; // 좌측 정렬이므로 상위 8비트만 읽어서 리턴 (0~255)
}

void delay_m(unsigned int m){
    unsigned int i, j;
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < 2100; j++);
    }
}
