//#include "QtTablewidget.h"
#include "sonarsimulationui.h"
//#include "excelengine.h"

QtTablewidget::QtTablewidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(u8"��������");
	ui.progressBar->setVisible(false);
	stop_cliked = false;

	QStringList headerString;

	ui.tableWidget->setColumnCount(24);
	ui.tableWidget->setRowCount(0);


	headerString << u8"ִ�н���" << u8"���" << u8"��ʼ��(��) " << u8"��ʼ��(��) " << u8"��ֹ��(��) " << u8"������(m)  " << u8"����ֱ��(*��)" <<
		u8"Ƶ����ʼƵ��(KHz)" << u8"Ƶ����ֹƵ��(KHz)" << u8"Զ������(m) " << u8"������(Hz)  " << u8"��������(s)" << u8"������(ms)" <<
		u8"��ʼƵ��(Hz)" << u8"����Ƶ��(Hz)" << u8"����ٶ�(m/s)" << u8"���ֵ���ϵ��" << u8"Ŀ���ٶ�1" << u8"Ŀ���ٶ�2" <<
		u8"����ϵ��" << u8"ģ�͵�ַ" << u8"���㿨ѡ��" << u8"����ϵ��ѡ��" << u8"����ģʽѡ��";
	ui.tableWidget->setHorizontalHeaderLabels(headerString);
	ui.tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);//�̶��и�
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//����Ӧ�п�
	//show_excel();
	ui.tableWidget->setAlternatingRowColors(true);//���б�ɫ  
}

QtTablewidget::~QtTablewidget()
{
}

std::vector<FileConfigModInfo>* QtTablewidget:: getFileConfigModInfosPtr()
{
	return &fileConfigModInfos;
}

ErrorLog* QtTablewidget::getErrorLogPtr()
{
	return &log;
}

void QtTablewidget::show_excel()
{
	//�������ݵ�tablewidget��

	//ExcelEngine excels(QObject::tr("D:\\document\\705\\GPU-UI-1207-unofficial\\SonarSimulationUI_addpara\\SonarSimulationUI\\model\\������1.xlsx"));
	//excels.Open();
	//excels.ReadDataToTable(ui.tableWidget); //���뵽widget��
	//excels.Close();
}

