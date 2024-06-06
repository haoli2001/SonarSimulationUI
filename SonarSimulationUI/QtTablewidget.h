#pragma once

#include <QWidget>
#include "ui_QtTablewidget.h"
#include "commonstruct.h"
#include "ErrorLog.h"

class QtTablewidget : public QWidget
{
	Q_OBJECT

public:
	QtTablewidget(QWidget *parent = Q_NULLPTR);
	~QtTablewidget();
	void show_excel();
	std::vector<FileConfigModInfo>* getFileConfigModInfosPtr();
	ErrorLog* getErrorLogPtr();
	vector<QProgressBar*> p;//定义每一行的进度条 zsr20220302
	bool stop_cliked;//暂停、运行
	bool saveFlag = true;//是否保存

private:
	Ui::QtTablewidget ui;

	std::vector<FileConfigModInfo> fileConfigModInfos;	//读excel得到的配置数据  姬梓遇
	QList<QList<QVariant>> m_datas;////读excel得到的配置数据  赵思茹
	QString filename;//配置文件地址   赵思茹
	QString filename_now = NULL;//更新后的配置文件地址   赵思茹
	QString Model_file = NULL;//单个添加的模型文件地址   赵思茹
	ErrorLog log;//日志类对象

public slots:
	//读取配置文件
	void castVariant2ListListVariant(const QVariant& var, QList<QList<QVariant> >& res);
	void read_excel();
	void parameter_pass();
	//保存配置文件
	void write_excel_save();
	//从excel快速读取至tabel表
	void parameter_updata();//将修改后的配置文件内容更新到内存里
	//将Qvariant转换为QList<QList<QVariant>>
	void Qvariant2listlistVariant(const QVariant& var, QList<QList<QVariant>>& ret);
	//快速将数据写入excel中
	void excel_write();
	//把QList<QList<QVariant> > 转为QVariant,用于快速写入的
	void castListListVariant2Variant(const QList<QList<QVariant> >& cells, QVariant& res);
	//添加一行参数
	void add_row(float start_beta);
	//添加多行参数
	void add_rows();

	void add_Model_file();
	//删除一行参数
	void delete_row();
	//更新每一行的进度条
	void OnUpdateProgressVale(float value, int idx);
	//暂停
	void OnStop();
	//退出
	void closeEvent(QCloseEvent* event);

signals:
	//批量读取配置信息完成后，更新ConfigStruct队列  姬梓遇
	void loadFileConfigModInfo(FileConfigModInfo* fileConfigModInfo, int maxNum, QString excelName);
	//暂停
	void stop_Sig();
	//运行
	void run_Sig();
	void getRunningStatusSig(int &status);
};
