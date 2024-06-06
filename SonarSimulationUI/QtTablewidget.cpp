//#include "QtTablewidget.h"
#include "sonarsimulationui.h"
//#include "excelengine.h"

QtTablewidget::QtTablewidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(u8"参数配置");
	ui.progressBar->setVisible(false);
	stop_cliked = false;

	QStringList headerString;

	ui.tableWidget->setColumnCount(24);
	ui.tableWidget->setRowCount(0);


	headerString << u8"执行进度" << u8"序号" << u8"初始β(度) " << u8"初始α(度) " << u8"终止α(度) " << u8"波长λ(m)  " << u8"管线直径(*λ)" <<
		u8"频域起始频率(KHz)" << u8"频域终止频率(KHz)" << u8"远场距离(m) " << u8"采样率(Hz)  " << u8"采样长度(s)" << u8"脉冲宽度(ms)" <<
		u8"起始频率(Hz)" << u8"截至频率(Hz)" << u8"相对速度(m/s)" << u8"积分调整系数" << u8"目标速度1" << u8"目标速度2" <<
		u8"反射系数" << u8"模型地址" << u8"计算卡选择" << u8"反射系数选择" << u8"计算模式选择";
	ui.tableWidget->setHorizontalHeaderLabels(headerString);
	ui.tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);//固定行高
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//自适应列宽
	//show_excel();
	ui.tableWidget->setAlternatingRowColors(true);//隔行变色  
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
	//导入数据到tablewidget中

	//ExcelEngine excels(QObject::tr("D:\\document\\705\\GPU-UI-1207-unofficial\\SonarSimulationUI_addpara\\SonarSimulationUI\\model\\工作簿1.xlsx"));
	//excels.Open();
	//excels.ReadDataToTable(ui.tableWidget); //导入到widget中
	//excels.Close();
}

