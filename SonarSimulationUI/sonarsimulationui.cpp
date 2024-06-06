#include "sonarsimulationui.h"
#include <qtimer.h>
#include "deviceselect.h"
#include <qlist.h>
#include <qvariant.h>
#include <qobject.h>
#include <direct.h>
#include <io.h>

SonarSimulationUI::SonarSimulationUI(QWidget *parent)
	: QMainWindow(parent), sumCores(0), deviceCount(0)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("复杂目标态势多任务仿真自动计算可视平台"));

	tablewidget = new QtTablewidget;
	datasave = new dataSavewidget;
	comboBoxModules = new QComboBox();
	ui.mainToolBar->addWidget(comboBoxModules);
	ui.progressBar->setVisible(false);

	log.AddBlockLine();
	log.INFO_log("打开界面");
	connect(comboBoxModules, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxModules(int)));

	//用一线程循环处理接收到的数据
	calcthread = new CalcThread();
	calcthread->start();

	memset(recvLength, 0, sizeof(int) * 5);

	//连接socket读取信号的处理槽函数
	connect(tablewidget, &QtTablewidget::stop_Sig, this, &SonarSimulationUI::OnActionStop);
	connect(tablewidget, &QtTablewidget::run_Sig, this, &SonarSimulationUI::OnActionRun);
	connect(tablewidget, &QtTablewidget::getRunningStatusSig, this, &SonarSimulationUI::OnGetRunningStatusSig);
	connect(tablewidget, &QtTablewidget::loadFileConfigModInfo, this, &SonarSimulationUI::OnLoadFileConfigModInfo);
	connect(this, &SonarSimulationUI::updateProgressValue, tablewidget, &QtTablewidget::OnUpdateProgressVale);

	connect(calcthread, SIGNAL(printInfo(QString)), ui.textBrowser_info, SLOT(append(QString)));
	connect(calcthread, SIGNAL(startLoadKDTree(KD_Node_V*, int, int)), this, SLOT(OnLoadKDTree(KD_Node_V*, int, int)));
	connect(calcthread, SIGNAL(updatePreTriangleResults(float*, int, int)), this, SLOT(OnUpdatePreTriangleResults(float*, int, int)));
	connect(calcthread, SIGNAL(updateTimeIntegrationResults(float*, int, int)), this, SLOT(OnUpdateTimeIntegrationResults(float*, int, int)));
	connect(calcthread, SIGNAL(calcOver(int)), this, SLOT(OnCalcOver(int)));


	qRegisterMetaType<CalcResult>("CalcResult");
	connect(calcthread, &CalcThread::sendResult, this, &SonarSimulationUI::OnRecvResult);

	qRegisterMetaType<GPUWatchStruct>("GPUWatchStruct");
	connect(calcthread, &CalcThread::sendGPUWatch, this, &SonarSimulationUI::OnRecvGPUWatch);

	qRegisterMetaType<DeviceInfo>("DeviceInfo");
	connect(calcthread, &CalcThread::sendDeviceInfo, this, &SonarSimulationUI::OnRecvDeviceInfo);

	ui.pushButton_deviceSelect->setEnabled(false);

	OnActionNew();
}

SonarSimulationUI::~SonarSimulationUI()
{

}

//新建或切换解算模型时，执行初始化
void SonarSimulationUI::InitAll()
{
	if (dataManagers[curModuleIndex].IsModuleLoaded() == false)
	{
		ui.openGLWidget->Clear();
		ui.widget_wave_2->Init(waveTime);//
	}
	else
	{
		ui.openGLWidget->LoadModel(dataManagers[curModuleIndex].GetModule());
		if (dataManagers[curModuleIndex].IsKdTreeLoaded() == true)
			ui.openGLWidget->OnLoadKDTree(dataManagers[curModuleIndex].GetBoxData());
		if (dataManagers[curModuleIndex].IsPreTrianglesResultsLoaded() == true)
			ui.openGLWidget->UpdatePreTriangleResults(dataManagers[curModuleIndex].GetPreTriangleResults(),
				dataManagers[curModuleIndex].GetPreTriangleResultsNum());
	}

	ConfigStruct config = dataManagers[curModuleIndex].GetConfig();
	ui.lineEdit_startBeta->setText(QString("%1").arg(config.start_beta));
	ui.lineEdit_startAlpha->setText(QString("%1").arg(config.start_alpha));
	ui.lineEdit_endAlpha->setText(QString("%1").arg(config.end_alpha));
	ui.lineEdit_waveLength->setText(QString("%1").arg(config.wave_length));
	ui.lineEdit_pipeSize->setText(QString("%1").arg(config.pipe_size));
	ui.lineEdit_endFrequency->setText(QString("%1").arg(config.end_frequency));
	ui.lineEdit_farDistance->setText(QString("%1").arg(config.far_distance));
	ui.radioButton_scanAngle->setChecked(dataManagers[curModuleIndex].GetStatus() == Angle);
	ui.radioButton_scanFrequency->setChecked(dataManagers[curModuleIndex].GetStatus() == Frequency);

	if (dataManagers[curModuleIndex].GetCalcType() == Angle)
		ui.widget_wave->Init(waveAngle);

	else
		ui.widget_wave->Init(waveFreq);

	ui.widget_wave_2->Init(waveTime);

	if (dataManagers[curModuleIndex].GetCalcType() == Angle)
	{
		//波形绘制数据
		for (int i = 0; i < dataManagers[curModuleIndex].GetCalcResult().size(); i++)
		{
			CalcResult result = dataManagers[curModuleIndex].GetCalcResult()[i];
			ui.widget_wave->PushResult(result.angle, result.TS);
		}

	}
	else if (dataManagers[curModuleIndex].GetCalcType() == Frequency)
	{
		//波形绘制数据
		for (int i = 0; i < dataManagers[curModuleIndex].GetCalcResult().size(); i++)
		{
			CalcResult result = dataManagers[curModuleIndex].GetCalcResult()[i];
			ui.widget_wave->PushResult(result.freq, result.TS);
		}

	}

	for (int i = 0; i < dataManagers[curModuleIndex].GetCalcResult().size(); i++)
	{
		BuildXYForTimeWave(dataManagers[curModuleIndex].GetTimeIntegrationResults(), dataManagers[curModuleIndex].GetTimeIntegrationResultsNum());
		ui.widget_wave_2->SetTimeData(XX, YY, maxYY, minYY);
	}


	if (!dataManagers[curModuleIndex].GetCalcResult().empty())
	{
		CalcResult result = dataManagers[curModuleIndex].GetCalcResult()[dataManagers[curModuleIndex].GetCalcResult().size() - 1];
		ui.openGLWidget->SetCurrentAngle(result.angle);
		ui.lineEdit_squareNum->setText(QString::number(result.squarenum));
		ui.lineEdit_rayNum->setText(QString::number(result.raysnum));
		ui.lineEdit_calResult->setText(QString::number(result.TS, 'g', 3));
		ui.lineEdit_calTime->setText(QString::number(result.calc_time, 'g', '4'));
		ui.lineEdit_KDNodeNum->setText(QString::number(calcthread->GetKDTreeInfo().length));
		ui.lineEdit_squareSize->setText(QString::number(result.width) + "*" + QString::number(result.height));
	}
	else
	{
		//ui.openGLWidget->SetCurrentAngle(result.angle);
		ui.lineEdit_squareNum->setText("");
		ui.lineEdit_rayNum->setText("");
		ui.lineEdit_calResult->setText("");
		ui.lineEdit_calTime->setText("");
		ui.lineEdit_KDNodeNum->setText("");
		ui.lineEdit_squareSize->setText("");
	}

}

