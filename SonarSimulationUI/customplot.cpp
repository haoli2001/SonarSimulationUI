#include "customplot.h"
#include "float.h"

CustomPlot::CustomPlot(QWidget *parent)
	: QCustomPlot(parent), maxResult(0), minResult(0)
{
	
}

CustomPlot::~CustomPlot()
{

}

void CustomPlot::OnRadioButtonScanAngle(bool value)
{

}

void CustomPlot::OnRadioButtonScanFrequrency(bool value)
{

}

void CustomPlot::PushResult(float x, float y)
{
	if (_isnan(y))
	{
		y = 0;
	}
	if (y > maxResult)
	{
		maxResult = y;
	}
	if (y < minResult)
	{
		minResult = y;
	}
	this->yAxis->setRange(minResult - 5, maxResult + 5);
	this->xVector.push_back(x);
	this->yVector.push_back(y);
	this->xAxis->setRange(0, xVector[xVector.length() - 1] > 5 ? xVector[xVector.length() - 1] : 5);

	this->graph(0)->setData(xVector, yVector);
	this->replot();
}

void CustomPlot::SetTimeData(std::vector<double> XX, std::vector<double> YY, double maxY, double minY)
{
	xVector = QVector<double>::fromStdVector(XX);
	yVector = QVector<double>::fromStdVector(YY);

	this->yAxis->setRange(minY - 5, maxY + 5);
	this->xAxis->setRange(0, xVector[xVector.length() - 1] > 5 ? xVector[xVector.length() - 1] : 5);

	this->graph(0)->setData(xVector, yVector);
	this->graph(0)->rescaleAxes();
	this->legend->setVisible(false);//����ʾͼ��  20220324 �¾�
	// �����û�������϶��᷶Χ�������������ţ����ѡ��ͼ��:
	//this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
	this->replot();
}


void CustomPlot::Init(WaveType type)
{
	xVector.clear();
	yVector.clear();

	this->legend->setVisible(true);
	this->legend->setFont(QFont("Helvetica", 9));
	this->legend->setRowSpacing(-3);
	this->rescaleAxes();
	if (this->graphCount() == 0)
		this->addGraph();
	if (type == waveTime)
	{
		this->xAxis->setLabel(QString::fromLocal8Bit("t/s"));
		this->yAxis->setLabel(QString::fromLocal8Bit("��ֵ"));
	}
	else if (type == waveAngle)
	{
		this->xAxis->setLabel(QString::fromLocal8Bit("�Ƕ�/��"));
		this->yAxis->setLabel(QString::fromLocal8Bit("TS/dB"));
	}
	else
	{
		this->xAxis->setLabel(QString::fromLocal8Bit("Ƶ��/kHz"));
		this->yAxis->setLabel(QString::fromLocal8Bit("TS/dB"));
	}

	this->graph(0)->setData(xVector, yVector);
	this->graph(0)->setPen(QPen(Qt::red));
	//this->graph(0)->setBrush(QBrush(Qt::green));
	this->legend->setVisible(false);//����ʾͼ��  20220324 �¾�
	/*this->graph(1)->setData(xVector, yVector);
	this->graph(1)->setPen(QPen(Qt::green));*/
	this->replot(); 
}