#include <iom128v.h>
#include <macros.h>

// --- 함수 선언 ---
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);
void delay_m(unsigned int m);
void write_string(char *pt);
void Display_Clock(unsigned char h, unsigned char m, unsigned char s);

// --- 전역 변수 (volatile 필수) ---
volatile int sec = 5;                 // 타이머 시간 (초)
volatile unsigned char timer_run = 0; // 0:정지/일시정지, 1:동작(RUN)
volatile unsigned int tick = 0;       // 1ms 틱 카운터

// 시계 변수 (12시간제 초기값: 12시 00분 00초)
volatile unsigned char clk_hour = 12;
volatile unsigned char clk_min = 0;
volatile unsigned char clk_sec = 0;

void main(void)
{
    // [C89 표준] 변수 선언은 함수 맨 윗부분에 위치해야 함
    unsigned char prev_sw = 1;
    unsigned char curr_sw;

    // ---------------------------------------------------------------
    // [매우 중요] JTAG 비활성화 (TCK핀을 스위치로 사용하기 위함)
    // 4사이클 내에 두 번 써야 하므로 최적화 방해를 받지 않게 연달아 작성
    // ---------------------------------------------------------------
    MCUCSR |= 0x80;
    MCUCSR |= 0x80;

    // --- 회로 설계 초기화 ---
    DDRG = 0x07;     // LCD 제어핀 출력
    DDRC = 0xff;     // LCD 데이터핀 출력
    DDRB = 0x10;     // OC0 (현재는 미사용이나 기존 회로 유지)

    // ** 스위치 핀 설정 (TCK = PF4) **
    DDRF &= ~0x10;   // PF4 입력 설정 (4번 비트 0으로)
    PORTF |= 0x10;   // PF4 내부 풀업 저항 활성화

    SREG |= 0x80;    // 전역 인터럽트 허용

    lcd_init();

    // --- Timer0 설정 (1ms 주기) ---
    // 모드: CTC (WGM01=1)
    // 분주비: 64 (CS02=1)
    TCCR0 = (0<<FOC0)|(1<<WGM01) | (0<<WGM00) | (0<<COM01) | (0<<COM00) | (1<<CS02) | (0<<CS01) | (0<<CS00);
    TIMSK = (1<<OCIE0) | (0<<TOIE0); // 비교매치 인터럽트 켜기

    OCR0 = 249; // 16MHz/64분주/1000Hz - 1 = 249
    // ----------------------------------

    while (1) {
        // --- 1. 스위치(PF4/TCK) 처리 ---
        // PINF 레지스터의 4번 비트(0x10)를 읽음
        curr_sw = (PINF & 0x10);

        // 버튼 누름 감지 (Falling Edge: 1 -> 0)
        // 풀업이므로 평소 1, 누르면 0
        if (prev_sw != 0 && curr_sw == 0) {

            if (timer_run == 1) {
                // 동작 중이면 -> 일시정지 (PAUSE)
                timer_run = 0;
            } else {
                // 정지/일시정지 상태면 -> 재생 (RUN)
                // 만약 시간이 0초(종료) 상태라면 5초로 리셋 후 시작
                if (sec == 0) sec = 5;
                timer_run = 1;
            }

            delay_m(50); // 디바운싱 (채터링 방지)
        }
        prev_sw = curr_sw;

        // --- 2. LCD 표시 ---

        // [Line 1] 타이머 상태 표시
        write_instruction(0x80);
        write_string("TMR: ");

        // 타이머 시간 표시 (두 자리수)
        write_data('0' + (sec / 10));
        write_data('0' + (sec % 10));
        write_string("s ");

        // 상태 텍스트 표시
        if (sec == 0)            write_string("END  ");
        else if (timer_run == 1) write_string("RUN  ");
        else                     write_string("PAUSE");

        // [Line 2] 시계 표시 (항상 동작)
        write_instruction(0xC0);
        write_string("CLK: ");
        Display_Clock(clk_hour, clk_min, clk_sec);

        delay_m(10); // 화면 갱신 주기
    }
}

// Timer0 비교매치 인터럽트: 1ms 마다 정확히 실행
#pragma interrupt_handler timer0_comp_isr:iv_TIM0_COMP
void timer0_comp_isr(void)
{
    tick++;

    if (tick >= 1000) { // 1000ms = 1초 경과
        tick = 0;

        // --- [시계 동작] 타이머 상태와 무관하게 계속 흐름 ---
        clk_sec++;
        if(clk_sec >= 60) {
            clk_sec = 0;
            clk_min++;
            if(clk_min >= 60) {
                clk_min = 0;
                clk_hour++;
                // 12시간제: 12시 다음은 1시 (12 -> 1)
                if(clk_hour > 12) clk_hour = 1;
            }
        }

        // --- [타이머 동작] timer_run 플래그가 1일 때만 감소 ---
        if (timer_run == 1) {
            if (sec > 0) {
                sec--;
                // 시간이 0이 되면 자동으로 정지 상태로 전환
                if (sec == 0) {
                    timer_run = 0;
                }
            }
        }
    }
}

// --- 보조 함수 ---
void Display_Clock(unsigned char h, unsigned char m, unsigned char s) {
    // 1자리수일 때 앞에 '0' 붙여서 출력 (01, 02...)
    if(h < 10) write_data('0');
    else write_data('0' + h/10);
    write_data('0' + h%10);
    write_data(':');
    write_data('0' + m/10);
    write_data('0' + m%10);
    write_data(':');
    write_data('0' + s/10);
    write_data('0' + s%10);
}

void write_data(char d){
    PORTG=0x06; delay_m(1); PORTC=d; delay_m(1); PORTG=0x04;
}
void write_instruction(char i){
    PORTG=0x02; delay_m(1); PORTC=i; delay_m(1); PORTG=0x00;
}
void lcd_init(void){
    write_instruction(0x30); delay_m(15);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x30); delay_m(1);
    write_instruction(0x38); delay_m(1);
    write_instruction(0x0e); delay_m(1);
    write_instruction(0x01); delay_m(15);
}
void write_string(char *pt){
    while((*pt>=' ')&&(*pt<='~')){
        write_data(*pt); pt++;
    }
}
void delay_m(unsigned int m){
    unsigned int i, j;
    for(i=0; i<m; i++) for(j=0; j<2100;j++);
}

// 사용하지 않는 인터럽트 더미 처리 (컴파일러 호환성)
#pragma interrupt_handler timer0_ovf_isr:iv_TIM0_OVF
void timer0_ovf_isr(void){}
