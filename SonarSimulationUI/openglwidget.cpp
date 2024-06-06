#include "openglwidget.h"
#include <qgridlayout.h>
#include <qlabel.h>

OpenGLWidget::OpenGLWidget(QWidget *parent)
	: QOpenGLWidget(parent), ctrlDown(false),
	viewTranslatex(0.0), viewTranslatey(0.0),
	mousePos(0.0, 0.0), modelMat(), viewMat(), projectionMat(),
	modeltranslateMat(), rotation(), showFace(true),
	showLine(true), showBox(true), showAllBox(true),
	showLeafBox(false), initFinished(false), modelLoaded(false), kdtreeLoaded(false)
{

	shaderProgram = new QOpenGLShaderProgram();
	camera = new Camera();
	vboTriangleModelNode = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	eboTriangleModelIndex = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
	vboBoxModelNode = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vboBoxLeafNode = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vboTriangleModelFace = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);


	AddColorBar();

	m_colorBar->setVisible(false);


}

OpenGLWidget::~OpenGLWidget()
{
	delete(vboTriangleModelFace);
	delete(eboTriangleModelIndex);
	delete(vboBoxModelNode);
	delete(vboBoxLeafNode);
	delete(shaderProgram);
	delete(camera);
	delete(vboTriangleModelNode);
	//delete(modelData);
}

void OpenGLWidget::LoadModel(FluentData &model)
{
	m_colorBar->setVisible(false);
	modelData = model;

	vboTriangleModelNode->create();

	if (false == vboTriangleModelNode->bind())
	{
		emit printInfo(QString::fromLocal8Bit("vboTriangleModelNode bind失败！"));
		return;
	}
	vboTriangleModelNode->setUsagePattern(QOpenGLBuffer::StaticDraw);
	vboTriangleModelNode->allocate(&(modelData.getNodev()[0]), modelData.getNodev().size()*sizeof(float));


	eboTriangleModelIndex->create();
	if (false == eboTriangleModelIndex->bind())
	{
		emit printInfo(QString::fromLocal8Bit("eboTriangleModelIndex bind失败！"));
		return;
	}
	eboTriangleModelIndex->setUsagePattern(QOpenGLBuffer::StaticDraw);
	eboTriangleModelIndex->allocate(&(modelData.getFacev()[0]), modelData.getFacev().size()*sizeof(unsigned int));


	vboTriangleModelNode->release();
	eboTriangleModelIndex->release();


	float *data_face = (float*)malloc(sizeof(float)*modelData.getFaceNum() * 18);
	if (data_face == NULL)
	{
		emit printInfo(QString::fromLocal8Bit("模型内存分配失败！"));
		return;
	}
	for (int i = 0; i < modelData.getFaceNum(); i++)
	{
		float value = rand() / (float)RAND_MAX;
		float r, g, b;
		r = 0;
		g = 0;
		b = 0;
	
		data_face[18 * i + 0] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 0] + 0]; //顶点0的值
		data_face[18 * i + 1] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 0] + 1];
		data_face[18 * i + 2] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 0] + 2];
		data_face[18 * i + 3] = r;
		data_face[18 * i + 4] = g;
		data_face[18 * i + 5] = b;
		data_face[18 * i + 6] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 1] + 0]; //顶点1的值
		data_face[18 * i + 7] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 1] + 1];
		data_face[18 * i + 8] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 1] + 2];
		data_face[18 * i + 9] = r;
		data_face[18 * i + 10] = g;
		data_face[18 * i + 11] = b;
		data_face[18 * i + 12] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 2] + 0]; //顶点2的值
		data_face[18 * i + 13] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 2] + 1];
		data_face[18 * i + 14] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 2] + 2];
		data_face[18 * i + 15] = r;
		data_face[18 * i + 16] = g;
		data_face[18 * i + 17] = b;
	}
	
	
	vboTriangleModelFace->create();
	
	if(vboTriangleModelFace->bind() == false)
	{
		emit printInfo(QString::fromLocal8Bit("vboTriangleModelFace bind失败！"));
		return;
	}
	vboTriangleModelFace->setUsagePattern(QOpenGLBuffer::StaticDraw);
	vboTriangleModelFace->allocate(data_face, modelData.getFaceNum() * 18 * sizeof(float));
	vboTriangleModelFace->release();
	
	
	free(data_face);

	modelLoaded = true;
	OnPushButtonResetViewer();
	update();
}

