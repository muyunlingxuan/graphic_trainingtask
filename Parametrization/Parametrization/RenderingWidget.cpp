#include "renderingwidget.h"
#include <QKeyEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <iostream>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QTextCodec>
#include <gl/GLU.h>
#include <GL/glut.h>
#include <algorithm>
#include <Eigen/dense>
#include <QOpenGLShaderProgram>
#include "MainWindow.h"
#include "GlobalFunctions.h"
#include "Parameterize_1.h"
#include "Parameterize_2.h"	
#include "Parameterize_3.h"
#include "MiniSurf.h"
#include "MiniSurf_global.h"

RenderingWidget::RenderingWidget(QWidget *parent, MainWindow* mainwindow)
	: QGLWidget(parent), 
	rot_x(0), rot_y(0), 
	ptr_mainwindow_(mainwindow), eye_distance_(5.0),
	has_lighting_(false), is_draw_point_(false), is_draw_edge_(true), is_draw_face_(false), is_draw_texture_(false)
{
	//ptr_arcball_ = new CArcBall(width(), height());
	ptr_mesh_ = new MyMesh();

	is_load_texture_ = false;
	is_draw_axes_ = false;

	eye_goal_[0] = eye_goal_[1] = eye_goal_[2] = 0.0;
	eye_direction_[0] = eye_direction_[1] = 0.0;
	eye_direction_[2] = 10.0;
}

RenderingWidget::~RenderingWidget()
{
	//SafeDelete(ptr_arcball_);
	SafeDelete(ptr_mesh_);
}

void RenderingWidget::initializeGL()
{
	glClearColor(0.3, 0.3, 0.3, 0.0);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1);
	SetLight();
}

void RenderingWidget::resizeGL(int w, int h)
{
	h = (h == 0) ? 1 : h;

	//ptr_arcball_->reSetBound(w, h);

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0, GLdouble(w) / GLdouble(h), 0.001, 1000);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void RenderingWidget::paintGL()
{
	glShadeModel(GL_SMOOTH);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (has_lighting_)
	{
		SetLight();
	}
	else
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	register std::vector<float> eyepos = { eye_distance_* eye_direction_[0] ,
										eye_distance_* eye_direction_[1],
										eye_distance_* eye_direction_[2] };
	gluLookAt(eyepos[0], eyepos[1], eyepos[2],
		eye_goal_[0], eye_goal_[1], eye_goal_[2],
		0.0, 1.0, 0.0);

	glRotatef(rot_x, 0, 1, 0);
	glRotatef(rot_y, 1, 0, 0);
	glPushMatrix();
	Render();
	glPopMatrix();
}

void RenderingWidget::timerEvent(QTimerEvent * e)
{
	updateGL();
}

void RenderingWidget::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		current_position_ = e->pos();
		break;
	case Qt::RightButton:
		current_position_ = e->pos();
		break;
	
	default:
		break;
	}

	updateGL();
}
void RenderingWidget::mouseMoveEvent(QMouseEvent *e)
{
	int dx = 0, dy = 0;
	switch (e->buttons())
	{
		setCursor(Qt::ClosedHandCursor);
	case Qt::LeftButton:
		rot_x += e->x() - current_position_.x();
		rot_y += e->y() - current_position_.y();
		rot_x = rot_x > 360 ? rot_x - 360 : rot_x;
		rot_x = rot_x < -360 ? rot_x + 360 : rot_x;
		rot_y = rot_y > 360 ? rot_y - 360 : rot_y;
		rot_y = rot_y < -360 ? rot_y + 360 : rot_y;
		current_position_ = e->pos();
		break;
	
	case Qt::RightButton:
		eye_goal_[0] -= 4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width());
		eye_goal_[1] += 4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height());
		current_position_ = e->pos();
		break;

	default:
		break;
	}

	updateGL();
}
void RenderingWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		break;
	default:
		break;
	}
	updateGL();
}
void RenderingWidget::mouseReleaseEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		//setCursor(Qt::ArrowCursor);
		rot_x += e->x() - current_position_.x();
		rot_y += e->y() - current_position_.y();
		rot_x = rot_x > 360 ? rot_x - 360 : rot_x;
		rot_x = rot_x < -360 ? rot_x + 360 : rot_x;
		rot_y = rot_y > 360 ? rot_y - 360 : rot_y;
		rot_y = rot_y < -360 ? rot_y + 360 : rot_y;
		current_position_ = e->pos();
		break;

	case Qt::RightButton:
		eye_goal_[0] -= 4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width());
		eye_goal_[1] += 4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height());
		current_position_ = e->pos();
		break;

	
	default:
		break;
	}
}

void RenderingWidget::wheelEvent(QWheelEvent *e)
{
	eye_distance_ -= e->delta()*0.001;
	eye_distance_ = eye_distance_ < 0 ? 0 : eye_distance_;

	updateGL();
}

void RenderingWidget::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}

