#include <iom128v.h>
#include <macros.h>
//스위치 3개 사용
// ============================================================================
// [함수 선언]
// ============================================================================
void write_data(char d);
void write_instruction(char i);
void lcd_init(void);
void delay_m(unsigned int m);
void Print_Custom_Hour(int h);
void Print_TwoDigit(int n);
void write_string(char *str);

// ============================================================================
// [전역 변수]
// ============================================================================

volatile char mode = 0; // 0: 시계 화면, 1: 스톱워치+랩타임 화면

// 시계 변수
volatile int c_hour = 0;
volatile int c_min = 0;
volatile int c_sec = 0;

// 스톱워치 변수
volatile int s_hour = 0;
volatile int s_min = 0;
volatile int s_sec = 0;
volatile char stw_run = 0; // 0:정지, 1:동작

// 랩타임 변수 (PE6 누를 때 저장)
volatile int l_hour = 0;
volatile int l_min = 0;
volatile int l_sec = 0;

// 제어 변수
volatile int tick = 0;
volatile int debounce_cnt = 0;

// 버튼 B (PE5 - 제어) 상태 변수
volatile char btn_b_state = 0;
volatile char action_handled = 0;
volatile int hold_cnt = 0;
volatile char reset_done = 0;

// 버튼 C (PE6 - 랩타임) 상태 변수
volatile char btn_c_state = 0;

// ============================================================================
// [메인 함수]
// ============================================================================
void main(void)
{
    // 포트 초기화
    DDRG = 0xFF; DDRC = 0xFF; // LCD

    // [수정됨] PE4(모드), PE5(제어), PE6(랩타임) 입력 설정
    // 0x70 = 0111 0000 (PE4, PE5, PE6 사용)
    DDRE &= ~0x70;
    PORTE |= 0x70; // 내부 풀업 활성화

    delay_m(200);
    lcd_init();

    // Timer0 (1ms)
    TCCR0 = (1 << WGM01) | (0 << CS02) | (1 << CS01) | (1 << CS00);
    OCR0 = 249;
    TIMSK = (1 << OCIE0);

    // 인터럽트 설정 (PE4만 인터럽트 사용)
    EICRB = 0x02; // INT4 Falling Edge (0000 0010)
    EIMSK = 0x10; // INT4 Enable (PE5, PE6는 타이머 폴링 사용)

    SEI();

    while (1) {

        if (mode == 0) {
            // ------------------------------------------
            // [화면 0] 시계 모드
            // ------------------------------------------
            write_instruction(0x80); // Line 1
            write_string("CLOCK ");
            Print_Custom_Hour(c_hour); write_data(':');
            Print_TwoDigit(c_min); write_data(':');
            Print_TwoDigit(c_sec); write_string("    ");

            write_instruction(0xC0); // Line 2
            write_string("                "); // 깨끗하게 비움
        }
        else {
            // ------------------------------------------
            // [화면 1] 스톱워치(L1) + 랩타임(L2)
            // ------------------------------------------

            // [Line 1] 스톱워치 (위로 올라옴)
            write_instruction(0x80);
            write_string("STW ");
            Print_TwoDigit(s_hour); write_data(':');
            Print_TwoDigit(s_min);  write_data(':');
            Print_TwoDigit(s_sec);

            if (stw_run) write_string(" RUN");
            else if (s_hour==0 && s_min==0 && s_sec==0) write_string(" CLR");
            else if (btn_b_state) write_string(" CHK");
            else write_string(" STP");

            // [Line 2] 랩타임 (새 기능)
            write_instruction(0xC0);
            write_string("LAP ");
            Print_TwoDigit(l_hour); write_data(':');
            Print_TwoDigit(l_min);  write_data(':');
            Print_TwoDigit(l_sec);
            write_string("    ");
        }

        delay_m(20);
    }
}

// ============================================================================
// [ISR: 스위치 A (PE4) - 화면 전환]
// ============================================================================
#pragma interrupt_handler int4_isr:iv_INT4
void int4_isr(void) {
    if (debounce_cnt == 0) {
        if (mode == 0) mode = 1;
        else mode = 0;
        debounce_cnt = 250;
    }
}

