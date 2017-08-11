/********************************************************************************
** Form generated from reading UI file 'RenderingWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RENDERINGWIDGET_H
#define UI_RENDERINGWIDGET_H

#include <QtCore/QVariant>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_RenderingWidget
{
public:

    void setupUi(QGLWidget *RenderingWidget)
    {
        if (RenderingWidget->objectName().isEmpty())
            RenderingWidget->setObjectName(QStringLiteral("RenderingWidget"));
        RenderingWidget->resize(400, 300);

        retranslateUi(RenderingWidget);

        QMetaObject::connectSlotsByName(RenderingWidget);
    } // setupUi

    void retranslateUi(QGLWidget *RenderingWidget)
    {
        RenderingWidget->setWindowTitle(QApplication::translate("RenderingWidget", "RenderingWidget", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class RenderingWidget: public Ui_RenderingWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENDERINGWIDGET_H
