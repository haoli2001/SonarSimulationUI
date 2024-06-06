#include "deviceselect.h"



DeviceSelect::~DeviceSelect()
{
	for (int i = 0; i < m_radioButton.size(); i++)
	{
		delete m_radioButton[i];
	}
	delete m_okButton;
}

DeviceSelect::DeviceSelect(QWidget *parent, int* v, int deviceNum, int* selectCount)
	: QDialog(parent), m_okButton(NULL), m_deviceNum(0)
{
	m_v = v;
	m_deviceNum = deviceNum;
	m_selectCount = selectCount;

	QGridLayout *layout = new QGridLayout(this);

	for (int i = 0; i < deviceNum; i++)
	{
		QCheckBox *button = new QCheckBox(this);
		button->setText(QString::number(i) + QString::fromLocal8Bit("卡"));
		button->setObjectName("card" + QString::number(i));
		button->setChecked(false);
		layout->addWidget(button, i / 5 + 1, i % 5, 1, 1);
	}

	if (deviceNum)
	{
		QCheckBox *button = new QCheckBox(this);
		button->setText(QString::fromLocal8Bit("全选"));
		button->setObjectName("all in");
		button->setChecked(false);
		layout->addWidget(button, 0, 0, 1, 1, Qt::AlignLeft);
		connect(button, SIGNAL(clicked()), this, SLOT(OnCheckButtonAllIn()));
	}

	for (int i = 0; i < *selectCount; i++)
	{
		this->findChild<QCheckBox*>("card" + QString::number(m_v[i]))->setChecked(true);
	}

	m_okButton = new QPushButton(this);
	m_okButton->setText(QString::fromLocal8Bit("确认"));
	m_okButton->setObjectName("deviceSelectOkButton");
	layout->addWidget(m_okButton, (deviceNum + 4) / 5 + 1, 1, 1, 3);
	this->setLayout(layout);
	connect(m_okButton, SIGNAL(clicked()), this, SLOT(OnPushButtonOk()));

}

void DeviceSelect::OnPushButtonOk()
{
	int index = 0;
	for (int i = 0; i < m_deviceNum; i++)
	{
		if (this->findChild<QCheckBox*>("card" + QString::number(i))->isChecked())
		{
			m_v[index] = i;
			index++;
		}
	}
	*m_selectCount = index;
	accept();
}

void DeviceSelect::OnCheckButtonAllIn()
{
	if (!isALLIN)
	{
		for (int i = 0; i < m_deviceNum; i++)
		{
			this->findChild<QCheckBox*>("card" + QString::number(i))->setChecked(true);
		}
		isALLIN = true;
	}
	else
	{
		for (int i = 0; i < m_deviceNum; i++)
		{
			this->findChild<QCheckBox*>("card" + QString::number(i))->setChecked(false);
		}
		isALLIN = false;
	}

}
