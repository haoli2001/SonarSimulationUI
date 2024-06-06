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
	//QSqlDatabase db;    //连接数据库
	QSqlQuery query;        //查询操作

	QString name;      //主体姓名
	QString account;   //主体账号
	QString password;  //主体密码


public:
	Usersql();

	bool DatabaseConnect(); //连接数据库
	//登录界面
	bool queryUser(const QString &account, const QString &password); //查询账户密码是否正确
	bool queryAccount(const QString &account);  //查询是否有该账户
	void queryUserInfo(const QString &account);    //查询账户信息
	void addUser(const QString &account, const QString &password);   //注册账户
	void reviseUserPassword(const QString &account, const QString &password); //修改密码

	//数据库保存界面
	void addSaveInfo(const QString &id, const QString &FileName, const QString &Filepath);//添加文件存储信息
	void PrintInfo();

	QString getAccount() const;
	void setAccount(const QString &value);
	QString getPassword() const;
	void setPassword(const QString &value);
};

