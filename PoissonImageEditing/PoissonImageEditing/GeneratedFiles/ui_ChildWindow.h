/********************************************************************************
** Form generated from reading UI file 'ChildWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHILDWINDOW_H
#define UI_CHILDWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChildWindow
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ChildWindow)
    {
        if (ChildWindow->objectName().isEmpty())
            ChildWindow->setObjectName(QStringLiteral("ChildWindow"));
        ChildWindow->resize(400, 300);
        menuBar = new QMenuBar(ChildWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        ChildWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ChildWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ChildWindow->addToolBar(mainToolBar);
        centralWidget = new QWidget(ChildWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        ChildWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ChildWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ChildWindow->setStatusBar(statusBar);

        retranslateUi(ChildWindow);

        QMetaObject::connectSlotsByName(ChildWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ChildWindow)
    {
        ChildWindow->setWindowTitle(QApplication::translate("ChildWindow", "ChildWindow", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ChildWindow: public Ui_ChildWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHILDWINDOW_H