//读取配置文件 zsr
void QtTablewidget::read_excel()
{
	//if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	//{
	//	QMessageBox::critical(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("网络未连接!"), QMessageBox::Ok);
	//	return;
	//}
	//打开文件
	filename_now = "";
	ui.tableWidget->setColumnCount(24);
	ui.tableWidget->setRowCount(0);
	m_datas.clear();//清空参数缓存区
	p.clear();//清空进度条缓存区

	//****文件路径编码转换，不然中文路径会加载出错
	filename = QFileDialog::getOpenFileName(NULL, "open", ".", QStringLiteral("EXCEL(*.xlsx)"));
	if (filename.isEmpty() == true)
		return;
	QTextCodec* textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("excel文件打开失败，请重新打开！"), QMessageBox::Ok);
		log.ERROR_log("exel配置文件打开失败");
		return;
	}
	std::string filename_stdstr = textCodec->fromUnicode(filename).data();

	ui.progressBar->setValue(0);   //设置进度条的值为0
	ui.progressBar->show();    //进度条需要在ui文件中加个progressBar控件

	ui.progressBar->setFormat(QString::fromLocal8Bit("正在读文件：%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));

	QAxObject* excel = new QAxObject("this");//1800ms

	excel->setControl("Excel.Application");//连接Excel控件
	//QMessageBox::StandardButton reply;
	//reply = QMessageBox::question(this, tr("question"), QString::fromLocal8Bit("打开excel文件？"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	//if (reply == QMessageBox::Yes)
	//{
	//	excel->dynamicCall("SetVisible(bool Visible)", true);//false不显示窗体
	//	excel->setProperty("DisplAlerts", true);//false不显示窗体

	//}
	//else if (reply == QMessageBox::No)
	//{
		excel->dynamicCall("SetVisible(bool Visible)", false);//false不显示窗体
		excel->setProperty("DisplAlerts", false);//false不显示窗体
	//}
	//else
		//questionLabel->setText(tr("Cancel"));

	//excel->setProperty("Value", d);//设置值
	//excel->setProperty("RowHeight", HIGHT + 9);//设置行高
	//excel->setProperty("ColumnWidth", WIDE-4);//设置单元格列宽


	//QList<QList<QVariant>> m_datas;

	ui.progressBar->setValue(10);

	QAxObject* workbooks = NULL;
	QAxObject* workbook = NULL;

	workbooks = excel->querySubObject("WorkBooks");
	//workbook = workbooks->querySubObject("Open(QString, QVariant)", QString(tr("d:\\ExcelTest.xlsx")));
	workbook = workbooks->querySubObject("Open(QString, QVariant)", filename);
	QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1);//打开第一个sheet


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
	ui.tableWidget->setColumnCount(m_datas[0].size()+1);//列
	//ui.tableWidget = new QTableWidget(m_datas.size(), m_datas[0].size()); // 构造了一个QTableWidget的对象，并且设置为10行，5列
	//tableWidget->setWindowTitle("配置文件");
	//tableWidget->resize(m_datas.size(), m_datas[0].size());  //设置表格
	//ui.tableWidget->repaint();
	ui.tableWidget->update();
	ui.tableWidget->viewport()->update();

	for (int i = 1; i < m_datas.size(); i++)
	{
		//设置每行进度条样式
		p.push_back(new QProgressBar(this));
		p[i - 1]->setFixedSize(150, 30);
		p[i - 1]->setMaximum(100);
		p[i - 1]->setMinimum(0);
		p[i - 1]->setFormat(QString::fromLocal8Bit("计算进度：%p%").arg(QString::number(p[i - 1]->value(), 'f', 1)));
		p[i - 1]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);//对齐方式6天
		p[i - 1]->setValue(0);   //设置进度条的值为0
		//每一行显示进度条
		ui.tableWidget->setCellWidget(i - 1, 0, p[i - 1]);

		//ui.progressBar->setValue((30+(i+1)*(70/ m_datas.size())));
		for (int j = 1; j <= m_datas[0].size(); j++)
		{
			//QString strVal = m_datas.at(i).at(j).toString();
			//将单元格的内容放置在table表中
			ui.tableWidget->setItem(i - 1, j, new QTableWidgetItem(m_datas[i][j-1].toString()));
			//ui.tableWidget->item(i, j)->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		}
		//ui.tableWidget->repaint();
		ui.tableWidget->update();
		ui.tableWidget->viewport()->update();
		ui.progressBar->setValue((30 + (i + 1) * (70 / m_datas.size())));
	}
	ui.progressBar->setValue(100);
	QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("配置文件读取完成！"), QMessageBox::Ok);
	ui.progressBar->setValue(0);
	ui.progressBar->hide();
	ui.progressBar->setVisible(false);
	workbook->dynamicCall("Close()");                                                   //关闭工作簿
	excel->dynamicCall("Quit()");                                                       //关闭excel
	delete excel;
	excel = NULL;
	//filename_now = filename;
}