void OpenGLWidget::initializeGL()
{
	initializeOpenGLFunctions();


	glClearColor(0.8, 0.8, 0.9, 1);

	shaderProgram->create();
	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "./shader/vertex_shader"))
	{
		emit printInfo(QString::fromLocal8Bit("vertex shader 文件加载失败！"));
		return;
	}
	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "./shader/fragment_shader"))
	{
		emit printInfo(QString::fromLocal8Bit("fragment shader 文件加载失败！"));
		return;
	}
	if (!shaderProgram->link())
	{
		emit printInfo(QString::fromLocal8Bit("shaderProgram连接错误！"));
		return;
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE); // Enabled by default on some drivers, but not all so always enable to make sure
	//	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);

	initFinished = true;

}

void OpenGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (initFinished == true)
	{
		shaderProgram->bind();
		glLineWidth(0.1);
		viewMat.setToIdentity();
		viewMat = QMatrix4x4(camera->m);

		projectionMat.setToIdentity();
		projectionMat.perspective(60, (GLfloat)width() / (GLfloat)height(), 0.1f, modelData.getMaxScale() * 10);

		modeltranslateMat.setToIdentity();
		float a = -(modelData.getMinX() + modelData.getMaxX()) / 2;
		float b = -(modelData.getMinY() + modelData.getMaxY()) / 2;
		float c = -(modelData.getMinZ() + modelData.getMaxZ()) / 2;
		modeltranslateMat.translate(a, b, c);

		modelMat.setToIdentity();
		modelMat.rotate(rotation);


		shaderProgram->setUniformValue("modeltranslate", modeltranslateMat);
		shaderProgram->setUniformValue("model", modelMat);
		shaderProgram->setUniformValue("view", viewMat);
		shaderProgram->setUniformValue("projection", projectionMat);


		if (showFace == true && modelLoaded == true)
		{
			vboTriangleModelFace->bind();

			shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
			shaderProgram->enableAttributeArray(0);
			shaderProgram->setAttributeBuffer(1, GL_FLOAT, (3 * sizeof(float)), 3, 6 * sizeof(float));
			shaderProgram->enableAttributeArray(1);
			shaderProgram->setUniformValue("drawtype", 2);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, modelData.getFaceNum() * 9);
			vboTriangleModelFace->release();
		}


		if (showLine == true && modelLoaded == true)
		{
			vboTriangleModelNode->bind();
			eboTriangleModelIndex->bind();
			QVector4D color = QVector4D(0, 255, 0, 1);
			shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
			shaderProgram->enableAttributeArray(0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shaderProgram->setUniformValue("drawtype", 1);
			glDrawElements(GL_TRIANGLES, modelData.getFaceNum() * 3, GL_UNSIGNED_INT, 0);

			vboTriangleModelNode->release();
			eboTriangleModelIndex->release();
		}

		if (showBox == true && kdtreeLoaded == true)
		{
			if (showAllBox == true)
			{
				vboBoxModelNode->bind();

				shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
				shaderProgram->enableAttributeArray(0);

				shaderProgram->setUniformValue("drawtype", 3);
				glDrawArrays(GL_LINES, 0, boxNum * 24);
				vboBoxModelNode->release();

			}
			if (showLeafBox == true)
			{
				vboBoxLeafNode->bind();

				shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
				shaderProgram->enableAttributeArray(0);

				
				shaderProgram->setUniformValue("drawtype", 3);
				glDrawArrays(GL_LINES, 0, leafBoxNum * 24);
				vboBoxLeafNode->release();
			}
		}
	}
}

void OpenGLWidget::resizeGL(int width, int height)
{

}

