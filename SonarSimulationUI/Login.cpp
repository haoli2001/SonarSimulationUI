#include "Login.h"

Login::Login(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.stackedWidget->hide();

	//�������ݿ�
	this->sql.DatabaseConnect();

	//�����˺������ʽ
	//ui.lineEdit_account->setValidator(new QRegExpValidator(QRegExp("[0-9]+$"))); //�˺�ֻ����������
	ui.lineEdit_password->setEchoMode(QLineEdit::Password);

	//������ ȡ��ϵͳ����
	this->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);//WindowStaysOnTopHint���ڶ���
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);//������ʹ������С��
	setAttribute(Qt::WA_TranslucentBackground); //����͸��

	//���ر�����̬ͼ
	m_movie = new QMovie(":/SonarSimulationUI/icon/mian.gif");
	m_size.setWidth(551);
	m_size.setHeight(151);
	m_movie->setScaledSize(m_size);
	ui.label_main->setMovie(m_movie);
	m_movie->start();

	//����˺�����ͼ��
	searchAction = new QAction(ui.lineEdit_account);
	searchAction->setIcon(QIcon(":/SonarSimulationUI/icon/account.png"));
	ui.lineEdit_account->addAction(searchAction, QLineEdit::LeadingPosition);
	searchAction_2 = new QAction(ui.lineEdit_password);
	searchAction_2->setIcon(QIcon(":/SonarSimulationUI/icon/suo.png"));
	ui.lineEdit_password->addAction(searchAction_2, QLineEdit::LeadingPosition);

	connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxCurrentIndexChanged(int)));
	connect(ui.lineEdit_account, SIGNAL(textChanged(const QString &)), this, SLOT(OnLineEditAccountChanged(const QString &)));

	//��ȡ������Ϣ
	//��ȡexe����Ŀ¼
	fileName1 = QCoreApplication::applicationDirPath() + "/UserFile";
	//��ȡĿ¼
	infoList = GetDirNameList(fileName1);
	//�����Զ���������б��
	m_AccountList = new QListWidget(this);
	m_AccountList->setFocusPolicy(Qt::NoFocus);
	ui.comboBox->setModel(m_AccountList->model());
	ui.comboBox->setView(m_AccountList);
	//������Ϣ
	int infosize = 0;
	myMapper = new QSignalMapper(this);
	for (int i = 0; i < infoList.size(); i++)
	{
		QString local_account = infoList.at(i);
		QString local_passwd;
		QFile file_my(fileName1 + QString("//%1//data.txt").arg(infoList.at(i)));
		if (!file_my.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "�ļ���ʧ��" << endl;
		}
		this->ispasswd = false;
		while (!file_my.atEnd())
		{
			infoPassword.append("");
			QByteArray line = file_my.readLine();
			QString str(QString::fromLocal8Bit(line.data()));
			if (this->ispasswd)
			{
				//��Ӧ�ò鿴�Ƿ���������ڣ���������ȡ������ʾ����
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

		horLayout->addWidget(la);//�����˺�
		horLayout->addWidget(b1);//ɾ����ť
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

//�˻��༭���ı��仯
void Login::OnLineEditAccountChanged(const QString &text)
{
	bool isuser = sql.queryAccount(text);//�������ݲ�ѯ
	if (!isuser)
	{
		ui.lineEdit_password->setText("");
		ui.checkBox_remember->setChecked(false);
	}
}

//��ȡ�˻���¼��Ϣ�б�
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

//����¼� ���� �ƶ� �ͷ�
void Login::mousePressEvent(QMouseEvent *event)
{
	//�ж����λ���Ƿ���comboBox���� ���Ժ�����Ҫ�����ֻ�ø�isPressedWidget����ֵ����
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
	isPressedWidget = false; // ����ɿ�ʱ����Ϊfalse
}

//�رհ�ť
void Login::OntoolButtonClose()
{
	QApplication *app;
	app->quit();
	this->close();
}

//��С����ť
void Login::OntoolButtonMin()
{
	this->showMinimized();
}

//��¼��ť
void Login::OnpushButtonSignin()
{
	if (ui.lineEdit_account->text().isEmpty() || ui.lineEdit_password->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("�������˺Ż�����!"));
		return;
	}
	bool isuser = sql.queryUser(ui.lineEdit_account->text(), ui.lineEdit_password->text());//�������ݲ�ѯ
	if (isuser)
	{
		QString useraccount = ui.lineEdit_account->text();
		sql.queryUserInfo(useraccount);
		QString userpassword = "";
		//�ж��û��Ƿ񱣴�����
		if (ui.checkBox_remember->isChecked())
		{
			userpassword = sql.getPassword();
		}
		//�����û��ļ���
		QString fileName = QCoreApplication::applicationDirPath() + "/UserFile" + QString("//%1").arg(useraccount);

		//��Ϣ����
		QDir * file = new QDir;
		//�ļ����Ƿ���ڣ����������ʾ��Ϣ�Ѿ����ڣ�ֻ��Ҫ�������ݼ���
		bool exist_1 = file->exists(fileName);
		if (exist_1)
		{
			QFile file(fileName + "//data.txt");
			if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
			{
				qDebug() << QString::fromLocal8Bit("txt�ļ������ɹ�");
			}
			QTextStream stream(&file);
			//д��
			if (userpassword == "")
				stream << useraccount << "\n";
			else
				stream << useraccount << "\n" << userpassword;
			file.close();
		}
		else
		{   //����������򴴽�
			bool ok = file->mkpath(fileName);
			if (ok)
			{
				QFile file(fileName + "//data.txt");
				if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
				{
					qDebug() << QString::fromLocal8Bit("txt�ļ������ɹ�");
				}
				QTextStream stream(&file);
				if (userpassword == "")
					stream << useraccount << "\n";
				else
					stream << useraccount << "\n" << userpassword;
				file.close();
			}
		}

		this->hide();//���ص�¼����

		//��ʼ��������
		w = new SonarSimulationUI;
		w->setAttribute(Qt::WA_DeleteOnClose);
		w->setWindowState(Qt::WindowMaximized);
		w->show();
	}
	else
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("�ʻ����������������������!"));
	}
}
//Enter��ݵ�¼
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