//连接按钮槽函数
void SonarSimulationUI::OnActionConnect()
{
	log.DEBUG_log("点击连接按钮");
	if (dataManagers.size() == 0)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("运行"), QString::fromLocal8Bit("请先添加实例"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() != WAITFORCONNECT)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("网络已连接"), QMessageBox::Ok);
		return;
	}

	//普通模式
	if (curRepeatNum == 0)
	{
		ConnectDialog *dialog = new ConnectDialog(&ip, &port);

		while (true)
		{
			if (QDialog::Accepted == dialog->exec())
			{
				QTcpSocket *socket = new QTcpSocket();
				socket->connectToHost(ip, port);
				socket->setObjectName(QString("socket%1").arg(sockets.size()));
				if (!socket->waitForConnected(3000))
				{
					QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("连接失败"), QMessageBox::Ok);
					log.ERROR_log("网络连接失败");
					continue;
				}

				emit printInfo(QString::fromLocal8Bit("连接成功！"));
				dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
				sockets.push_back(socket);
				calcthread->AddFrameQueue();
				connect(socket, SIGNAL(readyRead()), this, SLOT(OnSocketReadyRead()));
				connect(socket, SIGNAL(disconnected()), this, SLOT(OnDisconnect()), Qt::QueuedConnection);
				log.INFO_log("网络连接成功");
				break;
			}

			break;
		}
		delete dialog;
	}

	//网络异常断开后重连
	else if (curRepeatNum > 0)
	{
		ip = "192.168.1.40";
		port = 3490;

		QTcpSocket *socket = new QTcpSocket();
		socket->connectToHost(ip, port);
		socket->setObjectName(QString("socket%1").arg(sockets.size()));
		if (!socket->waitForConnected(3000))
		{
			emit printInfo(QString::fromLocal8Bit("网络重连失败"));
			log.ERROR_log("网络重连失败");
			ErrorHandlerFunction();
			return;
		}

		emit printInfo(QString::fromLocal8Bit("重连成功！"));
		dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
		sockets.push_back(socket);
		calcthread->AddFrameQueue();
		connect(socket, SIGNAL(readyRead()), this, SLOT(OnSocketReadyRead()));
		connect(socket, SIGNAL(disconnected()), this, SLOT(OnDisconnect()), Qt::QueuedConnection);
		log.INFO_log("网络重连成功");
		curRepeatNum = 0;
		Sleep(3000);
		OnActionOpen();
	}
	

}

//网络断开时触发 jzy
void SonarSimulationUI::OnDisconnect()
{
	dataManagers[curModuleIndex].SetStatus(WAITFORCONNECT);
	log.WARNING_log("网络已断开");
	emit(printInfo(QString::fromLocal8Bit("网络已断开！")));

	sockets.clear();
	calcthread->ClearFrameVector();

	if (dataManagers[curModuleIndex].getCalcuedAngle() != -1)
	{
		string str = "已计算到第" + std::to_string(curFileConfigModInfoIndex) + "行的第" + std::to_string(dataManagers[curModuleIndex].getCalcuedAngle()) + "度";
		log.INFO_log(str);
	}
	ErrorHandlerFunction();
}


//读取配置文件界面开启 zsr
void SonarSimulationUI::OnActionexcel()
{
	log.DEBUG_log("点击读配置文件按钮");
	////导入数据到tablewidget中

	//ExcelEngine excels(QObject::tr("c:\\Import.xls"));
	//excels.Open();
	//excels.ReadDataToTable(ui_table.tableWidget); //导入到widget中
	//excels.Close();

	//实际运行时打开
	/*if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("网络未连接!"), QMessageBox::Ok);
		return;
	}*/

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("正在计算！"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONF)
	{
		if (QMessageBox::No == QMessageBox::question(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("模型已经加载，是否重新配置？"), QMessageBox::Yes, QMessageBox::No))
		{
			return;
		}
		log.WARNING_log("点击文件配置按钮，会重新加载模型");
	}



	tablewidget->show();


}

