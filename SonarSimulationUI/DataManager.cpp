#include "DataManager.h"


DataManager::DataManager()
{
	isModuleLoaded = false;
	isKdTreeLoaded = false;
	isPreTrianglesResultsLoaded = false;
	isStop = false;
	CalcuedAngle = -1;
}


DataManager::~DataManager()
{
}

void DataManager::SetConfig(ConfigStruct config)
{
	this->config = config;
}

ConfigStruct& DataManager::GetConfig()
{
	return config;
}

void DataManager::SetModule(std::string filename)
{
	if (module.isLoaded())
		module.clear();
	module.load(filename);

	isModuleLoaded = module.isLoaded();
}

FluentData& DataManager::GetModule()
{
	return module;
}
void DataManager::SetWaveData(std::string filename)
{
	if (module.isWaveLoaded())
		module.waveclear();
	module.loadwave(filename);

	isModuleLoaded = true;
}

void DataManager::SetBoxData(KD_Node_V *kd_tree, int length)
{
	if (boxdata.isLoaded())
		boxdata.clear();
	boxdata.load(kd_tree, length);

	isKdTreeLoaded = true;
}

BoxData& DataManager::GetBoxData()
{
	return boxdata;
}

int DataManager::GetStatus()
{
	return runningStatus;
}

void DataManager::SetStatus(int value)
{
	runningStatus = value;
}

void DataManager::SetCalcType(int value)
{
	CalcType = value;
}

int DataManager::GetCalcType()
{
	return CalcType;
}

void DataManager::SetConfigType(int value)
{
	ConfigType = value;

	//转换configtype时，重置所有与暂停重启有关的标志
	isStop = false;
	CalcuedAngle = -1;
	isContinueRuning = false;
}

int DataManager::GetConfigType()
{
	return ConfigType;
}

void DataManager::PushCalcResult(CalcResult result)
{
	calcResultVector.push_back(result);
}

void DataManager::ClearCalcResult()
{
	calcResultVector.clear();
}

bool DataManager::IsCalcEmpty()
{
	return calcResultVector.empty();
}

std::vector<CalcResult> & DataManager::GetCalcResult()
{
	return calcResultVector;
}

void DataManager::SetPreTriangleResults(float *data)
{
	preTriangleResults = data;
}

void DataManager::SetPreTriangleResultsNum(int num)
{
	preTriangleResultsNum = num;
}

float* DataManager::GetPreTriangleResults()
{
	return preTriangleResults;
}

int DataManager::GetPreTriangleResultsNum()
{
	return preTriangleResultsNum;
}

void DataManager::SetTimeIntegrationResults(float *data)
{
	TimeIntegrationResults = data;
}

void DataManager::SetTimeIntegrationResultsNum(int num)
{
	TimeIntegrationResultsNum = num;
}

float* DataManager::GetTimeIntegrationResults()
{
	return TimeIntegrationResults;
}

int DataManager::GetTimeIntegrationResultsNum()
{
	return TimeIntegrationResultsNum;
}

bool DataManager::IsContinueRuning()
{
	return isContinueRuning;
}

int DataManager::getCalcuedAngle()
{
	return CalcuedAngle;
}

void DataManager::setIsContinueRuning(bool value)
{
	isContinueRuning = value;
}

void DataManager::setCalcuedAngle(int angle)
{
	CalcuedAngle = angle;
}

void DataManager::updateCalcuedAngle()
{
	setCalcuedAngle(calcResultVector.back().angle);
}

void DataManager::Reset()
{
	isKdTreeLoaded = false;
	isModuleLoaded = false;
	isPreTrianglesResultsLoaded = false;
	//isContinueRuning = false;
}



