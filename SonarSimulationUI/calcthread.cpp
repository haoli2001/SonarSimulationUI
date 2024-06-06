#include "calcthread.h"

CalcThread::CalcThread(QObject *parent)
	: QThread(parent), kdTreeRecvedLength(0), kdTree(NULL),
	preTriangleResults(NULL), preTriangleResultsRecvLength(0)
{
	
}

CalcThread::~CalcThread()
{
	free(kdTree);
}


void CalcThread::PushFrame(Frame frame, int i)
{
	mutex.lock();
	frameVector[i].push(frame);
	mutex.unlock();
//	semaphare.release();
	
}

void CalcThread::ClearFrameVector()
{
	frameVector.clear();
}

QMutex& CalcThread::GetMutex()
{
	return mutex;
}

void CalcThread::run()
{
	//循环处理接收到的数据
	while (true)
	{
	//	semaphare.acquire();
		bool sleepFlag = true;

		//防止不同线程处理同一数据冲突，这里加锁
		mutex.lock();
		int N = frameVector.size();
		mutex.unlock();
		for (int i = 0; i < N; i++)
		{
			mutex.lock();
			bool M = frameVector[i].empty();
			mutex.unlock();
			if (M != true)
			{
				mutex.lock();
				Frame frame = frameVector[i].front();
				frameVector[i].pop();
				mutex.unlock();
				if (!strcmp(frame.command, "WaitForConfig"))
				{
					if (i == currentModule)
					{
						emit printInfo(QString::fromLocal8Bit("模型加载完成，等待配置开始计算！"));
					}
					continue;
				}
				if (!strcmp(frame.command, "KDTreeTime"))
				{
					memcpy(&kdTreeInfo, frame.data, sizeof(KDTreeInfo));
					if (i == currentModule)
					{
						emit printInfo(QString::fromLocal8Bit("KDTree构建完成，用时") + QString::number(kdTreeInfo.time) + QString("ms"));
						emit printInfo(QString::fromLocal8Bit("KDTree节点个数为：") + QString::number(kdTreeInfo.length));
					}
					if (kdTree != NULL)
						free(kdTree);
					kdTree = (KD_Node_V*)malloc(sizeof(KD_Node_V)*kdTreeInfo.length);
					continue;
				}
				if (!strcmp(frame.command, "KDTreeDate"))
				{
					memcpy((char *)kdTree + kdTreeRecvedLength, frame.data, frame.length);
					kdTreeRecvedLength += frame.length;
					if (kdTreeRecvedLength == kdTreeInfo.length*sizeof(KD_Node_V))
					{
						kdTreeRecvedLength = 0;
						if (i == currentModule)
						{
							emit printInfo(QString::fromLocal8Bit("KDTree 传输完成"));
						}
						emit startLoadKDTree(kdTree, kdTreeInfo.length, i);
					}
					//continue;
				}


				if (!strcmp(frame.command, "PreTriangleResult"))
				{
					mutex.lock();
					memcpy((char *)preTriangleResults[i] + preTriangleResultsRecvLength[i], frame.data, frame.length);
					mutex.unlock();
					preTriangleResultsRecvLength[i] += frame.length;
					if (preTriangleResultsRecvLength[i] == triangleNum[i]*sizeof(float))
					{
						if (i == currentModule)
						{
							emit printInfo(QString::fromLocal8Bit("接收长度") + QString::number(preTriangleResultsRecvLength[i]));

						}
						
						emit updatePreTriangleResults(preTriangleResults[i], triangleNum[i], i);
						preTriangleResultsRecvLength[i] = 0;
					}
					//continue;

				}

				if (!strcmp(frame.command, "re"))
				{
					mutex.lock();
					memcpy((char *)TimeIntegrationResults[i] + TimeIntegrationResultsRecvLength[i], frame.data, frame.length);
					mutex.unlock();
					TimeIntegrationResultsRecvLength[i] += frame.length;
					if (TimeIntegrationResultsRecvLength[i] == TimeIntegrationNum[i] * sizeof(float))
					{
						if (i == currentModule)
						{
							emit printInfo(QString::fromLocal8Bit("时域积分接收长度") + QString::number(TimeIntegrationResultsRecvLength[i]));

						}

						emit updateTimeIntegrationResults(TimeIntegrationResults[i], TimeIntegrationNum[i], i);
						TimeIntegrationResultsRecvLength[i] = 0;
					}
					//continue;

				}

				if (!strcmp(frame.command, "CalcResult"))
				{
					CalcResult calcResult;
					memcpy((char*)&calcResult, frame.data, frame.length);
					emit sendResult(calcResult, i);
				}

				if (!strcmp(frame.command, "CalcOver"))
				{
					emit calcOver(i);
				}

				if (!strcmp(frame.command, "GPUWatch"))
				{
					if (i == 0)
					{
						GPUWatchStruct gpuWatch;
						memcpy((char*)&gpuWatch, frame.data, frame.length);
						emit sendGPUWatch(gpuWatch);
					}
				}
				if (!strcmp(frame.command, "DeviceInfo"))
				{
					if (i == 0)
					{
						DeviceInfo deviceInfo;
						memcpy((char*)&deviceInfo, frame.data, frame.length);
						emit sendDeviceInfo(deviceInfo);
					}
				}
				sleepFlag = false;
			}
		}
		if (sleepFlag == true)
		{
			msleep(50);
		}
	}
}

KDTreeInfo& CalcThread::GetKDTreeInfo()
{
	return kdTreeInfo;
}

void CalcThread::SetTriangleLength(float length)
{

	mutex.lock();
	this->triangleNum[currentModule] = length;
	mutex.unlock();
	if (preTriangleResults[currentModule] == NULL)
	{
		mutex.lock();
		preTriangleResults[currentModule] = (float *)malloc(sizeof(float)*triangleNum[currentModule]);
		mutex.unlock();
	}
	else
	{
		mutex.lock();
		free(preTriangleResults[currentModule]);
		preTriangleResults[currentModule] = (float *)malloc(sizeof(float)*triangleNum[currentModule]);
		mutex.unlock();
	}
}

void CalcThread::SetTimeIntegrationResultsLength(float length)
{
	mutex.lock();
	this->TimeIntegrationNum[currentModule] = length;
	mutex.unlock();
	if (TimeIntegrationResults[currentModule] == NULL)
	{
		mutex.lock();
		TimeIntegrationResults[currentModule] = (float *)malloc(sizeof(float)*TimeIntegrationNum[currentModule]);
		mutex.unlock();
	}
	else
	{
		mutex.lock();
		free(TimeIntegrationResults[currentModule]);
		TimeIntegrationResults[currentModule] = (float *)malloc(sizeof(float)*TimeIntegrationNum[currentModule]);
		mutex.unlock();
	}
}

void CalcThread::AddFrameQueue()
{
	std::queue<Frame> q;
	mutex.lock();
	frameVector.push_back(q);
	mutex.unlock();
}

void CalcThread::AddPreTriangleResults()
{
	mutex.lock();
	preTriangleResults.push_back(NULL);
	preTriangleResultsRecvLength.push_back(0);
	triangleNum.push_back(0);
	mutex.unlock();
}

void CalcThread::AddTimeIntegrationResults()
{
	mutex.lock();
	TimeIntegrationResults.push_back(NULL);
	TimeIntegrationResultsRecvLength.push_back(0);
	TimeIntegrationNum.push_back(0);
	mutex.unlock();
}

