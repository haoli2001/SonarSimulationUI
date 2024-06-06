#ifndef EXCELHELPER_H
#define EXCELHELPER_H
#include <QAxObject>
#include <QDir>
#include <QFile>
class ExcelHelper
{
public:
	ExcelHelper();

	bool newExcel(const QString &fileName);
	void appendSheet(const QString &sheetName);
	void setCellValue(int row, int column, const QString &value);
	void setRange(int row, int col, QList<QVariant> &content);
	void saveExcel(const QString &fileName);
	void freeExcel();

	QAxObject *pApplication;
	QAxObject *pWorkBooks;
	QAxObject *pWorkBook;
	QAxObject *pSheets;
	QAxObject *pSheet;
};
#endif //?EXCELHELPER_H

