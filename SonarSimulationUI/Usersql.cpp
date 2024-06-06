#include "Usersql.h"

Usersql::Usersql()
{
}

bool Usersql::DatabaseConnect()
{
	//�������ݿ�
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	db.setHostName("127.0.0.1");
	db.setDatabaseName("UserInfo.db");//���ݿ�����
	if (!db.open())
	{
		QMessageBox::warning(0, "warning", db.lastError().text());	// ��ʾ������Ϣ
		return false;
	}
	else
	{
		this->query = (QSqlQuery)db; //���а� �˺����ʹ��query��������ݿ���в���
		
		query.exec("SET NAMES 'Latin1'");
		//����user��¼��dataSave���ݴ洢��
		query.exec("create table user (Account varchar primary key," "Password varchar(255))");
		query.exec("create table dataSave (id int(255)," "ExcelName varchar(255), Time varchar(255), FilePath varchar(255))");

		//���õ�¼�������Ա��Ϣ
		query.prepare("insert into user(Account, Password) values(?, ?)");
		QVariantList accountList;
		accountList << "Administrator";
		QVariantList passwordList;
		passwordList << "000";
		query.addBindValue(accountList);		
		query.addBindValue(passwordList);
		query.execBatch();
	}
}

//��ѯ�˻������� ��¼�� 
bool Usersql::queryUser(const QString &account, const QString &password)
{
	this->query.exec("select * from user");
	while (query.next())
	{
		//�����˻�
		if (account == (this->query.value(0).toString()))
		{
			if (password == (this->query.value(1).toString()))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

//��ѯ�˻�  ע����
bool Usersql::queryAccount(const QString &account)
{
	this->query.exec("select * from user");
	while (query.next())
	{

		if (account == (this->query.value(0).toString()))
		{
			return true;
		}
	}
	return false;
}

//��ȡ�˻���Ϣ
void Usersql::queryUserInfo(const QString &account)
{
	this->query.exec("select * from user");
	while (query.next())
	{
		if (account == (this->query.value(0).toString()))
		{
			this->account = this->query.value(0).toString();    //�˺�	
			this->password = this->query.value(1).toString();   //����
		}
	}
}

//����û� ע����
void Usersql::addUser(const QString &account, const QString &password)
{
	QString str = QString("insert into user (Account, Password) values ('%1','%2')").arg(account, password);
	query.exec(str);
}
//�޸��˻�����  ע����
void Usersql::reviseUserPassword(const QString &account, const QString &password)
{
	QString str = QString("update user set Password = '%1' where Account = '%2'").arg(password).arg(account);
	query.exec(str);
}

//����ļ��洢��Ϣ �����ļ���Ϣ��
void Usersql::addSaveInfo(const QString &id, const QString &FileName, const QString &Filepath)
{
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QString str = QString("insert into dataSave (id,ExcelName,Time,FilePath) values ('%1','%2','%3','%4')").arg(id, FileName, datatime, Filepath);
	this->query.exec(str);
}

void Usersql::PrintInfo()
{
	query.exec("select * from dataSave");
	while (query.next())
	{
		qDebug() << query.value(0).toInt() << query.value(1).toString() << query.value(2).toString() << query.value(3).toString();
	}
}

QString Usersql::getAccount() const
{
	return account;
}
void Usersql::setAccount(const QString &value)
{
	account = value;
}

QString Usersql::getPassword() const
{
	return password;
}
void Usersql::setPassword(const QString &value)
{
	password = value;
}