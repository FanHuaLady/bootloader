#include "main.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"
#include "W25Q64.h"
#include "Flash_Manage.h"

#include <stdio.h>

void BootLoader_Branch(void)
{
    if(OTA_Info.OTA_Flag == OTA_SET_FLAG)
    {
        printf("Jump to BootLoader...\r\n");
    }
    else
    {
        printf("Jump to UserApp...\r\n");
    }
}
