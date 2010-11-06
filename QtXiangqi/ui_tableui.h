/********************************************************************************
** Form generated from reading UI file 'tableui.ui'
**
** Created: Sun Apr 11 22:15:38 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TABLEUI_H
#define UI_TABLEUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TableUI
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *replayBeginButton;
    QPushButton *replayPrevButton;
    QPushButton *replayNextButton;
    QPushButton *replayEndButton;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *resetButton;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *TableUI)
    {
        if (TableUI->objectName().isEmpty())
            TableUI->setObjectName(QString::fromUtf8("TableUI"));
        TableUI->resize(560, 700);
        verticalLayout = new QVBoxLayout(TableUI);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(-1, -1, -1, 0);
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(7);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        replayBeginButton = new QPushButton(TableUI);
        replayBeginButton->setObjectName(QString::fromUtf8("replayBeginButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(replayBeginButton->sizePolicy().hasHeightForWidth());
        replayBeginButton->setSizePolicy(sizePolicy);
        replayBeginButton->setMinimumSize(QSize(22, 22));
        replayBeginButton->setMaximumSize(QSize(22, 22));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/go_first.png"), QSize(), QIcon::Normal, QIcon::Off);
        replayBeginButton->setIcon(icon);
        replayBeginButton->setIconSize(QSize(22, 22));
        replayBeginButton->setFlat(true);

        horizontalLayout->addWidget(replayBeginButton);

        replayPrevButton = new QPushButton(TableUI);
        replayPrevButton->setObjectName(QString::fromUtf8("replayPrevButton"));
        sizePolicy.setHeightForWidth(replayPrevButton->sizePolicy().hasHeightForWidth());
        replayPrevButton->setSizePolicy(sizePolicy);
        replayPrevButton->setMinimumSize(QSize(22, 22));
        replayPrevButton->setMaximumSize(QSize(22, 22));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/go_previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        replayPrevButton->setIcon(icon1);
        replayPrevButton->setIconSize(QSize(22, 22));
        replayPrevButton->setFlat(true);

        horizontalLayout->addWidget(replayPrevButton);

        replayNextButton = new QPushButton(TableUI);
        replayNextButton->setObjectName(QString::fromUtf8("replayNextButton"));
        sizePolicy.setHeightForWidth(replayNextButton->sizePolicy().hasHeightForWidth());
        replayNextButton->setSizePolicy(sizePolicy);
        replayNextButton->setMinimumSize(QSize(22, 22));
        replayNextButton->setMaximumSize(QSize(22, 22));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/go_next.png"), QSize(), QIcon::Normal, QIcon::Off);
        replayNextButton->setIcon(icon2);
        replayNextButton->setIconSize(QSize(22, 22));
        replayNextButton->setFlat(true);

        horizontalLayout->addWidget(replayNextButton);

        replayEndButton = new QPushButton(TableUI);
        replayEndButton->setObjectName(QString::fromUtf8("replayEndButton"));
        sizePolicy.setHeightForWidth(replayEndButton->sizePolicy().hasHeightForWidth());
        replayEndButton->setSizePolicy(sizePolicy);
        replayEndButton->setMinimumSize(QSize(22, 22));
        replayEndButton->setMaximumSize(QSize(22, 22));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/go_last.png"), QSize(), QIcon::Normal, QIcon::Off);
        replayEndButton->setIcon(icon3);
        replayEndButton->setIconSize(QSize(22, 22));
        replayEndButton->setFlat(true);

        horizontalLayout->addWidget(replayEndButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        resetButton = new QPushButton(TableUI);
        resetButton->setObjectName(QString::fromUtf8("resetButton"));
        sizePolicy.setHeightForWidth(resetButton->sizePolicy().hasHeightForWidth());
        resetButton->setSizePolicy(sizePolicy);
        resetButton->setMinimumSize(QSize(22, 22));
        resetButton->setMaximumSize(QSize(22, 22));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/reset.png"), QSize(), QIcon::Normal, QIcon::Off);
        resetButton->setIcon(icon4);
        resetButton->setIconSize(QSize(22, 22));
        resetButton->setFlat(true);

        horizontalLayout->addWidget(resetButton);

        horizontalSpacer = new QSpacerItem(302, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(TableUI);

        QMetaObject::connectSlotsByName(TableUI);
    } // setupUi

    void retranslateUi(QWidget *TableUI)
    {
        TableUI->setWindowTitle(QApplication::translate("TableUI", "Form", 0, QApplication::UnicodeUTF8));
        replayBeginButton->setText(QString());
        replayPrevButton->setText(QString());
        replayNextButton->setText(QString());
        replayEndButton->setText(QString());
#ifndef QT_NO_TOOLTIP
        resetButton->setToolTip(QApplication::translate("TableUI", "Reset game", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        resetButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class TableUI: public Ui_TableUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TABLEUI_H
