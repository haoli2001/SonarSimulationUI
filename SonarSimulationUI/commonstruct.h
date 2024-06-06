#ifndef COMMON_STRUCT
#define COMMON_STRUCT
#include <qstring.h>




struct KDTreeStruct
{
	float time;            //建树时间
	int size;              //树的长度
	int card_num;
	
};

struct ResultStruct
{
	int angle;                       //此结果的角度
	//float time_virtual_surface;       //虚拟孔径面建立时间
	//float time_rays_trace;           //射线追踪用时
	//float time_intergral;            //积分用时
	float result_intergral;          //积分TS强度值
	float translate_time;
	float calc_time;
	int raysnum;                     //10月28，孙力
	int squarenum;					 //10月28，孙力

};

struct Triangle
{
	int Points[3];//存储顶点的编号
	int TriangleIndex;//该三角面元的编号
};

struct Element
{
	float point[3];//下标 0 1 2 3分别代表该点x,y,z的值
	int PointsIndex;//该点的编号值
};
struct Box
{	//包围盒的两个顶点
	float bmin[3];//离坐标原点最近的三个值 x y z
	float bmax[3];//离坐标原点最远的三个值 x y z
};


struct KD_Node_V
{
	int Split_Axis;//分割轴 0表示x,1表示y，2表示z
	int PrimCount;//三角面元片数
	int begin;
	int end;//给该节点分配的数组起始下标和结束下标
	Element  SplitPos;//分割点的位置
	bool IsLeaf;//是否是叶节点
	bool IsEmpty;
	struct Box box;//包围盒的信息
	int LeftIndex;
	int RightIndex;
	int RopeIndex[6];
	int index;
};

struct Frame   //command类型 Triangles, Elements, Start(start中带有配置信息)
{
	char command[20];
	int length;
	char data[1024];
};

struct ConfigStruct
{
	float start_beta;  //初始beta
	float start_alpha;  //初始alpha
	float end_alpha;    //终止alpha
	float wave_length;        //波长
	float pipe_size;           //管线划分尺寸
	float start_frequency;   //起始频率
	float end_frequency;       //终止频率
	float far_distance;               //远场距离
	int card_num;             //  计算卡数量
	int select_device_list[20];

	//2021.8.30姬梓遇  时域积分参数配置
	float sampling_rate; //采样率
	float sampling_width;//采样长度
	float tao;			//脉冲宽度
	float time_start_frequency;	//（时域积分）起始频率
	float time_end_frequency;		//（时域积分）终止频率
	float relative_velocity;	//相对速度
	float integral_gain;		//积分调整系数
	float velocity1;			//目标速度1
	float velocity2;			//目标速度2
	float reflect_coeff;		//反射系数
	bool reflect_coeff_Auto_flag;//反射系数配置模式，ture为自动计算，flase为用户输入值
};

//文件配置模式中，传回从excel中读取的配置信息  姬梓遇
struct FileConfigModInfo
{
	ConfigStruct config;
	int index;
	char modelPath[200];
	int cal_mood; //计算模式选择，1为扫频，0为扫角
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
	int maxsize;					//时域积分结果总数 !!
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