void RenderingWidget::keyReleaseEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}

void RenderingWidget::Render()
{
	DrawAxes(is_draw_axes_);

	DrawPoints(is_draw_point_);
	DrawEdge(is_draw_edge_);
	DrawFace(is_draw_face_);
	DrawTexture(is_draw_texture_);
}

void RenderingWidget::SetLight()
{
	static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	static GLfloat mat_shininess[] = { 50.0 };
	static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	static GLfloat white_light[] = { 0.8, 0.8, 0.8, 1.0 };
	static GLfloat lmodel_ambient[] = { 0.3, 0.3, 0.3, 1.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void RenderingWidget::SetBackground()
{
	QColor color = QColorDialog::getColor(Qt::white, this, tr("background color"));
	GLfloat r = (color.red()) / 255.0f;
	GLfloat g = (color.green()) / 255.0f;
	GLfloat b = (color.blue()) / 255.0f;
	GLfloat alpha = color.alpha() / 255.0f;
	glClearColor(r, g, b, alpha);
	updateGL();
}

void RenderingWidget::ReadMesh()
{
	QString filename = QFileDialog::
		getOpenFileName(this, tr("Read Mesh"),
			"..", tr("Meshes (*.obj)"));

	if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Read Mesh Failed!")));
		return;
	}
	//中文路径支持
	QTextCodec *code = QTextCodec::codecForName("gd18030");
	QTextCodec::setCodecForLocale(code);

	QByteArray byfilename = filename.toLocal8Bit();
	if (!OpenMesh::IO::read_mesh(*ptr_mesh_, filename.toStdString())) {
		std::cerr << "read_error\n";
		exit(1);
	}
	ptr_mesh_->request_face_normals();
	ptr_mesh_->request_vertex_normals();
	ptr_mesh_->request_halfedge_normals();
	ptr_mesh_->request_vertex_texcoords2D();
	emit(operatorInfo(QString("Read Mesh from") + filename + QString(" Done")));
	updateGL();
}

void RenderingWidget::WriteMesh()
{
	
	QString filename = QFileDialog::
		getSaveFileName(this, tr("Write Mesh"),
			"..", tr("Meshes (*.obj)"));

	if (filename.isEmpty())
		return;

	// write mesh to output.obj
	try
	{
		if (!OpenMesh::IO::write_mesh(*ptr_mesh_, filename.toStdString()))
		{
			std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
		}
	}
	catch (std::exception& x)
	{
		std::cerr << x.what() << std::endl;
	}

	emit(operatorInfo(QString("Write Mesh to ") + filename + QString(" Done")));
}

void RenderingWidget::LoadTexture()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Load Texture"),
		"..", tr("Images(*.bmp *.jpg *.png *.jpeg)"));
	if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Load Texture Failed!")));
		return;
	}

	glGenTextures(1, &texture_[0]);
	QImage tex1, buf;
	if (!buf.load(filename))
	{emit(operatorInfo(QString("Load Texture Failed!")));
		return;
	}

	tex1 = QGLWidget::convertToGLFormat(buf);
	glBindTexture(GL_TEXTURE_2D, texture_[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
		GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());

	is_load_texture_ = true;
	emit(operatorInfo(QString("Load Texture from ") + filename + QString(" Done")));
}

void RenderingWidget::CheckDrawPoint(bool bv)
{
	is_draw_point_ = bv;
	updateGL();
}
void RenderingWidget::CheckDrawEdge(bool bv)
{
	is_draw_edge_ = bv;
	updateGL();
}
void RenderingWidget::CheckDrawFace(bool bv)
{
	is_draw_face_ = bv;
	updateGL();
}
void RenderingWidget::CheckLight(bool bv)
{
	has_lighting_ = bv;
	updateGL();
}
void RenderingWidget::CheckDrawTexture(bool bv)
{
	is_draw_texture_ = bv;
	if (is_draw_texture_)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	updateGL();
}
void RenderingWidget::CheckDrawAxes(bool bV)
{
	is_draw_axes_ = bV;
	updateGL();
}

void RenderingWidget::DrawAxes(bool bV)
{
	if (!bV)
		return;
	//x axis
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.7, 0.0, 0.0);
	glEnd();
	glPushMatrix();
	glTranslatef(0.7, 0, 0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	//y axis
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.0, 0.7, 0.0);
	glEnd();

	glPushMatrix();
	glTranslatef(0.0, 0.7, 0);
	glRotatef(90, -1.0, 0.0, 0.0);
	glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	//z axis
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0.0, 0.0, 0.7);
	glEnd();
	glPushMatrix();
	glTranslatef(0.0, 0, 0.7);
	glutSolidCone(0.02, 0.06, 20, 10);
	glPopMatrix();

	glColor3f(1.0, 1.0, 1.0);
}

