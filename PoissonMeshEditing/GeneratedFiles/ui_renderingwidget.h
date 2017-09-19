/********************************************************************************
** Form generated from reading UI file 'renderingwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RENDERINGWIDGET_H
#define UI_RENDERINGWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_renderingwidget
{
public:

    void setupUi(QWidget *renderingwidget)
    {
        if (renderingwidget->objectName().isEmpty())
            renderingwidget->setObjectName(QStringLiteral("renderingwidget"));
        renderingwidget->resize(400, 300);

        retranslateUi(renderingwidget);

        QMetaObject::connectSlotsByName(renderingwidget);
    } // setupUi

    void retranslateUi(QWidget *renderingwidget)
    {
        renderingwidget->setWindowTitle(QApplication::translate("renderingwidget", "renderingwidget", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class renderingwidget: public Ui_renderingwidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RENDERINGWIDGET_H
