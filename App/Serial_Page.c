#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"
#include <string.h>

extern UCB_CB U0CB;
extern uint8_t U0_RxBuff[U0_RX_SIZE];

// ����UI����λ�úʹ�С
#define BUFFER_BAR_X        4
#define BUFFER_BAR_Y        8
#define BUFFER_BAR_WIDTH    120
#define BUFFER_BAR_HEIGHT   10

// ����ͳ����ʾλ��
#define PERCENT_X           BUFFER_BAR_X
#define TOTAL_DATA_X        (BUFFER_BAR_X + 40)  // �ٷֱ��Ҳ���ʾ��������
#define STAT_Y              (BUFFER_BAR_Y + BUFFER_BAR_HEIGHT + 1)

#define PACKET_GRID_X       4
#define PACKET_GRID_Y       37
#define PACKET_GRID_WIDTH   120
#define PACKET_GRID_HEIGHT  10
#define GRID_CELL_WIDTH     (PACKET_GRID_WIDTH / NUM)  // ÿ�����ݰ����ӵĿ��

// ���ݰ�������ʾλ��ƫ�ƣ�������ʾ�ã�
#define LENGTH_POS_DOWN     (PACKET_GRID_Y + PACKET_GRID_HEIGHT + 1)  // �����·�
#define LENGTH_POS_UP       (LENGTH_POS_DOWN + 8)   // �����Ϸ�

// �������ݰ�����
uint16_t GetPacketLength(UCB_URxBuffptr *packet)
{
    // �����������Ƶ����
    if (packet->end >= packet->start)
        return packet->end - packet->start + 1;
    else
        return (U0_RxBuff + U0_RX_SIZE - packet->start) + (packet->end - U0_RxBuff) + 1;
}

// ���ƴ�������ҳ��
void Draw_Serial_Page(void)
{
    uint8_t i;
    uint16_t usedPercentage;
    uint16_t filledWidth;
    
    // �����Ļ
    OLED_Clear();
    
	OLED_ShowString(4,0,"Buffer",OLED_6X8);
	OLED_ShowString(4,29,"Data",OLED_6X8);
	
    // 1. �����ܻ�����״̬
    // �������
    OLED_DrawRectangle(BUFFER_BAR_X, BUFFER_BAR_Y, 
                      BUFFER_BAR_WIDTH, BUFFER_BAR_HEIGHT, 
                      OLED_UNFILLED);
    
    // ������ʹ�ñ��������
    usedPercentage = (U0CB.URxCounter * 100) / U0_RX_SIZE;
    filledWidth = (U0CB.URxCounter * BUFFER_BAR_WIDTH) / U0_RX_SIZE;
    
    OLED_DrawRectangle(BUFFER_BAR_X, BUFFER_BAR_Y, 
                      filledWidth, BUFFER_BAR_HEIGHT, 
                      OLED_FILLED);
    
    // ��ʾ������ʹ�ðٷֱ�
    OLED_Printf(PERCENT_X, STAT_Y, OLED_6X8, "%d%%", usedPercentage);
    
    // ��ʾ�����������������ܣ�
    OLED_Printf(TOTAL_DATA_X, STAT_Y, OLED_6X8, "Total: %d", U0CB.URxCounter);
    
    // 2. �������ݰ���������
    // �������
    OLED_DrawRectangle(PACKET_GRID_X, PACKET_GRID_Y, 
                      PACKET_GRID_WIDTH, PACKET_GRID_HEIGHT, 
                      OLED_UNFILLED);
    
    // �����ڲ����ӷָ���
    for (i = 1; i < NUM; i++)
    {
        OLED_DrawLine(PACKET_GRID_X + i * GRID_CELL_WIDTH, PACKET_GRID_Y,
                     PACKET_GRID_X + i * GRID_CELL_WIDTH, PACKET_GRID_Y + PACKET_GRID_HEIGHT - 1);
    }
    
    // ����ѽ��յ����ݰ�����
    for (i = 0; i < NUM; i++)
    {
        // ����Ƿ����ѽ��յ����ݰ�
        if (&U0CB.URxDataPtr[i] < U0CB.URxDataIN)
        {
            // ������
            OLED_DrawRectangle(PACKET_GRID_X + i * GRID_CELL_WIDTH + 1, 
                              PACKET_GRID_Y + 1,
                              GRID_CELL_WIDTH - 2, 
                              PACKET_GRID_HEIGHT - 1, 
                              OLED_FILLED);
            
            // ��ʾ���ݰ����ȣ�����λ����ʾ����ֹ�ڵ���
            uint16_t len = GetPacketLength(&U0CB.URxDataPtr[i]);
            uint8_t lengthY;
            
            // ż��������ʾ���·�������������ʾ���Ϸ�
            if (i % 2 == 0)
                lengthY = LENGTH_POS_DOWN;
            else
                lengthY = LENGTH_POS_UP;
            
            // ������ʾ�ڸ����·�/�Ϸ�
            OLED_ShowNum(PACKET_GRID_X + i * GRID_CELL_WIDTH + (GRID_CELL_WIDTH / 2 - 8),
                        lengthY, len, 3, OLED_6X8);
        }
    }
    
    // ������ʾ
    OLED_Update();
}

// ��������ҳ����ѭ��
void Serial_Page(void)
{
    while(1)
    {

		Draw_Serial_Page();

    }
}
