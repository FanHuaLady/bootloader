#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"
#include <string.h>
#include <stdio.h>

extern UCB_CB U0CB;
extern uint8_t U0_RxBuff[U0_RX_SIZE];

// 定义UI区域位置和大小
#define BUFFER_BAR_X        4
#define BUFFER_BAR_Y        8
#define BUFFER_BAR_WIDTH    120
#define BUFFER_BAR_HEIGHT   10

// 数据统计显示位置
#define PERCENT_X           BUFFER_BAR_X
#define TOTAL_DATA_X        (BUFFER_BAR_X + 40)  // 百分比右侧显示总数据量
#define STAT_Y              (BUFFER_BAR_Y + BUFFER_BAR_HEIGHT + 1)

#define PACKET_GRID_X       4
#define PACKET_GRID_Y       37
#define PACKET_GRID_WIDTH   120
#define PACKET_GRID_HEIGHT  10
#define GRID_CELL_WIDTH     (PACKET_GRID_WIDTH / NUM)  // 每个数据包格子的宽度

// 数据包长度显示位置偏移（交替显示用）
#define LENGTH_POS_DOWN     (PACKET_GRID_Y + PACKET_GRID_HEIGHT + 1)  // 格子下方
#define LENGTH_POS_UP       (LENGTH_POS_DOWN + 8)   // 格子上方

uint16_t GetPacketLength(UCB_URxBuffptr *packet)
{
    // 如果数据包无效，返回0
    uint8_t index = packet - U0CB.URxDataPtr;
    if(index >= NUM || !U0CB.packetValid[index] || packet->start == NULL || packet->end == NULL)
        return 0;
    
    // 计算起始和结束索引
    uint32_t start_idx = packet->start - U0_RxBuff;
    uint32_t end_idx = packet->end - U0_RxBuff;
    
    // 处理缓冲区环绕的情况
    if(end_idx >= start_idx)
        return end_idx - start_idx + 1;
    else
        return (U0_RX_SIZE - start_idx) + end_idx + 1;
}

// 绘制串口数据页面
void Draw_Serial_Page(void)
{
    uint8_t i;
    uint16_t usedPercentage;
    uint16_t filledWidth;
    
    // 清空屏幕
    OLED_Clear();
    
    OLED_ShowString(4,0,"Buffer",OLED_6X8);
    OLED_ShowString(4,29,"Data",OLED_6X8);
    
    // 1. 绘制总缓冲区状态
    // 绘制外框
    OLED_DrawRectangle(BUFFER_BAR_X, BUFFER_BAR_Y, 
                      BUFFER_BAR_WIDTH, BUFFER_BAR_HEIGHT, 
                      OLED_UNFILLED);
    
    // 计算已使用比例并填充
    usedPercentage = (U0CB.URxCounter * 100) / U0_RX_SIZE;
    filledWidth = (U0CB.URxCounter * BUFFER_BAR_WIDTH) / U0_RX_SIZE;
    
    OLED_DrawRectangle(BUFFER_BAR_X, BUFFER_BAR_Y, 
                      filledWidth, BUFFER_BAR_HEIGHT, 
                      OLED_FILLED);
    
    // 显示缓冲区使用百分比
    OLED_Printf(PERCENT_X, STAT_Y, OLED_6X8, "%d%%", usedPercentage);
    
    // 显示总数据量（新增功能）
    OLED_Printf(TOTAL_DATA_X, STAT_Y, OLED_6X8, "Total: %d", U0CB.URxCounter);
    
    // 2. 绘制数据包格子区域
    // 绘制外框
    OLED_DrawRectangle(PACKET_GRID_X, PACKET_GRID_Y, 
                      PACKET_GRID_WIDTH, PACKET_GRID_HEIGHT, 
                      OLED_UNFILLED);
    
    // 绘制内部格子分隔线
    for (i = 1; i < NUM; i++)
    {
        OLED_DrawLine(PACKET_GRID_X + i * GRID_CELL_WIDTH, PACKET_GRID_Y,
                     PACKET_GRID_X + i * GRID_CELL_WIDTH, PACKET_GRID_Y + PACKET_GRID_HEIGHT - 1);
    }
    
    // 填充已接收的数据包格子
	for(i = 0; i < NUM; i++)
	{
		// 检查是否是下一个将要被填充的数据包
		if(i == U0CB.nextPacketIndex)
		{
			// 在格子下方绘制一个小三角形作为标记
			OLED_DrawLine(PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2 - 2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 2,
						 PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 5);
			OLED_DrawLine(PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 5,
						 PACKET_GRID_X + i * GRID_CELL_WIDTH + GRID_CELL_WIDTH/2 + 2,
						 PACKET_GRID_Y + PACKET_GRID_HEIGHT + 2);
		}
		// 检查数据包是否有有效数据
		else if(U0CB.packetValid[i])
		{
			// 计算数据包长度
			uint16_t len = GetPacketLength(&U0CB.URxDataPtr[i]);
			
			// 填充格子
			OLED_DrawRectangle(PACKET_GRID_X + i * GRID_CELL_WIDTH + 1, 
							  PACKET_GRID_Y + 1,
							  GRID_CELL_WIDTH - 2, 
							  PACKET_GRID_HEIGHT - 1, 
							  OLED_FILLED);
			
			// 显示数据包长度（交替位置显示，防止遮挡）
			uint8_t lengthY;
			
			// 偶数索引显示在下方，奇数索引显示在上方
			if(i % 2 == 0)
				lengthY = LENGTH_POS_DOWN;
			else
				lengthY = LENGTH_POS_UP;
			
			// 居中显示在格子下方/上方
			OLED_ShowNum(PACKET_GRID_X + i * GRID_CELL_WIDTH + (GRID_CELL_WIDTH / 2 - 8),
						lengthY, len, 3, OLED_6X8);
		}
	}
    
    // 更新显示
    OLED_Update();
}

