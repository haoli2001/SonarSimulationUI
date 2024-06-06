#ifndef COMMON_STRUCT
#define COMMON_STRUCT
#include <qstring.h>




struct KDTreeStruct
{
	float time;            //����ʱ��
	int size;              //���ĳ���
	int card_num;
	
};

struct ResultStruct
{
	int angle;                       //�˽���ĽǶ�
	//float time_virtual_surface;       //����׾��潨��ʱ��
	//float time_rays_trace;           //����׷����ʱ
	//float time_intergral;            //������ʱ
	float result_intergral;          //����TSǿ��ֵ
	float translate_time;
	float calc_time;
	int raysnum;                     //10��28������
	int squarenum;					 //10��28������

};

struct Triangle
{
	int Points[3];//�洢����ı��
	int TriangleIndex;//��������Ԫ�ı��
};

struct Element
{
	float point[3];//�±� 0 1 2 3�ֱ����õ�x,y,z��ֵ
	int PointsIndex;//�õ�ı��ֵ
};
struct Box
{	//��Χ�е���������
	float bmin[3];//������ԭ�����������ֵ x y z
	float bmax[3];//������ԭ����Զ������ֵ x y z
};


struct KD_Node_V
{
	int Split_Axis;//�ָ��� 0��ʾx,1��ʾy��2��ʾz
	int PrimCount;//������ԪƬ��
	int begin;
	int end;//���ýڵ�����������ʼ�±�ͽ����±�
	Element  SplitPos;//�ָ���λ��
	bool IsLeaf;//�Ƿ���Ҷ�ڵ�
	bool IsEmpty;
	struct Box box;//��Χ�е���Ϣ
	int LeftIndex;
	int RightIndex;
	int RopeIndex[6];
	int index;
};

struct Frame   //command���� Triangles, Elements, Start(start�д���������Ϣ)
{
	char command[20];
	int length;
	char data[1024];
};

struct ConfigStruct
{
	float start_beta;  //��ʼbeta
	float start_alpha;  //��ʼalpha
	float end_alpha;    //��ֹalpha
	float wave_length;        //����
	float pipe_size;           //���߻��ֳߴ�
	float start_frequency;   //��ʼƵ��
	float end_frequency;       //��ֹƵ��
	float far_distance;               //Զ������
	int card_num;             //  ���㿨����
	int select_device_list[20];

	//2021.8.30������  ʱ����ֲ�������
	float sampling_rate; //������
	float sampling_width;//��������
	float tao;			//������
	float time_start_frequency;	//��ʱ����֣���ʼƵ��
	float time_end_frequency;		//��ʱ����֣���ֹƵ��
	float relative_velocity;	//����ٶ�
	float integral_gain;		//���ֵ���ϵ��
	float velocity1;			//Ŀ���ٶ�1
	float velocity2;			//Ŀ���ٶ�2
	float reflect_coeff;		//����ϵ��
	bool reflect_coeff_Auto_flag;//����ϵ������ģʽ��tureΪ�Զ����㣬flaseΪ�û�����ֵ
};

//�ļ�����ģʽ�У����ش�excel�ж�ȡ��������Ϣ  ������
struct FileConfigModInfo
{
	ConfigStruct config;
	int index;
	char modelPath[200];
	int cal_mood; //����ģʽѡ��1ΪɨƵ��0Ϊɨ��
};

struct GPUWatchStruct
{
	int device_id;
	int gpu;
	int memory;
	int temp;
	int shutdown_temp;
	int slowdown_temp;
	long long int total;
	long long int used;
	long long int free;
};

struct CalcResult
{
	int angle;
	float freq;
	float TS;
	float calc_time;
	int raysnum;
	int squarenum;
	int height;
	int width;
	int maxsize;					//ʱ����ֽ������ !!
};

struct DeviceInfo
{
	int deviceID;
	char deviceName[30];
	int deviceCount;
	int coresPreMutiprocess;
	int mutiprocessCount;
};
#endif