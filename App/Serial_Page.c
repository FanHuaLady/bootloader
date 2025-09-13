#include "stm32f10x.h"
#include "Serial.h"
#include "OLED.h"
#include <string.h>

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

// 计算数据包长度
uint16_t GetPacketLength(UCB_URxBuffptr *packet)
{
    // 处理缓冲区环绕的情况
    if (packet->end >= packet->start)
        return packet->end - packet->start + 1;
    else
        return (U0_RxBuff + U0_RX_SIZE - packet->start) + (packet->end - U0_RxBuff) + 1;
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
    for (i = 0; i < NUM; i++)
    {
        // 检查是否是已接收的数据包
        if (&U0CB.URxDataPtr[i] < U0CB.URxDataIN)
        {
            // 填充格子
            OLED_DrawRectangle(PACKET_GRID_X + i * GRID_CELL_WIDTH + 1, 
                              PACKET_GRID_Y + 1,
                              GRID_CELL_WIDTH - 2, 
                              PACKET_GRID_HEIGHT - 1, 
                              OLED_FILLED);
            
            // 显示数据包长度（交替位置显示，防止遮挡）
            uint16_t len = GetPacketLength(&U0CB.URxDataPtr[i]);
            uint8_t lengthY;
            
            // 偶数索引显示在下方，奇数索引显示在上方
            if (i % 2 == 0)
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

// 串口数据页面主循环
void Serial_Page(void)
{
    while(1)
    {

		Draw_Serial_Page();

    }
}
