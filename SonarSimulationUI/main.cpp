#include "sonarsimulationui.h"
#include <QtWidgets/QApplication>
#include <iostream>
//�¾� 20211230
#include "Login.h"

int main(int argc, char *argv[])
{
	std::cout << "hello";
	if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);

	/*SonarSimulationUI w;
	w.setWindowState(Qt::WindowMaximized);
	w.show();*/
	//�¾� 20211230
	/*
	QString qss;
	QFile qssFile(":/Resources/Ubuntu.qss");
	qssFile.open(QFile::ReadOnly);

	if (qssFile.isOpen())
	{
		qss = QLatin1String(qssFile.readAll());
		qApp->setStyleSheet(qss);
		qssFile.close();
	}
	else
		std::cout << "û���ҵ�qss�ļ�" << endl;
	*/
	Login n;
	n.show();
	return a.exec();

}
