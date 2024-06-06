#include "dataSavewidget.h"

dataSavewidget::dataSavewidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("���ݴ洢����"));

	connect(ui.pushButton_wave, SIGNAL(clicked()), this, SLOT(OnPushbuttonShowWave()));
	//connect(ui.tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OntableView_doubleClicked(const QModelIndex &)));

	sql.DatabaseConnect();
	ui.label_dbconnect->setText(QString::fromLocal8Bit("���ݿ����ӳɹ�"));
	ui.label_dbconnect->setStyleSheet("color:orange;");

	model = new QSqlTableModel(this);
	model->setTable("dataSave");
	model->select();
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	 
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit(" ��� "));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("   ���������ļ�   "));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("   ����ʱ��   "));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("�ļ�����·��"));
	model->insertColumn(4);//��ģ�͵ĵ�4�к������һ��
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("   �鿴����ͼ   "));

	//������ģ����TableView��
	ui.tableView->setModel(model);
	//���һ�в���򿪰�ť
	for (int i = 0; i < getModelRowCount(); i++)
	{
		QPushButton *btn = new QPushButton(QString::fromLocal8Bit("��"));
		btn->setProperty("row", i);
		connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
		ui.tableView->setIndexWidget(model->index(i, 4), btn);
	}

	//�����п� �и�   �̶��и�����Ӧ�п�
	ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	ui.tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

	ui.tableView->setAlternatingRowColors(true);//���б�ɫ  
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ��ʱΪ����ѡ��     																					  																					 
	//ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//�������޸�tableview������
}

dataSavewidget::~dataSavewidget()
{
}
//��ȡ���ݿ����ݵ�����
int dataSavewidget::getModelRowCount()
{
	//���������ȡ���������Ϊ256
	while (model->canFetchMore())
	{
		model->fetchMore();
	}
	return model->rowCount();
}

//�鿴����ͼ ͨ��������һ�а�ť�鿴
void dataSavewidget::OnPushbuttonCheck()
{
	QPushButton *btn = (QPushButton *)sender();
	int i = btn->property("row").toInt();

	QAbstractItemModel *Imodel = ui.tableView->model();
	QModelIndex Iindex = Imodel->index(i, 3);//index.row()Ϊ��ѡ����кš�1Ϊ��ѡ���еĵ�һ�С���
	QVariant datatemp = Imodel->data(Iindex);
	QString folderpath = datatemp.toString();
	folderpath = folderpath.replace("/", "\\");

	QFileInfo fileInfo(folderpath);

	if (!fileInfo.exists())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("�ļ�������"));
		return;
	}

	QString txtName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ�����ļ�"), folderpath, QStringLiteral("TXT(*.txt)"));
	if (txtName.isEmpty())
		return;
	showChart* chart = new showChart(txtName);
	chart->show();
}


//�鿴����ͼ  ���в���ʾ���һ�а�ť ͨ��˫����Ԫ����в鿴
//void dataSavewidget::OntableView_doubleClicked(const QModelIndex &index)
//{
//	QAbstractItemModel *Imodel = ui.tableView->model();
//	QModelIndex Iindex = Imodel->index(index.row(), 3);//index.row()Ϊ��ѡ����кš�1Ϊ��ѡ���еĵ�һ�С���
//	QVariant datatemp = Imodel->data(Iindex);
//	QString folderpath = datatemp.toString();//name��Ϊ��ѡ���еĵ�һ�е�ֵ��
//	folderpath = folderpath.replace("/", "\\");
//	QString txtName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ�����ļ�"), folderpath, QStringLiteral("TXT(*.txt)"));
//	if (txtName.isEmpty())
//		return;
//	showChart* chart = new showChart(txtName);
//	chart->show();
//}