//文件配置模式，加载配置信息槽函数 姬梓遇
void SonarSimulationUI::OnLoadFileConfigModInfo(FileConfigModInfo* fileConfigModInfo, int maxNum, QString excelName)
{
	curConfigExcelName.clear();

	//得到配置excel文件的名称
	int idx = excelName.lastIndexOf("/");
	curConfigExcelName = excelName.right(excelName.size() - idx - 1);
	// 获取当前时间，并规范表示
	curConfigExcelName = curConfigExcelName + QString::fromStdString(log.getCurTimeString());

	dataManagers[curModuleIndex].setIsContinueRuning(false);
	dataManagers[curModuleIndex].isStop = false;
	fileConfigModInfos.clear();
	curFileConfigModInfoIndex = 0;

	ui.progressBar->reset();
	//ui.progressBar->setValue(0);   //设置进度条的值为0
	ui.progressBar->setRange(0, maxNum * 100);
	ui.progressBar->setVisible(true);
	ui.progressBar->setFormat(QString::fromLocal8Bit("文件配置模式，计算中：%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));


	for (int i = 0; i < maxNum; i++)
	{
		fileConfigModInfos.push_back(fileConfigModInfo[i]);
		emit updateProgressValue(0, i);
	}

	if (checkResultDeskInfo() == false)
		return;

	dataManagers[curModuleIndex].SetConfigType(FILECONFIGMOD);
	dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
	dataManagers[curModuleIndex].SetCalcType(fileConfigModInfos[curFileConfigModInfoIndex].cal_mood);	//扫角0、false  扫频1、ture
	dataManagers[curModuleIndex].setCalcuedAngle(-1);

	log.INFO_log("成功加载文件配置信息");

	OnActionOpen();
}

//判断结果文件大小是否会超过当前保存路径的磁盘空间
bool SonarSimulationUI::checkResultDeskInfo()
{
	unsigned long long tmpDesk = 0;
	for (int i = 0; i < fileConfigModInfos.size(); i++)
	{
		tmpDesk += fileConfigModInfos[i].config.sampling_width * fileConfigModInfos[i].config.sampling_rate * 14.8 * 181;
	}
	tmpDesk /= (1024 * 1024 * 1024);

	unsigned _int64 FreeBytesAvailabel = 0;
	ULARGE_INTEGER TotalNumberOfBytes;
	ULARGE_INTEGER TotalNumberOfFreeBytes;
	if (false == GetDiskFreeSpaceEx(NULL, (ULARGE_INTEGER *)&FreeBytesAvailabel, &TotalNumberOfBytes, &TotalNumberOfFreeBytes))
	{
		log.ERROR_log("检查磁盘空间出错");
		return false;
	}
	FreeBytesAvailabel /= (1024 * 1024 * 1024);

	if (tmpDesk > FreeBytesAvailabel)
	{
		log.ERROR_log("磁盘空间不足，无法存放配置表内的所有结果");
		QMessageBox::critical(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("磁盘空间不足，无法存放配置表内的所有结果！"), QMessageBox::Ok);
		return false;
	}
	return true;
}

//打开按钮槽函数
void SonarSimulationUI::OnActionOpen()
{
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		log.DEBUG_log("点击打开按钮");
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		log.DEBUG_log("调用函数OnActionOpen()");

	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("网络未连接!"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("正在计算！"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONF)
	{
		if (QMessageBox::No == QMessageBox::question(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("模型已经加载，是否重新加载？"), QMessageBox::Yes, QMessageBox::No))
		{
			return;
		}
		log.WARNING_log("重新加载模型");
	}

	dataManagers[curModuleIndex].Reset();

	//打开文件
	QString filename;
	//****文件路径编码转换，不然中文路径会加载出错 判断ConfigMod
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
	{
		filename = QFileDialog::getOpenFileName(NULL, "open", ".");
		dataManagers[curModuleIndex].setIsContinueRuning(false);
	}
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		if (curFileConfigModInfoIndex < fileConfigModInfos.size())
		{
			filename = QString(QString::fromLocal8Bit(this->fileConfigModInfos[curFileConfigModInfoIndex].modelPath));
			ui.progressBar->setValue(curFileConfigModInfoIndex * 100);
		}
		else
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("配置条目计算完毕！请重新加载配置文件！"), QMessageBox::Ok);
			log.INFO_log("配置条目计算完毕");
			ui.progressBar->setVisible(false);
			dataManagers[curModuleIndex].SetConfigType(CLIENTCONFIGMOD);
			curConfigExcelName.clear();
			fileConfigModInfos.clear();
			return;
		}
	}

	if (filename.isEmpty() == true)
		return;
	QTextCodec *textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("文件打开失败，请重新打开！"), QMessageBox::Ok);
		log.ERROR_log("模型文件打开失败");
		ErrorHandlerFunction();
		return;
	}
	std::string filename_stdstr = textCodec->fromUnicode(filename).data();

	dataManagers[curModuleIndex].SetModule(filename_stdstr);
	if (dataManagers[curModuleIndex].IsModuleLoaded() == false)
	{
		ErrorHandlerFunction();
		return;
	}



	//加载模型，获取模型的数据大小

	Triangle *p_Face = (Triangle*)(&dataManagers[curModuleIndex].GetModule().getFacesWK()[0]);
	Element *p_Node = (Element*)&dataManagers[curModuleIndex].GetModule().getNodesWK()[0];
	int faceDataLength = sizeof(Triangle)*dataManagers[curModuleIndex].GetModule().getFaceNum();
	int nodeDataLength = sizeof(Element)*dataManagers[curModuleIndex].GetModule().getNodeNum();

	//将面元数量设置给calcthread 对象
	calcthread->SetTriangleLength(dataManagers[curModuleIndex].GetModule().getFaceNum());


	//BuildXForTimeWave(ui.lineEdit_SamplingRate->text().toInt() * 2);

	//更新modelInfo textBrowser 内容

	{
		QString str;
		str = QString::fromLocal8Bit("面元数量：") + QString::number(dataManagers[curModuleIndex].GetModule().getFaceNum());
		emit(updateModelInfo(str));
		str.clear();
		str = QString::fromLocal8Bit("节点数量：") + QString::number(dataManagers[curModuleIndex].GetModule().getNodeNum());
		emit(updateModelInfo(str));

		str.clear();
		str = QString::fromLocal8Bit("min:");
		emit(updateModelInfo(str));
		str.clear();
		str = QString::fromLocal8Bit("    x:") + QString::number(dataManagers[curModuleIndex].GetModule().getMinX());
		emit(updateModelInfo(str));

		str.clear();
		str = QString::fromLocal8Bit("    y:") + QString::number(dataManagers[curModuleIndex].GetModule().getMinY());
		emit(updateModelInfo(str));

		str.clear();
		str = QString::fromLocal8Bit("    z:") + QString::number(dataManagers[curModuleIndex].GetModule().getMinZ());
		emit(updateModelInfo(str));

		str.clear();
		str = QString::fromLocal8Bit("max:");
		emit(updateModelInfo(str));
		str.clear();
		str = QString::fromLocal8Bit("    x:") + QString::number(dataManagers[curModuleIndex].GetModule().getMaxX());
		emit(updateModelInfo(str));

		str.clear();
		str = QString::fromLocal8Bit("    y:") + QString::number(dataManagers[curModuleIndex].GetModule().getMaxY());
		emit(updateModelInfo(str));

		str.clear();
		str = QString::fromLocal8Bit("    z:") + QString::number(dataManagers[curModuleIndex].GetModule().getMaxZ());
		emit(updateModelInfo(str));
	}

	ui.lineEdit_modelFaceNum->setText(QString::number(dataManagers[curModuleIndex].GetModule().getFaceNum()));
	ui.lineEdit_modelNodeNum->setText(QString::number(dataManagers[curModuleIndex].GetModule().getNodeNum()));
	//OpenGLWidget绘制模型
	ui.openGLWidget->LoadModel(dataManagers[curModuleIndex].GetModule());

	emit printInfo(QString::fromLocal8Bit("模型加载成功！"));
	emit printInfo(QString::fromLocal8Bit("开始向服务器传输数据..."));
	log.INFO_log("模型加载成功，开始向服务器传输数据");

	//设置发送的命令帧 Triangles
	strcpy(frame.command, "Triangles");
	frame.length = faceDataLength;

	int l = 0;
	int frameSize = sizeof(Frame);
	//发送命令帧
	l = sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	if (l != frameSize)
	{
		//QMessageBox::critical(this, "error", "Triangles命令帧发送错误", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Triangles命令帧发送错误"));
		log.ERROR_log("Triangles命令帧发送错误");
		ErrorHandlerFunction();
		return;
	}

	//发送数据
	l = sockets[curModuleIndex]->write((char*)p_Face, faceDataLength);
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	if (l != faceDataLength)
	{
		//QMessageBox::critical(this, "error", "Triangles数据发送错误", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Triangles数据发送错误"));
		log.ERROR_log("Triangles数据发送错误");
		ErrorHandlerFunction();
		return;
	}

	//设置发送的命令帧 Element
	strcpy(frame.command, "Elements");
	frame.length = nodeDataLength;
	//发送命令帧
	l = sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	if (l != frameSize)
	{
		//QMessageBox::critical(this, "error", "Elements命令帧发送错误", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Elements命令帧发送错误"));
		log.ERROR_log("Elements命令帧发送错误");
		ErrorHandlerFunction();
		return;
	}
	
	//发送数据
	l = sockets[curModuleIndex]->write((char*)p_Node, nodeDataLength);
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	//Sleep(1000);
	if (l != nodeDataLength)
	{
		//QMessageBox::critical(this, "error", "Triangles数据发送错误", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Triangles数据发送错误"));
		log.ERROR_log("Triangles数据发送错误");
		ErrorHandlerFunction();
		return;
	}

	if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		//若点击了停止按钮，则退出
		if (dataManagers[curModuleIndex].isStop)
		{
			dataManagers[curModuleIndex].isStop = false;
			return;
		}
		else
		{
			dataManagers[curModuleIndex].SetStatus(WAITFORCONF);
			OnActionRun();
			return;
		}
	}

	else
	{
		dataManagers[curModuleIndex].SetStatus(WAITFORCONF);
		ui.pushButton_deviceSelect->setEnabled(true);
	}
}

