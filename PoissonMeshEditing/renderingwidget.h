#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QGLWidget>
#include <QEvent>
#include <vector>
#include "HE_mesh/Vec.h"
#include "poissonMeshEdit.h"
#include <Eigen/dense>

using std::vector;
using trimesh::vec;
using trimesh::point;

class MainWindow;
class CArcBall;
class Mesh3D;

struct color
{
	double r;
	double g;
	double b;
};

class RenderingWidget : public QGLWidget
{
	Q_OBJECT

public:

	RenderingWidget(QWidget *parent, MainWindow* mainwindow=0);
	~RenderingWidget();

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void timerEvent(QTimerEvent *e);

	// mouse events
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent *e);

public:
	void keyPressEvent(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);

signals:
	void meshInfo(int, int, int);
	void operatorInfo(QString);

private:
	void Render();
	void SetLight();

	public slots:
	void SetBackground();
	void ReadMesh();
	void WriteMesh();
	void LoadTexture();
	void SelectPoint(bool bv);
	void SelectMode(int mode);
	void EditPoint(bool bv);

	void Reset();

	void CheckDrawPoint(bool bv);
	void CheckDrawEdge(bool bv);
	void CheckDrawFace(bool bv);
	void CheckLight(bool bv);
	void CheckDrawTexture(bool bv);
	void CheckDrawAxes(bool bv);

private:
	void DrawAxes(bool bv);
	void DrawPoints(bool);
	void DrawEdge(bool);
	void DrawFace(bool);
	void DrawTexture(bool);
	void DrawXORRect();

	void PickVert();

	void getReferenceLine(Vec3f position);
	void Rotate_editable_points(Eigen::Matrix3f );
	void Translate_editable_points(Eigen::Matrix4f );
	void setActiveAxis(QPoint mouse);
	void setActiveAxis(int axis_);
	Eigen::Matrix3f GetRotateMatrix(int axis_, double dx, double dy);
	void GetCenterPoint();

public:
	MainWindow					*ptr_mainwindow_;
	CArcBall					*ptr_arcball_;
	Mesh3D						*ptr_mesh_;

	Mesh3D						*mesh_back_;

	// Texture
	GLuint						texture_[1];
	bool						is_load_texture_;

	// eye
	GLfloat						eye_distance_;
	point						eye_goal_;
	vec							eye_direction_;
	QPoint						current_position_;
	QPoint						start_position_;

	// Render information
	bool						is_draw_point_;
	bool						is_draw_edge_;
	bool						is_draw_face_;
	bool						is_draw_texture_;
	bool						has_lighting_;
	bool						is_draw_axes_;

	// Interactive
	bool						is_selecting_;
	bool						is_dragging_;
	bool						is_selecting_fixed_points_;
	bool						is_selecting_editable_points_;
	bool						is_editing_;

	// Interactive
	vector<int>					*current_index_;
	vector<color>				colors_;

	vector<int>					current_index_editable_;
	vector<int>					current_index_fixed_;

	poissonMeshEdit				*meshEdit;

	vector<Vec3f>				reference_x_;
	vector<Vec3f>				reference_y_;
	vector<Vec3f>				reference_z_;

	Vec3f						normal_x_;
	Vec3f						normal_y_;
	Vec3f						normal_z_;

	color						color_x_;
	color						color_y_;
	color						color_z_;
	int							axis_;
	bool						is_set_axis_;

	Vec3f						center_;

private:
	
};

#endif // RENDERINGWIDGET_H
