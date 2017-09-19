#include "MainWindow.h"
#include "ImageWidget.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	setGeometry(300, 150, 800, 450);

	imagewidget_ = new ImageWidget();
	setCentralWidget(imagewidget_);

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	menu_file_->addAction(action_open_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_saveAs_);


	// Add separator in toolbar 
	toolbar_file_->addSeparator();

	toolbar_edit_ = addToolBar(tr("Edit"));
	
	toolbar_edit_->addAction(action_invert_);
	toolbar_edit_->addAction(action_mirror_);
	toolbar_edit_->addAction(action_gray_);
	toolbar_edit_->addAction(action_restore_);
	toolbar_edit_->addAction(action_eraseLiner_);
	toolbar_edit_->addAction(action_warp_);
	toolbar_edit_->addAction(action_IDW_warp_);
	toolbar_edit_->addAction(action_IDW2_warp_);
	toolbar_edit_->addAction(action_RBF_warp_);
	toolbar_edit_->addAction(action_RBF2_warp_);
}


void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::CreateActions()
{
	
	action_open_ = new QAction(QIcon("Resources/images/open.png"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, SIGNAL(triggered()), imagewidget_, SLOT(Open()));



	action_saveAs_ = new QAction(tr("SaveAs"), this);
	action_saveAs_->setStatusTip(tr("save as"));
	connect(action_saveAs_, SIGNAL(triggered()), imagewidget_, SLOT(SaveAs()));

	action_invert_ = new QAction(tr("Inverse"), this);
	action_invert_->setStatusTip(tr("Invert all pixel value in the image"));
	connect(action_invert_, SIGNAL(triggered()), imagewidget_, SLOT(Invert()));

	action_mirror_ = new QAction(tr("Mirror"), this);
	action_mirror_->setStatusTip(tr("Mirror image vertically or horizontally"));
	connect(action_mirror_, SIGNAL(triggered()), imagewidget_, SLOT(Mirror()));

	action_gray_ = new QAction(tr("Grayscale"), this);
	action_gray_->setStatusTip(tr("Gray-scale map"));
	connect(action_gray_, SIGNAL(triggered()), imagewidget_, SLOT(TurnGray()));

	action_restore_ = new QAction(tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, SIGNAL(triggered()), imagewidget_, SLOT(Restore()));

	action_warp_ = new QAction(tr("warp"), this);
	action_warp_->setStatusTip(tr("warp"));
	connect(action_warp_, SIGNAL(triggered()), imagewidget_, SLOT(SetControlPoint()));

	action_IDW_warp_ = new QAction(tr("IDW warp"), this);
	action_IDW_warp_->setStatusTip(tr("IDW warp"));
	connect(action_IDW_warp_, SIGNAL(triggered()), imagewidget_, SLOT(IDW_Warp()));

	action_IDW2_warp_ = new QAction(tr("IDW2 warp"), this);
	action_IDW2_warp_->setStatusTip(tr("IDW2 warp"));
	connect(action_IDW2_warp_, SIGNAL(triggered()), imagewidget_, SLOT(IDW_D_Warp()));

	action_RBF_warp_ = new QAction(tr("RBF warp"), this);
	action_RBF_warp_->setStatusTip(tr("RBF warp"));
	connect(action_RBF_warp_, SIGNAL(triggered()), imagewidget_, SLOT(RBF_Warp()));

	action_RBF2_warp_ = new QAction(tr("RBF2 warp"), this);
	action_RBF2_warp_->setStatusTip(tr("RBF2 warp"));
	connect(action_RBF2_warp_, SIGNAL(triggered()), imagewidget_, SLOT(RBF_T_Warp()));

	action_eraseLiner_ = new QAction(tr("eraseLiner"), this);
	action_eraseLiner_->setStatusTip(tr("eraseLiner"));
	connect(action_eraseLiner_, SIGNAL(triggered()), imagewidget_, SLOT(EraseLines()));

}

