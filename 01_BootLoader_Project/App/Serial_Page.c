#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"
#include <string.h>
#include <stdio.h>

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

uint16_t GetPacketLength(UCB_URxBuffptr *packet)
{
    // ������ݰ���Ч������0
    uint8_t index = packet - U0CB.URxDataPtr;
    if(index >= NUM || !U0CB.packetValid[index] || packet->start == NULL || packet->end == NULL)
        return 0;
    
    // ������ʼ�ͽ�������
    uint32_t start_idx = packet->start - U0_RxBuff;
    uint32_t end_idx = packet->end - U0_RxBuff;
    
    // �����������Ƶ����
    if(end_idx >= start_idx)
        return end_idx - start_idx + 1;
    else
        return (U0_RX_SIZE - start_idx) + end_idx + 1;
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
	for(i = 0; i < NUM; i++)
	{
		// ����Ƿ�����һ����Ҫ���������ݰ�
		if(i == U0CB.nextPacketIndex)
		{
			// �ڸ����·�����һ��С��������Ϊ���
			OLED_DrawLine(PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2 - 2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 2,
						 PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 5);
			OLED_DrawLine(PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 5,
						 PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2 + 2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 2);
		}
		// ������ݰ��Ƿ�����Ч����
		else if(U0CB.packetValid[i])
		{
			// �������ݰ�����
			uint16_t len = GetPacketLength(&U0CB.URxDataPtr[i]);
			
			// ������
			OLED_DrawRectangle(PACKET_GRID_X + i * GRID_CELL_WIDTH + 1, 
							  PACKET_GRID_Y + 1,
							  GRID_CELL_WIDTH - 2, 
							  PACKET_GRID_HEIGHT - 1, 
							  OLED_FILLED);
			
			// ��ʾ���ݰ����ȣ�����λ����ʾ����ֹ�ڵ���
			uint8_t lengthY;
			
			// ż��������ʾ���·�������������ʾ���Ϸ�
			if(i % 2 == 0)
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

// �򻯰棺�Ƴ����д�ӡ�߼������������ṹ���ɰ���ɾ������������
void Test_PrintReceivedData(void)
{
    uint8_t i;
    uint8_t temp_buf[U0_RX_MAX + 1];

    // �����������ݰ�
    for (i = 0; i < NUM; i++)
    {
        // ֻ������Ч���ݰ�
        if (U0CB.packetValid[i] && U0CB.URxDataPtr[i].start && U0CB.URxDataPtr[i].end)
        {
            uint8_t *start_ptr = U0CB.URxDataPtr[i].start;
            uint8_t *end_ptr = U0CB.URxDataPtr[i].end;
            uint16_t data_len = GetPacketLength(&U0CB.URxDataPtr[i]);
            uint16_t read_cnt = 0;

            // �����ʱ������
            memset(temp_buf, 0, sizeof(temp_buf));

            // ��ȡ���ݰ����ݵ���ʱ���������������ݶ�ȡ�߼����ɰ���ɾ����
            if (start_ptr <= end_ptr)
            {
                // �����������޿绺������
                while (start_ptr <= end_ptr && read_cnt < sizeof(temp_buf) - 1)
                {
                    temp_buf[read_cnt] = *start_ptr;
                    start_ptr++;
                    read_cnt++;
                }
            }
            else
            {
                // ���ݿ绺�������ȶ���������ĩβ��
                uint8_t *buf_end = U0_RxBuff + U0_RX_SIZE - 1;
                while (start_ptr <= buf_end && read_cnt < sizeof(temp_buf) - 1)
                {
                    temp_buf[read_cnt] = *start_ptr;
                    start_ptr++;
                    read_cnt++;
                }
                
                // �ٴӻ�������ͷ����end
                start_ptr = U0_RxBuff;
                while (start_ptr <= end_ptr && read_cnt < sizeof(temp_buf) - 1)
                {
                    temp_buf[read_cnt] = *start_ptr;
                    start_ptr++;
                    read_cnt++;
                }
            }

            // ȷ���ַ�����null��β����������ɾ�����ݶ�ȡ�߼���һ��ɾ����
            temp_buf[read_cnt] = '\0';
        }
    }
}

// ��������ҳ����ѭ��
void Serial_Page(void)
{
	int8_t delay = 100;
    while(1)
    {
		Draw_Serial_Page();
		delay--;
		if (delay <= 0)
		{
			delay = 100;
			Test_PrintReceivedData();  // ���������޴�ӡ���ɰ���ɾ���˵���
		}
    }
}
