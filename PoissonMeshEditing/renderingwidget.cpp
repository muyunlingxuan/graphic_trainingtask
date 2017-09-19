#include "renderingwidget.h"
#include <QKeyEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <iostream>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QTextCodec>
#include <gl/GLU.h>
#include <gl/glut.h>
#include <algorithm>
 #include <utility>
#include "mainwindow.h"
#include "ArcBall.h"
#include "globalFunctions.h"
#include "HE_mesh/Mesh3D.h"


using std::pair;

RenderingWidget::RenderingWidget(QWidget *parent, MainWindow* mainwindow)
: QGLWidget(parent), ptr_mainwindow_(mainwindow), eye_distance_(5.0),
has_lighting_(false), is_draw_point_(true), is_draw_edge_(false), 
is_draw_face_(false), is_draw_texture_(false), is_selecting_(false), 
is_selecting_fixed_points_(true),is_selecting_editable_points_(false),
is_editing_(false),is_dragging_(false)
{
	ptr_arcball_ = new CArcBall(width(), height());
	ptr_mesh_ = new Mesh3D();

	is_load_texture_ = false;
	is_draw_axes_ = false;

	eye_goal_[0] = eye_goal_[1] = eye_goal_[2] = 0.0;
	eye_direction_[0] = eye_direction_[1] = 0.0;
	eye_direction_[2] = 1.0;
}

RenderingWidget::~RenderingWidget()
{
	SafeDelete(ptr_arcball_);
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

	ptr_arcball_->reSetBound(w, h);

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

	register vec eyepos = eye_distance_*eye_direction_;
	gluLookAt(eyepos[0], eyepos[1], eyepos[2],
		eye_goal_[0], eye_goal_[1], eye_goal_[2],
		0.0, 1.0, 0.0);
	glPushMatrix();

	if (!is_editing_)
	{
		glMultMatrixf(ptr_arcball_->GetBallMatrix());
	}

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
		if (is_selecting_)
		{
			is_dragging_ = true;
			if (is_selecting_fixed_points_) 
			{
				current_index_ = &current_index_fixed_;
			}
			else
			{
				current_index_ = &current_index_editable_;
			}
		}
		else if (is_editing_)
		{
			//ptr_arcball_->MouseDown(e->pos());
			current_position_ = e->pos();
		}
		else
		{
			ptr_arcball_->MouseDown(e->pos());
		}

		break;

	case Qt::MidButton:
		if (is_editing_)
		{
			current_position_ = e->pos();
		}
		else
		{
			current_position_ = e->pos();
		}
		break;
	case Qt::RightButton:
		axis_ = (axis_ + 1) % 3;
		setActiveAxis(axis_);
		break;

	default:
		break;
	}

	start_position_ = e->pos();
	current_position_ = start_position_;

	updateGL();
}

