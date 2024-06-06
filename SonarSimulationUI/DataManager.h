#pragma once
#include "commonstruct.h"
#include <vector>
#include <FluentData.h>
#include <BoxData.h>



class DataManager
{
public:
	DataManager();
	~DataManager();

	void SetConfig(ConfigStruct config);

	ConfigStruct& GetConfig();

	void SetModule(std::string filename);

	FluentData& GetModule();

	void SetBoxData(KD_Node_V *kd_tree, int length);

	BoxData& GetBoxData();

	int GetStatus();

	void SetStatus(int);

	void SetCalcType(int);

	int GetCalcType();

	void SetConfigType(int);

	int GetConfigType();

	void PushCalcResult(CalcResult );

	void ClearCalcResult();

	bool IsCalcEmpty();

	std::vector<CalcResult> & GetCalcResult();

/*****************三角面元************************/
	void SetPreTriangleResults(float *data);

	void SetPreTriangleResultsNum(int num);

	float* GetPreTriangleResults();

	int GetPreTriangleResultsNum();
/*****************三角面元************************/

/*****************时域积分************************/
	void SetTimeIntegrationResults(float *data);

	void SetTimeIntegrationResultsNum(int num);

	float* GetTimeIntegrationResults();

	int GetTimeIntegrationResultsNum();
/*****************时域积分************************/

	bool IsModuleLoaded(){
		return isModuleLoaded;
	};
	bool IsKdTreeLoaded(){
		return isKdTreeLoaded;
	}
	bool IsPreTrianglesResultsLoaded(){
		return isPreTrianglesResultsLoaded;
	}

	void SetIsPreTrianglesResultsLoaded(bool value)
	{
		isPreTrianglesResultsLoaded = value;
	}

	//当加载完模型后，通过调用reset函数将dataManager重置
	void Reset();

	void SetWaveData(std::string filename);

/*****************计算进度相关*****************/
	bool IsContinueRuning();
	void setIsContinueRuning(bool value);
	int getCalcuedAngle();
	void setCalcuedAngle(int);
	void updateCalcuedAngle();
	bool isStop;

private:
	ConfigStruct config;             //模型的配置参数
	std::vector<float>  waveData;    //
	FluentData module;               //模型数据
	BoxData boxdata;                 //kdtree数据

	int ConfigType;		//配置模式：0，ClientConfigMod 手动配置  1，FileConfigMod 文件配置  姬梓遇

	int runningStatus;

	int CalcType;

	std::vector<CalcResult> calcResultVector;

	float *preTriangleResults;
	int preTriangleResultsNum;

	float *TimeIntegrationResults;
	int TimeIntegrationResultsNum;
	

	bool isModuleLoaded;
	bool isKdTreeLoaded;
	bool isPreTrianglesResultsLoaded;
	bool isContinueRuning;	//是否继续之前暂停的运行

	int CalcuedAngle;	//记录已计算完成的角度，未有计算结果时记为-1
};
