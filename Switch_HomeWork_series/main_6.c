#include <iom128v.h>

void delay_m(unsigned int m);

// FND에 표시할 'L'OW(0x74), 'H'IGH(0x22) 모양 데이터 배열입니다.
unsigned char LowHigh[2] = {0x74, 0x22};

int main(void)
{
    // --- 1. 초기 설정 ---

    // 스위치가 연결된 E포트 4번(PE4)과 5번(PE5)핀을 '입력'으로 설정합니다.
    // DDRE 레지스터의 4번, 5번 비트만 0으로 만들어야 합니다. (2진수 11001111 = 16진수 0xCF)
    DDRE = 0xCF;

    // FND가 연결된 D포트 8개 핀 모두를 '출력'으로 설정합니다.
    DDRD = 0xFF;

    // --- 2. 무한 반복 실행 ---
    while(1)
    {
        // --- 3. 두 개의 마스크를 이용한 스위치 상태 확인 ---
        // if - else if - else 구조를 사용하여, 한 번에 하나의 조건만 실행되도록 합니다.

        // 조건 1: 만약 5번 스위치를 눌러서 5번 핀의 상태가 '1'(HIGH) 이라면?
        // 0x20 (00100000) 마스크로 5번 핀의 상태를 확인합니다.
        if ((PINE & 0x20) == 0x20)
        {
            // FND에 'H' 모양(LowHigh[1])을 출력합니다.
            PORTD = LowHigh[1];
        }
        // 조건 2: 5번 스위치가 안 눌렸고, 4번 스위치를 눌러 4번 핀이 '1'(HIGH) 이라면?
        // 0x10 (00010000) 마스크로 4번 핀의 상태를 확인합니다.
        else if ((PINE & 0x10) == 0x10)
        {
            // FND에 'L' 모양(LowHigh[0])을 출력합니다.
            PORTD = LowHigh[0];
        }
        // 조건 3: 두 스위치 모두 눌리지 않았다면?
        else
        {
            // FND의 모든 세그먼트를 끕니다.
            // 커먼 애노드 방식이므로, 모든 핀에 1(HIGH)을 보내면 전류가 흐르지 않아 꺼집니다.
            PORTD = 0xFF;
        }
    }

    return 0;
}

// '잠시 멈춤' 기능을 실제로 구현하는 부분
void delay_m(unsigned int m)
{
    unsigned int i, j;
    for(i=0; i<m; i++)
    {
        for(j=0; j<2100; j++)
        {
            ;
        }
    }
}
