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

	setWindowTitle(QString::fromLocal8Bit("����Ŀ��̬�ƶ���������Զ��������ƽ̨"));

	tablewidget = new QtTablewidget;
	datasave = new dataSavewidget;
	comboBoxModules = new QComboBox();
	ui.mainToolBar->addWidget(comboBoxModules);
	ui.progressBar->setVisible(false);

	log.AddBlockLine();
	log.INFO_log("�򿪽���");
	connect(comboBoxModules, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxModules(int)));

	//��һ�߳�ѭ��������յ�������
	calcthread = new CalcThread();
	calcthread->start();

	memset(recvLength, 0, sizeof(int) * 5);

	//����socket��ȡ�źŵĴ���ۺ���
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

//�½����л�����ģ��ʱ��ִ�г�ʼ��
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
		//���λ�������
		for (int i = 0; i < dataManagers[curModuleIndex].GetCalcResult().size(); i++)
		{
			CalcResult result = dataManagers[curModuleIndex].GetCalcResult()[i];
			ui.widget_wave->PushResult(result.angle, result.TS);
		}

	}
	else if (dataManagers[curModuleIndex].GetCalcType() == Frequency)
	{
		//���λ�������
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

//���Ӱ�ť�ۺ���
void SonarSimulationUI::OnActionConnect()
{
	log.DEBUG_log("������Ӱ�ť");
	if (dataManagers.size() == 0)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("�������ʵ��"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() != WAITFORCONNECT)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����������"), QMessageBox::Ok);
		return;
	}

	//��ͨģʽ
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
					QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ʧ��"), QMessageBox::Ok);
					log.ERROR_log("��������ʧ��");
					continue;
				}

				emit printInfo(QString::fromLocal8Bit("���ӳɹ���"));
				dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
				sockets.push_back(socket);
				calcthread->AddFrameQueue();
				connect(socket, SIGNAL(readyRead()), this, SLOT(OnSocketReadyRead()));
				connect(socket, SIGNAL(disconnected()), this, SLOT(OnDisconnect()), Qt::QueuedConnection);
				log.INFO_log("�������ӳɹ�");
				break;
			}

			break;
		}
		delete dialog;
	}

	//�����쳣�Ͽ�������
	else if (curRepeatNum > 0)
	{
		ip = "192.168.1.40";
		port = 3490;

		QTcpSocket *socket = new QTcpSocket();
		socket->connectToHost(ip, port);
		socket->setObjectName(QString("socket%1").arg(sockets.size()));
		if (!socket->waitForConnected(3000))
		{
			emit printInfo(QString::fromLocal8Bit("��������ʧ��"));
			log.ERROR_log("��������ʧ��");
			ErrorHandlerFunction();
			return;
		}

		emit printInfo(QString::fromLocal8Bit("�����ɹ���"));
		dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
		sockets.push_back(socket);
		calcthread->AddFrameQueue();
		connect(socket, SIGNAL(readyRead()), this, SLOT(OnSocketReadyRead()));
		connect(socket, SIGNAL(disconnected()), this, SLOT(OnDisconnect()), Qt::QueuedConnection);
		log.INFO_log("���������ɹ�");
		curRepeatNum = 0;
		Sleep(3000);
		OnActionOpen();
	}
	

}

//����Ͽ�ʱ���� jzy
void SonarSimulationUI::OnDisconnect()
{
	dataManagers[curModuleIndex].SetStatus(WAITFORCONNECT);
	log.WARNING_log("�����ѶϿ�");
	emit(printInfo(QString::fromLocal8Bit("�����ѶϿ���")));

	sockets.clear();
	calcthread->ClearFrameVector();

	if (dataManagers[curModuleIndex].getCalcuedAngle() != -1)
	{
		string str = "�Ѽ��㵽��" + std::to_string(curFileConfigModInfoIndex) + "�еĵ�" + std::to_string(dataManagers[curModuleIndex].getCalcuedAngle()) + "��";
		log.INFO_log(str);
	}
	ErrorHandlerFunction();
}


