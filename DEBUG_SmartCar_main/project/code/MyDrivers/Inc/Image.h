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
    bool Cross_Fill;//ʮ�ֱ�־λ
}Flag_Handle;

//Ԫ�ز����ṹ��
typedef struct {
	uint8 forkpoint;//�����
	uint8 cross_up;//ʮ���Ϲյ�
	uint8 cross_down;//ʮ���¹յ�
	uint8 circle_up;//�����Ϲյ�
	uint8 circle_down;//�����¹յ�
	uint8 circlepoint;//�������ĵ�
	uint8 garage_up;//�����Ϲյ�
	uint8 garage_down;//�����¹յ�
	uint8 fill_start;//������� tip��������С���յ�
	uint8 fill_end;//�����յ�
}Element_Typedef;


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
}Ring_Handle;

typedef struct {
	float slope_rate;//������
	float intercept;//������
	bool dir[Image_H];//�б��־λ
	uint8 valid_num; //�߽���Ч����ͳ��
}Pram_Typedef;

//���������ṹ��
typedef struct {
	uint8 white_num;					 //�׵���
	uint8 black_num;					 //�ڵ���
	Border_Typedef l_border[Image_H];   //���������
	Border_Typedef r_border[Image_H];   //�ұ�������
	Border_Typedef center_line[Image_H];//������������
	Pram_Typedef   l_pram;				 //��߽�Ĳ���
	Pram_Typedef   r_pram;				 //�ұ߽�Ĳ���
	Element_Typedef   l_elem;			 //��߽��Ԫ�ز���
	Element_Typedef   r_elem;			 //�ұ߽��Ԫ�ز���
	uint8 l_start;						 //�ұ��������
	uint8 r_start;						 //�ұ��������
	uint8 hightest;					 //���
	uint8 longest;						 //�������
}Road_Typedef;

#endif /* MYDRIVERS_INC_IMAGE_H_ */
