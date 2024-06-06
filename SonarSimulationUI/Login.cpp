#include "Login.h"

Login::Login(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.stackedWidget->hide();

	//连接数据库
	this->sql.DatabaseConnect();

	//设置账号密码格式
	//ui.lineEdit_account->setValidator(new QRegExpValidator(QRegExp("[0-9]+$"))); //账号只能输入数字
	ui.lineEdit_password->setEchoMode(QLineEdit::Password);

	//窗体风格 取消系统标题
	this->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);//WindowStaysOnTopHint窗口顶置
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);//任务栏使程序最小化
	setAttribute(Qt::WA_TranslucentBackground); //窗体透明

	//加载背景动态图
	m_movie = new QMovie(":/SonarSimulationUI/icon/mian.gif");
	m_size.setWidth(551);
	m_size.setHeight(151);
	m_movie->setScaledSize(m_size);
	ui.label_main->setMovie(m_movie);
	m_movie->start();

	//添加账号密码图标
	searchAction = new QAction(ui.lineEdit_account);
	searchAction->setIcon(QIcon(":/SonarSimulationUI/icon/account.png"));
	ui.lineEdit_account->addAction(searchAction, QLineEdit::LeadingPosition);
	searchAction_2 = new QAction(ui.lineEdit_password);
	searchAction_2->setIcon(QIcon(":/SonarSimulationUI/icon/suo.png"));
	ui.lineEdit_password->addAction(searchAction_2, QLineEdit::LeadingPosition);

	connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxCurrentIndexChanged(int)));
	connect(ui.lineEdit_account, SIGNAL(textChanged(const QString &)), this, SLOT(OnLineEditAccountChanged(const QString &)));

	//获取本地信息
	//获取exe运行目录
	fileName1 = QCoreApplication::applicationDirPath() + "/UserFile";
	//获取目录
	infoList = GetDirNameList(fileName1);
	//加载自定义的下拉列表框
	m_AccountList = new QListWidget(this);
	m_AccountList->setFocusPolicy(Qt::NoFocus);
	ui.comboBox->setModel(m_AccountList->model());
	ui.comboBox->setView(m_AccountList);
	//创建信息
	int infosize = 0;
	myMapper = new QSignalMapper(this);
	for (int i = 0; i < infoList.size(); i++)
	{
		QString local_account = infoList.at(i);
		QString local_passwd;
		QFile file_my(fileName1 + QString("//%1//data.txt").arg(infoList.at(i)));
		if (!file_my.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "文件打开失败" << endl;
		}
		this->ispasswd = false;
		while (!file_my.atEnd())
		{
			infoPassword.append("");
			QByteArray line = file_my.readLine();
			QString str(QString::fromLocal8Bit(line.data()));
			if (this->ispasswd)
			{
				//还应该查看是否有密码存在，如果有则读取，并显示勾中
				infoPassword.insert(i, str);
				local_passwd.append(str);
				this->ispasswd = false;
			}
			else
			{
				this->ispasswd = true;
			}
		}
		horLayout = new QHBoxLayout();
		la = new QLabel(QString("%1").arg(local_account));
		b1 = new QPushButton();
		b1->setFixedSize(32, 32);
		b1->setStyleSheet("QPushButton{background:rgba(200,200,200,0);border-style:solid;border-image: url(:/SonarSimulationUI/icon/delete.png);}"
			"QPushButton:hover{background:rgba(200,200,200,0);border-style:solid;border-image: url(:/SonarSimulationUI/icon/delete2.png);}");
		connect(b1, SIGNAL(clicked(bool)), myMapper, SLOT(map()));
		myMapper->setMapping(b1, infosize);
		infoListsign.append(infosize);
		infosize++;

		horLayout->addWidget(la);//名字账号
		horLayout->addWidget(b1);//删除按钮
		QWidget *widget = new QWidget(this);
		widget->setLayout(horLayout);
		QListWidgetItem * Listitem = new QListWidgetItem(m_AccountList);
		m_AccountList->setItemWidget(Listitem, widget);
	}
	connect(myMapper, SIGNAL(mapped(int)), this, SLOT(deleteaccount(int)));
}

Login::~Login()
{
	delete m_movie;
	delete searchAction;
	delete searchAction_2;
	this->hide();
	this->close();
}

//账户编辑框文本变化
void Login::OnLineEditAccountChanged(const QString &text)
{
	bool isuser = sql.queryAccount(text);//输入内容查询
	if (!isuser)
	{
		ui.lineEdit_password->setText("");
		ui.checkBox_remember->setChecked(false);
	}
}

