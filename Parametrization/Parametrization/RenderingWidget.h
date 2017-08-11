#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include <QGLWidget>
#include <QEvent>
#include <OpenMesh\Core\IO\MeshIO.hh>
#include  <OpenMesh\Core\Mesh\TriMesh_ArrayKernelT.hh>

using namespace OpenMesh;
typedef TriMesh_ArrayKernelT<>  MyMesh;

class MainWindow;


class RenderingWidget : public QGLWidget
{
	Q_OBJECT

public:
	RenderingWidget(QWidget *parent, MainWindow* mainwindow = 0);
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
	void Parameterize1();
	void Parameterize2();
	void Parameterize3();
	void MiniSurf1();
	void MiniSurf2();


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

public:
	MainWindow					*ptr_mainwindow_;
	//CArcBall					*ptr_arcball_;
	MyMesh 						*ptr_mesh_;

	// Texture
	GLuint						texture_[1];
	bool						is_load_texture_;

	// eye
	GLfloat						eye_distance_;
	std::vector<float> 			eye_goal_ = std::vector<float>(3);
	std::vector<float> 			eye_direction_ = std::vector<float>(3);
	QPoint						current_position_;

	// Render information
	bool						is_draw_point_;
	bool						is_draw_edge_;
	bool						is_draw_face_;
	bool						is_draw_texture_;
	bool						has_lighting_;
	bool						is_draw_axes_;

private:
	int rot_x;
	int rot_y;
};

#endif // RENDERINGWIDGET_H
