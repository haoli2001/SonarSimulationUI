#ifndef DEVICESELECT_H
#define DEVICESELECT_H

#include <QDialog>
#include <qvector.h>
#include <qgridlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>


class DeviceSelect : public QDialog
{
	Q_OBJECT

public:
	DeviceSelect(QWidget *parent, int *v, int deviceNum, int* selectCount);
	~DeviceSelect();

private slots:
	void OnPushButtonOk();
	void OnCheckButtonAllIn();

private:
	QVector<QRadioButton*> m_radioButton;
	QPushButton *m_okButton;

	int* m_v;
	int m_deviceNum;
	int *m_selectCount;

	bool isALLIN = false;
	
};

#endif // DEVICESELECT_H
