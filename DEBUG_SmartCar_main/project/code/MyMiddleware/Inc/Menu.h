#ifndef MEAU_H_
#define MEAU_H_

#include "UserMain.h"

void Menu_Display();
void Flash_Init();
//�еĺ궨��
#define Line_0 (16*0)//������
#define Line_1 (16*1)//��һ��
#define Line_2 (16*2)//�ڶ���
#define Line_3 (16*3)//������
#define Line_4 (16*4)//������
#define Line_5 (16*5)//������
#define Line_6 (16*6)//������
#define Line_7 (16*7)//������

//�еĺ궨��
#define Row_0 (8*0)//������
#define Row_1 (8*1)//��һ��
#define Row_2 (8*2)//�ڶ���
#define Row_3 (8*3)//������
#define Row_4 (8*4)//������
#define Row_5 (8*5)//������
#define Row_6 (8*6)//������
#define Row_7 (8*7)//������
#define Row_8 (8*8)//�ڰ���
#define Row_9 (8*9)//�ھ���
#define Row_10 (8*10)//��ʮ��
#define Row_11 (8*11)//��ʮһ��
#define Row_12 (8*12)//��ʮ����
#define Row_13 (8*13)//��ʮ����
#define Row_14 (8*14)//��ʮ����
#define Row_15 (8*15)//��ʮ����
#define Row_16 (8*16)//��ʮ����
#define Row_17 (8*17)//��ʮ����
#define Row_18 (8*18)//��ʮ����
#define Row_19 (8*19)//��ʮ����
#define Row_20 (8*20)//�ڶ�ʮ��

typedef enum
{
	Page0 = 0,
	Page1 = 1,
	Page2 = 2,
	Page3 = 3,
	Page4 = 4,
	Page5 = 5,
	Page6 = 6,
	Page7 = 7,
	Page_Select = 8,//ҳ��ѡ��
}Menu_Handle;

typedef enum
{
	Normal_Mode = 0,
	Flash_Mode,
}Mode_Handle;

typedef struct
{
	uint8 Set_Line;//���õ���
	uint8 Flash_Set;
	Mode_Handle Set_Mode;
	bool  Image_Show;//�Ƿ�Ϊͼ����ʾ
}Menu_;

extern int Show_Mode;
extern Menu_ Menu;
#endif