void QtTablewidget::parameter_pass()
{
	if (saveFlag == false)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("未保存!"), QMessageBox::Ok);
		return;
	}
	int runningStatus;
	emit getRunningStatusSig(runningStatus);

	if (runningStatus == WAITFORCONNECT)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("网络未连接！"), QMessageBox::Ok);
		return;
	}

	if (runningStatus == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("正在计算！"), QMessageBox::Ok);
		return;
	}


	stop_cliked = false;//暂停初始化
	//ui.stopBtn->setText(QString::fromLocal8Bit("暂停"));

	if (filename_now != NULL)
	{
		parameter_updata();//
	}
	else
	{
		filename_now = filename;
	}

	ui.progressBar->setValue(0);   //设置进度条的值为0
	ui.progressBar->show();    //进度条需要在ui文件中加个progressBar控件
	ui.progressBar->setFormat(QString::fromLocal8Bit("正在传递参数：%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));
	fileConfigModInfos.clear();//每次参数传递前清除内存空间  2022.01.25
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
			QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("模型文件路径过长！"), QMessageBox::Ok);
			log.ERROR_log("(excel配置页面)模型文件路径过长");
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
		if (temp == "自动")
		{
			fileConfigModInfo.config.reflect_coeff_Auto_flag = true;
		}
		else
		{
			fileConfigModInfo.config.reflect_coeff_Auto_flag = false;
		}
		//if (temp == "扫频")
		//{
		//	fileConfigModInfo.cal_mood = Frequency;
		//}
		//else
		//{
			fileConfigModInfo.cal_mood = Angle;//固定为扫角 3.24
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
*函数功能：把QVariant转换为QList<QList<QVariant> >
*输入：
*	var:原QVariant
*	res:新的QList<QList<QVariant> >
*输出：
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
	const int rowCount = varRows.size();//行数
	QVariantList rowData;
	for (int i = 0; i < rowCount; ++i)
	{
		rowData = varRows[i].toList();//将每一行的值存入到list中
		res.push_back(rowData);
	}
}
//参数文件更改后更新内存内参数
void QtTablewidget::parameter_updata()
{
	//if (dataManagers[curModuleIndex].GetStatus() == WAITFORCONNECT)
	//{
	//	QMessageBox::critical(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("网络未连接!"), QMessageBox::Ok);
	//	return;
	//}
	//打开文件
	m_datas.clear();
	ui.progressBar->setValue(0);   //设置进度条的值为0
	ui.progressBar->show();    //进度条需要在ui文件中加个progressBar控件
	ui.progressBar->setFormat(QString::fromLocal8Bit("正在更新内存内参数：%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));
	QAxObject* excel = new QAxObject("this");//1800ms

	excel->setControl("Excel.Application");//连接Excel控件

	ui.progressBar->setValue(10);

	QAxObject* workbooks = NULL;
	QAxObject* workbook = NULL;

	workbooks = excel->querySubObject("WorkBooks");
	//workbook = workbooks->querySubObject("Open(QString, QVariant)", QString(tr("d:\\ExcelTest.xlsx")));
	workbook = workbooks->querySubObject("Open(QString, QVariant)", filename_now);
	QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1);//打开第一个sheet

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
	QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("参数更新完成！"), QMessageBox::Ok);
	ui.progressBar->setValue(0);
	ui.progressBar->hide();
	ui.progressBar->setVisible(false);
	workbook->dynamicCall("Close()");                                                   //关闭工作簿
	excel->dynamicCall("Quit()");                                                       //关闭excel
	delete excel;
	excel = NULL;

}

//将Qvariant转换为QList<QList<QVariant>>
void QtTablewidget::Qvariant2listlistVariant(const QVariant & var, QList<QList<QVariant>> &ret)
{
	QVariantList varrows = var.toList();
	if (varrows.isEmpty())
	{
		return;
	}
	else {
		const int rowcount = varrows.size();//行数
		QVariantList rowdata;
		for (int i = 0; i < rowcount; i++)
		{
			rowdata = varrows[i].toList();//将每一行的值存入到list中
			ret.push_back(rowdata);
		}
	}
}