//运行按钮槽函数
void SonarSimulationUI::OnActionRun()
{
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		log.DEBUG_log("点击运行按钮");
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		log.DEBUG_log("调用函数OnActionRun()");

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("正在计算！"), QMessageBox::Ok);
		return;
	}
	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("网络未连接"), QMessageBox::Ok);
		return;
	}
	if (dataManagers[curModuleIndex].GetStatus() == WAITFORLOAD)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("模型未加载"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].IsCalcEmpty() == false)
	{
		dataManagers[curModuleIndex].ClearCalcResult();
	}

	
	//获取配置数据
	ConfigStruct &configStruct = dataManagers[curModuleIndex].GetConfig();

	//若需要继续运行，则从上次计算的角度开始计算
	if (dataManagers[curModuleIndex].IsContinueRuning())
	{
		if (dataManagers[curModuleIndex].getCalcuedAngle() != -1)
		{
			if (dataManagers[curModuleIndex].getCalcuedAngle() < configStruct.end_alpha)
			{
				configStruct.start_alpha = dataManagers[curModuleIndex].getCalcuedAngle() + 1;
				emit printInfo(QString::fromLocal8Bit("从") + QString::number(dataManagers[curModuleIndex].getCalcuedAngle(), 10) + QString::fromLocal8Bit("度开始继续计算"));
				log.INFO_log("从" + std::to_string(dataManagers[curModuleIndex].getCalcuedAngle()) + "度开始继续计算");
			}
			else
				dataManagers[curModuleIndex].setIsContinueRuning(false);
		}
		else
			dataManagers[curModuleIndex].setIsContinueRuning(false);
	}

	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
	{
		configStruct.start_beta = ui.lineEdit_startBeta->text().toInt();
		configStruct.start_alpha = ui.lineEdit_startAlpha->text().toInt();
		configStruct.end_alpha = ui.lineEdit_endAlpha->text().toDouble();
		configStruct.pipe_size = ui.lineEdit_pipeSize->text().toDouble();
		configStruct.wave_length = ui.lineEdit_waveLength->text().toDouble();
		configStruct.start_frequency = ui.lineEdit_startFrequency->text().toDouble();
		configStruct.end_frequency = ui.lineEdit_endFrequency->text().toDouble();
		configStruct.far_distance = ui.lineEdit_farDistance->text().toDouble();
		//2021.8.30姬梓遇  配置时域积分数据
		configStruct.sampling_rate = ui.lineEdit_SamplingRate->text().toDouble();
		configStruct.tao = ui.lineEdit_PulseWidth->text().toDouble();
		configStruct.time_start_frequency = ui.lineEdit_StartingFreq->text().toDouble();
		configStruct.time_end_frequency = ui.lineEdit_EndingFreq->text().toDouble();
		configStruct.relative_velocity = ui.lineEdit_RelativeSpeed->text().toDouble();
		configStruct.integral_gain = ui.lineEdit_IntegralAdjust->text().toDouble();
		configStruct.velocity1 = ui.lineEdit_Velocity1->text().toDouble();
		configStruct.velocity2 = ui.lineEdit_Velocity2->text().toDouble();
		configStruct.reflect_coeff = ui.lineEdit_ReflectCoeff->text().toDouble();
		configStruct.reflect_coeff_Auto_flag = ui.radioButton_ConfigReflectCoeffAuto->isChecked() ? true : false;
		configStruct.sampling_width = ui.lineEdit_SamplingWidth->text().toDouble();
		//dataManagers[curModuleIndex].SetCalcType(calcType);
		//设置积分结果总数   * sampling_width
		calcthread->SetTimeIntegrationResultsLength(int(ui.lineEdit_SamplingRate->text().toInt() * ui.lineEdit_SamplingWidth->text().toDouble()));
	}
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		//若不为暂停后继续运行，则直接读取config struct
		if(dataManagers[curModuleIndex].IsContinueRuning() == false)
			configStruct = fileConfigModInfos[curFileConfigModInfoIndex].config;

		//设置积分结果总数   * sampling_width
		calcthread->SetTimeIntegrationResultsLength(int(configStruct.sampling_rate * configStruct.sampling_width));
	}


	if (configStruct.card_num <= 0)
	{
		if(dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
			QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("未选择任何一块计算卡！"), QMessageBox::Ok);

		log.ERROR_log("未选择任何一块计算卡");
		ErrorHandlerFunction();
		return;
	}

	if (deviceInfoVector.size() == 0)
	{
		log.ERROR_log("服务器未检测到计算卡");
		ErrorHandlerFunction();
		return;
	}
	if (configStruct.card_num > deviceInfoVector.size())
	{
		log.ERROR_log("所选的计算卡数量超过服务器计算卡上限");
		ErrorHandlerFunction();
		return;
	}

	//计算总的核心数量
	int coresNum = 0;
	for (int i = 0; i < configStruct.card_num; i++)
	{
		coresNum += deviceInfoVector[i].coresPreMutiprocess*deviceInfoVector[i].mutiprocessCount;
	}
	ui.lineEdit_coresNum->setText(QString::number(coresNum));


	if (configStruct.start_alpha > configStruct.end_alpha)
	{
		if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
			QMessageBox::information(this, "error", QString::fromLocal8Bit("起始α角度大于终止α角度！"), QMessageBox::Ok);
		log.ERROR_log("配置信息错误，起始α角度大于终止α角度");
		ErrorHandlerFunction();
		return;
	}
	if (configStruct.start_frequency > configStruct.end_frequency)
	{
		if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
			QMessageBox::information(this, "error", QString::fromLocal8Bit("起始积分频率大于终止积分频率！"), QMessageBox::Ok);
		log.ERROR_log("起始积分频率大于终止积分频率！");
		ErrorHandlerFunction();
		return;
	}

	//设置波形显示的初始值

	ui.widget_wave_2->Init(waveTime);

	if (dataManagers[curModuleIndex].GetCalcType() == Angle)
		ui.widget_wave->Init(waveAngle);
	else
		ui.widget_wave->Init(waveFreq);


	if (dataManagers[curModuleIndex].isStop)
	{
		dataManagers[curModuleIndex].isStop = false;
		return;
	}

	// 向服务器发送开始计算信号
	strcpy(frame.command, "Configuration");
	frame.length = sizeof(configStruct);
	memcpy(frame.data, &configStruct, sizeof(configStruct));
	sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));

	emit printInfo(QString::fromLocal8Bit("配置完成，开始计算"));
	log.DEBUG_log("配置完成，开始计算");

	ui.lineEdit_GPUnum->setText(QString::number(configStruct.card_num));
	//正在计算标志变量设置为true                                                                                                                                                                          
	//isRunning = true;


	dataManagers[curModuleIndex].SetStatus(RUNNING);

}

