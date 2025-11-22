#include <iom128v.h>

void delay_m(unsigned int m);

// FND에 표시할 'LOW'(0x74), 'HIGH'(0x22) 모양 데이터 배열입니다.
unsigned char LowHigh[2] = {0x74, 0x22};

int main(void)
{
    // --- 1. 초기 설정 ---

    // 스위치가 연결된 E포트 5번(PE5)핀을 '입력'으로 설정합니다.
    // DDRE 레지스터의 5번 비트만 0으로 만들어야 합니다. (2진수 11011111 = 16진수 0xDF)
    DDRE = 0xDF;

    // FND가 연결된 D포트 8개 핀 모두를 '출력'으로 설정합니다.
    DDRD = 0xFF;

    // --- 2. 무한 반복 실행 ---
    while(1)
    {
        // --- 3. 마스킹을 이용한 스위치 상태 확인 ---

        // PINE 레지스터 값을 읽어온 뒤, 0x20 (2진수 00100000) 가면(마스크)을 씌웁니다.
        // '&' 연산을 통해 다른 핀의 상태는 모두 0으로 지우고,
        // 오직 우리가 궁금한 5번 핀의 상태(0 또는 1)만 남깁니다.

        // 만약 스위치를 누르지 않아서 5번 핀의 상태가 '0'(LOW) 이라면?
        if ((PINE & 0x20) == 0x00)
        {
            // FND에 'LOW' 모양(LowHigh[0])을 출력합니다.
            PORTD = LowHigh[0];
        }
        // 그게 아니라면 (스위치를 눌러서 5번 핀의 상태가 '1'(HIGH) 이라면)?
        if((PINE & 0x20) == 0x20)
        {
            // FND에 'HIGH' 모양(LowHigh[1])을 출력합니다.
            PORTD = LowHigh[1];
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