//��ȡ�����ļ����濪�� zsr
void SonarSimulationUI::OnActionexcel()
{
	log.DEBUG_log("����������ļ���ť");
	////�������ݵ�tablewidget��

	//ExcelEngine excels(QObject::tr("c:\\Import.xls"));
	//excels.Open();
	//excels.ReadDataToTable(ui_table.tableWidget); //���뵽widget��
	//excels.Close();

	//ʵ������ʱ��
	/*if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����δ����!"), QMessageBox::Ok);
		return;
	}*/

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ڼ��㣡"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONF)
	{
		if (QMessageBox::No == QMessageBox::question(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("ģ���Ѿ����أ��Ƿ��������ã�"), QMessageBox::Yes, QMessageBox::No))
		{
			return;
		}
		log.WARNING_log("����ļ����ð�ť�������¼���ģ��");
	}



	tablewidget->show();


}

//�ļ�����ģʽ������������Ϣ�ۺ��� ������
void SonarSimulationUI::OnLoadFileConfigModInfo(FileConfigModInfo* fileConfigModInfo, int maxNum, QString excelName)
{
	curConfigExcelName.clear();

	//�õ�����excel�ļ�������
	int idx = excelName.lastIndexOf("/");
	curConfigExcelName = excelName.right(excelName.size() - idx - 1);
	// ��ȡ��ǰʱ�䣬���淶��ʾ
	curConfigExcelName = curConfigExcelName + QString::fromStdString(log.getCurTimeString());

	dataManagers[curModuleIndex].setIsContinueRuning(false);
	dataManagers[curModuleIndex].isStop = false;
	fileConfigModInfos.clear();
	curFileConfigModInfoIndex = 0;

	ui.progressBar->reset();
	//ui.progressBar->setValue(0);   //���ý�������ֵΪ0
	ui.progressBar->setRange(0, maxNum * 100);
	ui.progressBar->setVisible(true);
	ui.progressBar->setFormat(QString::fromLocal8Bit("�ļ�����ģʽ�������У�%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));


	for (int i = 0; i < maxNum; i++)
	{
		fileConfigModInfos.push_back(fileConfigModInfo[i]);
		emit updateProgressValue(0, i);
	}

	if (checkResultDeskInfo() == false)
		return;

	dataManagers[curModuleIndex].SetConfigType(FILECONFIGMOD);
	dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
	dataManagers[curModuleIndex].SetCalcType(fileConfigModInfos[curFileConfigModInfoIndex].cal_mood);	//ɨ��0��false  ɨƵ1��ture
	dataManagers[curModuleIndex].setCalcuedAngle(-1);

	log.INFO_log("�ɹ������ļ�������Ϣ");

	OnActionOpen();
}

//�жϽ���ļ���С�Ƿ�ᳬ����ǰ����·���Ĵ��̿ռ�
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
		log.ERROR_log("�����̿ռ����");
		return false;
	}
	FreeBytesAvailabel /= (1024 * 1024 * 1024);

	if (tmpDesk > FreeBytesAvailabel)
	{
		log.ERROR_log("���̿ռ䲻�㣬�޷�������ñ��ڵ����н��");
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���̿ռ䲻�㣬�޷�������ñ��ڵ����н����"), QMessageBox::Ok);
		return false;
	}
	return true;
}