//快速将数据写入excel中
void QtTablewidget::excel_write()
{
	saveFlag = true;
	ui.progressBar->setValue(0);   //设置进度条的值为0
	ui.progressBar->show();    //进度条需要在ui文件中加个progressBar控件
	ui.progressBar->setFormat(QString::fromLocal8Bit("文件另存为：%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));
	QString fileName = QFileDialog::getSaveFileName(this, tr("Excle file"), QString("./parameter_list.xlsx"), tr("Excel Files(*.xlsx)"));    //设置保存的文件名
	if (fileName != "")
	{
		ui.progressBar->setValue(10);
		QAxObject* excel = new QAxObject;
		if (excel->setControl("Excel.Application"))
		{
			excel->dynamicCall("SetVisible (bool Visible)", false);
			excel->setProperty("DisplayAlerts", false);
			QAxObject* workbooks = excel->querySubObject("WorkBooks");            //获取工作簿集合
			workbooks->dynamicCall("Add");                                        //新建一个工作簿
			QAxObject* workbook = excel->querySubObject("ActiveWorkBook");        //获取当前工作簿
			QAxObject* worksheet = workbook->querySubObject("Worksheets(int)", 1);
			QAxObject* cell;

			/*添加Excel表头数据*/
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

			/*将form列表中的数据依此保存到Excel文件中*/
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

			/*将生成的Excel文件保存到指定目录下*/
			workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName)); //保存至fileName
			workbook->dynamicCall("Close()");                                                   //关闭工作簿
			excel->dynamicCall("Quit()");                                                       //关闭excel
			delete excel;
			excel = NULL;

			ui.progressBar->setValue(100);
			if (QMessageBox::question(NULL, QString::fromUtf8(u8"完成"), QString::fromUtf8(u8"文件已经导出，是否现在打开？"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
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

//把QList<QList<QVariant> > 转为QVariant,用于快速写入的
void QtTablewidget::castListListVariant2Variant(const QList<QList<QVariant> > &cells, QVariant & res)
{
	QVariantList vars;
	const int rows = cells.size();//获取行数
	for (int i = 0; i < rows; ++i)
	{
		vars.append(QVariant(cells[i]));//将list(i)添加到QVariantList中 QVariant(cells[i])强制转换
	}
	res = QVariant(vars);//强制转换
}

void QtTablewidget::write_excel_save()
{
	saveFlag = true;
	ui.progressBar->setValue(0);   //设置进度条的值为0
	ui.progressBar->show();    //进度条需要在ui文件中加个progressBar控件
	ui.progressBar->setFormat(QString::fromLocal8Bit("文件保存：%p%").arg(QString::number(ui.progressBar->value(), 'f', 1)));

	//QString fileName = QFileDialog::getSaveFileName(this, tr("Excle file"), QString("./parameter_list.xlsx"), tr("Excel Files(*.xlsx)"));    //设置保存的文件名
	if (filename != "")
	{
		ui.progressBar->setValue(10);
		QAxObject* excel = new QAxObject;
		if (excel->setControl("Excel.Application"))
		{
			excel->dynamicCall("SetVisible (bool Visible)", false);
			excel->setProperty("DisplayAlerts", false);
			QAxObject* workbooks = excel->querySubObject("WorkBooks");            //获取工作簿集合
			workbooks->dynamicCall("Add");                                        //新建一个工作簿
			QAxObject* workbook = excel->querySubObject("ActiveWorkBook");        //获取当前工作簿
			QAxObject* worksheet = workbook->querySubObject("Worksheets(int)", 1);
			QAxObject* cell;

			/*添加Excel表头数据*/
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

			/*将form列表中的数据依此保存到Excel文件中*/
			for (int j = 2; j <= ui.tableWidget->rowCount() + 1; j++)
			{
				for (int k = 1; k <= ui.tableWidget->columnCount()-1; k++)
				{
					cell = worksheet->querySubObject("Cells(int,int)", j, k);
					//cell->setProperty("RowHeight", 20);
					//cell->setProperty("ColumnWidth", 15);
					cell->dynamicCall("SetValue(const QString&)", ui.tableWidget->item(j - 2, k)->text());// + "\t");制表符  每个单元格大小一致
				}
				if (ui.progressBar->value() < 80)
				{
					ui.progressBar->setValue(50 + j * 5);
				}
			}

			/*将生成的Excel文件保存到指定目录下*/
			workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(filename)); //保存至fileName
			workbook->dynamicCall("Close()");                                                   //关闭工作簿
			excel->dynamicCall("Quit()");                                                       //关闭excel
			delete excel;
			excel = NULL;

			ui.progressBar->setValue(100);
			if (QMessageBox::question(NULL, QString::fromUtf8(u8"完成"), QString::fromUtf8(u8"文件已经导出，是否现在打开？"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
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
		QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("未选择文件路径，请点击另存为！"), QMessageBox::Ok);

	}
	ui.progressBar->setVisible(false);
	//parameter_updata();//
}

void QtTablewidget::add_Model_file()
{
	//saveFlag = false;
	//****文件路径编码转换，不然中文路径会加载出错
	Model_file = QFileDialog::getOpenFileName(NULL, "open", ".");
	if (Model_file.isEmpty() == true)
		return;
	QTextCodec* textCodec = QTextCodec::codecForName("gb18030");
	if (textCodec == NULL)
	{
		QMessageBox::information(this, "error", QString::fromLocal8Bit("文件打开失败，请重新打开！"), QMessageBox::Ok);
		log.ERROR_log("(excel配置页面)模型文件打开失败");
		return;
	}
	//Model_filename = textCodec->fromUnicode(Model_file).data();

	QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("模型添加成功！"), QMessageBox::Ok);
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
		QMessageBox::information(this, "error", QString::fromLocal8Bit("没有添加模型文件，请重新添加！"), QMessageBox::Ok);
		log.WARNING_log("(excel配置页面)没有添加模型文件");
		return;
	}

	ui.tableWidget->setRowCount(ui.tableWidget->rowCount() + 1);
	ui.tableWidget->setColumnCount(ui.tableWidget->columnCount());//列

	//设置每行进度条样式
	int num = p.size();
	p.push_back(new QProgressBar(this));
	p[num]->setFixedSize(150, 30);
	p[num]->setMaximum(100);
	p[num]->setMinimum(0);
	p[num]->setFormat(QString::fromLocal8Bit("计算进度：%p%").arg(QString::number(p[num]->value(), 'f', 1)));
	p[num]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);//对齐方式6天
	p[num]->setValue(0);   //设置进度条的值为0
	//每一行显示进度条
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
		temp = u8"自动";
		ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 21 + 1, new QTableWidgetItem(temp));
	}
	else if(ui.radioButton_ConfigReflectCoeffText->isChecked())
	{
		ConfigModInfo_single.config.reflect_coeff_Auto_flag = false;
		temp = u8"手动";
		ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 21 + 1, new QTableWidgetItem(temp));
	}

	/*if (ui.radioButton_scanFrequency->isChecked())
	{
		ConfigModInfo_single.cal_mood = Frequency;
		temp = u8"扫频";
		ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 22 + 1, new QTableWidgetItem(temp));
	}
	else if (ui.radioButton_ConfigReflectCoeffText->isChecked())
	{*/
	ConfigModInfo_single.cal_mood = Angle;//固定为扫角 3.24
	temp = u8"扫角";
	ui.tableWidget->setItem(ui.tableWidget->rowCount() - 1, 22 + 1, new QTableWidgetItem(temp));
	//}
}

void QtTablewidget::delete_row()
{
	saveFlag = false;
	QList<QTableWidgetItem*>items = ui.tableWidget->selectedItems();
	int count = items.count();//一共多少列
	int row_num = count / ui.tableWidget->columnCount();
	int row = ui.tableWidget->row(items.at(0));//获取选中的首行
	for (int i = 0; i <= row_num; i++)
	{
		ui.tableWidget->removeRow(row);//清除已有的行列
	}
	//int row = ui.tableWidget->row(items.at(0));//获取选中的行
	//ui.tableWidget->removeRow(2);//清除已有的行列
	//ui.tableWidget->removeRow(3);//清除已有的行列


	/*更新每行序号*/
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
		//ui.stopBtn->setText(QString::fromLocal8Bit("运行"));
		emit stop_Sig();
	}
	else if (stop_cliked == true)
	{
		stop_cliked = false;
		//ui.stopBtn->setText(QString::fromLocal8Bit("暂停"));
		emit run_Sig();
	}
	
}

//关闭事件的处理函数
void QtTablewidget::closeEvent(QCloseEvent* event)
{
	int runningStatus;
	emit getRunningStatusSig(runningStatus);

	if (runningStatus == RUNNING)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("正在计算！请暂停后再关闭"), QMessageBox::Ok);
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