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
	vector<QProgressBar*> p;//����ÿһ�еĽ����� zsr20220302
	bool stop_cliked;//��ͣ������
	bool saveFlag = true;//�Ƿ񱣴�

private:
	Ui::QtTablewidget ui;

	std::vector<FileConfigModInfo> fileConfigModInfos;	//��excel�õ�����������  ������
	QList<QList<QVariant>> m_datas;////��excel�õ�����������  ��˼��
	QString filename;//�����ļ���ַ   ��˼��
	QString filename_now = NULL;//���º�������ļ���ַ   ��˼��
	QString Model_file = NULL;//������ӵ�ģ���ļ���ַ   ��˼��
	ErrorLog log;//��־�����

public slots:
	//��ȡ�����ļ�
	void castVariant2ListListVariant(const QVariant& var, QList<QList<QVariant> >& res);
	void read_excel();
	void parameter_pass();
	//���������ļ�
	void write_excel_save();
	//��excel���ٶ�ȡ��tabel��
	void parameter_updata();//���޸ĺ�������ļ����ݸ��µ��ڴ���
	//��Qvariantת��ΪQList<QList<QVariant>>
	void Qvariant2listlistVariant(const QVariant& var, QList<QList<QVariant>>& ret);
	//���ٽ�����д��excel��
	void excel_write();
	//��QList<QList<QVariant> > תΪQVariant,���ڿ���д���
	void castListListVariant2Variant(const QList<QList<QVariant> >& cells, QVariant& res);
	//���һ�в���
	void add_row(float start_beta);
	//��Ӷ��в���
	void add_rows();

	void add_Model_file();
	//ɾ��һ�в���
	void delete_row();
	//����ÿһ�еĽ�����
	void OnUpdateProgressVale(float value, int idx);
	//��ͣ
	void OnStop();
	//�˳�
	void closeEvent(QCloseEvent* event);

signals:
	//������ȡ������Ϣ��ɺ󣬸���ConfigStruct����  ������
	void loadFileConfigModInfo(FileConfigModInfo* fileConfigModInfo, int maxNum, QString excelName);
	//��ͣ
	void stop_Sig();
	//����
	void run_Sig();
	void getRunningStatusSig(int &status);
};
