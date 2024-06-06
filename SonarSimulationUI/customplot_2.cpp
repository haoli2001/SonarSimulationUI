#include "customplot.h"
#include "float.h"

CustomPlot::CustomPlot(QWidget *parent)
	: QCustomPlot(parent), maxResult(0), minResult(0)
{
	Init(waveAngle);
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
	if (type == waveAngle)
		this->xAxis->setLabel(QString::fromLocal8Bit("½Ç¶È/¶È"));
	else
		this->xAxis->setLabel(QString::fromLocal8Bit("ÆµÂÊ/kHz"));

	this->yAxis->setLabel(QString::fromLocal8Bit("TS/dB"));

	this->graph(0)->setData(xVector, yVector);
	this->graph(0)->setPen(QPen(Qt::red));
	//this->graph(0)->setBrush(QBrush(Qt::green));
	this->replot(); 
}