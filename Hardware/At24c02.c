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
	
	AT24C02_W_SCL(1);											// ����I2C���򣬳�ʼ����Ϊ��
	AT24C02_W_SDA(1);											// ����I2C���򣬳�ʼ����Ϊ��
}

// ͨѶ��ʼ
void AT24C02_Start(void)
{
	AT24C02_W_SDA(1);											// �ͷ�SDA��ȷ��SDAΪ�ߵ�ƽ
	AT24C02_W_SCL(1);											// �ͷ�SCL��ȷ��SCLΪ�ߵ�ƽ
	AT24C02_W_SDA(0);											// ��SCL�ߵ�ƽ�ڼ䣬����SDA��������ʼ�ź�
	AT24C02_W_SCL(0);											// ��ʼ���SCLҲ���ͣ���Ϊ��ռ�����ߣ�ҲΪ�˷�������ʱ���ƴ��
}

// ͨѶ����
void AT24C02_Stop(void)
{
	AT24C02_W_SDA(0);											// ȷ��SDAΪ�͵�ƽ
	AT24C02_W_SCL(1);											// �ͷ�SCL��ȷ��SCLΪ�ߵ�ƽ
	AT24C02_W_SDA(1);											// ��SCL�ߵ�ƽ�ڼ䣬����SDA��������ֹ�ź�
}

// ����һ���ֽ�
void AT24C02_Send_Byte(uint8_t data)
{
	for (int i = 0; i < 8; i++)									// ѭ��8�Σ����η���8λ����
	{
		AT24C02_W_SDA((data & 0x80) >> 7);						// ȡdata�����λ��д��SDA
		AT24C02_W_SCL(1);										// �ͷ�SCL��ȷ��SCLΪ�ߵ�ƽ���ӻ���SCL�ߵ�ƽ�ڼ��ȡSDA
		AT24C02_W_SCL(0);										// ����SCL��������ʼ������һλ����
		data <<= 1;												// data����һλ��Ϊ������һλ������׼��
	}

	// ���ݷ�����ϣ��������Ĳ�����Ϊ�˴ӻ�Ӧ��
	// ֻ�е�SCLΪ�͵�ƽʱ���ӻ����������ܸı�SDA��״̬
	AT24C02_W_SCL(0);											// ����SCL������׼������Ӧ���ź�
	AT24C02_W_SDA(1);											// �ͷ�SDA��ȷ��SDAΪ�ߵ�ƽ���ӻ���������SDA��Ӧ��
}

// �ȴ��ӻ�Ӧ��
uint8_t AT24C02_Wait_Ack(int16_t timeout)
{
	do{
		timeout--;
		Delay_us(2);
	}
	while (AT24C02_R_SDA() && timeout >= 0);					// �ȴ�SDA�����ͣ���ʾ�ӻ�Ӧ��	
	if(timeout < 0) return 1;									// ����1����ʾ�ȴ�Ӧ��ʧ��
	AT24C02_W_SCL(1);											// �ͷ�SCL��ȷ��SCLΪ�ߵ�ƽ���ӻ���SCL�ߵ�ƽ�ڼ��ȡSDA
	if(AT24C02_R_SDA() != 0) return 2;							// ����2����ʾ�ӻ�δ����SDA��δӦ��
	AT24C02_W_SCL(0);											// ����SCL������׼��������һλ����
	return 0;													// ����0����ʾӦ��ɹ�
}