//获取账户登录信息列表
QStringList Login::GetDirNameList(const QString &strDirpath)
{
	QDir dir(strDirpath);
	QFileInfoList Info_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	QListIterator<QFileInfo> Iterator(Info_list);
	QStringList strDirNameList;
	QFileInfo Info;
	while (Iterator.hasNext())
	{
		Info = Iterator.next();
		if (Info.isDir())
		{
			strDirNameList << QDir(Info.absoluteFilePath()).dirName();
		}
	}
	return strDirNameList;
}

void Login::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
		QPainter painter(this);
	QPixmap pixmap(":/SonarSimulationUI/icon/background.png");
	qDrawBorderPixmap(&painter, this->rect(), QMargins(0, 0, 0, 0), pixmap);
	QRect rect(this->rect().x() + 32, this->rect().y() + 32, this->rect().width() - 64, this->rect().height() - 64);
	painter.fillRect(rect, QColor(255, 255, 255));
}

//鼠标事件 按下 移动 释放
void Login::mousePressEvent(QMouseEvent *event)
{
	//判断鼠标位置是否在comboBox区域 测试好像不需要这个，只用给isPressedWidget赋初值即可
	if (ui.comboBox->underMouse())
	{
		isPressedWidget = false;
	}
	else
	{
		isPressedWidget = true;
		last = event->globalPos();
	}
}
void Login::mouseMoveEvent(QMouseEvent *event)
{
	if (isPressedWidget)
	{
		int dx = event->globalX() - last.x();
		int dy = event->globalY() - last.y();
		last = event->globalPos();
		move(x() + dx, y() + dy);
	}
}
void Login::mouseReleaseEvent(QMouseEvent *event)
{
	int dx = event->globalX() - last.x();
	int dy = event->globalY() - last.y();
	move(x() + dx, y() + dy);
	isPressedWidget = false; // 鼠标松开时，置为false
}

//关闭按钮
void Login::OntoolButtonClose()
{
	QApplication *app;
	app->quit();
	this->close();
}

//最小化按钮
void Login::OntoolButtonMin()
{
	this->showMinimized();
}

//登录按钮
void Login::OnpushButtonSignin()
{
	if (ui.lineEdit_account->text().isEmpty() || ui.lineEdit_password->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("请输入账号或密码!"));
		return;
	}
	bool isuser = sql.queryUser(ui.lineEdit_account->text(), ui.lineEdit_password->text());//输入内容查询
	if (isuser)
	{
		QString useraccount = ui.lineEdit_account->text();
		sql.queryUserInfo(useraccount);
		QString userpassword = "";
		//判断用户是否保存密码
		if (ui.checkBox_remember->isChecked())
		{
			userpassword = sql.getPassword();
		}
		//创建用户文件夹
		QString fileName = QCoreApplication::applicationDirPath() + "/UserFile" + QString("//%1").arg(useraccount);

		//信息保存
		QDir * file = new QDir;
		//文件夹是否存在，若存在则表示信息已经存在，只需要更新内容即可
		bool exist_1 = file->exists(fileName);
		if (exist_1)
		{
			QFile file(fileName + "//data.txt");
			if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
			{
				qDebug() << QString::fromLocal8Bit("txt文件创建成功");
			}
			QTextStream stream(&file);
			//写入
			if (userpassword == "")
				stream << useraccount << "\n";
			else
				stream << useraccount << "\n" << userpassword;
			file.close();
		}
		else
		{   //如果不存在则创建
			bool ok = file->mkpath(fileName);
			if (ok)
			{
				QFile file(fileName + "//data.txt");
				if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
				{
					qDebug() << QString::fromLocal8Bit("txt文件创建成功");
				}
				QTextStream stream(&file);
				if (userpassword == "")
					stream << useraccount << "\n";
				else
					stream << useraccount << "\n" << userpassword;
				file.close();
			}
		}

		this->hide();//隐藏登录窗口

		//初始化主界面
		w = new SonarSimulationUI;
		w->setAttribute(Qt::WA_DeleteOnClose);
		w->setWindowState(Qt::WindowMaximized);
		w->show();
	}
	else
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("帐户名或密码错误，请重新输入!"));
	}
}
//Enter快捷登录
void Login::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
		OnpushButtonSignin();
		break;
	default:
		break;
	}
	QWidget::keyPressEvent(event);
}

