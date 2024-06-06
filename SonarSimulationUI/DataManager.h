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

/*****************������Ԫ************************/
	void SetPreTriangleResults(float *data);

	void SetPreTriangleResultsNum(int num);

	float* GetPreTriangleResults();

	int GetPreTriangleResultsNum();
/*****************������Ԫ************************/

/*****************ʱ�����************************/
	void SetTimeIntegrationResults(float *data);

	void SetTimeIntegrationResultsNum(int num);

	float* GetTimeIntegrationResults();

	int GetTimeIntegrationResultsNum();
/*****************ʱ�����************************/

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

	//��������ģ�ͺ�ͨ������reset������dataManager����
	void Reset();

	void SetWaveData(std::string filename);

/*****************����������*****************/
	bool IsContinueRuning();
	void setIsContinueRuning(bool value);
	int getCalcuedAngle();
	void setCalcuedAngle(int);
	void updateCalcuedAngle();
	bool isStop;

private:
	ConfigStruct config;             //ģ�͵����ò���
	std::vector<float>  waveData;    //
	FluentData module;               //ģ������
	BoxData boxdata;                 //kdtree����

	int ConfigType;		//����ģʽ��0��ClientConfigMod �ֶ�����  1��FileConfigMod �ļ�����  ������

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
	bool isContinueRuning;	//�Ƿ����֮ǰ��ͣ������

	int CalcuedAngle;	//��¼�Ѽ�����ɵĽǶȣ�δ�м�����ʱ��Ϊ-1
};