//�򿪰�ť�ۺ���
void SonarSimulationUI::OnActionOpen()
{
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		log.DEBUG_log("����򿪰�ť");
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		log.DEBUG_log("���ú���OnActionOpen()");

	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����δ����!"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ڼ��㣡"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONF)
	{
		if (QMessageBox::No == QMessageBox::question(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("ģ���Ѿ����أ��Ƿ����¼��أ�"), QMessageBox::Yes, QMessageBox::No))
		{
			return;
		}
		log.WARNING_log("���¼���ģ��");
	}

	dataManagers[curModuleIndex].Reset();

	//���ļ�
	QString filename;
	//****�ļ�·������ת������Ȼ����·������س��� �ж�ConfigMod
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
			QMessageBox::warning(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("������Ŀ������ϣ������¼��������ļ���"), QMessageBox::Ok);
			log.INFO_log("������Ŀ�������");
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
		QMessageBox::information(this, "error", QString::fromLocal8Bit("�ļ���ʧ�ܣ������´򿪣�"), QMessageBox::Ok);
		log.ERROR_log("ģ���ļ���ʧ��");
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



	//����ģ�ͣ���ȡģ�͵����ݴ�С

	Triangle *p_Face = (Triangle*)(&dataManagers[curModuleIndex].GetModule().getFacesWK()[0]);
	Element *p_Node = (Element*)&dataManagers[curModuleIndex].GetModule().getNodesWK()[0];
	int faceDataLength = sizeof(Triangle)*dataManagers[curModuleIndex].GetModule().getFaceNum();
	int nodeDataLength = sizeof(Element)*dataManagers[curModuleIndex].GetModule().getNodeNum();

	//����Ԫ�������ø�calcthread ����
	calcthread->SetTriangleLength(dataManagers[curModuleIndex].GetModule().getFaceNum());


	//BuildXForTimeWave(ui.lineEdit_SamplingRate->text().toInt() * 2);

	//����modelInfo textBrowser ����

	{
		QString str;
		str = QString::fromLocal8Bit("��Ԫ������") + QString::number(dataManagers[curModuleIndex].GetModule().getFaceNum());
		emit(updateModelInfo(str));
		str.clear();
		str = QString::fromLocal8Bit("�ڵ�������") + QString::number(dataManagers[curModuleIndex].GetModule().getNodeNum());
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
	//OpenGLWidget����ģ��
	ui.openGLWidget->LoadModel(dataManagers[curModuleIndex].GetModule());

	emit printInfo(QString::fromLocal8Bit("ģ�ͼ��سɹ���"));
	emit printInfo(QString::fromLocal8Bit("��ʼ���������������..."));
	log.INFO_log("ģ�ͼ��سɹ�����ʼ���������������");

	//���÷��͵�����֡ Triangles
	strcpy(frame.command, "Triangles");
	frame.length = faceDataLength;

	int l = 0;
	int frameSize = sizeof(Frame);
	//��������֡
	l = sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	if (l != frameSize)
	{
		//QMessageBox::critical(this, "error", "Triangles����֡���ʹ���", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Triangles����֡���ʹ���"));
		log.ERROR_log("Triangles����֡���ʹ���");
		ErrorHandlerFunction();
		return;
	}

	//��������
	l = sockets[curModuleIndex]->write((char*)p_Face, faceDataLength);
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	if (l != faceDataLength)
	{
		//QMessageBox::critical(this, "error", "Triangles���ݷ��ʹ���", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Triangles���ݷ��ʹ���"));
		log.ERROR_log("Triangles���ݷ��ʹ���");
		ErrorHandlerFunction();
		return;
	}

	//���÷��͵�����֡ Element
	strcpy(frame.command, "Elements");
	frame.length = nodeDataLength;
	//��������֡
	l = sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	if (l != frameSize)
	{
		//QMessageBox::critical(this, "error", "Elements����֡���ʹ���", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Elements����֡���ʹ���"));
		log.ERROR_log("Elements����֡���ʹ���");
		ErrorHandlerFunction();
		return;
	}
	
	//��������
	l = sockets[curModuleIndex]->write((char*)p_Node, nodeDataLength);
	sockets[curModuleIndex]->waitForBytesWritten(3000);
	sockets[curModuleIndex]->flush();
	qDebug("State:%d\n", sockets[curModuleIndex]->state());
	//Sleep(1000);
	if (l != nodeDataLength)
	{
		//QMessageBox::critical(this, "error", "Triangles���ݷ��ʹ���", QMessageBox::Ok);
		emit printInfo(QString::fromLocal8Bit("Triangles���ݷ��ʹ���"));
		log.ERROR_log("Triangles���ݷ��ʹ���");
		ErrorHandlerFunction();
		return;
	}

	if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		//�������ֹͣ��ť�����˳�
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

//���а�ť�ۺ���
void SonarSimulationUI::OnActionRun()
{
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		log.DEBUG_log("������а�ť");
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		log.DEBUG_log("���ú���OnActionRun()");

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ڼ��㣡"), QMessageBox::Ok);
		return;
	}
	if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����δ����"), QMessageBox::Ok);
		return;
	}
	if (dataManagers[curModuleIndex].GetStatus() == WAITFORLOAD)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("ģ��δ����"), QMessageBox::Ok);
		return;
	}

	if (dataManagers[curModuleIndex].IsCalcEmpty() == false)
	{
		dataManagers[curModuleIndex].ClearCalcResult();
	}

	
	//��ȡ��������
	ConfigStruct &configStruct = dataManagers[curModuleIndex].GetConfig();

	//����Ҫ�������У�����ϴμ���ĽǶȿ�ʼ����
	if (dataManagers[curModuleIndex].IsContinueRuning())
	{
		if (dataManagers[curModuleIndex].getCalcuedAngle() != -1)
		{
			if (dataManagers[curModuleIndex].getCalcuedAngle() < configStruct.end_alpha)
			{
				configStruct.start_alpha = dataManagers[curModuleIndex].getCalcuedAngle() + 1;
				emit printInfo(QString::fromLocal8Bit("��") + QString::number(dataManagers[curModuleIndex].getCalcuedAngle(), 10) + QString::fromLocal8Bit("�ȿ�ʼ��������"));
				log.INFO_log("��" + std::to_string(dataManagers[curModuleIndex].getCalcuedAngle()) + "�ȿ�ʼ��������");
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
		//2021.8.30������  ����ʱ���������
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
		//���û��ֽ������   * sampling_width
		calcthread->SetTimeIntegrationResultsLength(int(ui.lineEdit_SamplingRate->text().toInt() * ui.lineEdit_SamplingWidth->text().toDouble()));
	}
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
	{
		//����Ϊ��ͣ��������У���ֱ�Ӷ�ȡconfig struct
		if(dataManagers[curModuleIndex].IsContinueRuning() == false)
			configStruct = fileConfigModInfos[curFileConfigModInfoIndex].config;

		//���û��ֽ������   * sampling_width
		calcthread->SetTimeIntegrationResultsLength(int(configStruct.sampling_rate * configStruct.sampling_width));
	}


	if (configStruct.card_num <= 0)
	{
		if(dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
			QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("δѡ���κ�һ����㿨��"), QMessageBox::Ok);

		log.ERROR_log("δѡ���κ�һ����㿨");
		ErrorHandlerFunction();
		return;
	}

	if (deviceInfoVector.size() == 0)
	{
		log.ERROR_log("������δ��⵽���㿨");
		ErrorHandlerFunction();
		return;
	}
	if (configStruct.card_num > deviceInfoVector.size())
	{
		log.ERROR_log("��ѡ�ļ��㿨�����������������㿨����");
		ErrorHandlerFunction();
		return;
	}

	//�����ܵĺ�������
	int coresNum = 0;
	for (int i = 0; i < configStruct.card_num; i++)
	{
		coresNum += deviceInfoVector[i].coresPreMutiprocess*deviceInfoVector[i].mutiprocessCount;
	}
	ui.lineEdit_coresNum->setText(QString::number(coresNum));


	if (configStruct.start_alpha > configStruct.end_alpha)
	{
		if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
			QMessageBox::information(this, "error", QString::fromLocal8Bit("��ʼ���Ƕȴ�����ֹ���Ƕȣ�"), QMessageBox::Ok);
		log.ERROR_log("������Ϣ������ʼ���Ƕȴ�����ֹ���Ƕ�");
		ErrorHandlerFunction();
		return;
	}
	if (configStruct.start_frequency > configStruct.end_frequency)
	{
		if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
			QMessageBox::information(this, "error", QString::fromLocal8Bit("��ʼ����Ƶ�ʴ�����ֹ����Ƶ�ʣ�"), QMessageBox::Ok);
		log.ERROR_log("��ʼ����Ƶ�ʴ�����ֹ����Ƶ�ʣ�");
		ErrorHandlerFunction();
		return;
	}

	//���ò�����ʾ�ĳ�ʼֵ

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

	// ����������Ϳ�ʼ�����ź�
	strcpy(frame.command, "Configuration");
	frame.length = sizeof(configStruct);
	memcpy(frame.data, &configStruct, sizeof(configStruct));
	sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));

	emit printInfo(QString::fromLocal8Bit("������ɣ���ʼ����"));
	log.DEBUG_log("������ɣ���ʼ����");

	ui.lineEdit_GPUnum->setText(QString::number(configStruct.card_num));
	//���ڼ����־��������Ϊtrue                                                                                                                                                                          
	//isRunning = true;


	dataManagers[curModuleIndex].SetStatus(RUNNING);

}