void RenderingWidget::mouseMoveEvent(QMouseEvent *e)
{
	switch (e->buttons())
	{
		setCursor(Qt::ClosedHandCursor);
	case Qt::RightButton:
		/*if (is_editing_ && is_set_axis_) {
			QPoint mouse = e->pos();
			setActiveAxis(mouse);
		}*/
		break;
	case Qt::LeftButton:
		if (is_selecting_)
		{
			current_position_ = e->pos();
			if (!current_index_->empty())
			{
				for (int i = 0; i != current_index_->size(); i++)
				{
					colors_[current_index_->at(i)].r = 1.0;
					colors_[current_index_->at(i)].g = 1.0;
					colors_[current_index_->at(i)].b = 1.0;
				}
			}
			glPushMatrix();
			glMultMatrixf(ptr_arcball_->GetBallMatrix());
			PickVert();
			glPopMatrix();
		}
		else if (is_editing_)
		{
			//ptr_arcball_->MouseMove(e->pos());
			//float * matrix = ptr_arcball_->GetBallMatrix();
			double dx = 10.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width());
			double dy = 10.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height());

			Eigen::Matrix3f rotate_matrix = GetRotateMatrix(axis_,dx,dy);
			Rotate_editable_points(rotate_matrix);
			
			current_position_ = e->pos();

			meshEdit->set_rotate_matrix(rotate_matrix, current_index_editable_,center_);
			meshEdit->Deformation();
		}
		else
		{
			ptr_arcball_->MouseMove(e->pos());
		}
		
		break;
	case Qt::MidButton:
		if (is_editing_)
		{
			double dx = 4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width());
			double dy = 4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height());
			Eigen::Matrix4f matrix;
			matrix << 1, 0, 0, dx,
				0, 1, 0, -dy,
				0, 0, 1, 0,
				0, 0, 0, 1;

			Translate_editable_points(matrix);
			//meshEdit->set_transform_matrix(matrix, current_index_editable_);
			meshEdit->Deformation();
			current_position_ = e->pos();
			GetCenterPoint();
			getReferenceLine(center_);

		}
		else
		{
			eye_goal_[0] -= 4.0*GLfloat(e->x() - current_position_.x()) / GLfloat(width());
			eye_goal_[1] += 4.0*GLfloat(e->y() - current_position_.y()) / GLfloat(height());
			current_position_ = e->pos();
		}
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
	current_position_ = e->pos();
	is_dragging_ = false;

	switch (e->button())
	{
	case Qt::LeftButton:
		if (is_selecting_)
		{
			if (!current_index_editable_.empty() && !current_index_fixed_.empty())
			{
				meshEdit->set_boundary(current_index_editable_, current_index_fixed_);
			}
		}
		else if (is_editing_)
		{
			ptr_arcball_->MouseUp(e->pos());
			
			//float * matrix = ptr_arcball_->GetBallMatrix(); 
			//Transform_editable_points(matrix);

			//meshEdit->set_transform_matrix(ptr_arcball_->GetBallMatrix(), current_index_editable_);
			//meshEdit->Deformation();
		}
		else
		{
			ptr_arcball_->MouseUp(e->pos());
			setCursor(Qt::ArrowCursor);
		}
		break;

	case Qt::RightButton:
		break;
	default:
		break;
	}
}

void RenderingWidget::wheelEvent(QWheelEvent *e)
{
	eye_distance_ += e->delta()*0.001;
	eye_distance_ = eye_distance_ < 0 ? 0 : eye_distance_;

	updateGL();
}

void RenderingWidget::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_X:
		axis_ = 0;
		break;
	case Qt::Key_Y:
		axis_ = 1;
		break;
	case Qt::Key_Z:
		axis_ = 2;
		break;
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

	if(is_selecting_ && is_dragging_)
	{
		DrawXORRect();
	}
}

void RenderingWidget::Reset()
{
	*(ptr_mesh_) = *(mesh_back_);
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
	ptr_mesh_->LoadFromOBJFile(byfilename.data());
	mesh_back_ = new Mesh3D();
	*mesh_back_ = *ptr_mesh_;
	colors_.resize(ptr_mesh_->num_of_vertex_list());
	for (size_t i=0; i<colors_.size(); i++)
	{
		colors_[i].r = 1.0;
		colors_[i].g = 1.0;
		colors_[i].b = 1.0;
	}

	//	m_pMesh->LoadFromOBJFile(filename.toLatin1().data());
	emit(operatorInfo(QString("Read Mesh from") + filename + QString(" Done")));
	emit(meshInfo(ptr_mesh_->num_of_vertex_list(), ptr_mesh_->num_of_edge_list(), ptr_mesh_->num_of_face_list()));
	meshEdit = new poissonMeshEdit(ptr_mesh_);
	updateGL();
}

