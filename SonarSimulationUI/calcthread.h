#ifndef CALCTHREAD_H
#define CALCTHREAD_H

#include <QThread>
#include <queue>
#include <qtcpsocket.h>
#include "commonstruct.h"
#include <qmutex.h>
#include <qsemaphore.h>

struct KDTreeInfo
{
	float time;
	int length;
};

class CalcThread : public QThread
{
	Q_OBJECT

public:
	CalcThread(QObject *parent = nullptr);
	~CalcThread();

	void PushFrame(Frame,int);
	void ClearFrameVector();

	virtual void run();

	QMutex& GetMutex();

	KDTreeInfo& GetKDTreeInfo();

	void SetTriangleLength(float);

	void SetTimeIntegrationResultsLength(float);

	void SetCurrentModule(int i)
	{
		currentModule = i;
	}

	void AddFrameQueue();
	void AddPreTriangleResults();
	void AddTimeIntegrationResults();

signals:
	void printInfo(QString);
	void startLoadKDTree(KD_Node_V*, int, int);

	void sendResult(CalcResult, int);

	void calcOver(int );

	void sendGPUWatch(GPUWatchStruct);

	void sendDeviceInfo(DeviceInfo);

	void updatePreTriangleResults(float *, int, int i);

	void updateTimeIntegrationResults(float *, int, int i);

private:
	std::vector<std::queue<Frame>> frameVector;
	
	KDTreeInfo kdTreeInfo;

	KD_Node_V *kdTree;
	int kdTreeRecvedLength;

	std::vector<float *> preTriangleResults;
	std::vector<int> preTriangleResultsRecvLength;
	std::vector<int> triangleNum;
	std::vector<float *> TimeIntegrationResults;
	std::vector<int> TimeIntegrationResultsRecvLength;
	std::vector<int> TimeIntegrationNum;

	//std::vector<>

	QMutex mutex;

	QSemaphore semaphare;

	int currentModule;
};

#endif // CALCTHREAD_H
