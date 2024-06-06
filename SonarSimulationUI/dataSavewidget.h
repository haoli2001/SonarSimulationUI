#pragma once

#include <QWidget>
#include "ui_dataSavewidget.h"
#include "Usersql.h"
#include "showChart.h"

#include <QString>
#include <QtSql>
#include <QtSql/QSqlDatabase>	
#include <QtSql/QSqlError>		
#include <QtSql/QSqlQuery>	
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QVariantList>			
#include <QDebug>

#include <QContextMenuEvent>


class dataSavewidget : public QWidget
{
	Q_OBJECT
public:
	dataSavewidget(QWidget *parent = Q_NULLPTR);
	~dataSavewidget();

	Usersql sql;               //Êý¾Ý¿âÀà
	Usersql getUsersql();
	int getModelRowCount();

public slots:
	void OnPushbuttonDisplay();
	void OnPushbuttonQuery();
	void OnPushbuttonAdd();
	void OnPushbuttonDelete();
	void OnPushbuttonSubmit();
	void OnPushbuttonCancel();

	void OnPushbuttonShowWave();
	//void OntableView_doubleClicked(const QModelIndex &index);
	void OnPushbuttonCheck();

	void keyPressEvent(QKeyEvent *event);

private:
	Ui::dataSavewidget ui;
	showChart* chart;
	QSqlTableModel *model;

};