//��ѯ
void dataSavewidget::OnPushbuttonQuery()
{
	QString id = ui.lineEdit_id->text();
	QString excelname = ui.lineEdit_excelname->text();
	QString time = ui.lineEdit_time->text();
	if (id.isEmpty() && excelname.isEmpty() && time.isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("��������Ҫ��ѯ������"));
		return;
	}

	//������������ɸѡ�� һ��Ҫʹ�õ�����   Ŀǰֻ�����ڵ���Ϣ��ѯ
	if (!id.isEmpty())
	{
		model->setFilter(QString("id = '%1'").arg(id));
	}
	else if (!excelname.isEmpty())
	{
		model->setFilter(QString("excelname LIKE '%1%'").arg(excelname));
	}
	else if (!time.isEmpty())
	{
		model->setFilter(QString("time LIKE '%1%'").arg(time));
	}

	model->select();

	model->insertColumn(4);
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("�鿴����ͼ"));

	ui.tableView->setModel(model);
	ui.tableView->resizeColumnsToContents();

	for (int i = 0; i < getModelRowCount(); i++)
	{
		QPushButton *btn = new QPushButton(QString::fromLocal8Bit("��"));
		btn->setProperty("row", i);
		connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
		ui.tableView->setIndexWidget(model->index(i, 4), btn);
	}
}

//������ʾ
void dataSavewidget::OnPushbuttonDisplay()
{
	
	model->setTable("dataSave");
	model->select();
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);

	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("���"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("���������ļ�"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("����ʱ��"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("�ļ�����·��"));
	model->insertColumn(4);
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("�鿴����ͼ"));

	ui.tableView->setModel(model);
	ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	ui.tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

	for (int i = 0; i < getModelRowCount(); i++)
	{
		QPushButton *btn = new QPushButton(QString::fromLocal8Bit("��"));
		btn->setProperty("row", i);
		connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
		ui.tableView->setIndexWidget(model->index(i, 4), btn);
	}
}

//���
void dataSavewidget::OnPushbuttonAdd()
{
	//��ӿռ�¼
	QSqlRecord record = model->record();
	//��ȡ�к�
	int row = getModelRowCount();
	model->insertRecord(row, record);
	QPushButton *btn = new QPushButton(QString::fromLocal8Bit("��"));
	btn->setProperty("row", row);
	connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
	ui.tableView->setIndexWidget(model->index(row, 4), btn);

}
//ɾ��
void dataSavewidget::OnPushbuttonDelete()
{
	////��ȡ�б༭����
	//QString name = ui.lineEdit_id->text();
	//QString sql = QString("delete from student where name = '%1'").arg(name);
	////����һ������
	//QSqlDatabase::database().transaction();//��������񶼿��Գ���
	//QSqlQuery query;
	//query.exec(sql);

	QItemSelectionModel *smodel = ui.tableView->selectionModel();//ȡ��ѡ�е�ģ��
	QModelIndexList list = smodel->selectedRows();//ȡ��ģ�͵�����
	//ɾ������ѡ�е���
	for (int i = 0; i < list.size(); i++)
	{
		model->removeRow(list.at(i).row());
	}
}

//�ύ�޸�
void dataSavewidget::OnPushbuttonSubmit()
{
	//ȷ��ɾ��
	//QSqlDatabase::database().commit();
	model->submitAll();
	OnPushbuttonDisplay();
}

//�����޸�
void dataSavewidget::OnPushbuttonCancel()
{
	//�ع�������
	//QSqlDatabase::database().rollback();
	model->revertAll();
}

Usersql dataSavewidget::getUsersql()
{
	return sql;
}

//�鿴����ͼ�ۺ���
void dataSavewidget::OnPushbuttonShowWave()
{
	QString txtName = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ�����ļ�"), "", QStringLiteral("TXT(*.txt)"));
	if (txtName.isEmpty())
		return;
	chart = new showChart(txtName);
	chart->show();
}

//Enter��ݲ�ѯ
void dataSavewidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Return:
		OnPushbuttonQuery();
		break;
	default:
		break;
	}
	QWidget::keyPressEvent(event);
}