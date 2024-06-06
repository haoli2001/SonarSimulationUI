#ifndef SONARSIMULATIONUI_H
#define SONARSIMULATIONUI_H

#include "QtTablewidget.h"//配置文件界面   zsr   20220109
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
//20211221 柯静 
#include "dataSavewidget.h"
//姬梓遇
#include "ErrorLog.h"


//0 初始未连接  1，连接了未加载模型  2，连接了模型加载了还未运行 3，正在运行
enum Satus { WAITFORCONNECT, WAITFORLOAD, WAITFORCONF, RUNNING };
//0，手动配置  1，文件配置  姬梓遇
enum ConfigType { CLIENTCONFIGMOD, FILECONFIGMOD };
//0扫角 1扫频
enum { Angle, Frequency };

class SonarSimulationUI : public QMainWindow
{
	Q_OBJECT

public:
	SonarSimulationUI(QWidget *parent = 0);
	~SonarSimulationUI();

	void InitAll();

	/*未传时域积分参数时做的提前处理，传参后应还需要*/
	void BuildXYForTimeWave(float *, int);

	//将时域积分结果写到txt中 jzy
	//返回值： 成功0  失败-1
	int CreatTimeIntegrationResultsFolder(int curAngle, int curFreq, int CalcuType);
	int SaveTimeIntegrationResultsFile();
	int SaveTSResultsFile(float TS);
	//错误处理函数 jzy
	void ErrorHandlerFunction();

	std::vector<double> XX;
	std::vector<double> YY;
	double maxYY, minYY;

public slots:
	//工具栏主要的开始打开连接等操作
	void OnActionConnect();
	void OnActionOpen();
	void OnActionRun();
	void OnActionSave();
	void OnActionStop();
	void OnActionNew();
	//工具栏数据库按钮  20211221 柯静
	void OnActionDataBase();
	//打开读取配置化文件界面按钮  20211221 赵思茹
	void OnActionexcel();

	//工具栏模型选择
	void OnComboBoxModules(int);

	//连接socket readyRead信号，有tcp数据进来就触发
	void OnSocketReadyRead();
	//连接socket disconnected信号, 网络断开时触发
	void OnDisconnect();
	//socket 接受到解算信息
	void OnRecvResult(CalcResult, int);
	//计算结束
	void OnCalcOver(int i);
	//扫频or扫角
	void OnRadioButtonScanAngle();
	void OnRadioButtonScanFrequency();

	//姬梓遇 反射系数选择，自动or用户配置
	void OnRadioButtonConfigReflectCoeffAuto();
	void OnRadioButtonConfigReflectCoeffText();

	//点击关闭按钮后出发槽函数，用以判断是否关闭
	void closeEvent(QCloseEvent *exent);
	//socket接收到的设备的监视信息，
	void OnRecvGPUWatch(GPUWatchStruct);
	//socket接收到设备的信息，在点积connect之后 就可以接受
	void OnRecvDeviceInfo(DeviceInfo);

	//5个显示窗口关闭后触发的槽函数
	void OnDockWidgetWave(bool);
	void OnDockWidgetConfigurate(bool);
	void OnDockWidgetCalcInfo(bool);
	void OnDockWidgetPrintInfo(bool);
	void OnDockWidgetGPUWatch(bool);

	//点击工具栏窗口显示相关的action的槽函数 
	void OnActionWave(bool value);
	void OnActionConfigurate(bool value);
	void OnActionCalcInfo(bool value);
	void OnActionPrintInfo(bool value);
	void OnActionGPUWatch(bool value);
	void OnPushbuttonDeviceSelect();

	//接收到KDTree数据后执行
	void OnLoadKDTree(KD_Node_V* kdtree, int length, int i);
	//更新三角面片的积分结果
	void OnUpdatePreTriangleResults(float *, int, int i);
	//更新时域积分结果
	void OnUpdateTimeIntegrationResults(float *, int, int i);
	//读excel获取配置信息后执行  姬梓遇
	void OnLoadFileConfigModInfo(FileConfigModInfo* fileConfigModInfo, int maxNum, QString excelName);
	void OnGetRunningStatusSig(int &status);
	//判断结果文件大小是否会超过当前保存路径的磁盘空间
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


private:  //数据
	Frame frame; //发送命令帧
	std::vector<DeviceInfo> deviceInfoVector;

	
private:

	int recvLength[5];
	Frame recvFrame[5];

	CalcThread *calcthread;//已经接收到的所有帧，记录在vector中，在另外一个线程中做处理

	ErrorLog log;	//日志类对象

	ExcelHelper excelHelper;   //保存Excel

	std::vector<FileConfigModInfo> fileConfigModInfos;	//读excel得到的配置数据  姬梓遇
	std::vector<DataManager> dataManagers;   //管理数据
	std::vector<QThread*> ReadreadyThreads;   //读波形数据对应线程

	int curModuleIndex;                  //当前模型的ID		dataManagers[]下标
	int curFileConfigModInfoIndex;		 //当前文件配置信息的ID	fileConfigModInfos[]下标  姬梓遇
	int curRepeatNum;					 //出现错误时当前重复次数 姬梓遇
	QString curTimeIntegrationResultsPath;	//当前时域积分结果txt文件存放路径
	QString curConfigExcelName;		//当前已加载的配置excel文件的名称
	QString curResultFolderPath;	//当前结果文件夹路径 数据库用

	QComboBox *comboBoxModules;

	dataSavewidget *datasave; //数据库 kj

	Usersql sql; //数据库 kj

	QtTablewidget* tablewidget;
};

#endif // SONARSIMULATIONUI_H