//注册账户按钮
void Login::OnpushButtonRegister()
{
	ui.stackedWidget->show();
	ui.stackedWidget->setCurrentIndex(0);
}
//忘记密码按钮
void Login::OnpushButtonForget()
{
	ui.stackedWidget->show();
	ui.stackedWidget->setCurrentIndex(1);
}

//注册页面
void Login::OnpushButtonRegister2()
{
	if (ui.lineEdit_account2->text().isEmpty() || ui.lineEdit_password2->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("账号和密码不能为空！"));
		return;
	}
	QString account = ui.lineEdit_account2->text();
	QString password = ui.lineEdit_password2->text();

	bool isAccount = sql.queryAccount(account);//输入内容查询
	if (isAccount)
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("该账户已存在！"));
	}
	else
	{
		this->sql.addUser(account, password);
		QMessageBox::information(this, "Information", QString::fromLocal8Bit("    注册成功!         "));
	}
}
void Login::OnpushButtonBack()
{
	ui.stackedWidget->hide();
}

//忘记密码页面
void Login::OnpushButtonSubmit()
{
	if (ui.lineEdit_account3->text().isEmpty() || ui.lineEdit_password3->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("账号和密码不能为空！"));
		return;
	}
	QString account = ui.lineEdit_account3->text();
	QString password = ui.lineEdit_password3->text();
	QString newpassword = ui.lineEdit_newpassword->text();

	bool isAccount = sql.queryAccount(account);//查询账户是否存在
	if (!isAccount)
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("该账户不存在！"));
	}
	else
	{
		//修改密码
		if (password != newpassword)
		{
			QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("两次输入密码不一致！"));
			return;
		}
		else
		{
			sql.reviseUserPassword(account, newpassword);
		}

		QMessageBox::information(this, "Information", QString::fromLocal8Bit("    密码修改成功         "));
	}
}
void Login::OnpushButtonBack2()
{
	ui.stackedWidget->hide();
}

//删除下拉键用户信息
void Login::deleteaccount(int i) //传进来的是标记数字
{
	QMessageBox *message = new QMessageBox(QMessageBox::Question, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("是否要删除该账号信息"), QMessageBox::Yes | QMessageBox::No, this);
	int result = message->exec();
	//infoListsign  0 1 2 3  0 1 2 3  0 1 2  1 2 3
	//这里有一个需要注意的点，当初给每个按钮标记的数字是不会变的，但是当我们删除列表中的一个item之后，列表中的其他item会自动排序
	//所以无法单靠当初标记的数字来进行简单删除，我们可以借助一个vector，将标记的数字存放，而正在要删除的值是标记数字的下标
	//如果单靠标记的数字来判断，比如我要删除第一行，就是0，这时它的下标也是0，这没有问题，但是第二次就会出问题，我要删除第二行，当我点击删除
	//其实传回的值应该是1，但是列表自动排序之后，原来的1变成了0，原来的2变了1.
	switch (result)
	{
	case QMessageBox::Yes:
	{
		//删除对应的用户信息文件夹
		int index = infoListsign.indexOf(i);
		QString fileName = fileName1 + QString("//%1").arg(infoList.at(index));
		QDir dir(fileName);
		dir.removeRecursively();

		infoList.removeAt(infoListsign.indexOf(i));
		infoPassword.removeAt(infoListsign.indexOf(i));

		//当前索引是否为删除对象，若是，则使用第一个
		if (infoListsign.indexOf(i) == ui.comboBox->currentIndex())
		{
			if (infoList.length() != 0)
			{
				ui.comboBox->setCurrentIndex(0);
			}
		}
		if (infoList.length() == 0)
		{
			ui.lineEdit_account->setText("");
			ui.lineEdit_password->setText("");
			this->m_AccountList->setItemHidden(this->m_AccountList->item(0), true);
			return;
		}
		//删除账号信息
		QListWidgetItem *item;
		item = this->m_AccountList->takeItem(infoListsign.indexOf(i));
		this->m_AccountList->removeItemWidget(item);
		delete item;
		infoListsign.erase(infoListsign.begin() + infoListsign.indexOf(i));
		break;
	}
	case QMessageBox::No:
		//什么也不做
		break;
	}
}

void Login::OnComboBoxCurrentIndexChanged(int index)
{
	ui.checkBox_remember->setChecked(false);
	ui.lineEdit_password->setText("");
	ui.lineEdit_account->setText(infoList.at(index));
	if (infoPassword.at(index) != "")
	{
		ui.lineEdit_password->setText(infoPassword.at(index));
		ui.checkBox_remember->setChecked(true);
	}
}