//��ȡ�����ļ� zsr
void QtTablewidget::read_excel()
{
	//if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	//{
	//	QMessageBox::critical(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����δ����!"), QMessageBox::Ok);
	//	return;
	//}
	//���ļ�
	filename_now = "";
	ui.tableWidget->setColumnCount(24);
	ui.tableWidget->setRowCount(0);
	m_datas.clear();//��ղ���������
	p.clear();//��ս�����������

	//****�ļ�·������ת������Ȼ����·������س���
	filename = QFileDialog::getOpenFileName(NULL, "open", ".", QStringLiteral("EXCEL(*.xlsx)"));
	if (filename.isEmpty() == true)
		return;
	QTextCodec* textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("excel�ļ���ʧ�ܣ������´򿪣�"), QMessageBox::Ok);
		log.ERROR_log("exel�����ļ���ʧ��");
		return;
	}
	std::string filename_stdstr = textCodec->fromUnicode(filename).data();

	ui.progressBar->setValue(0);   //���ý�������ֵΪ0
	ui.progressBar->show();    //��������Ҫ��ui�ļ��мӸ�progressBar�ؼ�

	ui.progressBar->setFormat(QString::fromLocal8Bit("���ڶ��ļ���%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));

	QAxObject* excel = new QAxObject("this");//1800ms

	excel->setControl("Excel.Application");//����Excel�ؼ�
	//QMessageBox::StandardButton reply;
	//reply = QMessageBox::question(this, tr("question"), QString::fromLocal8Bit("��excel�ļ���"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	//if (reply == QMessageBox::Yes)
	//{
	//	excel->dynamicCall("SetVisible(bool Visible)", true);//false����ʾ����
	//	excel->setProperty("DisplAlerts", true);//false����ʾ����

	//}
	//else if (reply == QMessageBox::No)
	//{
		excel->dynamicCall("SetVisible(bool Visible)", false);//false����ʾ����
		excel->setProperty("DisplAlerts", false);//false����ʾ����
	//}
	//else
		//questionLabel->setText(tr("Cancel"));

	//excel->setProperty("Value", d);//����ֵ
	//excel->setProperty("RowHeight", HIGHT + 9);//�����и�
	//excel->setProperty("ColumnWidth", WIDE-4);//���õ�Ԫ���п�


	//QList<QList<QVariant>> m_datas;

	ui.progressBar->setValue(10);

	QAxObject* workbooks = NULL;
	QAxObject* workbook = NULL;

	workbooks = excel->querySubObject("WorkBooks");
	//workbook = workbooks->querySubObject("Open(QString, QVariant)", QString(tr("d:\\ExcelTest.xlsx")));
	workbook = workbooks->querySubObject("Open(QString, QVariant)", filename);
	QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1);//�򿪵�һ��sheet


	QVariant var;
	if (worksheet != NULL && !worksheet->isNull())
	{
		QAxObject* usedRange = worksheet->querySubObject("UsedRange");
		if (NULL == usedRange || usedRange->isNull())
		{
			;//return var;
		}
		if (!(NULL == usedRange || usedRange->isNull()))
		{
			var = usedRange->dynamicCall("Value");
			delete usedRange;
		}

	}
	//return var;

	castVariant2ListListVariant(var, m_datas);
	ui.progressBar->setValue(30);

	ui.tableWidget->setRowCount(m_datas.size() - 1);
	ui.tableWidget->setColumnCount(m_datas[0].size()+1);//��
	//ui.tableWidget = new QTableWidget(m_datas.size(), m_datas[0].size()); // ������һ��QTableWidget�Ķ��󣬲�������Ϊ10�У�5��
	//tableWidget->setWindowTitle("�����ļ�");
	//tableWidget->resize(m_datas.size(), m_datas[0].size());  //���ñ��
	//ui.tableWidget->repaint();
	ui.tableWidget->update();
	ui.tableWidget->viewport()->update();

	for (int i = 1; i < m_datas.size(); i++)
	{
		//����ÿ�н�������ʽ
		p.push_back(new QProgressBar(this));
		p[i - 1]->setFixedSize(150, 30);
		p[i - 1]->setMaximum(100);
		p[i - 1]->setMinimum(0);
		p[i - 1]->setFormat(QString::fromLocal8Bit("������ȣ�%p%").arg(QString::number(p[i - 1]->value(), 'f', 1)));
		p[i - 1]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);//���뷽ʽ6��
		p[i - 1]->setValue(0);   //���ý�������ֵΪ0
		//ÿһ����ʾ������
		ui.tableWidget->setCellWidget(i - 1, 0, p[i - 1]);

		//ui.progressBar->setValue((30+(i+1)*(70/ m_datas.size())));
		for (int j = 1; j <= m_datas[0].size(); j++)
		{
			//QString strVal = m_datas.at(i).at(j).toString();
			//����Ԫ������ݷ�����table����
			ui.tableWidget->setItem(i - 1, j, new QTableWidgetItem(m_datas[i][j-1].toString()));
			//ui.tableWidget->item(i, j)->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		}
		//ui.tableWidget->repaint();
		ui.tableWidget->update();
		ui.tableWidget->viewport()->update();
		ui.progressBar->setValue((30 + (i + 1) * (70 / m_datas.size())));
	}
	ui.progressBar->setValue(100);
	QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("�����ļ���ȡ��ɣ�"), QMessageBox::Ok);
	ui.progressBar->setValue(0);
	ui.progressBar->hide();
	ui.progressBar->setVisible(false);
	workbook->dynamicCall("Close()");                                                   //�رչ�����
	excel->dynamicCall("Quit()");                                                       //�ر�excel
	delete excel;
	excel = NULL;
	//filename_now = filename;
}

