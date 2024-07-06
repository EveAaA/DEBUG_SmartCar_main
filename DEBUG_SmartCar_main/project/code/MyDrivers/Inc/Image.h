#ifndef MYDRIVERS_INC_IMAGE_H_
#define MYDRIVERS_INC_IMAGE_H_
#include "zf_common_headfile.h"
void Get_Bin_Image();
void Image_Process();

#define Image_H 80//ͼ��߶�
#define Image_W 148//ͼ����
#define White_Pixel 255
#define Black_Pixel 0
#define Border_Max  Image_W-2 //�߽����ֵ
#define Border_Min  1   //�߽���Сֵ
extern uint8 Original_Image[Image_H][Image_W];
extern float Image_Erro;
extern float Image_Erro_Y;
extern uint8 Image_Thereshold;
extern uint8 L_Border[Image_H];//��������
extern uint8 R_Border[Image_H];//��������
extern uint8 Center_Line[Image_H];//��������
extern uint8 Bin_Image[Image_H][Image_W];//��ֵ��ͼ������
extern uint8 Hightest;//��ߵ�

//�����ṹ��
typedef struct {
	uint8 x;//������
	uint8 y;//������
	uint8 valid_flag;//�б��־λ
}Border_Typedef;

typedef enum {
	LeftLine,
	RightLine,
}lineTypeDef;

typedef struct
{
    uint8 Cross_Fill;//ʮ�ֱ�־λ
	bool Left_Ring;
	bool Right_Ring;
	bool Zerba;
	bool Roadblock;
	uint8 Cross_Type;
}Flag_Handle;

typedef enum {
	Ring_Front = 0,//�뻷ǰ
	Enter_Ring_First,//��һ�ν���
	Leave_Ring_First,//��һ���뿪��
	In_Ring,   // �뻷
	Ready_Out_Ring,  // ׼������  
	Leave_Ring     // ����
}Ring_State_Handle;

typedef struct {
	bool LeftStraightLine; // ����Ƿ���ֱ��
	bool Stright_Line;//ֱ��
	uint8 Ring_Front_Flag;//ǰ���Ƿ����Բ��
	bool Ring;
	bool Enter_Ring_First_Flag;
	bool Leave_Ring_First_Flag;
	Ring_State_Handle Ring_State;
	uint16 Clear_Time;
}Ring_Handle;

typedef struct {
	float slope_rate;//������
	float intercept;//������
	bool dir[Image_H];//�б��־λ
	uint8 valid_num; //�߽���Ч����ͳ��
}Pram_Typedef;
extern Ring_Handle RightRing;
extern Ring_Handle LeftRing;
extern Flag_Handle Image_Flag;
#endif /* MYDRIVERS_INC_IMAGE_H_ */