// 简化版：移除所有打印逻辑，保留函数结构（可按需删除整个函数）
void Test_PrintReceivedData(void)
{
    uint8_t i;
    uint8_t temp_buf[U0_RX_MAX + 1];

    // 遍历所有数据包
    for (i = 0; i < NUM; i++)
    {
        // 只处理有效数据包
        if (U0CB.packetValid[i] && U0CB.URxDataPtr[i].start && U0CB.URxDataPtr[i].end)
        {
            uint8_t *start_ptr = U0CB.URxDataPtr[i].start;
            uint8_t *end_ptr = U0CB.URxDataPtr[i].end;
            uint16_t data_len = GetPacketLength(&U0CB.URxDataPtr[i]);
            uint16_t read_cnt = 0;

            // 清空临时缓冲区
            memset(temp_buf, 0, sizeof(temp_buf));

            // 读取数据包内容到临时缓冲区（保留数据读取逻辑，可按需删除）
            if (start_ptr <= end_ptr)
            {
                // 数据连续（无跨缓冲区）
                while (start_ptr <= end_ptr && read_cnt < sizeof(temp_buf) - 1)
                {
                    temp_buf[read_cnt] = *start_ptr;
                    start_ptr++;
                    read_cnt++;
                }
            }
            else
            {
                // 数据跨缓冲区（先读至缓冲区末尾）
                uint8_t *buf_end = U0_RxBuff + U0_RX_SIZE - 1;
                while (start_ptr <= buf_end && read_cnt < sizeof(temp_buf) - 1)
                {
                    temp_buf[read_cnt] = *start_ptr;
                    start_ptr++;
                    read_cnt++;
                }
                
                // 再从缓冲区开头读至end
                start_ptr = U0_RxBuff;
                while (start_ptr <= end_ptr && read_cnt < sizeof(temp_buf) - 1)
                {
                    temp_buf[read_cnt] = *start_ptr;
                    start_ptr++;
                    read_cnt++;
                }
            }

            // 确保字符串以null结尾（保留，若删除数据读取逻辑可一并删除）
            temp_buf[read_cnt] = '\0';
        }
    }
}

// 串口数据页面主循环
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
			Test_PrintReceivedData();  // 函数内已无打印，可按需删除此调用
		}
    }
}
