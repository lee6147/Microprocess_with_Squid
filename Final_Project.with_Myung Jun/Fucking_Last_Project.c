#include <iom128v.h>

// 함수 선언
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);
void delay_m(unsigned int m);
void write_string(char *pt);
void NumbertoTwoDigit(int a);
void delay_short(void);

// 전역 변수
int count, sw_count;
char sec = 10, min = 30, hour = 14; // 초기값 설정
char sw_sec, sw_min, sw_hour, sw_msec;
char mode = 0, flag = 0, sw_stop = 0, sw_flag;

// 메인 함수
int main(void)
{
    DDRG = 0x07;
    DDRC = 0xFF;
    lcd_init();

    // 타이머 및 인터럽트 설정
    TCCR0 = (0<<FOC0)|(1<<WGM01)|(0<<WGM00)|(0<<COM01)|(0<<COM00)|(1<<CS02)|(0<<CS01)|(0<<CS00); // N=64
    TIMSK = (1<<OCIE0)|(0<<TOIE0);
    OCR0 = 249;
    EICRB = 0x0B; // INT4, INT5 설정
    EIMSK = 0x30; // INT4, INT5 활성화
    SREG |= 0x80; // 전역 인터럽트 허용

    // [초기 시간 보정 로직] 24시간제 입력 -> 12시간제 + AM/PM 설정
    if(hour >= 12) {
        flag = 1;       // 12 이상이면 오후(PM)
        if(hour > 12) {
            hour = hour - 12; // 13 -> 1, 14 -> 2 로 변환
        }
    }
    else {
        flag = 0;       // 12 미만이면 오전(AM)
        if(hour == 0) {
            hour = 12;  // 0시는 12시로 표기
        }
    }

    while(1) {
        // 1. 시간 계산 로직 (Watch)
        if(sec == 60) {
            sec = 0;
            min++;
            if(min == 60) {
                min = 0;
                hour++;
                if(hour == 12) {
                    if(flag == 1) { // PM -> AM
                        flag = 0;
                        // hour는 12 그대로 유지 (오전 12시)
                    }
                    else { // AM -> PM
                        flag = 1;
                    }
                }
                else if(hour == 13) {
                    hour = 1;
                }
            }
        }

        // 2. 스톱워치 계산 로직 (Stopwatch)
        if(sw_msec >= 100) {
            sw_msec = 0;
            sw_sec++;
            if(sw_sec == 60) {
                sw_sec = 0;
                sw_min++;
                if(sw_min == 60) {
                    sw_min = 0;
                    sw_hour++;
                    if(sw_hour == 24) {
                        sw_hour = 0;
                    }
                }
            }
        }

        // 3. LCD 표시 로직 (여기가 while문 안에 있어야 합니다!)
        if(mode == 0)
        {   // Watch Mode
            write_instruction(0x80); // 커서 홈

            // 시간(Hour) 표시: 오전 '0', 오후 ' '
            if(hour < 10)
            {
                if(flag == 0) // AM
                {
                    write_data('0');
                    write_data(hour + 0x30);
                }
                else          // PM
                {
                    write_data(' ');
                    write_data(hour + 0x30);
                }
            }
            else
            {
                NumbertoTwoDigit(hour);
            }

            write_data(':');

            // 분(Minute)
            if(min < 10) { write_data('0'); write_data(min + 0x30); }
            else { NumbertoTwoDigit(min); }

            write_data(':');

            // 초(Second)
            if(sec < 10) { write_data('0'); write_data(sec + 0x30); }
            else { NumbertoTwoDigit(sec); }

            write_string("   "); // 잔상 제거
        }
        else
        {   // Stopwatch Mode
            write_instruction(0x80);

            // 스톱워치 시간
            if(sw_hour < 10) { write_data('0'); write_data(sw_hour + 0x30); }
            else { NumbertoTwoDigit(sw_hour); }

            write_data(':');

            if(sw_min < 10) { write_data('0'); write_data(sw_min + 0x30); }
            else { NumbertoTwoDigit(sw_min); }

            write_data(':');

            if(sw_sec < 10) { write_data('0'); write_data(sw_sec + 0x30); }
            else { NumbertoTwoDigit(sw_sec); }

            write_data(':');

            if(sw_msec < 10) { write_data('0'); write_data(sw_msec + 0x30); }
            else { NumbertoTwoDigit(sw_msec); }
        }
    } // while(1) 끝

    return 0;
} // main 끝

// === 인터럽트 핸들러 함수들 ===

#pragma interrupt_handler int4_isr:iv_INT4
void int4_isr(void) // 모드 변경 (시계 <-> 스톱워치)
{
    delay_m(200); // 디바운싱
    mode = !mode;
    EIFR = 0x10;
}

#pragma interrupt_handler int5_isr:iv_INT5
void int5_isr(void) // 스톱워치 시작/정지 및 리셋
{
    int press_time = 0;

    // 버튼이 눌려있는 동안 체크
    while((PINE & 0x20) == 0) {
        delay_m(10);
        press_time++;

        // 2초 이상 누르면 리셋
        if(press_time > 200) {
            sw_msec = 0;
            sw_sec = 0;
            sw_min = 0;
            sw_hour = 0;

            sw_stop = 0; // 리셋 후 정지 상태 유지

            EIFR = 0x20;
            return; // 즉시 리턴하여 화면 갱신
        }
    }

    // 짧게 눌렀다 떼면 Start/Stop 토글
    if(sw_stop == 0) {
        sw_stop = 1; // 정지 -> 시작
    }
    else {
        sw_stop = 0; // 시작 -> 정지
    }

    delay_m(200); // 디바운싱
    EIFR = 0x20;
}

#pragma interrupt_handler timer0_comp_isr:iv_TIM0_COMP
void timer0_comp_isr(void)
{
    count++;
    if(count >= 1000){ // 1초
        count = 0;
        sec++;
    }

    // 스톱워치가 동작 중(sw_stop == 1)일 때만 카운트 증가
    if(sw_stop == 1){
        sw_count++;
        if(sw_count >= 10){ // 10ms 단위
            sw_count = 0;
            sw_msec++;
        }
    }
}

// === 유틸리티 함수들 ===

void delay_short(void){
    int k;
    for(k=0; k<50; k++);
}

void write_data(char d){
    PORTG = 0x06;
    delay_short();
    PORTC = d;
    delay_short();
    PORTG = 0x04;
}

void write_instruction(char i){
    PORTG = 0x02;
    delay_short();
    PORTC = i;
    delay_short();
    PORTG = 0x00;
}

void lcd_init(void)
{
    write_instruction(0x30); delay_m(15);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x38); delay_m(1);
    write_instruction(0x0e); delay_m(1);
    write_instruction(0x01); delay_m(1);
}

void delay_m(unsigned int m){
    unsigned int i, j;
    for(i=0; i<m; i++){
        for(j=0; j<2100; j++);
    }
}

void write_string(char *pt)
{
    while((*pt >= ' ') && (*pt <= '~')){
        write_data(*pt);
        pt++;
    }
}

void NumbertoTwoDigit(int a){
    char dummy_10;
    char dummy_1;

    dummy_10 = a / 10;
    dummy_1 = a % 10;

    if(dummy_10 == 0){
        write_data('0'); // 0~9일 때 앞자리에 0을 채움 (선택사항, 필요없으면 공백 처리)
    }
    else{
        write_data(dummy_10 + 0x30);
    }
    write_data(dummy_1 + 0x30);
}
