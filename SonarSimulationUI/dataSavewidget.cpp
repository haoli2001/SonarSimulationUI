#include "dataSavewidget.h"

dataSavewidget::dataSavewidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("数据存储管理"));

	connect(ui.pushButton_wave, SIGNAL(clicked()), this, SLOT(OnPushbuttonShowWave()));
	//connect(ui.tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OntableView_doubleClicked(const QModelIndex &)));

	sql.DatabaseConnect();
	ui.label_dbconnect->setText(QString::fromLocal8Bit("数据库连接成功"));
	ui.label_dbconnect->setStyleSheet("color:orange;");

	model = new QSqlTableModel(this);
	model->setTable("dataSave");
	model->select();
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	 
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit(" 序号 "));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("   参数配置文件   "));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("   保存时间   "));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("文件保存路径"));
	model->insertColumn(4);//在模型的第4列后面插入一列
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("   查看波形图   "));

	//将数据模型与TableView绑定
	ui.tableView->setModel(model);
	//最后一列插入打开按钮
	for (int i = 0; i < getModelRowCount(); i++)
	{
		QPushButton *btn = new QPushButton(QString::fromLocal8Bit("打开"));
		btn->setProperty("row", i);
		connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
		ui.tableView->setIndexWidget(model->index(i, 4), btn);
	}

	//设置列宽 行高   固定行高自适应列宽
	ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	ui.tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

	ui.tableView->setAlternatingRowColors(true);//隔行变色  
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中时为整行选中     																					  																					 
	//ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不允许修改tableview的内容
}

dataSavewidget::~dataSavewidget()
{
}
//获取数据库内容的行数
int dataSavewidget::getModelRowCount()
{
	//不加这个获取的最大行数为256
	while (model->canFetchMore())
	{
		model->fetchMore();
	}
	return model->rowCount();
}

//查看波形图 通过点击最后一列按钮查看
void dataSavewidget::OnPushbuttonCheck()
{
	QPushButton *btn = (QPushButton *)sender();
	int i = btn->property("row").toInt();

	QAbstractItemModel *Imodel = ui.tableView->model();
	QModelIndex Iindex = Imodel->index(i, 3);//index.row()为算选择的行号。1为所选中行的第一列。。
	QVariant datatemp = Imodel->data(Iindex);
	QString folderpath = datatemp.toString();
	folderpath = folderpath.replace("/", "\\");

	QFileInfo fileInfo(folderpath);

	if (!fileInfo.exists())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("文件不存在"));
		return;
	}

	QString txtName = QFileDialog::getOpenFileName(this, QStringLiteral("选择波形文件"), folderpath, QStringLiteral("TXT(*.txt)"));
	if (txtName.isEmpty())
		return;
	showChart* chart = new showChart(txtName);
	chart->show();
}


//查看波形图  表中不显示最后一列按钮 通过双击单元格进行查看
//void dataSavewidget::OntableView_doubleClicked(const QModelIndex &index)
//{
//	QAbstractItemModel *Imodel = ui.tableView->model();
//	QModelIndex Iindex = Imodel->index(index.row(), 3);//index.row()为算选择的行号。1为所选中行的第一列。。
//	QVariant datatemp = Imodel->data(Iindex);
//	QString folderpath = datatemp.toString();//name即为所选择行的第一列的值。
//	folderpath = folderpath.replace("/", "\\");
//	QString txtName = QFileDialog::getOpenFileName(this, QStringLiteral("选择波形文件"), folderpath, QStringLiteral("TXT(*.txt)"));
//	if (txtName.isEmpty())
//		return;
//	showChart* chart = new showChart(txtName);
//	chart->show();
//}

//查询
void dataSavewidget::OnPushbuttonQuery()
{
	QString id = ui.lineEdit_id->text();
	QString excelname = ui.lineEdit_excelname->text();
	QString time = ui.lineEdit_time->text();
	if (id.isEmpty() && excelname.isEmpty() && time.isEmpty())
	{
		QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("请输入需要查询的内容"));
		return;
	}

	//根据姓名进行筛选， 一定要使用单引号   目前只适用于单信息查询
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
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("查看波形图"));

	ui.tableView->setModel(model);
	ui.tableView->resizeColumnsToContents();

	for (int i = 0; i < getModelRowCount(); i++)
	{
		QPushButton *btn = new QPushButton(QString::fromLocal8Bit("打开"));
		btn->setProperty("row", i);
		connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
		ui.tableView->setIndexWidget(model->index(i, 4), btn);
	}
}

//界面显示
void dataSavewidget::OnPushbuttonDisplay()
{
	
	model->setTable("dataSave");
	model->select();
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);

	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("序号"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("参数配置文件"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("保存时间"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("文件保存路径"));
	model->insertColumn(4);
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("查看波形图"));

	ui.tableView->setModel(model);
	ui.tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	ui.tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

	for (int i = 0; i < getModelRowCount(); i++)
	{
		QPushButton *btn = new QPushButton(QString::fromLocal8Bit("打开"));
		btn->setProperty("row", i);
		connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
		ui.tableView->setIndexWidget(model->index(i, 4), btn);
	}
}

//添加
void dataSavewidget::OnPushbuttonAdd()
{
	//添加空记录
	QSqlRecord record = model->record();
	//获取行号
	int row = getModelRowCount();
	model->insertRecord(row, record);
	QPushButton *btn = new QPushButton(QString::fromLocal8Bit("打开"));
	btn->setProperty("row", row);
	connect(btn, SIGNAL(clicked()), this, SLOT(OnPushbuttonCheck()));
	ui.tableView->setIndexWidget(model->index(row, 4), btn);

}
//删除
void dataSavewidget::OnPushbuttonDelete()
{
	////获取行编辑内容
	//QString name = ui.lineEdit_id->text();
	//QString sql = QString("delete from student where name = '%1'").arg(name);
	////开启一个事务
	//QSqlDatabase::database().transaction();//往后的事务都可以撤销
	//QSqlQuery query;
	//query.exec(sql);

	QItemSelectionModel *smodel = ui.tableView->selectionModel();//取出选中的模型
	QModelIndexList list = smodel->selectedRows();//取出模型的索引
	//删除所有选中的行
	for (int i = 0; i < list.size(); i++)
	{
		model->removeRow(list.at(i).row());
	}
}

//提交修改
void dataSavewidget::OnPushbuttonSubmit()
{
	//确定删除
	//QSqlDatabase::database().commit();
	model->submitAll();
	OnPushbuttonDisplay();
}

//撤销修改
void dataSavewidget::OnPushbuttonCancel()
{
	//回滚，撤销
	//QSqlDatabase::database().rollback();
	model->revertAll();
}

Usersql dataSavewidget::getUsersql()
{
	return sql;
}

//查看波形图槽函数
void dataSavewidget::OnPushbuttonShowWave()
{
	QString txtName = QFileDialog::getOpenFileName(this, QStringLiteral("选择波形文件"), "", QStringLiteral("TXT(*.txt)"));
	if (txtName.isEmpty())
		return;
	chart = new showChart(txtName);
	chart->show();
}

//Enter快捷查询
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