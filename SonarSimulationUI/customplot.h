#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include "qcustomplot.h"
#include <QVector>
#include "FluentData.h"

enum WaveType{waveAngle,waveFreq,waveTime};

class CustomPlot : public QCustomPlot
{
	Q_OBJECT

public:
	CustomPlot(QWidget *parent=nullptr);
	~CustomPlot();

	//std::vector<float> x;//
//	std::vector<float> y;//

	//void analyze_timedomain(std::string filename);//

	void PushResult(float x, float y);

	//void SetTimeData(FluentData model);
	void SetTimeData(std::vector<double>, std::vector<double>, double maxY, double minY);
	

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