void OpenGLWidget::wheelEvent(QWheelEvent *event)
{
	setFocus();
	QPoint numDegrees = event->angleDelta() / 8;

	if (numDegrees.y() < 0) {

		camera->slide(0, 0, modelData.getMinScale() / 30);
	}
	else if (numDegrees.y() > 0) {

		camera->slide(0, 0, -modelData.getMinScale() / 30);
	}

	this->update();
	event->accept();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		if (!ctrlDown)
		{

			QVector2D newPos = (QVector2D)event->pos();
			QVector2D diff = newPos - mousePos;
			qreal angle = (diff.length()) / 3.6;

			QVector3D rotationAxis = QVector3D(0, diff.y(), diff.x()).normalized();
			rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angle) * rotation;

			mousePos = newPos;


			this->update();

		}
		else
		{
			QVector2D newPos = (QVector2D)event->pos();
			viewTranslatex = (newPos.x() - mousePos.x()) * 5;
			viewTranslatey = (newPos.y() - mousePos.y()) * 5;
			mousePos = newPos;

			camera->slide(-viewTranslatex, viewTranslatey, 0);

			this->update();

		}
	}
	event->accept();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
	setFocus();
	mousePos = QVector2D(event->pos());
	event->accept();
}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
	// 是否按下Ctrl键      特殊按键
	if (event->modifiers() == Qt::ControlModifier)
	{
		setCursor(Qt::ClosedHandCursor);
		ctrlDown = true;

	}
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Control)
	{
		setCursor(Qt::ArrowCursor);
		ctrlDown = false;
	}
}

void OpenGLWidget::OnPushButtonResetViewer()
{
	float a = modelData.getMaxScale() * 2 / 3;
	camera->setCamera(a, 0, 0, 0, 0, 0, 0, 0, 1);
	rotation = QQuaternion();
	repaint();
}

void OpenGLWidget::OnCheckBoxBox(bool value)
{
	showBox = value;
	repaint();
}

void OpenGLWidget::OnCheckBoxFace(bool value)
{
	showFace = value;
	repaint();
}

void OpenGLWidget::OnCheckBoxLine(bool value)
{
	showLine = value;
	repaint();
}

void OpenGLWidget::OnRadioButtonAllBox(bool value)
{
	showAllBox = value;
	showLeafBox = !value;
	repaint();
}

void OpenGLWidget::OnRadioButtonLeafBox(bool value)
{
	showLeafBox = value;
	showAllBox = !value;
	repaint();
}

void OpenGLWidget::OnSpinBoxSelectAngle(int)
{

}

void OpenGLWidget::OnLoadKDTree(BoxData &boxData)
{
	if (boxData.isLoaded() == false)
		return;
	boxNum = boxData.getBoxnum();
	leafBoxNum = boxData.getLeafBoxnum();
	vboBoxModelNode->create();
	vboBoxModelNode->bind();
	vboBoxModelNode->setUsagePattern(QOpenGLBuffer::StaticDraw);
	vboBoxModelNode->allocate(&(boxData.getData()[0]), boxData.getData().size()*sizeof(float));
	vboBoxModelNode->release();

	vboBoxLeafNode->create();
	vboBoxLeafNode->bind();
	vboBoxLeafNode->setUsagePattern(QOpenGLBuffer::StaticDraw);
	vboBoxLeafNode->allocate(&(boxData.getLeafData()[0]), boxData.getLeafData().size()*sizeof(float));
	vboBoxLeafNode->release();

	kdtreeLoaded = true;

	repaint();
}

void OpenGLWidget::Value2RGB(float value, float *r, float *g, float *b)
{
	//250是因为不想让他的最大值和最小值一样的颜色，让最大值为红色，最小值为蓝色
	float H = (1-value) * 250;    
	float S = 1;
	float V = 1;

	int h = H / 60;
	float f = H / 60 - h;
	float p = V*(1 - S);
	float q = V*(1 - f*S);
	float t = V*(1 - (1 - f)*S);

	if (h == 0)
	{
		*r = V; *g = t; *b = p;
	}
	else if (h == 1)
	{
		*r = q; *g = V; *b = p;
	}
	else if (h == 2)
	{
		*r = p; *g = V; *b = t;
	}
	else if (h == 3)
	{
		*r = p; *g = q; *b = V;
	}
	else if (h == 4)
	{
		*r = t; *g = p; *b = V;
	}
	else if (h == 5)
	{
		*r = V; *g = p; *b = q;
	}

}