//保存按钮槽函数
void SonarSimulationUI::OnActionSave()
{
	log.DEBUG_log("点击保存按钮");
	if (dataManagers[curModuleIndex].IsCalcEmpty())
	{
		QMessageBox::information(this, "warning", QString::fromLocal8Bit("没有数据存储"), QMessageBox::Ok);
		return;
	}

	QString filename = QFileDialog::getSaveFileName(NULL, "open", ".", "*.txt");
	if (filename.isEmpty() == true)
		return;
	QTextCodec *textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "bug", QString::fromLocal8Bit("文件打开失败，请重新打开！"), QMessageBox::Ok);
		return;
	}

	if (!excelHelper.newExcel(filename))
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("MiscoSoft Office COM open failed！"), QMessageBox::Ok);
		return;
	}


	QList<QVariant> rowdata;

	QList<QVariant> headline;
	headline.append(QString::fromLocal8Bit("角度"));
	headline.append(QString::fromLocal8Bit("频率"));
	headline.append(QString::fromLocal8Bit("TS值"));
	headline.append(QString::fromLocal8Bit("计算时间"));
	headline.append(QString::fromLocal8Bit("射线数量"));
	headline.append(QString::fromLocal8Bit("管线数量"));
	headline.append(QString::fromLocal8Bit("宽"));
	headline.append(QString::fromLocal8Bit("高"));

	QVariant head(headline);
	rowdata.append(head);

	for (int i = 0; i < dataManagers[curModuleIndex].GetCalcResult().size(); i++)
	{
		QList<QVariant> aline;
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].angle));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].freq));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].TS, 'g', 4));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].calc_time, 'g', 4));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].raysnum));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].squarenum));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].width));
		aline.append(QString::number(dataManagers[curModuleIndex].GetCalcResult()[i].height));

		QVariant line(aline);
		rowdata.append(line);
	}
	excelHelper.setRange(dataManagers[curModuleIndex].GetCalcResult().size() + 1, 8, rowdata);

	excelHelper.saveExcel(filename);
	excelHelper.freeExcel();

	QMessageBox::information(this, "Save", QString::fromLocal8Bit("保存成功"), QMessageBox::Ok);
}

//停止按钮槽函数
void SonarSimulationUI::OnActionStop()
{
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		log.DEBUG_log("点击停止按钮");
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		log.DEBUG_log("调用函数OnActionStop()");

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		//curRepeatNum != 0说明是计算过程中出现错误而调用此函数
		if (curRepeatNum != 0)
		{
			strcpy(frame.command, "Stop");
			frame.length = 0;
			sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));

			emit printInfo(QString::fromLocal8Bit("计算终止！"));
			log.WARNING_log("计算终止");

			if (dataManagers[curModuleIndex].IsCalcEmpty() == false)
			{
				dataManagers[curModuleIndex].SetStatus(WAITFORCONF);
				OnActionRun();
				return;
			}
			else
			{
				dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
				OnActionOpen();
				return;
			}
		}

		else
		//else if (QMessageBox::Yes == QMessageBox::question(this, "warning", QString::fromLocal8Bit("正在计算中，是否停止？"), QMessageBox::Yes, QMessageBox::No))
		{
			dataManagers[curModuleIndex].isStop = true;
			dataManagers[curModuleIndex].setIsContinueRuning(true);

			emit printInfo(QString::fromLocal8Bit("计算终止！"));
			log.WARNING_log("计算终止");

			//此判断用于确定用户点击确定终止后的运行状态
			if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
			{
				strcpy(frame.command, "Stop");
				frame.length = 0;
				sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));
				dataManagers[curModuleIndex].isStop = false;
				dataManagers[curModuleIndex].SetStatus(WAITFORCONF);
			}
			
			
		}
	}

}

//新建按钮槽函数
void SonarSimulationUI::OnActionNew()
{
	log.DEBUG_log("点击新建按钮");
	curRepeatNum = 0;
	curFileConfigModInfoIndex = 0;

	calcthread->AddFrameQueue();
	calcthread->AddPreTriangleResults();
	calcthread->AddTimeIntegrationResults();
	DataManager dataManager;

	ConfigStruct configStruct;
	configStruct.start_beta = 90;
	configStruct.start_alpha = 0;
	configStruct.end_alpha = 180;
	configStruct.pipe_size = 0.5;
	configStruct.wave_length = 0.05;
	configStruct.start_frequency = 30;
	configStruct.end_frequency = 30;
	configStruct.far_distance = 1000;
	configStruct.card_num = -1;

	dataManager.SetStatus(WAITFORCONNECT);
	dataManager.SetCalcType(Angle);
	dataManager.SetConfig(configStruct);
	dataManager.SetConfigType(CLIENTCONFIGMOD);

	dataManagers.push_back(dataManager);

	comboBoxModules->addItem(QString::fromLocal8Bit("模型 %1").arg(dataManagers.size()));

	curModuleIndex = comboBoxModules->count() - 1;
	comboBoxModules->setCurrentIndex(curModuleIndex);
	calcthread->SetCurrentModule(curModuleIndex);


	InitAll();
}

//浮动框的显示开关，
void SonarSimulationUI::OnDockWidgetWave(bool value)
{
	ui.actionWave->setChecked(value);
}

void SonarSimulationUI::OnDockWidgetConfigurate(bool value)
{
	ui.actionConfigrate->setChecked(value);
}

void SonarSimulationUI::OnDockWidgetCalcInfo(bool value)
{
	ui.actionCalcInfo->setChecked(value);
}

void SonarSimulationUI::OnDockWidgetPrintInfo(bool value)
{
	ui.actionPrintInfo->setChecked(value);
}

void SonarSimulationUI::OnDockWidgetGPUWatch(bool value)
{
	ui.actionGPUWatch->setChecked(value);
}

//工具栏控制浮动框显示与关闭的开关
void SonarSimulationUI::OnActionWave(bool value)
{
	ui.dockWidget_wave->setVisible(value);
}

void SonarSimulationUI::OnActionConfigurate(bool value)
{
	ui.dockWidget_configurate->setVisible(value);
}

