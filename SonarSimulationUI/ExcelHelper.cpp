
#include "ExcelHelper.h"

ExcelHelper::ExcelHelper()
{

}

//�½�Execl�ļ�
bool ExcelHelper::newExcel(const QString &fileName)
{
	pApplication = new QAxObject();
	if (!pApplication->setControl("Excel.Application")) //����Excel�ؼ�
	{
		return false;
	}
    pApplication->dynamicCall("SetVisible(bool)", false); //false����ʾ����
    pApplication->setProperty("DisplayAlerts", false); //����ʾ�κξ�����Ϣ��
    pWorkBooks = pApplication->querySubObject("Workbooks");
    QFile file(fileName);
    if(file.exists()){
        pWorkBook = pWorkBooks->querySubObject("Open(const QString &)", fileName);
    }
    else{
        pWorkBooks->dynamicCall("Add");
        pWorkBook = pApplication->querySubObject("ActiveWorkBook");
    }
    //Ĭ����һ��sheet
    pSheets = pWorkBook->querySubObject("Sheets");
    pSheet = pSheets->querySubObject("Item(int)", 1);
	return true;
}

//����1��Worksheet������Ҫע�⣺Ĭ����һ��sheet����Ҫ��ʱ���ٵ����������
void ExcelHelper::appendSheet(const QString &sheetName)
{
    int cnt = 1;
    QAxObject *pLastSheet = pSheets->querySubObject("Item(int)", cnt);
    pSheets->querySubObject("Add(QVariant)", pLastSheet->asVariant());
    pSheet = pSheets->querySubObject("Item(int)", cnt);
    pLastSheet->dynamicCall("Move(QVariant)", pSheet->asVariant());
    pSheet->setProperty("Name", sheetName);
}

//��Excel��Ԫ����д������ 
//PS:�����漰�� �������ݸ�ʽ������
void ExcelHelper::setCellValue(int row, int column, const QString &value)
{
    QAxObject *pRange = pSheet->querySubObject("Cells(int,int)", row, column);
    pRange->dynamicCall("Value", value);
    //���ݾ���
    pRange->setProperty("HorizontalAlignment", -4108);
    pRange->setProperty("VerticalAlignment", -4108);
       // pRange->setProperty("RowHeight", 50); //���õ�Ԫ���и�
       // pRange->setProperty("ColumnWidth", 30); //���õ�Ԫ���п�
       // pRange->setProperty("HorizontalAlignment", -4108); //����루xlLeft����-4131 ���У�xlCenter����-4108 �Ҷ��루xlRight����-4152
       // pRange->setProperty("VerticalAlignment", -4108); //�϶��루xlTop��-4160 ���У�xlCenter����-4108 �¶��루xlBottom����-4107
       // pRange->setProperty("WrapText", true); //���ݹ��࣬�Զ�����
       // pRange->dynamicCall("ClearContents()"); //��յ�Ԫ������
       // QAxObject* interior =pRange->querySubObject("Interior");
       // interior->setProperty("Color", QColor(0, 255, 0)); //���õ�Ԫ�񱳾�ɫ����ɫ��
       // QAxObject* border = cell->querySubObject("Borders");
       // border->setProperty("Color", QColor(0, 0, 255)); //���õ�Ԫ��߿�ɫ����ɫ��
       // QAxObject *font = cell->querySubObject("Font"); //��ȡ��Ԫ������
       // font->setProperty("Name", QStringLiteral("���Ĳ���")); //���õ�Ԫ������
       // font->setProperty("Bold", true); //���õ�Ԫ������Ӵ�
       // font->setProperty("Size", 20); //���õ�Ԫ�������С
       // font->setProperty("Italic", true); //���õ�Ԫ������б��
       // font->setProperty("Underline", 2); //���õ�Ԫ���»���
       // font->setProperty("Color", QColor(255, 0, 0)); //���õ�Ԫ��������ɫ����ɫ��
    if(row == 1){
    //�Ӵ�
        QAxObject *font = pRange->querySubObject("Font"); //��ȡ��Ԫ������
        font->setProperty("Bold",true); //���õ�Ԫ������Ӵ�
    }
}

void ExcelHelper::setRange(int row, int col, QList<QVariant> &content)
{
	char c = 'A' + col - 1;
	QString s(c);
	QString A = "A" + QString::number(1) + ":" + s + QString::number(row);
	QAxObject *range = pSheet->querySubObject("Range(QString)", A);

	range->setProperty("Value", content);
	
}

//����Excel
void ExcelHelper::saveExcel(const QString &fileName)
{
    pWorkBook->dynamicCall("SaveAs(const QString &)",
    QDir::toNativeSeparators(fileName));
}

//�ͷ�Excel
void ExcelHelper::freeExcel()
{
    if (pApplication != NULL){
        pApplication->dynamicCall("Quit()");
        delete pApplication;
        pApplication = NULL;
    }
}

