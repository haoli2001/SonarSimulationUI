#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include "qcustomplot.h"
#include <QVector>

enum WaveType{waveAngle,waveFreq};

class CustomPlot : public QCustomPlot
{
	Q_OBJECT

public:
	CustomPlot(QWidget *parent=nullptr);
	~CustomPlot();

	void PushResult(float x, float y);

	void Init(WaveType type);

public slots:
	void OnRadioButtonScanAngle(bool);
	void OnRadioButtonScanFrequrency(bool);

private:
	QVector<double> yVector;
	QVector<double> xVector;
	
	float maxResult;
	float minResult;

	WaveType type;
};

#endif // CUSTOMPLOT_H
