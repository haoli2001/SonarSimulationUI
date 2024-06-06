#include "connectdialog.h"

ConnectDialog::ConnectDialog(QString *ip, int *port, QWidget *parent)
	: QDialog(parent), ip(ip), port(port)
{
	ui.setupUi(this);
}

ConnectDialog::~ConnectDialog()
{

}

void ConnectDialog::OnPushButtonConnect()
{
	QString iptemp = ui.lineEdit_ip->text();
	QString porttemp = ui.lineEdit_port->text();

	QString patternIP("\\d*\\.\\d*\\.\\d*\\.\\d*");
	QString patternPort("\\d*");

	QRegExp rxIP(patternIP);
	QRegExp rxPort(patternPort);

	if (!rxIP.exactMatch(iptemp))
	{
		QMessageBox::information(this, "error", "IP is error", QMessageBox::Ok);
		return;
	}
	if (!rxPort.exactMatch(porttemp))
	{
		QMessageBox::information(this, "error", "Port is error", QMessageBox::Ok);
		return;
	}
	*ip = iptemp;
	*port = porttemp.toInt();
	accept();
	
}