void RenderingWidget::DrawPoints(bool bv)
{
	if (!bv || ptr_mesh_ == NULL)
		return;

	//创建点显示列表
	GLuint Vertex_List = glGenLists(1);//新建一个显示的表指针
	glNewList(Vertex_List, GL_COMPILE);//分配空间
	glLineWidth(1.0f);
	glDisable(GL_LIGHTING);
	//qglColor(Qt::red);
	//glBegin(GL_LINES);
	glBegin(GL_POINTS);
	for (MyMesh::VertexIter ve_it = ptr_mesh_->vertices_begin();ve_it != ptr_mesh_->vertices_end(); ++ve_it) {
		glNormal3fv(ptr_mesh_->normal(*ve_it).data());
		glVertex3fv(ptr_mesh_->point(*ve_it).data());
	}//放前后的两个点

	glEnd();
	glEndList();//结束表的绘制

	glCallList(Vertex_List);

}

void RenderingWidget::DrawEdge(bool bv)
{
	if (!bv || ptr_mesh_ == NULL)
		return;

	//创建边显示列表
	GLuint Edge_List = glGenLists(1);//新建一个显示的表指针
	glNewList(Edge_List, GL_COMPILE);//分配空间
	glLineWidth(1.0f);
	glDisable(GL_LIGHTING);
	//qglColor(Qt::red);
	//glBegin(GL_LINES);
	glBegin(GL_LINES);
	for (MyMesh::HalfedgeIter he_it = ptr_mesh_->halfedges_begin(); he_it != ptr_mesh_->halfedges_end(); ++he_it) {
		glVertex3fv(ptr_mesh_->point(ptr_mesh_->from_vertex_handle(*he_it)).data());
		glVertex3fv(ptr_mesh_->point(ptr_mesh_->to_vertex_handle(*he_it)).data());
		glNormal3fv(ptr_mesh_->normal(*he_it).data());
	}//放前后的两个点

	glEnd();
	glEndList();//结束表的绘制

	glCallList(Edge_List);

}

void RenderingWidget::DrawFace(bool bv)
{
	if (!bv || ptr_mesh_ == NULL)
		return;


	//创建边显示列表
	GLuint Edge_List = glGenLists(1);//新建一个显示的表指针
	glNewList(Edge_List, GL_COMPILE);//分配空间
	glLineWidth(1.0f);
	glDisable(GL_LIGHTING);
	//qglColor(Qt::red);
	glBegin(GL_TRIANGLES);
	int i = 0;
	for (MyMesh::FaceIter fc_it = ptr_mesh_->faces_begin(); fc_it != ptr_mesh_->faces_end(); ++fc_it) {
		for (MyMesh::FaceVertexIter fv_it = ptr_mesh_->fv_begin(*fc_it); fv_it != ptr_mesh_->fv_end(*fc_it); ++fv_it) {
			glNormal3fv(ptr_mesh_->normal(*fv_it).data());
			glVertex3fv(ptr_mesh_->point(*fv_it).data());
		}
	}//放前后的两个点

	glEnd();
	glEndList();//结束表的绘制

	glCallList(Edge_List);
}

void RenderingWidget::DrawTexture(bool bv)
{
	if (!bv)
		return;
	if (ptr_mesh_ == NULL || !is_load_texture_)
		return;

	//默认使用球面纹理映射，效果不好
	MyMesh mesh = *ptr_mesh_;
	Parameterize_2 param(&mesh);
	param.setTex(ptr_mesh_);
	glBindTexture(GL_TEXTURE_2D, texture_[0]);
	glBegin(GL_TRIANGLES);

	for (auto f_it = ptr_mesh_->faces_begin(); f_it != ptr_mesh_->faces_end(); ++f_it) {
		MyMesh::FaceVertexIter vf_it = ptr_mesh_->fv_begin(*f_it);
		MyMesh::FaceVertexIter vf_end = ptr_mesh_->fv_end(*f_it);

		do {
			glTexCoord2fv(ptr_mesh_->texcoord2D(*vf_it).data());
			glVertex3fv(ptr_mesh_->point(*vf_it).data());
			glNormal3fv(ptr_mesh_->normal(*vf_it).data());
		} while (++vf_it != vf_end);
	}
	glEnd();
}

void RenderingWidget::MiniSurf1() {
	MiniSurf mini(ptr_mesh_);			//调用极小化操作
	updateGL();
}

void RenderingWidget::MiniSurf2() {
	MiniSurf_global mini(ptr_mesh_);			//调用极小化操作
	updateGL();
}


void RenderingWidget::Parameterize1() {
	Parameterize_1 param1(ptr_mesh_);//调用参数化操作
	Eigen::MatrixXd  B_matrix_1 = param1.B_matrix_;
	updateGL();
}


void RenderingWidget::Parameterize2() {
	Parameterize_2 param(ptr_mesh_);			//调用参数化操作
	updateGL();
}
void RenderingWidget::Parameterize3() {
	Parameterize_3 param(ptr_mesh_);			//调用参数化操作
	updateGL();
}