void RenderingWidget::WriteMesh()
{
	if (ptr_mesh_->num_of_vertex_list() == 0)
	{
		emit(QString("The Mesh is Empty !"));
		return;
	}
	QString filename = QFileDialog::
		getSaveFileName(this, tr("Write Mesh"),
		"..", tr("Meshes (*.obj)"));

	if (filename.isEmpty())
		return;

	ptr_mesh_->WriteToOBJFile(filename.toLatin1().data());

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
	{
		//        QMessageBox::warning(this, tr("Load Fialed!"), tr("Cannot Load Image %1").arg(filenames.at(0)));
		emit(operatorInfo(QString("Load Texture Failed!")));
		return;
		/*
		QImage dummy(128, 128, QImage::Format_ARGB32);
		dummy.fill(Qt::green);
		buf = dummy;
		*/
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

void RenderingWidget::SelectMode(int mode)
{
	if (mode == 0)
	{
		is_selecting_fixed_points_ = true;
		is_selecting_editable_points_ = false;
	}
	else {
		is_selecting_fixed_points_ = false;
		is_selecting_editable_points_ = true;
	}
}

void RenderingWidget::SelectPoint(bool bv)
{
	is_selecting_ = bv;
}

void RenderingWidget::EditPoint(bool bv)
{
	is_editing_ = bv;
	if (is_editing_)
	{
		GetCenterPoint();
		getReferenceLine(center_);

		/*Eigen::Vector3f p(center.at(0), center.at(1), center.at(2));
		Eigen::Matrix4f translate;
		translate << 1, 0, 0, -center.at(0),
			0, 1, 0, -center.at(1),
			0, 0, 1, -center.at(2),
			0, 0, 0, 1;
		std::vector<HE_vert* >* vertexs = ptr_mesh_->get_vertex_list();
		for (int i = 0; i < vertexs->size(); i++)
		{
			HE_vert* vert = vertexs->at(i);
			Eigen::Vector4f position, position_new;
			position << vertexs->at(i)->position().at(0), 
				vertexs->at(i)->position().at(1), 
				vertexs->at(i)->position().at(2),
				1;
			position_new = translate*position;
			vertexs->at(i)->position() = Vec3f(position_new(0), position_new(1), position_new(2));
		}*/
	}
	else {
		center_ = Vec3f(0, 0, 0);

		reference_x_.clear();
		reference_y_.clear();
		reference_z_.clear();
	}
	
//	ptr_arcball_->Place(center, 1);
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
	if (ptr_mesh_->num_of_vertex_list() == 0)
	{
		return;
	}

	const std::vector<HE_vert*>& verts = *(ptr_mesh_->get_vertex_list());
	glBegin(GL_POINTS);
	for (size_t i = 0; i != ptr_mesh_->num_of_vertex_list(); ++i)
	{
		glColor3f(colors_[i].r, colors_[i].g, colors_[i].b);
		glNormal3fv(verts[i]->normal().data());
		glVertex3fv(verts[i]->position().data());
	}

	for (int i = 0; i < reference_x_.size(); i++) {
		glColor3f(color_x_.r, color_x_.g, color_x_.b);
		glNormal3fv(normal_x_);
		glVertex3fv(reference_x_.at(i));
	}
	for (int i = 0; i < reference_y_.size(); i++) {
		glColor3f(color_y_.r, color_y_.g, color_y_.b);
		glNormal3fv(normal_y_);
		glVertex3fv(reference_y_.at(i));
	}
	for (int i = 0; i < reference_z_.size(); i++) {
		glColor3f(color_z_.r, color_z_.g, color_z_.b);
		glNormal3fv(normal_z_);
		glVertex3fv(reference_z_.at(i));
	}
	glEnd();
}

void RenderingWidget::DrawEdge(bool bv)
{
	if (!bv || ptr_mesh_ == NULL)
		return;

	if (ptr_mesh_->num_of_face_list() == 0)
	{
		return;
	}

	const std::vector<HE_face *>& faces = *(ptr_mesh_->get_faces_list());
	for (size_t i = 0; i != faces.size(); ++i)
	{
		glBegin(GL_LINE_LOOP);
		HE_edge *pedge(faces.at(i)->pedge_);
		do
		{
			glNormal3fv(pedge->pvert_->normal().data());
			glVertex3fv(pedge->pvert_->position().data());

			pedge = pedge->pnext_;

		} while (pedge != faces.at(i)->pedge_);
		glEnd();
	}
}

void RenderingWidget::DrawFace(bool bv)
{
	if (!bv || ptr_mesh_ == NULL)
		return;

	if (ptr_mesh_->num_of_face_list() == 0)
	{
		return;
	}

	const std::vector<HE_face *>& faces = *(ptr_mesh_->get_faces_list());

	glBegin(GL_TRIANGLES);

	for (size_t i = 0; i != faces.size(); ++i)
	{
		HE_edge *pedge(faces.at(i)->pedge_);
		do
		{
			glNormal3fv(pedge->pvert_->normal().data());
			glVertex3fv(pedge->pvert_->position().data());

			pedge = pedge->pnext_;

		} while (pedge != faces.at(i)->pedge_);
	}

	glEnd();
}

void RenderingWidget::DrawTexture(bool bv)
{
	if (!bv)
		return;
	if (ptr_mesh_->num_of_face_list() == 0 || !is_load_texture_)
		return;

	//默认使用球面纹理映射，效果不好
	ptr_mesh_->SphereTex();

	const std::vector<HE_face *>& faces = *(ptr_mesh_->get_faces_list());

	glBindTexture(GL_TEXTURE_2D, texture_[0]);
	glBegin(GL_TRIANGLES);
	for (size_t i = 0; i != faces.size(); ++i)
	{
		HE_edge *pedge(faces.at(i)->pedge_);
		do
		{
			/*请在此处绘制纹理，添加纹理坐标即可*/
			glTexCoord2fv(pedge->pvert_->texCoord_.data());
			glNormal3fv(pedge->pvert_->normal().data());
			glVertex3fv(pedge->pvert_->position().data());

			pedge = pedge->pnext_;

		} while (pedge != faces.at(i)->pedge_);
	}

	glEnd();
}

void RenderingWidget::DrawXORRect()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width(), height(),0,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
	glColor3f(1.0, 1.0, 1.0);

	glBegin(GL_LINE_LOOP);
	glVertex2f(start_position_.x(), start_position_.y());
	glVertex2f(current_position_.x(), start_position_.y());
	glVertex2f(current_position_.x(), current_position_.y());
	glVertex2f(start_position_.x(), current_position_.y());
	glEnd();
	glDisable(GL_LOGIC_OP);

	glPopAttrib();
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void RenderingWidget::PickVert()
{
	current_index_->clear();
	QPoint mid = (start_position_ + current_position_)/2;
	mid.setY(height() - mid.y());
	QPoint wid = (start_position_ - current_position_);
	if(wid.x() < 0) wid.setX(-wid.x());
	if(wid.y() < 0) wid.setY(-wid.y());
	if(wid.x() == 0 || wid.y() == 0) return;
	int sz = 5 * ptr_mesh_->num_of_vertex_list();
	GLuint *selectBuf = new GLuint[sz];
	glSelectBuffer(sz, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(-1);
	double mp[16];
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glGetDoublev(GL_PROJECTION_MATRIX, mp);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mid.x(), mid.y(), wid.x(), wid.y(), viewport);
	glMultMatrixd(mp);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	for(int i = 0; i != ptr_mesh_->num_of_vertex_list(); i++)
	{
		glLoadName(i);
		glBegin(GL_POINTS);
		glVertex3fv(ptr_mesh_->get_vertex(i)->position());
		glEnd();
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	long hits = glRenderMode(GL_RENDER);
	vector<pair<double,unsigned int>> H;
	for(long i=0; i < hits; i++)
	{
		H.push_back(pair<double,unsigned int>(selectBuf[i * 4 + 1]/4294967295.0,selectBuf[i * 4 + 3]));
	}
	sort(H.begin(), H.end());
	current_index_->resize(H.size());
	for(int i = 0; i != hits; i++)
	{
		current_index_->at(i) = H[i].second;
		
		
		if (is_selecting_fixed_points_)
		{
			colors_[H[i].second].r = 1.0;
			colors_[H[i].second].g = 0.0;
			colors_[H[i].second].b = 0.0;
		}
		else
		{
			colors_[H[i].second].r = 0.0;
			colors_[H[i].second].g = 1.0;
			colors_[H[i].second].b = 0.0;
		}
	}
	delete [] selectBuf;
}

void RenderingWidget::Translate_editable_points(Eigen::Matrix4f matrix)
{
	for (int i = 0; i < current_index_editable_.size(); i++)
	{
		int index = current_index_editable_.at(i);
		HE_vert *vert = ptr_mesh_->get_vertex(index);
		Vec3f position = vert->position();
		Eigen::Vector4f position_old, position_new, unit;
		position_old << position.at(0), position.at(1), position.at(2),1;
		position_new = matrix*position_old;
		position = Vec3f(position_new(0), position_new(1), position_new(2));
		vert->position() = position;
	}
}

void RenderingWidget::Rotate_editable_points(Eigen::Matrix3f matrix)
{
	/*Eigen::Matrix4f transform;
	for (int i = 0; i < 16; i++)
	{
		int row = i / 4;
		int col = i % 4;
		transform(row, col) = matrix[i];
	}*/

	for (int i = 0; i < current_index_editable_.size(); i++)
	{
		int index = current_index_editable_.at(i);
		HE_vert *vert = ptr_mesh_->get_vertex(index);
		Vec3f position = vert->position();
		Eigen::Vector3f position_old, position_new, center;
		position_old << position.at(0), position.at(1), position.at(2);
		center<< center_.at(0), center_.at(1), center_.at(2);
		position_new = matrix*position_old - matrix*center + center;
		position = Vec3f(position_new(0), position_new(1), position_new(2));
		vert->position() = position;
	}
}

void RenderingWidget::getReferenceLine(Vec3f center)
{
	reference_x_.clear();
	reference_y_.clear();
	reference_z_.clear();

	Eigen::Vector3f x_start(0, 1, 0);
	Eigen::Vector3f y_start(0, 0, 1);
	Eigen::Vector3f z_start(1, 0, 0);

	normal_x_ = Vec3f(1, 0, 0);
	normal_y_ = Vec3f(0, 1, 0);
	normal_z_ = Vec3f(0, 0, 1);

	Vec3f v_x(center.at(0), center.at(1)+1, center.at(2));
	Vec3f v_y(center.at(0), center.at(1), center.at(2)+1);
	Vec3f v_z(center.at(0)+1, center.at(1), center.at(2));
	
	reference_x_.push_back(v_x);
	reference_y_.push_back(v_y);
	reference_z_.push_back(v_z);

	color_x_.r = 0; color_x_.r = 0; color_x_.b = 1;
	color_y_.r = 0; color_y_.r = 0; color_y_.b = 1;
	color_z_.r = 0; color_z_.r = 0; color_z_.b = 1;

	Eigen::Matrix3f transform;
	for (int i = 0; i < 360; i += 1)
	{
		double ang = i / (2 * M_PI);
		transform << 1, 0, 0,
			0, cos(ang), -sin(ang),
			0, sin(ang), cos(ang);
		Eigen::Vector3f x = transform * x_start;
		Vec3f v_x(x(0) + center.at(0), x(1) + center.at(1), x(2) + center.at(2));
		reference_x_.push_back(v_x);
	}

	for (int i = 0; i < 360; i += 1)
	{
		double ang = i / (2 * M_PI);
		transform << cos(ang), 0, sin(ang),
			0, 1, 0,
			-sin(ang), 0, cos(ang);
		Eigen::Vector3f y = transform * y_start;
		Vec3f v_y(y(0) + center.at(0), y(1) + center.at(1), y(2) + center.at(2));
		reference_y_.push_back(v_y);
	}

	for (int i = 0; i < 360; i += 1)
	{
		double ang = i / (2 * M_PI);
		transform << cos(ang), -sin(ang), 0,
			sin(ang), cos(ang), 0,
			0, 0, 1;
		Eigen::Vector3f z = transform * z_start;
		Vec3f v_z(z(0) + center.at(0), z(1) + center.at(1), z(2) + center.at(2));
		reference_z_.push_back(v_z);
	}
	
}

void RenderingWidget::setActiveAxis(QPoint mouse)
{
	double distance_x_min = DBL_MAX;
	double distance_y_min = DBL_MAX;
	double distance_z_min = DBL_MAX;

	color_x_.r = 0; color_x_.r = 0; color_x_.b = 1;
	color_y_.r = 0; color_y_.r = 0; color_y_.b = 1;
	color_z_.r = 0; color_z_.r = 0; color_z_.b = 1;

	for (int i = 0; i < reference_x_.size(); i++)
	{
		Vec3f x = reference_x_.at(i);
		double distance = (x.at(0) - mouse.x())*(x.at(0) - mouse.x()) + (x.at(1) - mouse.y())*(x.at(1) - mouse.y());
		distance_x_min = distance_x_min > distance ? distance : distance_x_min;
	}

	for (int i = 0; i < reference_y_.size(); i++)
	{
		Vec3f y = reference_y_.at(i);
		double distance = (y.at(0) - mouse.x())*(y.at(0) - mouse.x()) + (y.at(1) - mouse.y())*(y.at(1) - mouse.y());
		distance_y_min = distance_y_min > distance ? distance : distance_y_min;
	}

	for (int i = 0; i < reference_z_.size(); i++)
	{
		Vec3f z = reference_z_.at(i);
		double distance = (z.at(0) - mouse.x())*(z.at(0) - mouse.x()) + (z.at(1) - mouse.y())*(z.at(1) - mouse.y());
		distance_z_min = distance_z_min > distance ? distance : distance_z_min;
	}
	
	if (distance_x_min < distance_z_min) {
		if (distance_x_min < distance_y_min) {
			color_x_.r = 1; color_x_.r = 0; color_x_.b = 0;
		}else {
			color_y_.r = 1; color_y_.r = 0; color_y_.b = 0;
		}
	}
	else {
		if (distance_z_min < distance_y_min) {
			color_z_.r = 1; color_z_.r = 0; color_z_.b = 0;
		}
		else {
			color_y_.r = 1; color_y_.r = 0; color_y_.b = 0;
		}
	}
}

void RenderingWidget::setActiveAxis(int axis)
{
	color_x_.r = 0; color_x_.g = 0; color_x_.b = 1;
	color_y_.r = 0; color_y_.g = 0; color_y_.b = 1;
	color_z_.r = 0; color_z_.g = 0; color_z_.b = 1;
	switch (axis)
	{
	case 0:
		color_x_.r = 1; color_x_.g = 0; color_x_.b = 0;
		break;
	case 1:
		color_y_.r = 1; color_y_.g = 0; color_y_.b = 0;
		break;
	case 2:
		color_z_.r = 1; color_z_.g = 0; color_z_.b = 0;
		break;
	default:
		break;
	}
}

Eigen::Matrix3f RenderingWidget::GetRotateMatrix(int axis,double dx, double dy)
{
	Eigen::Matrix3f matrix;
	
	dx = dx  * 2 * M_PI / 360;
	dy = dy  * 2 * M_PI / 360;
	std::cout << dx << "," << dy << std::endl;
	switch (axis) 
	{
	case 0://x-axis
		matrix << 1, 0, 0,
			0, cos(dy), -sin(dy),
			0, sin(dy), cos(dy);
		break;
	case 1://y-axis
		matrix << cos(dx), 0, sin(dx), 
			0, 1, 0, 
			-sin(dx), 0, cos(dx);
		break;
	case 2://z-axis
		matrix << cos(dy), -sin(dy), 0,
			sin(dy), cos(dy), 0,
			0, 0, 1;
		break;
	default:
		matrix << 1, 0, 0,
			0, 1, 0, 
			0, 0, 1;
		break;
	}
	return matrix;
}

void RenderingWidget::GetCenterPoint()
{
	double x_min = DBL_MAX, y_min = DBL_MAX, z_min = DBL_MAX;
	double x_max = -1 * DBL_MAX, y_max = -1 * DBL_MAX, z_max = -1 * DBL_MAX;
	for (int i = 0; i < current_index_editable_.size(); i++)
	{

		HE_vert *vert = ptr_mesh_->get_vertex(current_index_editable_.at(i));
		Vec3f position = vert->position();
		x_min = x_min < position.at(0) ? x_min : position.at(0);
		y_min = y_min < position.at(1) ? y_min : position.at(1);
		z_min = z_min < position.at(2) ? z_min : position.at(2);

		x_max = x_max > position.at(0) ? x_max : position.at(0);
		y_max = y_max > position.at(1) ? y_max : position.at(1);
		z_max = z_max > position.at(2) ? z_max : position.at(2);
	}
	center_ = Vec3f((x_min + x_max) / 2, (y_min + y_max) / 2, (z_min + z_max) / 2);
}