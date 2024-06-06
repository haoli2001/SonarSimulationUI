#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <qopenglvertexarrayobject.h>
#include <qopenglbuffer.h>
#include <qopenglshaderprogram.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qopenglfunctions_3_3_core.h>
#include "camera.hpp"
#include "FluentData.h"
#include "BoxData.h"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
	Q_OBJECT

public:
	OpenGLWidget(QWidget *parent);
	~OpenGLWidget();

	void LoadModel(FluentData &model);

	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

	void SetCurrentAngle(int value);

	void AddColorBar();

	void Clear();

	void UpdatePreTriangleResults(float *data, int);

signals:
	void printInfo(QString);

public slots:
	void OnCheckBoxBox(bool);
	void OnCheckBoxFace(bool);
	void OnCheckBoxLine(bool);
	void OnPushButtonResetViewer();
	void OnRadioButtonAllBox(bool);
	void OnRadioButtonLeafBox(bool);
	void OnSpinBoxSelectAngle(int);
	void OnLoadKDTree(BoxData&);


private:
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void RotateX(float angle);
	void RotateY(float angle);
	void ResetViewport();

	void Value2RGB(float value, float *r, float *g, float *b);
	void Value2RGB_RainBow(float value, float *r, float *g, float *b);

private:
	QOpenGLBuffer *vboTriangleModelNode;        //三角形边线
	QOpenGLBuffer *eboTriangleModelIndex;

	QOpenGLBuffer *vboBoxModelNode;

	QOpenGLBuffer *vboBoxLeafNode;

	QOpenGLBuffer *vboTriangleModelFace;          //三角形面片

	QOpenGLShaderProgram *shaderProgram;

	Camera *camera;

	FluentData modelData;



private:
	bool ctrlDown;
	float viewTranslatex;
	float viewTranslatey;
	QVector2D mousePos;
	//鼠标操作相关变量
	QMatrix4x4 modelMat, viewMat, projectionMat;
	QMatrix4x4 modeltranslateMat;
	QQuaternion rotation;

private:  //标志，是否显示三角面片 包围盒 边线等
	bool showFace;
	bool showLine;
	bool showBox;
	bool showAllBox;
	bool showLeafBox;
private:
	bool initFinished;
	bool modelLoaded;
	bool kdtreeLoaded;

private:
	QWidget *m_colorBar;
	int boxNum;
	int leafBoxNum;
};

#endif // OPENGLWIDGET_H