void QtTablewidget::parameter_pass()
{
	if (saveFlag == false)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("δ����!"), QMessageBox::Ok);
		return;
	}
	int runningStatus;
	emit getRunningStatusSig(runningStatus);

	if (runningStatus == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����δ���ӣ�"), QMessageBox::Ok);
		return;
	}

	if (runningStatus == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ڼ��㣡"), QMessageBox::Ok);
		return;
	}


	stop_cliked = false;//��ͣ��ʼ��
	//ui.stopBtn->setText(QString::fromLocal8Bit("��ͣ"));

	if (filename_now != NULL)
	{
		parameter_updata();//
	}
	else
	{
		filename_now = filename;
	}

	ui.progressBar->setValue(0);   //���ý�������ֵΪ0
	ui.progressBar->show();    //��������Ҫ��ui�ļ��мӸ�progressBar�ؼ�
	ui.progressBar->setFormat(QString::fromLocal8Bit("���ڴ��ݲ�����%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));
	fileConfigModInfos.clear();//ÿ�β�������ǰ����ڴ�ռ�  2022.01.25
	FileConfigModInfo fileConfigModInfo;
	for (int i = 1; i < m_datas.size(); i++)
	{
		//for (int j = 0; j < m_datas[i].size(); j++)
		//{
		fileConfigModInfo.index = m_datas[i][0].toInt();
		fileConfigModInfo.config.start_beta = m_datas[i][1].toFloat();
		fileConfigModInfo.config.start_alpha = m_datas[i][2].toFloat();
		fileConfigModInfo.config.end_alpha = m_datas[i][3].toFloat();
		fileConfigModInfo.config.wave_length = m_datas[i][4].toFloat();
		fileConfigModInfo.config.pipe_size = m_datas[i][5].toFloat();
		fileConfigModInfo.config.start_frequency = m_datas[i][6].toFloat();
		fileConfigModInfo.config.end_frequency = m_datas[i][7].toFloat();
		fileConfigModInfo.config.far_distance = m_datas[i][8].toFloat();
		fileConfigModInfo.config.sampling_rate = m_datas[i][9].toFloat();
		fileConfigModInfo.config.sampling_width = m_datas[i][10].toFloat();
		fileConfigModInfo.config.tao = m_datas[i][11].toFloat();
		fileConfigModInfo.config.time_start_frequency = m_datas[i][12].toFloat();
		fileConfigModInfo.config.time_end_frequency = m_datas[i][13].toFloat();
		fileConfigModInfo.config.relative_velocity = m_datas[i][14].toFloat();
		fileConfigModInfo.config.integral_gain = m_datas[i][15].toFloat();
		fileConfigModInfo.config.velocity1 = m_datas[i][16].toFloat();
		fileConfigModInfo.config.velocity2 = m_datas[i][17].toFloat();
		fileConfigModInfo.config.reflect_coeff = m_datas[i][18].toFloat();
		QString fil = m_datas[i][19].toString();
		//fileConfigModInfo.modelPath[] = m_datas[i][18];
		memset(fileConfigModInfo.modelPath, 0, 200);
		int RealLen = (fil.toLocal8Bit().length() < 200 ? fil.toLocal8Bit().length() : 200);
		if (RealLen != fil.toLocal8Bit().length())
		{
			QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("ģ���ļ�·��������"), QMessageBox::Ok);
			log.ERROR_log("(excel����ҳ��)ģ���ļ�·������");
			return;
		}
		memcpy(fileConfigModInfo.modelPath, fil.toLocal8Bit().data(), RealLen);
		fileConfigModInfo.modelPath[200 - 1] = 0;
		fileConfigModInfo.config.card_num = m_datas[i][20].toInt();
		for (int idx = 0; idx < fileConfigModInfo.config.card_num; idx++)
		{
			fileConfigModInfo.config.select_device_list[idx] = idx;
		}

		QString temp;
		temp = m_datas[i][21].toString();
		if (temp == "�Զ�")
		{
			fileConfigModInfo.config.reflect_coeff_Auto_flag = true;
		}
		else
		{
			fileConfigModInfo.config.reflect_coeff_Auto_flag = false;
		}
		//if (temp == "ɨƵ")
		//{
		//	fileConfigModInfo.cal_mood = Frequency;
		//}
		//else
		//{
			fileConfigModInfo.cal_mood = Angle;//�̶�Ϊɨ�� 3.24
		//}
		//memcpy(fileConfigModInfo.modelPath, fil.toLocal8Bit().data(), m_datas[i][18].Size);
		fileConfigModInfos.push_back(fileConfigModInfo);
		//qDebug() << m_datas[i][j];
	//}

	//printf("%d\n", m_datas[i]);
	ui.progressBar->setValue((i  * (100 / m_datas.size())));
	}
	//loadFileConfigModInfo(&fileConfigModInfos, (m_datas.size() - 1));
	ui.progressBar->setValue(100);
	ui.progressBar->setValue(0);
	ui.progressBar->hide();
	ui.progressBar->setVisible(false);

	emit(loadFileConfigModInfo(&fileConfigModInfos.front(), m_datas.size()-1 , filename_now));
}