void OpenGLWidget::Value2RGB_RainBow(float value, float *r, float *g, float *b)
{
	int rainbox_map[256 * 3] = {
		0,	0,	0,
			125,	0,	255,
			121,	0,	255,
			116,	0,	255,
			112,	0,	255,
			108,	0,	255,
			103,	0,	255,
			99,	0,	255,
			95,	0,	255,
			91,	0,	255,
			86,	0,	255,
			82,	0,	255,
			78,	0,	255,
			73,	0,	255,
			69,	0,	255,
			65,	0,	255,
			60,	0,	255,
			56,	0,	255,
			52,	0,	255,
			47,	0,	255,
			43,	0,	255,
			39,	0,	255,
			34,	0,	255,
			30,	0,	255,
			26,	0,	255,
			22,	0,	255,
			17,	0,	255,
			13,	0,	255,
			9,	0,	255,
			4,	0,	255,
			0,	2,	255,
			0,	7,	255,
			0,	11,	255,
			0,	16,	255,
			0,	20,	255,
			0,	25,	255,
			0,	29,	255,
			0,	34,	255,
			0,	38,	255,
			0,	43,	255,
			0,	47,	255,
			0,	52,	255,
			0,	56,	255,
			0,	61,	255,
			0,	65,	255,
			0,	70,	255,
			0,	74,	255,
			0,	79,	255,
			0,	83,	255,
			0,	88,	255,
			0,	92,	255,
			0,	97,	255,
			0,	101,	255,
			0,	106,	255,
			0,	110,	255,
			0,	115,	255,
			0,	119,	255,
			0,	124,	255,
			0,	128,	255,
			0,	133,	255,
			0,	137,	255,
			0,	142,	255,
			0,	146,	255,
			0,	151,	255,
			0,	155,	255,
			0,	160,	255,
			0,	164,	255,
			0,	169,	255,
			0,	173,	255,
			0,	178,	255,
			0,	182,	255,
			0,	187,	255,
			0,	191,	255,
			0,	196,	255,
			0,	200,	255,
			0,	205,	255,
			0,	209,	255,
			0,	214,	255,
			0,	218,	255,
			0,	223,	255,
			0,	227,	255,
			0,	232,	255,
			0,	236,	255,
			0,	241,	255,
			0,	245,	255,
			0,	250,	255,
			0,	255,	255,
			0,	255,	255,
			0,	255,	250,
			0,	255,	245,
			0,	255,	241,
			0,	255,	236,
			0,	255,	231,
			0,	255,	226,
			0,	255,	222,
			0,	255,	217,
			0,	255,	212,
			0,	255,	208,
			0,	255,	203,
			0,	255,	198,
			0,	255,	193,
			0,	255,	189,
			0,	255,	184,
			0,	255,	179,
			0,	255,	175,
			0,	255,	170,
			0,	255,	165,
			0,	255,	160,
			0,	255,	156,
			0,	255,	151,
			0,	255,	146,
			0,	255,	142,
			0,	255,	137,
			0,	255,	132,
			0,	255,	128,
			0,	255,	123,
			0,	255,	118,
			0,	255,	113,
			0,	255,	109,
			0,	255,	104,
			0,	255,	99,
			0,	255,	95,
			0,	255,	90,
			0,	255,	85,
			0,	255,	80,
			0,	255,	76,
			0,	255,	71,
			0,	255,	66,
			0,	255,	62,
			0,	255,	57,
			0,	255,	52,
			0,	255,	47,
			0,	255,	43,
			0,	255,	38,
			0,	255,	33,
			0,	255,	29,
			0,	255,	24,
			0,	255,	19,
			0,	255,	14,
			0,	255,	10,
			0,	255,	5,
			0,	255,	0,
			0,	255,	0,
			3,	255,	0,
			7,	255,	0,
			12,	255,	0,
			16,	255,	0,
			21,	255,	0,
			25,	255,	0,
			30,	255,	0,
			34,	255,	0,
			39,	255,	0,
			43,	255,	0,
			48,	255,	0,
			52,	255,	0,
			57,	255,	0,
			61,	255,	0,
			66,	255,	0,
			70,	255,	0,
			75,	255,	0,
			79,	255,	0,
			84,	255,	0,
			88,	255,	0,
			93,	255,	0,
			97,	255,	0,
			102,	255,	0,
			106,	255,	0,
			111,	255,	0,
			115,	255,	0,
			120,	255,	0,
			124,	255,	0,
			129,	255,	0,
			133,	255,	0,
			137,	255,	0,
			142,	255,	0,
			146,	255,	0,
			151,	255,	0,
			155,	255,	0,
			160,	255,	0,
			164,	255,	0,
			169,	255,	0,
			173,	255,	0,
			178,	255,	0,
			182,	255,	0,
			187,	255,	0,
			191,	255,	0,
			196,	255,	0,
			200,	255,	0,
			205,	255,	0,
			209,	255,	0,
			214,	255,	0,
			218,	255,	0,
			223,	255,	0,
			227,	255,	0,
			232,	255,	0,
			236,	255,	0,
			241,	255,	0,
			245,	255,	0,
			250,	255,	0,
			254,	255,	0,
			255,	252,	0,
			255,	247,	0,
			255,	243,	0,
			255,	238,	0,
			255,	234,	0,
			255,	229,	0,
			255,	225,	0,
			255,	220,	0,
			255,	216,	0,
			255,	211,	0,
			255,	206,	0,
			255,	202,	0,
			255,	197,	0,
			255,	193,	0,
			255,	188,	0,
			255,	184,	0,
			255,	179,	0,
			255,	175,	0,
			255,	170,	0,
			255,	165,	0,
			255,	161,	0,
			255,	156,	0,
			255,	152,	0,
			255,	147,	0,
			255,	143,	0,
			255,	138,	0,
			255,	134,	0,
			255,	129,	0,
			255,	124,	0,
			255,	120,	0,
			255,	115,	0,
			255,	111,	0,
			255,	106,	0,
			255,	102,	0,
			255,	97,	0,
			255,	93,	0,
			255,	88,	0,
			255,	83,	0,
			255,	79,	0,
			255,	74,	0,
			255,	70,	0,
			255,	65,	0,
			255,	61,	0,
			255,	56,	0,
			255,	52,	0,
			255,	47,	0,
			255,	42,	0,
			255,	38,	0,
			255,	33,	0,
			255,	29,	0,
			255,	24,	0,
			255,	20,	0,
			255,	15,	0,
			255,	11,	0,
			255,	6,	0,
			255,	255,	255
	};
	int index = value * 255;
	*r = rainbox_map[index * 3 + 0]/255.0;
	*g = rainbox_map[index * 3 + 1]/255.0;
	*b = rainbox_map[index * 3 + 2]/255.0;

	
	return;

}

