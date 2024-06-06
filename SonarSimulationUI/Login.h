#pragma once
#include "ui_Login.h"
#include "usersql.h"
#include "sonarsimulationui.h"

#include <QtWidgets/QWidget>
#include<QSystemTrayIcon>
#include<QMouseEvent>
#include<QPoint>
#include<QStringList>
#include<QDebug>
#include<QMessageBox>
#include<QMenu>
#include<QMovie>
#include<QSize>
#include<QPainter>
#include<qdrawutil.h>

class Login : public QWidget
{
	Q_OBJECT

		Usersql sql;               //���ݿ���
	SonarSimulationUI *w;      //��������

	QMovie *m_movie = NULL;            //��Ӷ�̬ͼ
	QSize  m_size;                     //��̬ͼѹ����С
	//������
	bool ispasswd = false;                //�ж��Ƿ�������
	QStringList infoList;                 //�û��˺�
	QStringList infoPassword;             //�û�����
	QListWidget * m_AccountList = NULL;   //�����б��
	QSignalMapper * myMapper = NULL;
	QVector<int> infoListsign;

	QAction * searchAction = NULL;    //�˺��������ͼ��
	QAction * searchAction_2 = NULL;
	QHBoxLayout *horLayout = NULL;
	QLabel * la = NULL;
	QPushButton * b1 = NULL;

	QString fileName1;


public:
	Login(QWidget *parent = Q_NULLPTR);
	~Login();

	QPoint last;                        //�����϶��ñ���
	bool isPressedWidget = false;
	void mousePressEvent(QMouseEvent *event);  //�����
	void mouseMoveEvent(QMouseEvent *event);   //����ƶ�
	void mouseReleaseEvent(QMouseEvent *event);//����ͷ�
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);

	QStringList GetDirNameList(const QString &strDirpath);

private slots:
	void OntoolButtonMin();       //��С������
	void OntoolButtonClose();     //�رս���
	void OnpushButtonSignin();    //��¼
	void OnpushButtonRegister();  //ע�ᰴť
	void OnpushButtonForget();   //�������밴ť

	void OnComboBoxCurrentIndexChanged(int index); //������ť
	void deleteaccount(int i);

	void OnLineEditAccountChanged(const QString &text);

	//ע��ҳ��
	void OnpushButtonRegister2();
	void OnpushButtonBack();

	//��������ҳ��
	void OnpushButtonSubmit();
	void OnpushButtonBack2();


private:
	Ui::Login ui;
};
