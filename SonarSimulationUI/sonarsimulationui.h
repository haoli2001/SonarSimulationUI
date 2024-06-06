#ifndef SONARSIMULATIONUI_H
#define SONARSIMULATIONUI_H

#include "QtTablewidget.h"//�����ļ�����   zsr   20220109
#include <QtWidgets/QMainWindow>
#include "ui_sonarsimulationui.h"
#include "connectdialog.h"
#include <qtcpsocket.h>
#include <qtextcodec.h>
#include <string>
#include <qregexp.h>
#include "calcthread.h"
#include <qmetatype.h>
#include "ExcelHelper.h"
#include "qvector.h"
#include "DataManager.h"
#include <qcombobox.h>
//20211221 �¾� 
#include "dataSavewidget.h"
//������
#include "ErrorLog.h"


//0 ��ʼδ����  1��������δ����ģ��  2��������ģ�ͼ����˻�δ���� 3����������
enum Satus { WAITFORCONNECT, WAITFORLOAD, WAITFORCONF, RUNNING };
//0���ֶ�����  1���ļ�����  ������
enum ConfigType { CLIENTCONFIGMOD, FILECONFIGMOD };
//0ɨ�� 1ɨƵ
enum { Angle, Frequency };

class SonarSimulationUI : public QMainWindow
{
	Q_OBJECT

public:
	SonarSimulationUI(QWidget *parent = 0);
	~SonarSimulationUI();

	void InitAll();

	/*δ��ʱ����ֲ���ʱ������ǰ�������κ�Ӧ����Ҫ*/
	void BuildXYForTimeWave(float *, int);

	//��ʱ����ֽ��д��txt�� jzy
	//����ֵ�� �ɹ�0  ʧ��-1
	int CreatTimeIntegrationResultsFolder(int curAngle, int curFreq, int CalcuType);
	int SaveTimeIntegrationResultsFile();
	int SaveTSResultsFile(float TS);
	//�������� jzy
	void ErrorHandlerFunction();

	std::vector<double> XX;
	std::vector<double> YY;
	double maxYY, minYY;

public slots:
	//��������Ҫ�Ŀ�ʼ�����ӵȲ���
	void OnActionConnect();
	void OnActionOpen();
	void OnActionRun();
	void OnActionSave();
	void OnActionStop();
	void OnActionNew();
	//���������ݿⰴť  20211221 �¾�
	void OnActionDataBase();
	//�򿪶�ȡ���û��ļ����水ť  20211221 ��˼��
	void OnActionexcel();

	//������ģ��ѡ��
	void OnComboBoxModules(int);

	//����socket readyRead�źţ���tcp���ݽ����ʹ���
	void OnSocketReadyRead();
	//����socket disconnected�ź�, ����Ͽ�ʱ����
	void OnDisconnect();
	//socket ���ܵ�������Ϣ
	void OnRecvResult(CalcResult, int);
	//�������
	void OnCalcOver(int i);
	//ɨƵorɨ��
	void OnRadioButtonScanAngle();
	void OnRadioButtonScanFrequency();

	//������ ����ϵ��ѡ���Զ�or�û�����
	void OnRadioButtonConfigReflectCoeffAuto();
	void OnRadioButtonConfigReflectCoeffText();

	//����رհ�ť������ۺ����������ж��Ƿ�ر�
	void closeEvent(QCloseEvent *exent);
	//socket���յ����豸�ļ�����Ϣ��
	void OnRecvGPUWatch(GPUWatchStruct);
	//socket���յ��豸����Ϣ���ڵ��connect֮�� �Ϳ��Խ���
	void OnRecvDeviceInfo(DeviceInfo);

	//5����ʾ���ڹرպ󴥷��Ĳۺ���
	void OnDockWidgetWave(bool);
	void OnDockWidgetConfigurate(bool);
	void OnDockWidgetCalcInfo(bool);
	void OnDockWidgetPrintInfo(bool);
	void OnDockWidgetGPUWatch(bool);

	//���������������ʾ��ص�action�Ĳۺ��� 
	void OnActionWave(bool value);
	void OnActionConfigurate(bool value);
	void OnActionCalcInfo(bool value);
	void OnActionPrintInfo(bool value);
	void OnActionGPUWatch(bool value);
	void OnPushbuttonDeviceSelect();

	//���յ�KDTree���ݺ�ִ��
	void OnLoadKDTree(KD_Node_V* kdtree, int length, int i);
	//����������Ƭ�Ļ��ֽ��
	void OnUpdatePreTriangleResults(float *, int, int i);
	//����ʱ����ֽ��
	void OnUpdateTimeIntegrationResults(float *, int, int i);
	//��excel��ȡ������Ϣ��ִ��  ������
	void OnLoadFileConfigModInfo(FileConfigModInfo* fileConfigModInfo, int maxNum, QString excelName);
	void OnGetRunningStatusSig(int &status);
	//�жϽ���ļ���С�Ƿ�ᳬ����ǰ����·���Ĵ��̿ռ�
	bool checkResultDeskInfo();
signals:
	void updateModelInfo(QString);
	void printInfo(QString);
	void SetWaveDataSig(std::string filenmae);
	void updateProgressValue(float value, int idx);


private:
	Ui::SonarSimulationUIClass ui;

	std::vector<QTcpSocket*> sockets;

	QString ip;
	int port;
	int sumCores;
	int deviceCount;


private:  //����
	Frame frame; //��������֡
	std::vector<DeviceInfo> deviceInfoVector;

	
private:

	int recvLength[5];
	Frame recvFrame[5];

	CalcThread *calcthread;//�Ѿ����յ�������֡����¼��vector�У�������һ���߳���������

	ErrorLog log;	//��־�����

	ExcelHelper excelHelper;   //����Excel

	std::vector<FileConfigModInfo> fileConfigModInfos;	//��excel�õ�����������  ������
	std::vector<DataManager> dataManagers;   //��������
	std::vector<QThread*> ReadreadyThreads;   //���������ݶ�Ӧ�߳�

	int curModuleIndex;                  //��ǰģ�͵�ID		dataManagers[]�±�
	int curFileConfigModInfoIndex;		 //��ǰ�ļ�������Ϣ��ID	fileConfigModInfos[]�±�  ������
	int curRepeatNum;					 //���ִ���ʱ��ǰ�ظ����� ������
	QString curTimeIntegrationResultsPath;	//��ǰʱ����ֽ��txt�ļ����·��
	QString curConfigExcelName;		//��ǰ�Ѽ��ص�����excel�ļ�������
	QString curResultFolderPath;	//��ǰ����ļ���·�� ���ݿ���

	QComboBox *comboBoxModules;

	dataSavewidget *datasave; //���ݿ� kj

	Usersql sql; //���ݿ� kj

	QtTablewidget* tablewidget;
};

#endif // SONARSIMULATIONUI_H