/***************************************
*�������ܣ���QVariantת��ΪQList<QList<QVariant> >
*���룺
*	var:ԭQVariant
*	res:�µ�QList<QList<QVariant> >
*�����
*	void
zsr
***************************************/
void QtTablewidget::castVariant2ListListVariant(const QVariant& var, QList<QList<QVariant> >& res)
{
	QVariantList varRows = var.toList();
	if (varRows.isEmpty())
	{
		return;
	}
	const int rowCount = varRows.size();//����
	QVariantList rowData;
	for (int i = 0; i < rowCount; ++i)
	{
		rowData = varRows[i].toList();//��ÿһ�е�ֵ���뵽list��
		res.push_back(rowData);
	}
}
//�����ļ����ĺ�����ڴ��ڲ���
void QtTablewidget::parameter_updata()
{
	//if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	//{
	//	QMessageBox::critical(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����δ����!"), QMessageBox::Ok);
	//	return;
	//}
	//���ļ�
	m_datas.clear();
	ui.progressBar->setValue(0);   //���ý�������ֵΪ0
	ui.progressBar->show();    //��������Ҫ��ui�ļ��мӸ�progressBar�ؼ�
	ui.progressBar->setFormat(QString::fromLocal8Bit("���ڸ����ڴ��ڲ�����%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));
	QAxObject* excel = new QAxObject("this");//1800ms

	excel->setControl("Excel.Application");//����Excel�ؼ�

	ui.progressBar->setValue(10);

	QAxObject* workbooks = NULL;
	QAxObject* workbook = NULL;

	workbooks = excel->querySubObject("WorkBooks");
	//workbook = workbooks->querySubObject("Open(QString, QVariant)", QString(tr("d:\\ExcelTest.xlsx")));
	workbook = workbooks->querySubObject("Open(QString, QVariant)", filename_now);
	QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1);//�򿪵�һ��sheet

	ui.progressBar->setValue(30);

	QVariant var;
	if (worksheet != NULL && !worksheet->isNull())
	{
		QAxObject* usedRange = worksheet->querySubObject("UsedRange");
		if (NULL == usedRange || usedRange->isNull())
		{
			;//return var;
		}
		if (!(NULL == usedRange || usedRange->isNull()))
		{
			var = usedRange->dynamicCall("Value");
			delete usedRange;
		}

	}
	ui.progressBar->setValue(60);
	castVariant2ListListVariant(var, m_datas);
	ui.progressBar->setValue(100);
	QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("����������ɣ�"), QMessageBox::Ok);
	ui.progressBar->setValue(0);
	ui.progressBar->hide();
	ui.progressBar->setVisible(false);
	workbook->dynamicCall("Close()");                                                   //�رչ�����
	excel->dynamicCall("Quit()");                                                       //�ر�excel
	delete excel;
	excel = NULL;

}