void OpenGLWidget::SetCurrentAngle(int value)
{
	QVector3D rotationAxis = QVector3D(0, 0, 1).normalized();
	rotation = QQuaternion::fromAxisAndAngle(rotationAxis, -value);
	
	repaint();
}

void OpenGLWidget::UpdatePreTriangleResults(float *data, int num)
{
	if (modelData.isLoaded() == false)
		return;
	//*************将数据转存一下，避免数据读写访问的冲突***********
	float *data_temp = (float *)malloc(sizeof(float)*num);
	memcpy(data_temp, data, sizeof(float)*num);
	
	//*******计算每一度中的最大最小值*********
	float min_value = 9999999;
	float max_value = -9999999;
	for (int i = 0; i < num; i++)
	{
		if (data_temp[i]>max_value)
			max_value = data_temp[i];
		if (data_temp[i] < min_value && data_temp[i]!=0)
			min_value = data_temp[i];
	}

	m_colorBar->findChild<QLabel*>("min_value")->setText(QString::number(min_value, 'g', 2));
	m_colorBar->findChild<QLabel*>("mid_value")->setText(QString::number((min_value+max_value)/2, 'g', 2));
	m_colorBar->findChild<QLabel*>("max_value")->setText(QString::number(max_value, 'g', 2));
	if (m_colorBar->isVisible() == false)
	{
		m_colorBar->setVisible(true);
	}

	float *data_face = (float*)malloc(sizeof(float)*num * 18);
	if (data_face == NULL)
	{
		emit printInfo(QString::fromLocal8Bit("模型内存分配失败！"));
		return;
	}
	//*********************更新一下渲染数据********************
	for (int i = 0; i < num; i++)
	{
		float value = ((data_temp[i] - min_value) / (max_value-min_value));
		float r, g, b;
		//Value2RGB_RainBow(value, &r, &g, &b);
		Value2RGB(value, &r, &g, &b);
		data_face[18 * i + 0] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 0] + 0]; //顶点0的值
		data_face[18 * i + 1] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 0] + 1];
		data_face[18 * i + 2] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 0] + 2];
		data_face[18 * i + 3] = r;
		data_face[18 * i + 4] = g;
		data_face[18 * i + 5] = b;
		data_face[18 * i + 6] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 1] + 0]; //顶点1的值
		data_face[18 * i + 7] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 1] + 1];
		data_face[18 * i + 8] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 1] + 2];
		data_face[18 * i + 9] = r;
		data_face[18 * i + 10] = g;
		data_face[18 * i + 11] = b;
		data_face[18 * i + 12] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 2] + 0]; //顶点2的值
		data_face[18 * i + 13] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 2] + 1];
		data_face[18 * i + 14] = modelData.getNodev()[3 * modelData.getFacev()[3 * i + 2] + 2];
		data_face[18 * i + 15] = r;
		data_face[18 * i + 16] = g;
		data_face[18 * i + 17] = b;
	}


	vboTriangleModelFace->create();

	if (vboTriangleModelFace->bind() == false)
	{
		emit printInfo(QString::fromLocal8Bit("vboTriangleModelFace bind失败！"));
		return;
	}
	vboTriangleModelFace->setUsagePattern(QOpenGLBuffer::StaticDraw);
	vboTriangleModelFace->allocate(data_face, num * 18 * sizeof(float));
	vboTriangleModelFace->release();


	free(data_face);
	free(data_temp);
	update();
}