// ============================================================================
// [ISR: Timer0 (1ms)] - 시계, 스톱워치, 버튼 B/C 처리
// ============================================================================
#pragma interrupt_handler timer0_comp_isr:iv_TIMER0_COMP
void timer0_comp_isr(void)
{
    if (debounce_cnt > 0) debounce_cnt--;
    tick++;

    // 1. 시계 로직 (항상 흐름)
    if (tick >= 1000) {
        tick = 0;
        c_sec++;
        if (c_sec >= 60) {
            c_sec = 0; c_min++;
            if (c_min >= 60) {
                c_min = 0; c_hour++;
                if (c_hour >= 24) c_hour = 0;
            }
        }

        // 스톱워치 카운트
        if (stw_run == 1) {
            s_sec++;
            if (s_sec >= 60) {
                s_sec = 0; s_min++;
                if (s_min >= 60) {
                    s_min = 0; s_hour++;
                    if (s_hour >= 100) s_hour = 0;
                }
            }
        }
    }

    // --------------------------------------------------------
    // 2. 버튼 B (PE5) 처리: Start / Stop / Reset
    // --------------------------------------------------------
    if ((PINE & 0x20) == 0) { // 눌림
        if (btn_b_state == 0) {
            btn_b_state = 1;
            // 동작 중 누르면 -> 정지
            if (stw_run == 1) {
                stw_run = 0;
                action_handled = 1;
            } else {
                action_handled = 0;
            }
        }

        // 정지 상태에서 꾹 누르면 -> 리셋
        if (stw_run == 0 && action_handled == 0) {
            if (reset_done == 0) {
                hold_cnt++;
                if (hold_cnt >= 1500) { // 1.5초
                    // [리셋 동작] 스톱워치 & 랩타임 모두 0으로
                    s_hour = 0; s_min = 0; s_sec = 0;
                    l_hour = 0; l_min = 0; l_sec = 0; // 랩타임도 초기화
                    reset_done = 1;
                }
            }
        }
    }
    else { // 뗌
        if (btn_b_state == 1) {
            // 리셋 안 하고 뗐으면 -> 시작
            if (stw_run == 0 && action_handled == 0 && reset_done == 0) {
                stw_run = 1;
            }
            btn_b_state = 0;
            hold_cnt = 0;
            action_handled = 0;
            reset_done = 0;
        }
    }

    // --------------------------------------------------------
    // 3. 버튼 C (PE6) 처리: 랩타임 기록 (Lap Time)
    // --------------------------------------------------------
    // PINE & 0x40 (Bit 6) 검사
    if ((PINE & 0x40) == 0) { // 눌림
        if (btn_c_state == 0) {
            btn_c_state = 1; // 눌림 처리

            // 스톱워치가 돌아가는 중에만 랩타임 기록
            if (stw_run == 1) {
                l_hour = s_hour;
                l_min  = s_min;
                l_sec  = s_sec;
            }
        }
    }
    else { // 뗌
        btn_c_state = 0;
    }
}

// ============================================================================
// [LCD 및 기타 함수]
// ============================================================================
#pragma interrupt_handler int6_isr:iv_INT6
void int6_isr(void) {}

void Print_Custom_Hour(int h) {
    if (h == 0) { write_data('0'); write_data('0'); }
    else if (h < 10) { write_data('0'); write_data('0' + h); }
    else if (h <= 12) { write_data('0' + (h / 10)); write_data('0' + (h % 10)); }
    else {
        int pm = h - 12;
        if (pm < 10) { write_data(' '); write_data('0' + pm); }
        else { write_data('0' + (pm / 10)); write_data('0' + (pm % 10)); }
    }
}

void Print_TwoDigit(int n) {
    write_data('0' + (n / 10));
    write_data('0' + (n % 10));
}

void write_string(char *str) { while(*str) write_data(*str++); }
void write_data(char d){ PORTG=0x06; PORTC=d; delay_m(1); PORTG=0x04; delay_m(1); }
void write_instruction(char i){ PORTG=0x02; PORTC=i; delay_m(1); PORTG=0x00; delay_m(1); }

void lcd_init(void){
    delay_m(50); write_instruction(0x38); delay_m(5); write_instruction(0x38);
    delay_m(1); write_instruction(0x38); write_instruction(0x0C);
    write_instruction(0x06); write_instruction(0x01); delay_m(2);
}

void delay_m(unsigned int m){
    volatile unsigned int i, j;
    for(i=0; i<m; i++)
        for(j=0; j<2000; j++);
}
