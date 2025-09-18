#include "W25Q64.h"

void W25Q64_W_CS(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)BitValue);
}

void W25Q64_SPI_Start(void)
{
    W25Q64_W_CS(0);
}

void W25Q64_SPI_Stop(void)
{
    W25Q64_W_CS(1);
}

// 初始化W25Q64的GPIO和SPI
void W25Q64_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                                   // 片选
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;                      // SCK和MOSI 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;                                   // MISO
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}

// SPI读写一个字节
uint8_t W25Q64_SPI_SwapByte(uint8_t data)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);              // 等待发送区空
    SPI_I2S_SendData(SPI1, data);                                               // 发送一个字节                 
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);             // 等待接收区非空
    return SPI_I2S_ReceiveData(SPI1);                                           // 返回收到的数据
}

// 写数据
void W25Q64_SPI_Write(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        W25Q64_SPI_SwapByte(data[i]);
    }
}

// 读数据
void W25Q64_SPI_Read(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = W25Q64_SPI_SwapByte(0xFF);
    }
}

void W25Q64_Init(void)
{
    W25Q64_GPIO_Init();
}

// 等待W25Q64忙结束
// 每次操作前都要调用此函数
void W25Q64_WaitBusy(void)
{
    uint8_t res;
    do{
        W25Q64_SPI_Start();
        W25Q64_SPI_SwapByte(0x05);                                              // 读状态寄存器
        res = W25Q64_SPI_SwapByte(0xFF);
        W25Q64_SPI_Stop();
    }while(res & 0x01 == 0x01);                                                 // 等待忙结束
}

// 写使能
void W25Q64_WriteEnable(void)
{
    W25Q64_WaitBusy();                                                          // 等待忙结束
    W25Q64_SPI_Start();
    W25Q64_SPI_SwapByte(0x06);                                                  // 写使能
    W25Q64_SPI_Stop();
}

// 擦除一个64K的块
// block_id: 块ID，0~127
void W25Q64_Erase64K(uint8_t block_id)
{
    uint8_t data[4];

    data[0] = 0xD8;                                                             // 批量擦除命令
    data[1] = (block_id * 64 * 1024) >> 16;                                     // 地址高8位
    data[2] = (block_id * 64 * 1024) >> 8;                                      // 地址中8位
    data[3] = (block_id * 64 * 1024);                                           // 地址低8位

    W25Q64_WaitBusy();                                                          // 等待忙结束
    W25Q64_WriteEnable();                                                       // 写使能
    W25Q64_SPI_Start();
    W25Q64_SPI_Write(data, 4);                                                  // 发送命令和地址
    W25Q64_SPI_Stop();
    W25Q64_WaitBusy();                                                          // 等待擦除结束
}

void W25Q64_WritePage(uint8_t *buff, uint16_t page_id)
{
    uint8_t data[4];

    data[0] = 0x02;                                                             // 页编程命令
    data[1] = (page_id * 256) >> 16;                                            // 地址高8位
    data[2] = (page_id * 256) >> 8;                                             // 地址中8位
    data[3] = (page_id * 256);                                                  // 地址低8位

    W25Q64_WaitBusy();                                                          // 等待忙结束
    W25Q64_WriteEnable();                                                       // 写使能
    W25Q64_SPI_Start();
    W25Q64_SPI_Write(data, 4);                                                  // 发送命令和地址
    W25Q64_SPI_Write(buff, 256);                                                // 写数据
    W25Q64_SPI_Stop();
}

void W25Q64_Read(uint8_t *buff, uint32_t addr, uint32_t len)
{
    uint8_t data[4];

    data[0] = 0x03;                                                             // 读数据命令
    data[1] = (addr) >> 16;                                                     // 地址高8位
    data[2] = (addr) >> 8;                                                      // 地址中8位
    data[3] = (addr);                                                           // 地址低8位

    W25Q64_WaitBusy();                                                          // 等待忙结束
    W25Q64_SPI_Start();
    W25Q64_SPI_Write(data, 4);                                                  // 发送命令和地址
    W25Q64_SPI_Read(buff, len);                                                 // 读数据
    W25Q64_SPI_Stop();
}