void OpenGLWidget::AddColorBar()
{
	m_colorBar = new QWidget(this);
	QGridLayout * g = new QGridLayout(m_colorBar);
	QLabel *bar = new QLabel(m_colorBar);
	bar->setObjectName("bar");
	bar->setText("bar");
	QImage img(20, 361, QImage::Format_RGB888);
	for (int i = 0; i < 361; i++)
	{
		float r, g, b;
		Value2RGB(i / 360.0, &r, &g, &b);
		for (int j = 0; j < 20;j++)
			img.setPixelColor(j, 360 - i, QColor(r*255, g*255, b*255));
	}
	QPixmap pixmap = QPixmap::fromImage(img);
	int height = m_colorBar->height();
	QPixmap fitpixmap = pixmap.scaled(20, height*7, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	bar->setPixmap(fitpixmap);


	QLabel *max_value = new QLabel(m_colorBar);
	max_value->setText("max_value");
	max_value->setObjectName("max_value");
	QLabel *mid_value = new QLabel(m_colorBar);
	mid_value->setText("mid_value");
	mid_value->setObjectName("mid_value");
	QLabel *min_value = new QLabel(m_colorBar);
	min_value->setText("min_value");
	min_value->setObjectName("min_value");
	g->addWidget(bar, 0, 1, 7, 1);
	g->addWidget(max_value, 0, 0, 1, 1);
	g->addWidget(mid_value, 3, 0, 1, 1);
	g->addWidget(min_value, 6, 0, 1, 1);
	m_colorBar->setLayout(g);
	
	QGridLayout *bg = new QGridLayout();
	QLabel *temp = new QLabel();

	
	bg->addWidget(temp,0,0,1,7);
	bg->addWidget(m_colorBar,0,7,1,1);
	this->setLayout(bg);

	
}

void OpenGLWidget::Clear()
{
	ctrlDown = false;
	viewTranslatex = 0.0;
	viewTranslatey = 0;
		
	
	//initFinished = false;
	modelLoaded = false;
	kdtreeLoaded = false;

	modelData.clear();
	//box.clear()

	m_colorBar->setVisible(false);

	repaint();
}
