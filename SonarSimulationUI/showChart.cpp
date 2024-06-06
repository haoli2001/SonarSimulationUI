#include "showChart.h"
#include <QFileDialog>
#include <vector>

showChart::showChart(QString txtName, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowTitle(QString::fromLocal8Bit("TS积分波形图"));

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

//按路径打开文件并画图
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

			Xdata.push_back(i);
			Ydata.push_back(y);
		}
		file.close();
	}
	ui.widget_wave->SetTimeData(Xdata, Ydata, maxYY, minYY);
}


//遍历文件夹下的所有txt文件
void showChart::Traversefolders(QString path)
{
	QDir dir(path);
	file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks, QDir::Time | QDir::Reversed);

}

//上一张
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
		QMessageBox::warning(this, " ", QString::fromLocal8Bit("已到达最开始"));
	}
	

}
//下一张
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
		QMessageBox::warning(this, " ", QString::fromLocal8Bit("已到达最后一张"));
	}
	
}

//读取文件中的数据
void showChart::getFileData()
{
	//选择的文件是随机数
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择日志文件"), "", QStringLiteral("TXT(*.txt)")); //选择路径
	if (fileName.isEmpty())     //如果未选择文件便确认，即返回
		return;

	QFile file(fileName);
	//QVector<QPointF> points;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))//只读、文本方式
	{
		for (int i = 0; !file.atEnd(); i++)
		{
			QByteArray line = file.readLine().trimmed();//读一行，去空格
			QString str(line);
			//points.append(QPointF(i, str.toDouble()));
		}
		file.close();
	}
}

bool showChart::OnPushButtonSave()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("保存图片"), "", QStringLiteral("Images(*.jpg);;Images(*.bmp);;Images(*.png)"));

	if (filename == "") 
	{
		QMessageBox::information(this, "fail", QString::fromLocal8Bit("保存失败"));
		return false;
	}
	if (filename.endsWith(".png")) 
	{
		QMessageBox::information(this, "success", QString::fromLocal8Bit("成功保存为png文件"));
		return ui.widget_wave->savePng(filename, ui.widget_wave->width(), ui.widget_wave->height());
	}
	if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) 
	{
		QMessageBox::information(this, "success", QString::fromLocal8Bit("成功保存为jpg文件"));
		return ui.widget_wave->saveJpg(filename, ui.widget_wave->width(), ui.widget_wave->height());
	}
	if (filename.endsWith(".bmp")) 
	{
		QMessageBox::information(this, "success", QString::fromLocal8Bit("成功保存为bmp文件"));
		return ui.widget_wave->saveBmp(filename, ui.widget_wave->width(), ui.widget_wave->height());
	}
	else 
	{
		//否则追加后缀名为.png保存文件
		QMessageBox::information(this, "success", QString::fromLocal8Bit("保存成功,已默认保存为png文件"));
		return ui.widget_wave->savePng(filename.append(".png"), ui.widget_wave->width(), ui.widget_wave->height());
	}
}