//��Qvariantת��ΪQList<QList<QVariant>>
void QtTablewidget::Qvariant2listlistVariant(const QVariant & var, QList<QList<QVariant>> &ret)
{
	QVariantList varrows = var.toList();
	if (varrows.isEmpty())
	{
		return;
	}
	else {
		const int rowcount = varrows.size();//����
		QVariantList rowdata;
		for (int i = 0; i < rowcount; i++)
		{
			rowdata = varrows[i].toList();//��ÿһ�е�ֵ���뵽list��
			ret.push_back(rowdata);
		}
	}
}

//���ٽ�����д��excel��
void QtTablewidget::excel_write()
{
	saveFlag = true;
	ui.progressBar->setValue(0);   //���ý�������ֵΪ0
	ui.progressBar->show();    //��������Ҫ��ui�ļ��мӸ�progressBar�ؼ�
	ui.progressBar->setFormat(QString::fromLocal8Bit("�ļ����Ϊ��%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));
	QString fileName = QFileDialog::getSaveFileName(this, tr("Excle file"), QString("./parameter_list.xlsx"), tr("Excel Files(*.xlsx)"));    //���ñ�����ļ���
	if (fileName != "")
	{
		ui.progressBar->setValue(10);
		QAxObject* excel = new QAxObject;
		if (excel->setControl("Excel.Application"))
		{
			excel->dynamicCall("SetVisible (bool Visible)", false);
			excel->setProperty("DisplayAlerts", false);
			QAxObject* workbooks = excel->querySubObject("WorkBooks");            //��ȡ����������
			workbooks->dynamicCall("Add");                                        //�½�һ��������
			QAxObject* workbook = excel->querySubObject("ActiveWorkBook");        //��ȡ��ǰ������
			QAxObject* worksheet = workbook->querySubObject("Worksheets(int)", 1);
			QAxObject* cell;

			/*���Excel��ͷ����*/
			for (int i = 1; i <= ui.tableWidget->columnCount()-1; i++)
			{
				cell = worksheet->querySubObject("Cells(int,int)", 1, i);
				//cell->setProperty("RowHeight",30);
				//cell->setProperty("ColumnWidth",15);
				cell->dynamicCall("SetValue(const QString&)", ui.tableWidget->horizontalHeaderItem(i)->data(0).toString());
				if (ui.progressBar->value() <= 50)
				{
					ui.progressBar->setValue(10 + i * 5);
				}
			}

			/*��form�б��е��������˱��浽Excel�ļ���*/
			for (int j = 2; j <= ui.tableWidget->rowCount() + 1; j++)
			{
				for (int k = 1; k <= ui.tableWidget->columnCount()-1; k++)
				{
					cell = worksheet->querySubObject("Cells(int,int)", j, k);
					//cell->setProperty("RowHeight",20);
					//cell->setProperty("ColumnWidth",15);
					cell->dynamicCall("SetValue(const QString&)", ui.tableWidget->item(j - 2, k)->text());
				}
				if (ui.progressBar->value() < 80)
				{
					ui.progressBar->setValue(50 + j * 5);
				}
			}

			/*�����ɵ�Excel�ļ����浽ָ��Ŀ¼��*/
			workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName)); //������fileName
			workbook->dynamicCall("Close()");                                                   //�رչ�����
			excel->dynamicCall("Quit()");                                                       //�ر�excel
			delete excel;
			excel = NULL;

			ui.progressBar->setValue(100);
			if (QMessageBox::question(NULL, QString::fromUtf8(u8"���"), QString::fromUtf8(u8"�ļ��Ѿ��������Ƿ����ڴ򿪣�"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			{
				QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(fileName)));
			}
			ui.progressBar->setValue(0);
			ui.progressBar->hide();
		}
		filename_now = fileName;
	}
	ui.progressBar->setVisible(false);
	//parameter_updata();//
}

//��QList<QList<QVariant> > תΪQVariant,���ڿ���д���
void QtTablewidget::castListListVariant2Variant(const QList<QList<QVariant> > &cells, QVariant & res)
{
	QVariantList vars;
	const int rows = cells.size();//��ȡ����
	for (int i = 0; i < rows; ++i)
	{
		vars.append(QVariant(cells[i]));//��list(i)��ӵ�QVariantList�� QVariant(cells[i])ǿ��ת��
	}
	res = QVariant(vars);//ǿ��ת��
}

void QtTablewidget::write_excel_save()
{
	saveFlag = true;
	ui.progressBar->setValue(0);   //���ý�������ֵΪ0
	ui.progressBar->show();    //��������Ҫ��ui�ļ��мӸ�progressBar�ؼ�
	ui.progressBar->setFormat(QString::fromLocal8Bit("�ļ����棺%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));

	//QString fileName = QFileDialog::getSaveFileName(this, tr("Excle file"), QString("./parameter_list.xlsx"), tr("Excel Files(*.xlsx)"));    //���ñ�����ļ���
	if (filename != "")
	{
		ui.progressBar->setValue(10);
		QAxObject* excel = new QAxObject;
		if (excel->setControl("Excel.Application"))
		{
			excel->dynamicCall("SetVisible (bool Visible)", false);
			excel->setProperty("DisplayAlerts", false);
			QAxObject* workbooks = excel->querySubObject("WorkBooks");            //��ȡ����������
			workbooks->dynamicCall("Add");                                        //�½�һ��������
			QAxObject* workbook = excel->querySubObject("ActiveWorkBook");        //��ȡ��ǰ������
			QAxObject* worksheet = workbook->querySubObject("Worksheets(int)", 1);
			QAxObject* cell;

			/*���Excel��ͷ����*/
			for (int i = 1; i <= ui.tableWidget->columnCount()-1; i++)
			{
				cell = worksheet->querySubObject("Cells(int,int)", 1, i);
				//cell->setProperty("RowHeight",30);
				//cell->setProperty("ColumnWidth",15);
				cell->dynamicCall("SetValue(const QString&)", ui.tableWidget->horizontalHeaderItem(i)->data(0).toString());
				if (ui.progressBar->value() <= 50)
				{
					ui.progressBar->setValue(10 + i * 2);
				}
			}

			/*��form�б��е��������˱��浽Excel�ļ���*/
			for (int j = 2; j <= ui.tableWidget->rowCount() + 1; j++)
			{
				for (int k = 1; k <= ui.tableWidget->columnCount()-1; k++)
				{
					cell = worksheet->querySubObject("Cells(int,int)", j, k);
					//cell->setProperty("RowHeight", 20);
					//cell->setProperty("ColumnWidth", 15);
					cell->dynamicCall("SetValue(const QString&)", ui.tableWidget->item(j - 2, k)->text());// + "\t");�Ʊ��  ÿ����Ԫ���Сһ��
				}
				if (ui.progressBar->value() < 80)
				{
					ui.progressBar->setValue(50 + j * 5);
				}
			}

			/*�����ɵ�Excel�ļ����浽ָ��Ŀ¼��*/
			workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(filename)); //������fileName
			workbook->dynamicCall("Close()");                                                   //�رչ�����
			excel->dynamicCall("Quit()");                                                       //�ر�excel
			delete excel;
			excel = NULL;

			ui.progressBar->setValue(100);
			if (QMessageBox::question(NULL, QString::fromUtf8(u8"���"), QString::fromUtf8(u8"�ļ��Ѿ��������Ƿ����ڴ򿪣�"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			{
				QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(filename)));
			}
			ui.progressBar->setValue(0);
			ui.progressBar->hide();
		}
		filename_now = filename;
	}
	else
	{
		QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("δѡ���ļ�·�����������Ϊ��"), QMessageBox::Ok);

	}
	ui.progressBar->setVisible(false);
	//parameter_updata();//
}

void QtTablewidget::add_Model_file()
{
	//saveFlag = false;
	//****�ļ�·������ת������Ȼ����·������س���
	Model_file = QFileDialog::getOpenFileName(NULL, "open", ".");
	if (Model_file.isEmpty() == true)
		return;
	QTextCodec* textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("�ļ���ʧ�ܣ������´򿪣�"), QMessageBox::Ok);
		log.ERROR_log("(excel����ҳ��)ģ���ļ���ʧ��");
		return;
	}
	//Model_filename = textCodec->fromUnicode(Model_file).data();

	QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("ģ����ӳɹ���"), QMessageBox::Ok);
}