void SonarSimulationUI::OnActionCalcInfo(bool value)
{
	ui.dockWidget_calcInfo->setVisible(value);
}

void SonarSimulationUI::OnActionPrintInfo(bool value)
{
	ui.dockWidget_printInfo->setVisible(value);
}

void SonarSimulationUI::OnActionGPUWatch(bool value)
{
	ui.dockWidget_gpuWatch->setVisible(value);
}

//设备选择按钮的槽函数
void SonarSimulationUI::OnPushbuttonDeviceSelect()
{
	log.DEBUG_log("点击设备选择按钮");
	ConfigStruct &configStruct = dataManagers[curModuleIndex].GetConfig();
	int *selectDevice = configStruct.select_device_list;
	DeviceSelect dialog(this, selectDevice, deviceCount, &configStruct.card_num);
	while (true)
	{
		if (QDialog::Accepted == dialog.exec())
		{
			if (configStruct.card_num <= 0)
			{
				QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("未选择任何一块计算卡！"), QMessageBox::Ok);
				log.WARNING_log("未选择任何一块计算卡");
				continue;
			}

			emit printInfo(QString::fromLocal8Bit("计算用卡："));
			QString s;
			for (int i = 0; i < configStruct.card_num; i++)
			{
				s = s + "card" + QString::number(selectDevice[i]) + " ";
			}
			emit printInfo(s);
			break;
		}
		break;
	}
}

//接收socket数据槽函数
void SonarSimulationUI::OnSocketReadyRead()
{
	//多模型解算时会有多个socket连接，这段代码主要是确定接收到的数据是哪个模型的
	QObject *object = QObject::sender();
	QTcpSocket *socket = qobject_cast<QTcpSocket *>(object);
	QString ob_name = socket->objectName();
	int i = ob_name.right(ob_name.size() - 6).toInt();

	while (true)
	{
		if (socket->bytesAvailable() != 0)
		{
			int length = socket->read(((char*)&recvFrame[i] + recvLength[i]), sizeof(Frame) - recvLength[i]);
			recvLength[i] += length;
			if (recvLength[i] == sizeof(Frame))
			{
				//接收到的数据直接push到calcthread 中去处理
				calcthread->PushFrame(recvFrame[i], i);

				recvLength[i] = 0;
				continue;
			}
		}
		else
		{
			break;
		}
	}


}

//处理扫角RadioButton的槽函数
void SonarSimulationUI::OnRadioButtonScanAngle()
{

	if (dataManagers[curModuleIndex].GetCalcType() == Angle)
	{
		return;
	}
	dataManagers[curModuleIndex].SetCalcType(Angle);
	ui.widget_wave->Init(waveAngle);
	ui.widget_wave_2->Init(waveTime);//
	//将频率的值统一并设置不可更改，避免误操作
	ui.lineEdit_endFrequency->setText(ui.lineEdit_startFrequency->text());
	ui.lineEdit_endFrequency->setEnabled(false);
	ui.lineEdit_endAlpha->setEnabled(true);

}

//处理扫频RadioButton的槽函数
void SonarSimulationUI::OnRadioButtonScanFrequency()
{
	if (dataManagers[curModuleIndex].GetCalcType() == Frequency)
	{
		return;
	}
	dataManagers[curModuleIndex].SetCalcType(Frequency);
	ui.widget_wave->Init(waveFreq);
	ui.widget_wave_2->Init(waveTime);//
	ui.lineEdit_endAlpha->setText(ui.lineEdit_startAlpha->text());
	ui.lineEdit_endAlpha->setEnabled(false);
	ui.lineEdit_startFrequency->setEnabled(true);
	ui.lineEdit_endFrequency->setEnabled(true);
}

//处理自动配置反射系数RadioButton的槽函数 姬梓遇
void SonarSimulationUI::OnRadioButtonConfigReflectCoeffAuto()
{
	ui.lineEdit_ReflectCoeff->setEnabled(false);
}

//处理手动配置反射系数RadioButton的槽函数 姬梓遇
void SonarSimulationUI::OnRadioButtonConfigReflectCoeffText()
{
	ui.lineEdit_ReflectCoeff->setEnabled(true);
}

//关闭事件的处理函数
void SonarSimulationUI::closeEvent(QCloseEvent *event)
{
	log.DEBUG_log("点击退出按钮");
	if (dataManagers.size() == 0)
	{
		event->accept();
		tablewidget->close();
		datasave->close();
		log.INFO_log("退出");
		return;
	}
	if (dataManagers[curModuleIndex].GetStatus() != WAITFORCONNECT)
	{
		if (QMessageBox::No == QMessageBox::question(this, QString::fromLocal8Bit("退出程序"), QString::fromLocal8Bit("是否断开网络并退出？"), QMessageBox::Yes | QMessageBox::No))
		{
			event->ignore();
			return;
		}
		else
		{
			/*for (int index = 0; index < dataManagers.size(); index++)
			{
			ReadreadyThreads[index]->terminate();
			ReadreadyThreads[index]->wait();
			}*/
			strcpy(frame.command, "Exit");
			frame.length = 0;
			for (int index = 0; index < sockets.size(); index++)
			{
				sockets[index]->write((char*)&frame, sizeof(frame));
				sockets[index]->waitForBytesWritten();
			}
			calcthread->terminate();
			calcthread->wait();

			event->accept();
			tablewidget->close();
			datasave->close();
			log.INFO_log("退出");
			return;
		}
	}
	else
	{
		event->accept();
		tablewidget->close();
		datasave->close();
		log.INFO_log("退出");
		return;
	}
}

