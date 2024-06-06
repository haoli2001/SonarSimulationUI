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

		Usersql sql;               //数据库类
	SonarSimulationUI *w;      //主界面类

	QMovie *m_movie = NULL;            //添加动态图
	QSize  m_size;                     //动态图压缩大小
	//下拉框
	bool ispasswd = false;                //判断是否是密码
	QStringList infoList;                 //用户账号
	QStringList infoPassword;             //用户密码
	QListWidget * m_AccountList = NULL;   //下拉列表框
	QSignalMapper * myMapper = NULL;
	QVector<int> infoListsign;

	QAction * searchAction = NULL;    //账号密码左侧图标
	QAction * searchAction_2 = NULL;
	QHBoxLayout *horLayout = NULL;
	QLabel * la = NULL;
	QPushButton * b1 = NULL;

	QString fileName1;


public:
	Login(QWidget *parent = Q_NULLPTR);
	~Login();

	QPoint last;                        //窗口拖动用变量
	bool isPressedWidget = false;
	void mousePressEvent(QMouseEvent *event);  //鼠标点击
	void mouseMoveEvent(QMouseEvent *event);   //鼠标移动
	void mouseReleaseEvent(QMouseEvent *event);//鼠标释放
	void paintEvent(QPaintEvent *event);
	void keyPressEvent(QKeyEvent *event);

	QStringList GetDirNameList(const QString &strDirpath);

private slots:
	void OntoolButtonMin();       //最小化界面
	void OntoolButtonClose();     //关闭界面
	void OnpushButtonSignin();    //登录
	void OnpushButtonRegister();  //注册按钮
	void OnpushButtonForget();   //忘记密码按钮

	void OnComboBoxCurrentIndexChanged(int index); //下拉框按钮
	void deleteaccount(int i);

	void OnLineEditAccountChanged(const QString &text);

	//注册页面
	void OnpushButtonRegister2();
	void OnpushButtonBack();

	//忘记密码页面
	void OnpushButtonSubmit();
	void OnpushButtonBack2();


private:
	Ui::Login ui;
};