// ��һ���ֽ�
// ֻ���������������ʱ���Ż���ô˺���
uint8_t AT24C02_Read_Byte(uint8_t ack)
{
	uint8_t receive = 0;										// ���ڴ�Ŷ���������
	
	// �����ʹӻ�ʹ�õĶ��ǿ�©�������Ϊ�������裬SDAĬ��Ϊ��
	// �����ʹӻ�ֻ����SCLΪ�͵�ƽʱ�����ܸı�SDA��״̬
	// �����ʹӻ����Խ�SDA����

	// ֮����ʹ��������ƣ�����Ϊ�������߶�����������SDA������SDA
	// ���һ�����ߣ�һ�����ͣ���ô�ͻ���ֵ��������ջ�оƬ�����
	// ����״̬��Ҫ��ͬ

	// �����ʹӻ�����˵�ǿ���ռ��SDA�ߣ���һ������ʱ����һ����������
	// ֻ�е�һ���ͷ�SDAʱ����һ���������߻�����SDA
	// �����I2C���ߵ���ƹ���

	AT24C02_W_SDA(1);											// �ͷ�SDA��ȷ��SDAΪ�ߵ�ƽ���ӻ��������߻�����SDA�Է�������

	for (int i = 0; i < 8; i++)									// ѭ��8�Σ����ζ�ȡ8λ����
	{
		receive <<= 1;											// receive����һλ��Ϊ������һλ������׼��
		AT24C02_W_SCL(1);										// �ͷ�SCL��ȷ��SCLΪ�ߵ�ƽ���ӻ���SCL�ߵ�ƽ�ڼ�ı�SDA
		if (AT24C02_R_SDA()) receive |= 0x01;					// ��SDA��״̬��������receive�����λ
		AT24C02_W_SCL(0);										// ����SCL������׼��������һλ����
	}

	// ���ݶ�ȡ��ϣ��������Ĳ�����Ϊ������Ӧ��
	if (ack) AT24C02_W_SDA(0);									// ���ackΪ1������SDA����ʾ����Ӧ��
	else AT24C02_W_SDA(1);										// ���ackΪ0���ͷ�SDA����ʾ������Ӧ��

	AT24C02_W_SCL(1);                                           // ����SCL���ôӻ�ʶ��Ӧ���ź�
    AT24C02_W_SCL(0);                                           // ����SCL��Ϊ����������׼��

	return receive;												// ���ض���������
}

uint8_t AT24C02_WriteByte(uint8_t addr, uint8_t data)
{
	AT24C02_Start();											// ������ʼ�ź�
	AT24C02_Send_Byte(AT24C02_WADDR);							// ���ʹӻ���ַ+д����
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 1;												// ����1����ʾд��ʧ��
	}
	
	AT24C02_Send_Byte(addr);									// �������ݵ�ַ
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 2;												// ����2����ʾд��ʧ��
	}
	
	AT24C02_Send_Byte(data);									// ��������
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 3;												// ����3����ʾд��ʧ��
	}
	
	AT24C02_Stop();												// ������ֹ�ź�
	return 0;													// ����0����ʾд��ɹ�
}

uint8_t AT24C02_WritePage(uint8_t addr, uint8_t *data)
{
	AT24C02_Start();											// ������ʼ�ź�
	AT24C02_Send_Byte(AT24C02_WADDR);							// ���ʹӻ���ַ+д����
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 1;												// ����1����ʾд��ʧ��
	}
	
	AT24C02_Send_Byte(addr);									// �������ݵ�ַ
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 2;												// ����2����ʾд��ʧ��
	}
	
	for (int i = 0; i < 8; i++)									// ѭ��8�Σ����η���8λ����
	{
		AT24C02_Send_Byte(data[i]);								// ��������
		if (AT24C02_Wait_Ack(1000) != 0)						// �ȴ��ӻ�Ӧ��
		{
			AT24C02_Stop();										// ������ֹ�ź�
			return 3 + i;										// ����3+i����ʾд��ʧ��
		}
	}
	
	AT24C02_Stop();												// ������ֹ�ź�
	return 0;													// ����0����ʾд��ɹ�
}

uint8_t AT24C02_Read(uint8_t addr, uint8_t *data, uint16_t len)
{
	AT24C02_Start();											// ������ʼ�ź�
	AT24C02_Send_Byte(AT24C02_WADDR);							// ���ʹӻ���ַ+д����
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 1;												// ����1����ʾ��ȡʧ��
	}
	
	AT24C02_Send_Byte(addr);									// �������ݵ�ַ
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 2;												// ����2����ʾ��ȡʧ��
	}
	
	AT24C02_Start();											// ������ʼ�ź�
	AT24C02_Send_Byte(AT24C02_RADDR);							// ���ʹӻ���ַ+������
	if (AT24C02_Wait_Ack(1000) != 0)							// �ȴ��ӻ�Ӧ��
	{
		AT24C02_Stop();											// ������ֹ�ź�
		return 3;												// ����3����ʾ��ȡʧ��
	}
	
	for (int i = 0; i < len-1; i++)								// ��ȡlen-1���ֽ�
	{
		data[i] = AT24C02_Read_Byte(1);							// ��ȡ�����ֽں�����Ӧ��
	}
	data[len-1] = AT24C02_Read_Byte(0);							// ��ȡ���һ���ֽں�������Ӧ��

	AT24C02_Stop();												// ������ֹ�ź�
	return 0;													// ����0����ʾ��ȡ�ɹ�
}

void AT24C02_Init(void)
{
	AT24C02_GPIO_Init();
}