//���水ť�ۺ���
void SonarSimulationUI::OnActionSave()
{
	log.DEBUG_log("������水ť");
	if (dataManagers[curModuleIndex].IsCalcEmpty())
	{
		QMessageBox::information(this, "warning", QString::fromLocal8Bit("û�����ݴ洢"), QMessageBox::Ok);
		return;
	}

	QString filename = QFileDialog::getSaveFileName(NULL, "open", ".", "*.txt");
	if (filename.isEmpty() == true)
		return;
	QTextCodec *textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "bug", QString::fromLocal8Bit("�ļ���ʧ�ܣ������´򿪣�"), QMessageBox::Ok);
		return;
	}

	if (!excelHelper.newExcel(filename))
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("MiscoSoft Office COM open failed��"), QMessageBox::Ok);
		return;
	}


	QList<QVariant> rowdata;

	QList<QVariant> headline;
	headline.append(QString::fromLocal8Bit("�Ƕ�"));
	headline.append(QString::fromLocal8Bit("Ƶ��"));
	headline.append(QString::fromLocal8Bit("TSֵ"));
	headline.append(QString::fromLocal8Bit("����ʱ��"));
	headline.append(QString::fromLocal8Bit("��������"));
	headline.append(QString::fromLocal8Bit("��������"));
	headline.append(QString::fromLocal8Bit("��"));
	headline.append(QString::fromLocal8Bit("��"));

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

	QMessageBox::information(this, "Save", QString::fromLocal8Bit("����ɹ�"), QMessageBox::Ok);
}