//ע���˻���ť
void Login::OnpushButtonRegister()
{
	ui.stackedWidget->show();
	ui.stackedWidget->setCurrentIndex(0);
}
//�������밴ť
void Login::OnpushButtonForget()
{
	ui.stackedWidget->show();
	ui.stackedWidget->setCurrentIndex(1);
}

//ע��ҳ��
void Login::OnpushButtonRegister2()
{
	if (ui.lineEdit_account2->text().isEmpty() || ui.lineEdit_password2->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("�˺ź����벻��Ϊ�գ�"));
		return;
	}
	QString account = ui.lineEdit_account2->text();
	QString password = ui.lineEdit_password2->text();

	bool isAccount = sql.queryAccount(account);//�������ݲ�ѯ
	if (isAccount)
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("���˻��Ѵ��ڣ�"));
	}
	else
	{
		this->sql.addUser(account, password);
		QMessageBox::information(this, "Information", QString::fromLocal8Bit("    ע��ɹ�!         "));
	}
}
void Login::OnpushButtonBack()
{
	ui.stackedWidget->hide();
}

//��������ҳ��
void Login::OnpushButtonSubmit()
{
	if (ui.lineEdit_account3->text().isEmpty() || ui.lineEdit_password3->text().isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("�˺ź����벻��Ϊ�գ�"));
		return;
	}
	QString account = ui.lineEdit_account3->text();
	QString password = ui.lineEdit_password3->text();
	QString newpassword = ui.lineEdit_newpassword->text();

	bool isAccount = sql.queryAccount(account);//��ѯ�˻��Ƿ����
	if (!isAccount)
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("���˻������ڣ�"));
	}
	else
	{
		//�޸�����
		if (password != newpassword)
		{
			QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("�����������벻һ�£�"));
			return;
		}
		else
		{
			sql.reviseUserPassword(account, newpassword);
		}

		QMessageBox::information(this, "Information", QString::fromLocal8Bit("    �����޸ĳɹ�         "));
	}
}
void Login::OnpushButtonBack2()
{
	ui.stackedWidget->hide();
}

//ɾ���������û���Ϣ
void Login::deleteaccount(int i) //���������Ǳ������
{
	QMessageBox *message = new QMessageBox(QMessageBox::Question, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("�Ƿ�Ҫɾ�����˺���Ϣ"), QMessageBox::Yes | QMessageBox::No, this);
	int result = message->exec();
	//infoListsign  0 1 2 3  0 1 2 3  0 1 2  1 2 3
	//������һ����Ҫע��ĵ㣬������ÿ����ť��ǵ������ǲ����ģ����ǵ�����ɾ���б��е�һ��item֮���б��е�����item���Զ�����
	//�����޷�����������ǵ����������м�ɾ�������ǿ��Խ���һ��vector������ǵ����ִ�ţ�������Ҫɾ����ֵ�Ǳ�����ֵ��±�
	//���������ǵ��������жϣ�������Ҫɾ����һ�У�����0����ʱ�����±�Ҳ��0����û�����⣬���ǵڶ��ξͻ�����⣬��Ҫɾ���ڶ��У����ҵ��ɾ��
	//��ʵ���ص�ֵӦ����1�������б��Զ�����֮��ԭ����1�����0��ԭ����2����1.
	switch (result)
	{
	case QMessageBox::Yes:
	{
		//ɾ����Ӧ���û���Ϣ�ļ���
		int index = infoListsign.indexOf(i);
		QString fileName = fileName1 + QString("//%1").arg(infoList.at(index));
		QDir dir(fileName);
		dir.removeRecursively();

		infoList.removeAt(infoListsign.indexOf(i));
		infoPassword.removeAt(infoListsign.indexOf(i));

		//��ǰ�����Ƿ�Ϊɾ���������ǣ���ʹ�õ�һ��
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
		//ɾ���˺���Ϣ
		QListWidgetItem *item;
		item = this->m_AccountList->takeItem(infoListsign.indexOf(i));
		this->m_AccountList->removeItemWidget(item);
		delete item;
		infoListsign.erase(infoListsign.begin() + infoListsign.indexOf(i));
		break;
	}
	case QMessageBox::No:
		//ʲôҲ����
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