void QtTablewidget::add_rows()
{
	saveFlag = false;
	QString temp;
	temp = ui.lineEdit_startBeta->text();
	float start = temp.toFloat();
	temp = ui.lineEdit_endBeta->text();
	float end = temp.toFloat();
	temp = ui.lineEdit_Betalen->text();
	float len = temp.toFloat();
	int num = 1 + (end - start) / len;
	for (int i = 0; i < num; i++)
	{
		add_row(start + i * len);
	}


}

void QtTablewidget::add_row(float start_beta)
{
	if (Model_file == NULL)
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("û�����ģ���ļ�����������ӣ�"), QMessageBox::Ok);
		log.WARNING_log("(excel����ҳ��)û�����ģ���ļ�");
		return;
	}

	ui.tableWidget->setRowCount(ui.tableWidget->rowCount() + 1);
	ui.tableWidget->setColumnCount(ui.tableWidget->columnCount());//��

	//����ÿ�н�������ʽ
	int num = p.size();
	p.push_back(new QProgressBar(this));
	p[num]->setFixedSize(150, 30);
	p[num]->setMaximum(100);
	p[num]->setMinimum(0);
	p[num]->setFormat(QString::fromLocal8Bit("������ȣ�%p%").arg(QString::number(p[num]->value(), 'f', 1)));
	p[num]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);//���뷽ʽ6��
	p[num]->setValue(0);   //���ý�������ֵΪ0
	//ÿһ����ʾ������
	ui.tableWidget->setCellWidget(ui.tableWidget->rowCount() - 1, 0, p[num]);

	FileConfigModInfo ConfigModInfo_single;
	QString temp;
	ConfigModInfo_single.index= ui.tableWidget->rowCount();
	temp = QString::number(ConfigModInfo_single.index, 10);
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 0+1, new QTableWidgetItem(temp));

	/*temp = ui.lineEdit_startBeta->text();
	ConfigModInfo_single.config.start_beta = temp.toFloat();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 1 + 1, new QTableWidgetItem(temp));*/
	ConfigModInfo_single.config.start_beta = start_beta;
	temp = QString("%1").arg(start_beta);
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 1 + 1, new QTableWidgetItem(temp));


	temp = ui.lineEdit_startAlpha->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 2 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.start_alpha = temp.toFloat();

	temp = ui.lineEdit_endAlpha->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 3 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.end_alpha = temp.toFloat();

	temp = ui.lineEdit_waveLength->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 4 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.wave_length = temp.toFloat();

	temp = ui.lineEdit_pipeSize->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 5 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.pipe_size = temp.toFloat();

	temp = ui.lineEdit_startFrequency->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 6 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.start_frequency = temp.toFloat();

	temp = ui.lineEdit_endFrequency->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 7 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.end_frequency = temp.toFloat();

	temp = ui.lineEdit_farDistance->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 8 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.far_distance = temp.toFloat();


	temp = ui.lineEdit_SamplingRate->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 9 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.sampling_rate = temp.toFloat();

	temp = ui.lineEdit_SamplingWidth->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 10 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.sampling_width = temp.toFloat();

	temp = ui.lineEdit_PulseWidth->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 11 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.tao = temp.toFloat();

	temp = ui.lineEdit_StartingFreq->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 12 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.time_start_frequency = temp.toFloat();

	temp = ui.lineEdit_EndingFreq->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 13 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.time_end_frequency = temp.toFloat();

	temp = ui.lineEdit_RelativeSpeed->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 14 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.relative_velocity = temp.toFloat();

	temp = ui.lineEdit_IntegralAdjust->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 15 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.integral_gain = temp.toFloat();

	temp = ui.lineEdit_Velocity1->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 16 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.velocity1 = temp.toFloat();

	temp = ui.lineEdit_Velocity2->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 17 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.velocity2 = temp.toFloat();

	temp = ui.lineEdit_ReflectCoeff->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 18 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.reflect_coeff = temp.toFloat();

	//temp = Model_filename.toString();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 19 + 1, new QTableWidgetItem(Model_file));

	temp = ui.lineEdit_card_num->text();
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 20 + 1, new QTableWidgetItem(temp));
	ConfigModInfo_single.config.card_num = temp.toFloat();

	if (ui.radioButton_ConfigReflectCoeffAuto->isChecked())
	{
		ConfigModInfo_single.config.reflect_coeff_Auto_flag = true;
		temp = u8"�Զ�";
		ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 21 + 1, new QTableWidgetItem(temp));
	}
	else if(ui.radioButton_ConfigReflectCoeffText->isChecked())
	{
		ConfigModInfo_single.config.reflect_coeff_Auto_flag = false;
		temp = u8"�ֶ�";
		ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 21 + 1, new QTableWidgetItem(temp));
	}

	/*if (ui.radioButton_scanFrequency->isChecked())
	{
		ConfigModInfo_single.cal_mood = Frequency;
		temp = u8"ɨƵ";
		ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 22 + 1, new QTableWidgetItem(temp));
	}
	else if (ui.radioButton_ConfigReflectCoeffText->isChecked())
	{*/
	ConfigModInfo_single.cal_mood = Angle;//�̶�Ϊɨ�� 3.24
	temp = u8"ɨ��";
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 22 + 1, new QTableWidgetItem(temp));
	//}
}