//处理回传计算结果槽函数
void SonarSimulationUI::OnRecvResult(CalcResult result, int i)
{
	dataManagers[i].PushCalcResult(result);
	if (i == curModuleIndex)
	{
		QString str1;
		str1.sprintf("%d", result.angle);

		QString str3;
		str3.sprintf("%0.4fms", result.calc_time);
		QString str5;
		str5.sprintf("%0.4fdB", result.TS);

		int progressValue = 0;

		//用该变量标志当前是否为当前配置行第一次接到结果
		int firstResultSignal = 0;

		if (dataManagers[curModuleIndex].GetCalcType() == Angle)
		{
			emit(printInfo(str1 + QString::fromLocal8Bit("度; 计算时间：") + str3 + QString::fromLocal8Bit("; 积分结果：") + str5));
			//波形绘制数据
			ui.widget_wave->PushResult(result.angle, result.TS);


			if (dataManagers[curModuleIndex].GetConfig().start_alpha == result.angle)
				firstResultSignal = 1;
			if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
				progressValue = (curFileConfigModInfoIndex + result.angle / (fileConfigModInfos[curFileConfigModInfoIndex].config.end_alpha - fileConfigModInfos[curFileConfigModInfoIndex].config.start_alpha)) * 100;
		}
		else if (dataManagers[curModuleIndex].GetCalcType() == Frequency)
		{
			emit(printInfo(QString::number(result.freq, 'f', 1) + QString::fromLocal8Bit("KHz:") + QString::fromLocal8Bit(" 计算时间：") + str3 + QString::fromLocal8Bit("; 积分结果：") + str5));
			//波形绘制数据
			ui.widget_wave->PushResult(result.freq, result.TS);

			if (dataManagers[curModuleIndex].GetConfig().start_frequency == result.freq)
				firstResultSignal = 1;
			if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
				progressValue = (curFileConfigModInfoIndex + result.angle / (fileConfigModInfos[curFileConfigModInfoIndex].config.end_frequency - fileConfigModInfos[curFileConfigModInfoIndex].config.start_frequency)) * 100;
		}
		emit(printInfo(QString::fromLocal8Bit("     虚拟孔径面线程数：") + QString::number(result.squarenum) +
			QString::fromLocal8Bit("; 射线追踪线程数：") + QString::number(result.raysnum) +
			QString::fromLocal8Bit("; 积分线程数：") + QString::number(result.squarenum)));
		ui.lineEdit_squareNum->setText(QString::number(result.squarenum));
		ui.lineEdit_rayNum->setText(QString::number(result.raysnum));
		ui.lineEdit_calResult->setText(QString::number(result.TS, 'g', 3));
		ui.lineEdit_calTime->setText(QString::number(result.calc_time, 'g', 4));
		ui.lineEdit_KDNodeNum->setText(QString::number(calcthread->GetKDTreeInfo().length));
		ui.lineEdit_squareSize->setText(QString::number(result.width) + "*" + QString::number(result.height));

		ui.openGLWidget->SetCurrentAngle(result.angle);

		//创建时域积分结果文件夹 姬梓遇
		if (CreatTimeIntegrationResultsFolder(result.angle, result.freq, dataManagers[curModuleIndex].GetCalcType()) == 0)
		{
			//TS结果写入txt
			if (SaveTSResultsFile(result.TS) == -1)
				return;

			if (firstResultSignal == 1 && dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD && dataManagers[curModuleIndex].IsContinueRuning() == false)
			{
				//一条配置信息计算完后向数据库写信息
				QString arg_id = QString::number(fileConfigModInfos[curFileConfigModInfoIndex].index);
				datasave->getUsersql().addSaveInfo(arg_id, curConfigExcelName, curResultFolderPath);
				//datasave->OnPushbuttonDisplay();
			}
		}
		else
			return;

		if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		{
			ui.progressBar->setValue(progressValue);
			emit(updateProgressValue(progressValue - curFileConfigModInfoIndex * 100, curFileConfigModInfoIndex));
		}
		//couts[i]++;
	}
}

//计算结束槽函数
void SonarSimulationUI::OnCalcOver(int i)
{
	dataManagers[i].SetStatus(WAITFORCONF);
	dataManagers[i].setCalcuedAngle(-1);
	dataManagers[i].setIsContinueRuning(false);
	if (i == curModuleIndex)
	{
		emit(printInfo(QString::fromLocal8Bit("计算完成！")));
		log.INFO_log("计算完成");

		if (dataManagers[i].GetConfigType() == FILECONFIGMOD)
		{
			dataManagers[i].SetStatus(WAITFORLOAD);

			char command[50];
			sprintf(command, "第%d条配置信息已计算完毕", curFileConfigModInfoIndex + 1);
			log.INFO_log(command);

			curResultFolderPath.clear();
			curTimeIntegrationResultsPath.clear();
			//curRepeatNum = 0;
			//log.setLogLevel(info);
			curFileConfigModInfoIndex += 1;
			OnActionOpen();
		}
	}

}

//接收到GPU信息的处理函数
void SonarSimulationUI::OnRecvGPUWatch(GPUWatchStruct info)
{
	ui.widget_GPUwatch->SetData(info);
}

//接收到设备信息的处理函数
void SonarSimulationUI::OnRecvDeviceInfo(DeviceInfo info)
{
	if (info.deviceID == 0)
	{
		/*for (int i = 0; i < info.deviceCount; i++)
		{
		ui.comboBox_GPUnum->addItem(QString::number(i + 1));
		ui.comboBox_GPUnum->setCurrentIndex(info.deviceCount - 1);
		}
		ui.comboBox_GPUnum->setCurrentIndex(info.deviceCount - 1);*/
		deviceCount = info.deviceCount;
		ui.widget_GPUwatch->Init(info.deviceCount);
	}
	sumCores += info.mutiprocessCount*info.coresPreMutiprocess;
	deviceInfoVector.push_back(info);

	emit printInfo(QString::fromLocal8Bit("设备") + QString::number(info.deviceID) + QString::fromLocal8Bit("，名称：") + QString(info.deviceName)
		+ QString::fromLocal8Bit("  流处理器数量：") + QString::number(info.mutiprocessCount) + QString::fromLocal8Bit("  核心数/SM：")
		+ QString::number(info.coresPreMutiprocess) + QString::fromLocal8Bit("  总核心数：") + QString::number(info.mutiprocessCount*info.coresPreMutiprocess));

}

//接收到KDTree数据
void SonarSimulationUI::OnLoadKDTree(KD_Node_V* kdtree, int length, int i)
{
	dataManagers[i].SetBoxData(kdtree, length);
	if (i == curModuleIndex)
		ui.openGLWidget->OnLoadKDTree(dataManagers[curModuleIndex].GetBoxData());
}

//工具栏上切换模型的槽函数
void SonarSimulationUI::OnComboBoxModules(int index)
{
	curModuleIndex = index;
	calcthread->SetCurrentModule(curModuleIndex);
	InitAll();

}

//接收到每个三角面片积分结果后，更新
void SonarSimulationUI::OnUpdatePreTriangleResults(float *data, int length, int i)
{
	dataManagers[i].SetIsPreTrianglesResultsLoaded(true);
	dataManagers[i].SetPreTriangleResults(data);
	dataManagers[i].SetPreTriangleResultsNum(length);
	if (i == curModuleIndex)
	{
		ui.openGLWidget->UpdatePreTriangleResults(data, length);
	}
}

//接收到每次时域积分结果后，更新
void SonarSimulationUI::OnUpdateTimeIntegrationResults(float *data, int length, int i)
{
	dataManagers[i].SetTimeIntegrationResults(data);
	dataManagers[i].SetTimeIntegrationResultsNum(length);
	if (i == curModuleIndex)
	{
		BuildXYForTimeWave(data, length);
		ui.widget_wave_2->SetTimeData(XX, YY, maxYY, minYY);
		//时域积分结果写入txt
		SaveTimeIntegrationResultsFile();

		curRepeatNum = 0;
		log.setLogLevel(info);
	}
}

void SonarSimulationUI::BuildXYForTimeWave(float * data, int couts)
{
	XX.clear();
	YY.clear();
	maxYY = -999999999, minYY = 999999999;
	double x, y;
	for (int i = 0; i < couts; i++)
	{
		y = data[i];
		
		//排除掉正无穷与负无穷的点 20220408
		if (y < -10e10 || y > 10e10)
			y = 0;
		if (_isnan(y))
		{
			y = 0;
		}
		if (y > maxYY)
		{
			maxYY = y;
		}
		if (y < minYY)
		{
			minYY = y;
		}

		XX.push_back(i);
		YY.push_back(y);
	}
}

