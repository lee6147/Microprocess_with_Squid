#include <iom128v.h>
#include <macros.h>

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

volatile char mode = 0; // 0: 시계, 1: 스톱워치

// 시계 변수
volatile int c_hour = 0;
volatile int c_min = 0;
volatile int c_sec = 0;

// 스톱워치 변수
volatile int s_hour = 0;
volatile int s_min = 0;
volatile int s_sec = 0;
volatile char stw_run = 0; // 0:정지, 1:동작

// 제어 변수
volatile int tick = 0;
volatile int debounce_cnt = 0;

// [버튼 B 제어용 변수] - 인터럽트 대신 타이머에서 씁니다
volatile char btn_state = 0;      // 0:뗌, 1:눌림
volatile char action_handled = 0; // 이번 누름에서 동작을 수행했는지
volatile int hold_cnt = 0;        // 꾹 누름 카운트
volatile char reset_done = 0;     // 리셋 완료 플래그

// ============================================================================
// [메인 함수]
// ============================================================================
void main(void)
{
    // 포트 초기화
    DDRG = 0xFF; DDRC = 0xFF;
    DDRE &= ~0x30; PORTE |= 0x30; // PE4, PE5 입력 & 풀업

    delay_m(200);
    lcd_init();

    // Timer0 (1ms)
    TCCR0 = (1 << WGM01) | (0 << CS02) | (1 << CS01) | (1 << CS00);
    OCR0 = 249;
    TIMSK = (1 << OCIE0);

    // 인터럽트 설정
    EICRB = 0x0A; // Falling Edge 설정

    // [중요] PE5(INT5)는 인터럽트를 끕니다! (0x10 = INT4만 켬)
    // PE5는 이제 타이머가 직접 관리하므로 시계가 멈추지 않습니다.
    EIMSK = 0x10;

    SEI();

    while (1) {
        // [Line 1] 시계
        write_instruction(0x80);
        write_string("CLK ");
        Print_Custom_Hour(c_hour); write_data(':');
        Print_TwoDigit(c_min); write_data(':');
        Print_TwoDigit(c_sec); write_string("    ");

        // [Line 2] 스톱워치
        write_instruction(0xC0);
        if (mode == 0) {
            write_string("--- CLOCK ---   ");
        }
        else {
            write_string("STW ");
            Print_TwoDigit(s_hour); write_data(':');
            Print_TwoDigit(s_min);  write_data(':');
            Print_TwoDigit(s_sec);

            if (stw_run) write_string(" RUN");
            else if (s_hour==0 && s_min==0 && s_sec==0) write_string(" CLR");
            else if (btn_state) write_string(" CHK"); // 버튼 누르는 중
            else write_string(" STP");
        }

        delay_m(20);
    }
}

// ============================================================================
// [ISR: 스위치 A (모드 변경)] - 이건 인터럽트로 유지
// ============================================================================
#pragma interrupt_handler int4_isr:iv_INT4
void int4_isr(void) {
    if (debounce_cnt == 0) {
        if (mode == 0) mode = 1;
        else mode = 0;
        debounce_cnt = 250;
    }
}

// (스위치 B 인터럽트 함수 int5_isr는 삭제했습니다. 타이머가 대신 합니다.)

// ============================================================================
// [ISR: Timer0 (1ms)] - 여기가 핵심!
// ============================================================================
#pragma interrupt_handler timer0_comp_isr:iv_TIMER0_COMP
void timer0_comp_isr(void)
{
    if (debounce_cnt > 0) debounce_cnt--;
    tick++;

    // 1. 시계 로직 (절대 안 멈춤)
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

        // 스톱워치 카운트 (동작 중일 때만)
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

    // 2. [핵심] 스위치 B (PE5) 직접 검사 (폴링)
    // 인터럽트를 쓰지 않으므로 시계 타이머를 방해하지 않습니다.

    // 버튼 눌림 확인 (Low Active)
    if ((PINE & 0x20) == 0) {
        if (btn_state == 0) {
            // [방금 막 눌렀을 때]
            btn_state = 1;

            // 만약 돌고 있었다면 -> 즉시 정지
            if (stw_run == 1) {
                stw_run = 0;
                action_handled = 1; // 이번 누름은 '정지'로 썼음 (떼도 시작 안 함)
            } else {
                action_handled = 0; // '시작' 또는 '리셋' 대기
            }
        }

        // [누르고 있는 동안] - 리셋 체크
        if (stw_run == 0 && action_handled == 0) {
            if (reset_done == 0) {
                hold_cnt++;
                // 1.5초(1500ms) 이상 누르면 리셋
                if (hold_cnt >= 1500) {
                    s_hour = 0; s_min = 0; s_sec = 0;
                    reset_done = 1;
                }
            }
        }
    }
    else {
        // 버튼 뗌 (High)
        if (btn_state == 1) {
            // [방금 막 뗐을 때]

            // 정지 상태였고, 정지 동작을 한 게 아니고, 리셋도 안 했다면 -> 시작
            if (stw_run == 0 && action_handled == 0 && reset_done == 0) {
                stw_run = 1;
            }

            // 변수 초기화 (다음 누름을 위해)
            btn_state = 0;
            hold_cnt = 0;
            action_handled = 0;
            reset_done = 0;
        }
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
