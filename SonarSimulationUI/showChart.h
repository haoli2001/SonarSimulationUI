#pragma once

#include <QWidget>
#include "ui_showChart.h"
#include "customplot.h"

class showChart : public QWidget
{
	Q_OBJECT

public:
	showChart(QString txtName, QWidget *parent = Q_NULLPTR);
	~showChart();

	void getFileData();
	void Traversefolders(QString path);
	void drawWave(QString fileName);


public slots:
	void OnPushButtonBefore();
	void OnPushButtonNext();
	bool OnPushButtonSave();

private:
	Ui::showChart ui;

	QFileInfoList file_list;//文件夹下的所有txt文件
	//QStringList files;
	QString TXTfilename = "";//txt文件名
};
