#include "At24c02.h"
#include "Delay.h"

void AT24C02_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)BitValue);
}

void AT24C02_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)BitValue);
}

uint8_t AT24C02_R_SDA(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
}

void AT24C02_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	AT24C02_W_SCL(1);											// 根据I2C规则，初始设置为高
	AT24C02_W_SDA(1);											// 根据I2C规则，初始设置为高
}

// 通讯开始
void AT24C02_Start(void)
{
	AT24C02_W_SDA(1);											// 释放SDA，确保SDA为高电平
	AT24C02_W_SCL(1);											// 释放SCL，确保SCL为高电平
	AT24C02_W_SDA(0);											// 在SCL高电平期间，拉低SDA，产生起始信号
	AT24C02_W_SCL(0);											// 起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

// 通讯结束
void AT24C02_Stop(void)
{
	AT24C02_W_SDA(0);											// 确保SDA为低电平
	AT24C02_W_SCL(1);											// 释放SCL，确保SCL为高电平
	AT24C02_W_SDA(1);											// 在SCL高电平期间，拉高SDA，产生终止信号
}

// 发送一个字节
void AT24C02_Send_Byte(uint8_t data)
{
	for (int i = 0; i < 8; i++)									// 循环8次，依次发送8位数据
	{
		AT24C02_W_SDA((data & 0x80) >> 7);						// 取data的最高位，写入SDA
		AT24C02_W_SCL(1);										// 释放SCL，确保SCL为高电平，从机在SCL高电平期间读取SDA
		AT24C02_W_SCL(0);										// 拉低SCL，主机开始发送下一位数据
		data <<= 1;												// data左移一位，为发送下一位数据做准备
	}

	// 数据发送完毕，接下来的操作是为了从机应答
	// 只有当SCL为低电平时，从机和主机才能改变SDA的状态
	AT24C02_W_SCL(0);											// 拉低SCL，主机准备接收应答信号
	AT24C02_W_SDA(1);											// 释放SDA，确保SDA为高电平，从机可以拉低SDA以应答
}

// 等待从机应答
uint8_t AT24C02_Wait_Ack(int16_t timeout)
{
	do{
		timeout--;
		Delay_us(2);
	}
	while (AT24C02_R_SDA() && timeout >= 0);					// 等待SDA被拉低，表示从机应答	
	if(timeout < 0) return 1;									// 返回1，表示等待应答失败
	AT24C02_W_SCL(1);											// 释放SCL，确保SCL为高电平，从机在SCL高电平期间读取SDA
	if(AT24C02_R_SDA() != 0) return 2;							// 返回2，表示从机未拉低SDA，未应答
	AT24C02_W_SCL(0);											// 拉低SCL，主机准备发送下一位数据
	return 0;													// 返回0，表示应答成功
}

// 读一个字节
// 只有主机发起读操作时，才会调用此函数
uint8_t AT24C02_Read_Byte(uint8_t ack)
{
	uint8_t receive = 0;										// 用于存放读到的数据
	
	// 主机和从机使用的都是开漏输出，因为上拉电阻，SDA默认为高
	// 主机和从机只有在SCL为低电平时，才能改变SDA的状态
	// 主机和从机可以将SDA拉低

	// 之所以使用这种设计，是因为不能两边都能随意拉高SDA和拉低SDA
	// 如果一边拉高，一边拉低，那么就会出现电流过大，烧坏芯片的情况
	// 两边状态需要相同

	// 主机和从机可以说是可以占用SDA线，当一方拉低时，另一方不能拉高
	// 只有当一方释放SDA时，另一方才能拉高或拉低SDA
	// 这就是I2C总线的设计规则

	AT24C02_W_SDA(1);											// 释放SDA，确保SDA为高电平，从机可以拉高或拉低SDA以发送数据

	for (int i = 0; i < 8; i++)									// 循环8次，依次读取8位数据
	{
		receive <<= 1;											// receive左移一位，为接收下一位数据做准备
		AT24C02_W_SCL(1);										// 释放SCL，确保SCL为高电平，从机在SCL高电平期间改变SDA
		if (AT24C02_R_SDA()) receive |= 0x01;					// 读SDA的状态，并存入receive的最低位
		AT24C02_W_SCL(0);										// 拉低SCL，主机准备接收下一位数据
	}

	// 数据读取完毕，接下来的操作是为了主机应答
	if (ack) AT24C02_W_SDA(0);									// 如果ack为1，拉低SDA，表示主机应答
	else AT24C02_W_SDA(1);										// 如果ack为0，释放SDA，表示主机不应答

	AT24C02_W_SCL(1);                                           // 拉高SCL，让从机识别应答信号
    AT24C02_W_SCL(0);                                           // 拉低SCL，为后续操作做准备

	return receive;												// 返回读到的数据
}

uint8_t AT24C02_WriteByte(uint8_t addr, uint8_t data)
{
	AT24C02_Start();											// 发送起始信号
	AT24C02_Send_Byte(AT24C02_WADDR);							// 发送从机地址+写命令
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 1;												// 返回1，表示写入失败
	}
	
	AT24C02_Send_Byte(addr);									// 发送数据地址
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 2;												// 返回2，表示写入失败
	}
	
	AT24C02_Send_Byte(data);									// 发送数据
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 3;												// 返回3，表示写入失败
	}
	
	AT24C02_Stop();												// 发送终止信号
	return 0;													// 返回0，表示写入成功
}

uint8_t AT24C02_WritePage(uint8_t addr, uint8_t *data)
{
	AT24C02_Start();											// 发送起始信号
	AT24C02_Send_Byte(AT24C02_WADDR);							// 发送从机地址+写命令
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 1;												// 返回1，表示写入失败
	}
	
	AT24C02_Send_Byte(addr);									// 发送数据地址
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 2;												// 返回2，表示写入失败
	}
	
	for (int i = 0; i < 8; i++)									// 循环8次，依次发送8位数据
	{
		AT24C02_Send_Byte(data[i]);								// 发送数据
		if (AT24C02_Wait_Ack(1000) != 0)						// 等待从机应答
		{
			AT24C02_Stop();										// 发送终止信号
			return 3 + i;										// 返回3+i，表示写入失败
		}
	}
	
	AT24C02_Stop();												// 发送终止信号
	return 0;													// 返回0，表示写入成功
}

uint8_t AT24C02_Read(uint8_t addr, uint8_t *data, uint16_t len)
{
	AT24C02_Start();											// 发送起始信号
	AT24C02_Send_Byte(AT24C02_WADDR);							// 发送从机地址+写命令
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 1;												// 返回1，表示读取失败
	}
	
	AT24C02_Send_Byte(addr);									// 发送数据地址
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 2;												// 返回2，表示读取失败
	}
	
	AT24C02_Start();											// 发送起始信号
	AT24C02_Send_Byte(AT24C02_RADDR);							// 发送从机地址+读命令
	if (AT24C02_Wait_Ack(1000) != 0)							// 等待从机应答
	{
		AT24C02_Stop();											// 发送终止信号
		return 3;												// 返回3，表示读取失败
	}
	
	for (int i = 0; i < len-1; i++)								// 读取len-1个字节
	{
		data[i] = AT24C02_Read_Byte(1);							// 读取其他字节后，主机应答
	}
	data[len-1] = AT24C02_Read_Byte(0);							// 读取最后一个字节后，主机不应答

	AT24C02_Stop();												// 发送终止信号
	return 0;													// 返回0，表示读取成功
}

void AT24C02_Init(void)
{
	AT24C02_GPIO_Init();
}
