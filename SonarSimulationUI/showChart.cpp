#include "showChart.h"
#include <QFileDialog>
#include <vector>

showChart::showChart(QString txtName, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("TS���ֲ���ͼ"));

	ui.widget_wave->Init(waveTime);

	TXTfilename = txtName;
	drawWave(TXTfilename);
	ui.groupBox->setTitle(TXTfilename);

	QString folderpath = TXTfilename.left(TXTfilename.lastIndexOf('/'));
	Traversefolders(folderpath);

}

showChart::~showChart()
{
}

//��·�����ļ�����ͼ
void showChart::drawWave(QString fileName)
{
	std::vector<double> Xdata;
	std::vector<double> Ydata;
	double maxYY, minYY;
	Xdata.clear();
	Ydata.clear();
	maxYY = -999999999, minYY = 999999999;


	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		for (int i = 0; !file.atEnd(); i++)
		{
			QByteArray line = file.readLine().trimmed();
			QString str(line);
			
			if (str == "-inf" || str == "inf")
				continue;

			double y = str.toDouble();		

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

			Xdata.push_back(i);
			Ydata.push_back(y);
		}
		file.close();
	}
	ui.widget_wave->SetTimeData(Xdata, Ydata, maxYY, minYY);
}


//�����ļ����µ�����txt�ļ�
void showChart::Traversefolders(QString path)
{
	QDir dir(path);
	file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks, QDir::Time | QDir::Reversed);

}

//��һ��
void showChart::OnPushButtonBefore()
{
	ui.widget_wave->Init(waveTime);
	static int before = 0;
	before++;

	TXTfilename = file_list.at(file_list.indexOf(TXTfilename) - 1).absoluteFilePath();
	ui.groupBox->setTitle(TXTfilename);
	drawWave(TXTfilename);

	ui.pushButton_next->setEnabled(true);
	if (file_list.indexOf(TXTfilename) == 0)
	{
		ui.pushButton_before->setEnabled(false);
		QMessageBox::warning(this, " ", QString::fromLocal8Bit("�ѵ����ʼ"));
	}
	

}
//��һ��
void showChart::OnPushButtonNext()
{
	ui.widget_wave->Init(waveTime);
	static int next = 0;
	next++;

	TXTfilename = file_list.at(file_list.indexOf(TXTfilename) + 1).absoluteFilePath();
	ui.groupBox->setTitle(TXTfilename);
	drawWave(TXTfilename);

	ui.pushButton_before->setEnabled(true);
	if (file_list.indexOf(TXTfilename) + 1 == file_list.count())
	{
		ui.pushButton_next->setEnabled(false);
		QMessageBox::warning(this, " ", QString::fromLocal8Bit("�ѵ������һ��"));
	}
	
}

//��ȡ�ļ��е�����
void showChart::getFileData()
{
	//ѡ����ļ��������
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ����־�ļ�"), "", QStringLiteral("TXT(*.txt)")); //ѡ��·��
	if (fileName.isEmpty())     //���δѡ���ļ���ȷ�ϣ�������
		return;

	QFile file(fileName);
	//QVector<QPointF> points;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))//ֻ�����ı���ʽ
	{
		for (int i = 0; !file.atEnd(); i++)
		{
			QByteArray line = file.readLine().trimmed();//��һ�У�ȥ�ո�
			QString str(line);
			//points.append(QPointF(i, str.toDouble()));
		}
		file.close();
	}
}

bool showChart::OnPushButtonSave()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("����ͼƬ"), "", QStringLiteral("Images(*.jpg);;Images(*.bmp);;Images(*.png)"));

	if (filename == "") 
	{
		QMessageBox::information(this, "fail", QString::fromLocal8Bit("����ʧ��"));
		return false;
	}
	if (filename.endsWith(".png")) 
	{
		QMessageBox::information(this, "success", QString::fromLocal8Bit("�ɹ�����Ϊpng�ļ�"));
		return ui.widget_wave->savePng(filename, ui.widget_wave->width(), ui.widget_wave->height());
	}
	if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) 
	{
		QMessageBox::information(this, "success", QString::fromLocal8Bit("�ɹ�����Ϊjpg�ļ�"));
		return ui.widget_wave->saveJpg(filename, ui.widget_wave->width(), ui.widget_wave->height());
	}
	if (filename.endsWith(".bmp")) 
	{
		QMessageBox::information(this, "success", QString::fromLocal8Bit("�ɹ�����Ϊbmp�ļ�"));
		return ui.widget_wave->saveBmp(filename, ui.widget_wave->width(), ui.widget_wave->height());
	}
	else 
	{
		//����׷�Ӻ�׺��Ϊ.png�����ļ�
		QMessageBox::information(this, "success", QString::fromLocal8Bit("����ɹ�,��Ĭ�ϱ���Ϊpng�ļ�"));
		return ui.widget_wave->savePng(filename.append(".png"), ui.widget_wave->width(), ui.widget_wave->height());
	}
}