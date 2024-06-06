#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include "ui_connectdialog.h"
#include <qregexp.h>
#include <qmessagebox.h>

class ConnectDialog : public QDialog
{
	Q_OBJECT

public:
	ConnectDialog(QString *ip, int *port, QWidget *parent = 0);
	~ConnectDialog();
	public slots:
	void OnPushButtonConnect();

private:
	Ui::ConnectDialog ui;

	QString *ip;
	int *port;
};

#endif // CONNECTDIALOG_H