//ֹͣ��ť�ۺ���
void SonarSimulationUI::OnActionStop()
{
	if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		log.DEBUG_log("���ֹͣ��ť");
	else if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
		log.DEBUG_log("���ú���OnActionStop()");

	if (dataManagers[curModuleIndex].GetStatus() == RUNNING)
	{
		//curRepeatNum != 0˵���Ǽ�������г��ִ�������ô˺���
		if (curRepeatNum != 0)
		{
			strcpy(frame.command, "Stop");
			frame.length = 0;
			sockets[curModuleIndex]->write((char*)&frame, sizeof(frame));

			emit printInfo(QString::fromLocal8Bit("������ֹ��"));
			log.WARNING_log("������ֹ");

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
		//else if (QMessageBox::Yes == QMessageBox::question(this, "warning", QString::fromLocal8Bit("���ڼ����У��Ƿ�ֹͣ��"), QMessageBox::Yes, QMessageBox::No))
		{
			dataManagers[curModuleIndex].isStop = true;
			dataManagers[curModuleIndex].setIsContinueRuning(true);

			emit printInfo(QString::fromLocal8Bit("������ֹ��"));
			log.WARNING_log("������ֹ");

			//���ж�����ȷ���û����ȷ����ֹ�������״̬
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

//�½���ť�ۺ���
void SonarSimulationUI::OnActionNew()
{
	log.DEBUG_log("����½���ť");
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

	comboBoxModules->addItem(QString::fromLocal8Bit("ģ�� %1").arg(dataManagers.size()));

	curModuleIndex = comboBoxModules->count() - 1;
	comboBoxModules->setCurrentIndex(curModuleIndex);
	calcthread->SetCurrentModule(curModuleIndex);


	InitAll();
}

//���������ʾ���أ�
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

//���������Ƹ�������ʾ��رյĿ���
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

//�豸ѡ��ť�Ĳۺ���
void SonarSimulationUI::OnPushbuttonDeviceSelect()
{
	log.DEBUG_log("����豸ѡ��ť");
	ConfigStruct &configStruct = dataManagers[curModuleIndex].GetConfig();
	int *selectDevice = configStruct.select_device_list;
	DeviceSelect dialog(this, selectDevice, deviceCount, &configStruct.card_num);
	while (true)
	{
		if (QDialog::Accepted == dialog.exec())
		{
			if (configStruct.card_num <= 0)
			{
				QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("δѡ���κ�һ����㿨��"), QMessageBox::Ok);
				log.WARNING_log("δѡ���κ�һ����㿨");
				continue;
			}

			emit printInfo(QString::fromLocal8Bit("�����ÿ���"));
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

//����socket���ݲۺ���
void SonarSimulationUI::OnSocketReadyRead()
{
	//��ģ�ͽ���ʱ���ж��socket���ӣ���δ�����Ҫ��ȷ�����յ����������ĸ�ģ�͵�
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
				//���յ�������ֱ��push��calcthread ��ȥ����
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

//����ɨ��RadioButton�Ĳۺ���
void SonarSimulationUI::OnRadioButtonScanAngle()
{

	if (dataManagers[curModuleIndex].GetCalcType() == Angle)
	{
		return;
	}
	dataManagers[curModuleIndex].SetCalcType(Angle);
	ui.widget_wave->Init(waveAngle);
	ui.widget_wave_2->Init(waveTime);//
	//��Ƶ�ʵ�ֵͳһ�����ò��ɸ��ģ����������
	ui.lineEdit_endFrequency->setText(ui.lineEdit_startFrequency->text());
	ui.lineEdit_endFrequency->setEnabled(false);
	ui.lineEdit_endAlpha->setEnabled(true);

}

//����ɨƵRadioButton�Ĳۺ���
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

//�����Զ����÷���ϵ��RadioButton�Ĳۺ��� ������
void SonarSimulationUI::OnRadioButtonConfigReflectCoeffAuto()
{
	ui.lineEdit_ReflectCoeff->setEnabled(false);
}

//�����ֶ����÷���ϵ��RadioButton�Ĳۺ��� ������
void SonarSimulationUI::OnRadioButtonConfigReflectCoeffText()
{
	ui.lineEdit_ReflectCoeff->setEnabled(true);
}

//�ر��¼��Ĵ�����
void SonarSimulationUI::closeEvent(QCloseEvent *event)
{
	log.DEBUG_log("����˳���ť");
	if (dataManagers.size() == 0)
	{
		event->accept();
		tablewidget->close();
		datasave->close();
		log.INFO_log("�˳�");
		return;
	}
	if (dataManagers[curModuleIndex].GetStatus() != WAITFORCONNECT)
	{
		if (QMessageBox::No == QMessageBox::question(this, QString::fromLocal8Bit("�˳�����"), QString::fromLocal8Bit("�Ƿ�Ͽ����粢�˳���"), QMessageBox::Yes | QMessageBox::No))
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
			log.INFO_log("�˳�");
			return;
		}
	}
	else
	{
		event->accept();
		tablewidget->close();
		datasave->close();
		log.INFO_log("�˳�");
		return;
	}
}

//����ش��������ۺ���
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

		//�øñ�����־��ǰ�Ƿ�Ϊ��ǰ�����е�һ�νӵ����
		int firstResultSignal = 0;

		if (dataManagers[curModuleIndex].GetCalcType() == Angle)
		{
			emit(printInfo(str1 + QString::fromLocal8Bit("��; ����ʱ�䣺") + str3 + QString::fromLocal8Bit("; ���ֽ����") + str5));
			//���λ�������
			ui.widget_wave->PushResult(result.angle, result.TS);


			if (dataManagers[curModuleIndex].GetConfig().start_alpha == result.angle)
				firstResultSignal = 1;
			if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
				progressValue = (curFileConfigModInfoIndex + result.angle / (fileConfigModInfos[curFileConfigModInfoIndex].config.end_alpha - fileConfigModInfos[curFileConfigModInfoIndex].config.start_alpha)) * 100;
		}
		else if (dataManagers[curModuleIndex].GetCalcType() == Frequency)
		{
			emit(printInfo(QString::number(result.freq, 'f', 1) + QString::fromLocal8Bit("KHz:") + QString::fromLocal8Bit(" ����ʱ�䣺") + str3 + QString::fromLocal8Bit("; ���ֽ����") + str5));
			//���λ�������
			ui.widget_wave->PushResult(result.freq, result.TS);

			if (dataManagers[curModuleIndex].GetConfig().start_frequency == result.freq)
				firstResultSignal = 1;
			if (dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD)
				progressValue = (curFileConfigModInfoIndex + result.angle / (fileConfigModInfos[curFileConfigModInfoIndex].config.end_frequency - fileConfigModInfos[curFileConfigModInfoIndex].config.start_frequency)) * 100;
		}
		emit(printInfo(QString::fromLocal8Bit("     ����׾����߳�����") + QString::number(result.squarenum) +
			QString::fromLocal8Bit("; ����׷���߳�����") + QString::number(result.raysnum) +
			QString::fromLocal8Bit("; �����߳�����") + QString::number(result.squarenum)));
		ui.lineEdit_squareNum->setText(QString::number(result.squarenum));
		ui.lineEdit_rayNum->setText(QString::number(result.raysnum));
		ui.lineEdit_calResult->setText(QString::number(result.TS, 'g', 3));
		ui.lineEdit_calTime->setText(QString::number(result.calc_time, 'g', 4));
		ui.lineEdit_KDNodeNum->setText(QString::number(calcthread->GetKDTreeInfo().length));
		ui.lineEdit_squareSize->setText(QString::number(result.width) + "*" + QString::number(result.height));

		ui.openGLWidget->SetCurrentAngle(result.angle);

		//����ʱ����ֽ���ļ��� ������
		if (CreatTimeIntegrationResultsFolder(result.angle, result.freq, dataManagers[curModuleIndex].GetCalcType()) == 0)
		{
			//TS���д��txt
			if (SaveTSResultsFile(result.TS) == -1)
				return;

			if (firstResultSignal == 1 && dataManagers[curModuleIndex].GetConfigType() == FILECONFIGMOD && dataManagers[curModuleIndex].IsContinueRuning() == false)
			{
				//һ��������Ϣ������������ݿ�д��Ϣ
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

//��������ۺ���
void SonarSimulationUI::OnCalcOver(int i)
{
	dataManagers[i].SetStatus(WAITFORCONF);
	dataManagers[i].setCalcuedAngle(-1);
	dataManagers[i].setIsContinueRuning(false);
	if (i == curModuleIndex)
	{
		emit(printInfo(QString::fromLocal8Bit("������ɣ�")));
		log.INFO_log("�������");

		if (dataManagers[i].GetConfigType() == FILECONFIGMOD)
		{
			dataManagers[i].SetStatus(WAITFORLOAD);

			char command[50];
			sprintf(command, "��%d��������Ϣ�Ѽ������", curFileConfigModInfoIndex + 1);
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

//���յ�GPU��Ϣ�Ĵ�����
void SonarSimulationUI::OnRecvGPUWatch(GPUWatchStruct info)
{
	ui.widget_GPUwatch->SetData(info);
}

//���յ��豸��Ϣ�Ĵ�����
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

	emit printInfo(QString::fromLocal8Bit("�豸") + QString::number(info.deviceID) + QString::fromLocal8Bit("�����ƣ�") + QString(info.deviceName)
		+ QString::fromLocal8Bit("  ��������������") + QString::number(info.mutiprocessCount) + QString::fromLocal8Bit("  ������/SM��")
		+ QString::number(info.coresPreMutiprocess) + QString::fromLocal8Bit("  �ܺ�������") + QString::number(info.mutiprocessCount*info.coresPreMutiprocess));

}

//���յ�KDTree����
void SonarSimulationUI::OnLoadKDTree(KD_Node_V* kdtree, int length, int i)
{
	dataManagers[i].SetBoxData(kdtree, length);
	if (i == curModuleIndex)
		ui.openGLWidget->OnLoadKDTree(dataManagers[curModuleIndex].GetBoxData());
}

//���������л�ģ�͵Ĳۺ���
void SonarSimulationUI::OnComboBoxModules(int index)
{
	curModuleIndex = index;
	calcthread->SetCurrentModule(curModuleIndex);
	InitAll();

}

//���յ�ÿ��������Ƭ���ֽ���󣬸���
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

//���յ�ÿ��ʱ����ֽ���󣬸���
void SonarSimulationUI::OnUpdateTimeIntegrationResults(float *data, int length, int i)
{
	dataManagers[i].SetTimeIntegrationResults(data);
	dataManagers[i].SetTimeIntegrationResultsNum(length);
	if (i == curModuleIndex)
	{
		BuildXYForTimeWave(data, length);
		ui.widget_wave_2->SetTimeData(XX, YY, maxYY, minYY);
		//ʱ����ֽ��д��txt
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
		
		//�ų����������븺����ĵ� 20220408
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

//����ʱ����ֽ���ļ��� ������
//����ֵ�� �ɹ�0  ʧ��-1
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
			log.ERROR_log("ʱ����ֽ���ļ�����ʧ��");
			ErrorHandlerFunction();
			return -1;
		}
	}

	//д�ļ���·��
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
				log.ERROR_log("ʱ����ֽ���ļ�����ʧ��");
				ErrorHandlerFunction();
				return -1;
			}
		}
		sprintf(folderPath, "%s\\FileConfigResult_%d", folderPath, fileConfigModInfos[curFileConfigModInfoIndex].index);
	}
	else if (dataManagers[curModuleIndex].GetConfigType() == CLIENTCONFIGMOD)
		sprintf(folderPath, ".\\data\\ClientConfigResult");

	//�����ļ���
	if (access(folderPath, 0))
	{
		if (mkdir(folderPath) == -1)
		{
			log.ERROR_log("ʱ����ֽ���ļ�����ʧ��");
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
			log.ERROR_log("ʱ����ֽ���ļ�·������");
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

//ʱ����ֽ��д��txt ������
//����ֵ�� �ɹ�0  ʧ��-1
int SonarSimulationUI::SaveTimeIntegrationResultsFile()
{
	FILE* file;
	file = fopen(curTimeIntegrationResultsPath.toLocal8Bit(), "w");
	if (file == NULL)
	{
		log.ERROR_log("ʱ����ֽ���ļ���������");
		ErrorHandlerFunction();
		return -1;
	}
	for (int i = 0; i < dataManagers[curModuleIndex].GetTimeIntegrationResultsNum(); i++)
	{
		if (fprintf(file, "%e\n", dataManagers[curModuleIndex].GetTimeIntegrationResults()[i]) < 0)
		{
			emit(printInfo(QString::fromLocal8Bit("ʱ����ֽ��д�ļ�����")));
			log.ERROR_log("ʱ����ֽ��д�ļ�����");
			ErrorHandlerFunction();
			return -1;
		}
	}

	fclose(file);

	//curRepeatNum��0ʱ��˵�����������ʱCalcuedAngle��¼����ʱ�̵ļ�����ȣ��˴���Ӧ����
	if (curRepeatNum == 0)
		dataManagers[curModuleIndex].updateCalcuedAngle();

	return 0;
}

//TS���д��txt ������
//����ֵ�� �ɹ�0  ʧ��-1
int SonarSimulationUI::SaveTSResultsFile(float TS)
{
	FILE* file;
	QString TSPath = curResultFolderPath + "//TS.txt";
	file = fopen(TSPath.toLocal8Bit(), "a");
	if (file == NULL)
	{
		log.ERROR_log("TS����ļ���������");
		ErrorHandlerFunction();
		return -1;
	}
	if (fprintf(file, "%e\n", TS) < 0)
	{
		emit(printInfo(QString::fromLocal8Bit("TS���д�ļ�����")));
		log.ERROR_log("TS���д�ļ�����");
		ErrorHandlerFunction();
		return -1;
	}
	fclose(file);
	return 0;
}

//�����������ݿ�洢�������ۺ���  2021.12.20 �¾� 
void SonarSimulationUI::OnActionDataBase()
{
	log.DEBUG_log("������ݿ����ť");
	/*if (dataManagers[curModuleIndex].IsCalcEmpty())
	{
		QMessageBox::information(this, "warning", QString::fromLocal8Bit("û�����ݴ洢"), QMessageBox::Ok);
		return;
	}*/
	datasave->show();
}

void SonarSimulationUI::OnGetRunningStatusSig(int &status)
{
	status = dataManagers[curModuleIndex].GetStatus();
}

//�������� ������
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
			str = "���ִ��󣬿�ʼ��" + to_string(curRepeatNum) + "������";
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
			emit(printInfo(QString::fromLocal8Bit("���ִ����������������Σ����������־")));
			log.ERROR_log("���ִ�����������������");
			if (dataManagers[curModuleIndex].getCalcuedAngle() != -1)
			{
				str = "�Ѽ��㵽��" + std::to_string(curFileConfigModInfoIndex + 1) + "�еĵ�" + std::to_string(dataManagers[curModuleIndex].getCalcuedAngle()) + "��";
				log.INFO_log(str);
				//OnActionOpen();
				return;
			}
			else if (curFileConfigModInfoIndex < fileConfigModInfos.size() - 1)
			{
				curFileConfigModInfoIndex += 1;
				str = "�ӵ�" + to_string(curFileConfigModInfoIndex + 1) + "�п�ʼ����";
				log.WARNING_log(str);
				dataManagers[curModuleIndex].SetStatus(WAITFORLOAD);
				OnActionOpen();
				return;
			}
			else
			{
				QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ִ����������������Σ����������־"), QMessageBox::Ok);
				return;
			}
		}
	}
	else
		return;
}