void QtTablewidget::delete_row()
{
	saveFlag = false;
	QList<QTableWidgetItem*>items = ui.tableWidget->selectedItems();
	int count = items.count();//һ��������
	int row_num = count / ui.tableWidget->columnCount();
	int row = ui.tableWidget->row(items.at(0));//��ȡѡ�е�����
	for (int i = 0; i <= row_num; i++)
	{
		ui.tableWidget->removeRow(row);//������е�����
	}
	//int row = ui.tableWidget->row(items.at(0));//��ȡѡ�е���
	//ui.tableWidget->removeRow(2);//������е�����
	//ui.tableWidget->removeRow(3);//������е�����


	/*����ÿ�����*/
	QString temp;
	for (int i = 1; i <= ui.tableWidget->rowCount(); i++)
	{
		temp = QString::number(i, 10);
		ui.tableWidget->setItem(i - 1, 1, new QTableWidgetItem(temp));
	}
}

void QtTablewidget::OnUpdateProgressVale(float value, int idx)
{
	p[idx]->setValue(value);
}


void QtTablewidget::OnStop()
{
	if (stop_cliked == false)
	{
		stop_cliked = true;
		//ui.stopBtn->setText(QString::fromLocal8Bit("����"));
		emit stop_Sig();
	}
	else if (stop_cliked == true)
	{
		stop_cliked = false;
		//ui.stopBtn->setText(QString::fromLocal8Bit("��ͣ"));
		emit run_Sig();
	}
	
}

//�ر��¼��Ĵ�����
void QtTablewidget::closeEvent(QCloseEvent* event)
{
	int runningStatus;
	emit getRunningStatusSig(runningStatus);

	if (runningStatus == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("���ڼ��㣡����ͣ���ٹر�"), QMessageBox::Ok);
		event->ignore();
		return;
	}
	else
	{
		//this->close();
		this->setVisible(false);
		return;
	}
}