#ifndef GPUWATCHWIDGET_H
#define GPUWATCHWIDGET_H

#include <QWidget>
#include <qgridlayout.h>
#include "qcustomplot.h"
#include <qvector.h>
#include "commonstruct.h"
class GPUWatchWidget : public QWidget
{
	Q_OBJECT

public:
	GPUWatchWidget(QWidget *parent);
	~GPUWatchWidget();

	void Init(int deviceCount);

	void SetData(GPUWatchStruct info);

private:

	QVector<QVector<double>> gpu;
	QVector<QVector<double>> memory;
	QVector<double> x;

	QVector<QCustomPlot*> gpu_wave;
	QVector<QCustomPlot*> memory_wave;
	QVector<QLabel *> temp_value;
	QVector<QLabel *> memory_value;

	QVector<QLabel*>  temp_v;

	QWidget *page;
	QGridLayout *pVlayout;
};

#endif // GPUWATCHWIDGET_H
