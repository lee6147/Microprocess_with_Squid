#include <iom128v.h>

void delay_m(unsigned int m);

// FND에 표시할 숫자 데이터 (Common Anode 타입 기준)
// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
unsigned char number[10] = {0x04, 0x2F, 0x18, 0x09, 0x23, 0x41, 0x40, 0x07, 0x00, 0x01};

int main(void)
{
    char count = 0;
    char is_running = 0; // 초기 상태는 '정지'

    // --- 1. 포트 초기 설정 ---
    // 풀다운 저항을 사용하므로 내부 풀업 저항은 비활성화합니다. (기존 코드와 동일)
    PORTE = 0x00;

    // Timer/Counter 3 기능 비활성화 (PE4, PE5를 I/O 포트로 사용)
    TCCR3A = 0x00;
    TCCR3B = 0x00;

    // --- 2. 포트 방향 설정 ---
    DDRE = 0xCF; // PE4(입력), PE5(입력)로 설정 (0b11001111) (기존 코드와 동일)
    DDRD = 0xFF; // D포트 전체를 출력으로 설정 (기존 코드와 동일)

    // --- 3. 무한 반복 실행 ---
    while(1)
    {
        // ================== 변경된 부분 시작 ================== //

        // '시작/재개' 버튼(PE5)이 눌렸는지 확인 (눌렀을 때 HIGH)
        if ((PINE & 0x20) == 0x20) // PINE의 5번째 비트가 1이면 (버튼 누름)
        {
            is_running = 1; // 카운팅 시작/재개
        }
        // '정지' 버튼(PE4)이 눌렸는지 확인 (눌렀을 때 HIGH)
        else if ((PINE & 0x10) == 0x10) // PINE의 4번째 비트가 1이면 (버튼 누름)
        {
            is_running = 0; // 카운팅 정지
        }

        // ================== 변경된 부분 끝 ==================== //


        // 'is_running' 상태가 1일 때만 카운트 증가 및 딜레이 적용
        if (is_running == 1)
        {
            count++;
            if (count == 10)
            {
                count = 0;
            }
            delay_m(500); // 약 0.5초 딜레이
        }

        // 현재 count 값을 FND에 항상 표시
        PORTD = number[count];
    }
    return 0;
}

// 딜레이 함수 (기존 코드와 동일)
void delay_m(unsigned int m)
{
    unsigned int i, j;
    for(i = 0; i < m; i++)
    {
        for(j = 0; j < 2100; j++)
        {
            ; // 아무 동작도 하지 않음 (시간 지연 목적)
        }
    }
}