//创建时域积分结果文件夹 姬梓遇
//返回值： 成功0  失败-1
int SonarSimulationUI::CreatTimeIntegrationResultsFolder(int curAngle, int curFreq, int CalcuType)
{
	char folderPath[100];
	char folderFullPath[200];
	char ResultName[30];
	char ResultPath[130];

	curTimeIntegrationResultsPath.clear();
	curResultFolderPath.clear();

	sprintf(folderPath, ".\\data");
	if (access(folderPath, 0))
	{
		if (mkdir(folderPath) == -1)
		{
			log.ERROR_log("时域积分结果文件创建失败");
			ErrorHandlerFunction();
			return -1;
		}
	}

	//写文件夹路径
	if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		char* str;
		QByteArray tempstr;
		tempstr = curConfigExcelName.toLocal8Bit();
		str = tempstr.data();
		sprintf(folderPath, ".\\data\\%s_Result", str);
		if (access(folderPath, 0))
		{
			if (mkdir(folderPath) == -1)
			{
				log.ERROR_log("时域积分结果文件创建失败");
				ErrorHandlerFunction();
				return -1;
			}
		}
		sprintf(folderPath, "%s\\FileConfigResult_%d", folderPath, fileConfigModInfos[curFileConfigModInfoIndex].index);
	}
	else if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		sprintf(folderPath, ".\\data\\ClientConfigResult");

	//创建文件夹
	if (access(folderPath, 0))
	{
		if (mkdir(folderPath) == -1)
		{
			log.ERROR_log("时域积分结果文件创建失败");
			ErrorHandlerFunction();
			return -1;
		}
	}

	if (CalcuType == Angle)
		sprintf(ResultName, "Angle_%d.txt", curAngle);
	else if (CalcuType == Frequency)
		sprintf(ResultName, "Freq_%d.txt", curFreq);

	sprintf(ResultPath, "%s\\%s", folderPath, ResultName);
	curTimeIntegrationResultsPath = QString::fromLocal8Bit(ResultPath);


	//if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	//{
		if (_fullpath(folderFullPath, folderPath, 200) == NULL)
		{
			log.ERROR_log("时域积分结果文件路径过长");
			ErrorHandlerFunction();
			return -1;
		}
		else
		{
			curResultFolderPath = QString::fromLocal8Bit(folderFullPath);	
			return 0;
		}
	//}

	return 0;
}

//时域积分结果写入txt 姬梓遇
//返回值： 成功0  失败-1
int SonarSimulationUI::SaveTimeIntegrationResultsFile()
{
	FILE* file;
	file = fopen(curTimeIntegrationResultsPath.toLocal8Bit(), "w");
	if (file == NULL)
	{
		log.ERROR_log("时域积分结果文件创建错误");
		ErrorHandlerFunction();
		return -1;
	}
	for (int i = 0; i < dataManagers[curModuleIndex].GetTimeIntegrationResultsNum(); i++)
	{
		if (fprintf(file, "%e\n", dataManagers[curModuleIndex].GetTimeIntegrationResults()[i]) < 0)
		{
			emit(printInfo(QString::fromLocal8Bit("时域积分结果写文件错误！")));
			log.ERROR_log("时域积分结果写文件错误");
			ErrorHandlerFunction();
			return -1;
		}
	}

	fclose(file);

	//curRepeatNum非0时，说明程序出错，此时CalcuedAngle记录出错时刻的计算进度，此处则不应更新
	if (curRepeatNum == 0)
		dataManagers[curModuleIndex].updateCalcuedAngle();

	return 0;
}

//TS结果写入txt 姬梓遇
//返回值： 成功0  失败-1
int SonarSimulationUI::SaveTSResultsFile(float TS)
{
	FILE* file;
	QString TSPath = curResultFolderPath + "//TS.txt";
	file = fopen(TSPath.toLocal8Bit(), "a");
	if (file == NULL)
	{
		log.ERROR_log("TS结果文件创建错误");
		ErrorHandlerFunction();
		return -1;
	}
	if (fprintf(file, "%e\n", TS) < 0)
	{
		emit(printInfo(QString::fromLocal8Bit("TS结果写文件错误！")));
		log.ERROR_log("TS结果写文件错误");
		ErrorHandlerFunction();
		return -1;
	}
	fclose(file);
	return 0;
}

//工具栏打开数据库存储管理界面槽函数  2021.12.20 柯静 
void SonarSimulationUI::OnActionDataBase()
{
	log.DEBUG_log("点击数据库管理按钮");
	/*if (dataManagers[curModuleIndex].IsCalcEmpty())
	{
		QMessageBox::information(this, "warning", QString::fromLocal8Bit("没有数据存储"), QMessageBox::Ok);
		return;
	}*/
	datasave->show();
}

void SonarSimulationUI::OnGetRunningStatusSig(int &status)
{
	status = dataManagers[curModuleIndex].GetStatus();
}

//错误处理函数 姬梓遇
void SonarSimulationUI::ErrorHandlerFunction()
{
	if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		dataManagers[curModuleIndex].setIsContinueRuning(true);
		log.setLogLevel(debug);

		string str;
		if (curRepeatNum < 3)
		{
			curRepeatNum += 1;
			str = "出现错误，开始第" + to_string(curRepeatNum) + "次运行";
			log.ERROR_log(str);

			if (dataManagers[curModuleIndex].GetStatus() == WAITFORLOAD)
			{
				OnActionOpen();
				return;
			}
			else if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONF)
			{
				dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
				OnActionOpen();
				return;
			}
			else if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
			{
				OnActionStop();
				return;
			}
			else if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
			{
				Sleep(500);
				OnActionConnect();
				return;
			}
		}
		else
		{
			curRepeatNum = 0;
			emit(printInfo(QString::fromLocal8Bit("出现错误，已重新运行三次，请检查错误日志")));
			log.ERROR_log("出现错误，已重新运行三次");
			if (dataManagers[curModuleIndex].getCalcuedAngle() != -1)
			{
				str = "已计算到第" + std::to_string(curFileConfigModInfoIndex + 1) + "行的第" + std::to_string(dataManagers[curModuleIndex].getCalcuedAngle()) + "度";
				log.INFO_log(str);
				//OnActionOpen();
				return;
			}
			else if (curFileConfigModInfoIndex < fileConfigModInfos.size() - 1)
			{
				curFileConfigModInfoIndex += 1;
				str = "从第" + to_string(curFileConfigModInfoIndex + 1) + "行开始计算";
				log.WARNING_log(str);
				dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
				OnActionOpen();
				return;
			}
			else
			{
				QMessageBox::warning(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("出现错误，已重新运行三次，请检查错误日志"), QMessageBox::Ok);
				return;
			}
		}
	}
	else
		return;
}