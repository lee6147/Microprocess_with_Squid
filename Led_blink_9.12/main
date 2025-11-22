#include <iom128v.h> // 9/12

// 함수 선언
void delay_m(unsigned int m);

int main(void)
{
    // PC6과 PC4 핀을 출력으로 설정합니다.
    DDRC = 0x50;  // 0101 0000 -> DDRC 레지스터의 bit6, bit4를 1로 설정

    while(1) {
        // PORTC의 6번, 4번 핀에 HIGH 신호를 출력하여 LED를 켭니다.
        PORTC = 0x50;  // 0101 0000 -> PC6, PC4 ON
        delay_m(500);

        // PORTC의 모든 핀에 LOW 신호를 출력하여 LED를 끕니다.
        PORTC = 0x00;  // 0000 0000 -> PC6, PC4 OFF
        delay_m(500);
    }
    return 0;
}

// m 밀리초(ms) 만큼 지연시키는 함수
void delay_m(unsigned int m) {
    unsigned int i, j;
    for (i = 0; i < m; i++) {
        for (j = 0; j < 2100; j++) {
            ; // 아무 동작도 하지 않고 시간 지연
        }
    }
}
