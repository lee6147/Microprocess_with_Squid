#include <iom128v.h>

void delay_m(unsigned int m);

unsigned char number[10] = {0x04, 0x2F, 0x18, 0x09, 0x23, 0x41, 0x40, 0x07, 0x00, 0x01};

int main(void)
{
    char count = 0;
    char is_running = 1; // '실행' 상태에서 시작

    DDRE = 0xCF;
    DDRD = 0xFF;

    while(1)
    {
        // --- 1. 버튼 입력 처리 ---
        if ((PINE & 0x10) == 0x10)
        {
            // 채터링 방지를 위한 짧은 딜레이 (디바운싱)
            delay_m(30);

            // 딜레이 후에도 여전히 눌려있다면, 실제 입력으로 간주
            if ((PINE & 0x10) == 0x10)
            {
                if (is_running == 1)
                {
                    is_running = 0; // 실행 중이었다면 '정지'로 변경
                }
                else
                {
                    is_running = 1; // 정지 상태였다면 '실행'으로 변경
                }
                // 버튼에서 손을 뗄 때까지 대기 (중복 토글 방지)
                while ((PINE & 0x10) == 0x10);
            }
        }

        // --- 2. 상태에 따른 카운터 값 변경 ---
        if (is_running == 1)
        {
            delay_m(500); // 먼저 0.5초 대기

            count++;
            if (count == 10)
            {
                count = 0;
            }
        }

        // --- 3. FND 출력 (한 번만 실행) ---
        //카운팅 중이든, 멈춤 상태이든 항상 현재 count 값을 출력
        PORTD = number[count];
    }
    return 0;
}

void delay_m(unsigned int m)
{
    unsigned int i, j;
    for(i = 0; i < m; i++)
    {
        for(j = 0; j < 2100; j++);
    }
}
