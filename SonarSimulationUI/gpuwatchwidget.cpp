#include "gpuwatchwidget.h"

GPUWatchWidget::GPUWatchWidget(QWidget *parent)
	: QWidget(parent)
{
	QGridLayout *layout = new QGridLayout();


	page = new QWidget();
	QScrollArea *s = new QScrollArea(this);
	s->setWidgetResizable(true);
	pVlayout = new QGridLayout();
	
	page->setLayout(pVlayout);

	s->setWidget(page);
	layout->addWidget(s);
	this->setLayout(layout);
}

GPUWatchWidget::~GPUWatchWidget()
{

}

void GPUWatchWidget::Init(int deviceCount)
{
	for (int j = 0; j < 10; j++)
	{
		x.push_back(j);
	}
	QGridLayout *layout = new QGridLayout();
	for (int i = 0; i < deviceCount; i++)
	{
		QVector<double> gpu_v;
		QVector<double> memory_v;
		for (int j = 0; j < 10; j++)
		{
			gpu_v.push_back(0);
			memory_v.push_back(0);
		}

		gpu.push_back(gpu_v);
		memory.push_back(memory_v);

		QCustomPlot *wave_gpu = new QCustomPlot();
		wave_gpu->addGraph();
		wave_gpu->yAxis->setRange(0, 100);
		wave_gpu->xAxis->setRange(0, 10);
		wave_gpu->xAxis->setTicks(false);
		wave_gpu->yAxis->setTicks(false);
		wave_gpu->axisRect()->setupFullAxesBox();
		wave_gpu->graph(0)->setBrush(QBrush(QColor(0, 0, 0, 15)));
		wave_gpu->plotLayout()->insertRow(0);
		//wave_gpu->graph(0)->setBrush(QBrush(Qt::green));
		QString title_gpu;
		title_gpu.sprintf("device %d:", i);
		wave_gpu->plotLayout()->addElement(0, 0, new QCPTextElement(wave_gpu, title_gpu + QString::fromLocal8Bit("GPU负载    "), QFont("宋体", 8)));
		wave_gpu->setFixedHeight(100);
		wave_gpu->graph(0)->setData(x, gpu_v);
		QCustomPlot *wave_memory = new QCustomPlot();
		wave_memory->addGraph();
		wave_memory->yAxis->setRange(0, 100);
		wave_memory->xAxis->setRange(0, 10);
		wave_memory->xAxis->setTicks(false);
		wave_memory->yAxis->setTicks(false);
		wave_memory->axisRect()->setupFullAxesBox();
		wave_memory->plotLayout()->insertRow(0);
		wave_memory->plotLayout()->addElement(0, 0, new QCPTextElement(wave_memory, title_gpu + QString::fromLocal8Bit("显存使用率    "), QFont("宋体", 8)));
		wave_memory->graph(0)->setData(x, memory_v);
		wave_memory->graph(0)->setBrush(QBrush(QColor(0, 0, 0, 15)));
		wave_memory->setFixedHeight(100);
		//wave_memory->graph(0)->setBrush(QBrush(Qt::green));

		this->gpu_wave.push_back(wave_gpu);
		this->memory_wave.push_back(wave_memory);

		QVBoxLayout *temp_layout = new QVBoxLayout();
		QLabel *temp_label = new QLabel(QString::fromLocal8Bit("温度"));
		//temp_label->setFont(QFont("Microsoft YaHei", 10));
		QLabel *temp_value_i = new QLabel();
		temp_value_i->setText(QString::fromLocal8Bit("0"));
		temp_value_i->setFont(QFont("Microsoft YaHei Light", 11));
		QLabel *memory_label = new QLabel(QString::fromLocal8Bit("显存"));
		QLabel *memory_value_i = new QLabel();
		memory_value_i->setText("0.00/0.00 GB");
		memory_value_i->setFont(QFont("Microsoft YaHei Light", 11));
		temp_value.push_back(temp_value_i);
		memory_value.push_back(memory_value_i);
		temp_layout->addWidget(temp_label);
		temp_layout->addWidget(temp_value[i]);
		temp_layout->addWidget(memory_label);
		temp_layout->addWidget(memory_value[i]);
		QVBoxLayout *layout_box = new QVBoxLayout();
		layout_box->addLayout(temp_layout);
		layout_box->addStretch();


		pVlayout->addWidget(wave_gpu, i, 0, 1, 2);
		pVlayout->addWidget(wave_memory, i, 2, 1, 2);
		pVlayout->addLayout(layout_box, i, 4, 1, 1);
	}  
}

void GPUWatchWidget::SetData(GPUWatchStruct info)
{
	if (this->gpu.size() == 0)
		return;
	double mem_total = info.total / 1024.0 / 1024.0 / 1024.0;
	double mem_used = info.used / 1024.0 / 1024.0 / 1024.0;
	float memory_used_pre = (mem_used / mem_total) * 100;
	this->gpu[info.device_id].pop_front();
	this->gpu[info.device_id].push_back(info.gpu);
	this->memory[info.device_id].pop_front();
	this->memory[info.device_id].push_back(memory_used_pre);
	this->gpu_wave[info.device_id]->graph(0)->setData(x, this->gpu[info.device_id]);


	this->memory_wave[info.device_id]->graph(0)->setData(x, this->memory[info.device_id]);

	//this->temp_v[id]->setText(QString::number(temp));
	//this->temp_value[id]->clear();
	this->temp_value[info.device_id]->setText(QString::number(info.temp));

	this->memory_value[info.device_id]->setText(QString::number(mem_used, 'f', 2) + QString("/") + QString::number(mem_total, 'f', 2) + QString("GB"));
	this->temp_value[info.device_id]->update();
	this->gpu_wave[info.device_id]->replot();
	this->memory_wave[info.device_id]->replot();
}
