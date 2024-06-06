#pragma once

#include<QSqlDatabase>
#include<QtSql/QSqlError>
#include<QSqlQuery>
#include<QString>
#include<QPixmap>
#include<QMessageBox>
#include<QDebug>
#include<QDateTime>

class Usersql
{
private:
	//QSqlDatabase db;    //�������ݿ�
	QSqlQuery query;        //��ѯ����

	QString name;      //��������
	QString account;   //�����˺�
	QString password;  //��������


public:
	Usersql();

	bool DatabaseConnect(); //�������ݿ�
	//��¼����
	bool queryUser(const QString &account, const QString &password); //��ѯ�˻������Ƿ���ȷ
	bool queryAccount(const QString &account);  //��ѯ�Ƿ��и��˻�
	void queryUserInfo(const QString &account);    //��ѯ�˻���Ϣ
	void addUser(const QString &account, const QString &password);   //ע���˻�
	void reviseUserPassword(const QString &account, const QString &password); //�޸�����

	//���ݿⱣ�����
	void addSaveInfo(const QString &id, const QString &FileName, const QString &Filepath);//����ļ��洢��Ϣ
	void PrintInfo();

	QString getAccount() const;
	void setAccount(const QString &value);
	QString getPassword() const;
	void setPassword(const QString &value);
};

