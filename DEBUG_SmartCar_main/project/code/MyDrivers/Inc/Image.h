#ifndef MYDRIVERS_INC_IMAGE_H_
#define MYDRIVERS_INC_IMAGE_H_
#include "zf_common_headfile.h"

void Image_Process();

#define Image_H 80//ͼ��߶�
#define Image_W 148//ͼ����
#define White_Pixel 255
#define Black_Pixel 0
#define Border_Max  Image_W-2 //�߽����ֵ
#define Border_Min  1   //�߽���Сֵ
extern uint8 Original_Image[Image_H][Image_W];
extern float Image_Erro;
extern uint8 Image_Thereshold;
extern uint8 L_Border[Image_H];//��������
extern uint8 R_Border[Image_H];//��������
extern uint8 Center_Line[Image_H];//��������
extern uint8 Bin_Image[Image_H][Image_W];//��ֵ��ͼ������
extern uint8 Hightest;//��ߵ�
typedef struct
{
    bool Cross_Fill;//ʮ�ֱ�־λ
}Flag_Handle;

typedef enum {
	Ring_Front = 0,//�뻷ǰ
	Enter_Ring_First,//��һ�ν���
	Leave_Ring_First,//��һ���뿪��
}Ring_State_Handle;

typedef struct {
	bool Stright_Line;//ֱ��
	bool Ring_Front_Flag;//ǰ���Ƿ����Բ��
	bool Enter_Ring_First_Flag;
	bool Leave_Ring_First_Flag;
	Ring_State_Handle Ring_State;
}Ring_Handle;

#endif /* MYDRIVERS_INC_IMAGE_H_ */
