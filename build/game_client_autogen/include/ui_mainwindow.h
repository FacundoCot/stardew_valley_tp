/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *backgroundwidget;
    QGridLayout *gridLayout;
    QStackedWidget *stackedWidget;
    QWidget *mainPage;
    QGridLayout *gridLayout_2;
    QWidget *internalWidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *titleHorizontalSpacerLeft;
    QLabel *titleLabel;
    QSpacerItem *titleHorizontalSpacerRight;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *buttonLayout;
    QSpacerItem *buttonsHorizontalSpacerLeft;
    QPushButton *createButton;
    QPushButton *continueButton;
    QPushButton *joinButton;
    QPushButton *exitButton;
    QSpacerItem *buttonsHorizontalSpacerRight;
    QWidget *page_2;
    QWidget *customizePage;
    QGridLayout *gridLayout_3;
    QWidget *widget;
    QGridLayout *gridLayout_4;
    QWidget *customizeMenu;
    QVBoxLayout *backButtonLayout1;
    QPushButton *backButton1;
    QSpacerItem *backButtonVSpacer1;
    QVBoxLayout *_hiddenlayout;
    QPushButton *_hiddenbutton;
    QSpacerItem *customMenuHSpacer2;
    QSpacerItem *customMenuHSpacer1;
    QWidget *page_3;
    QSpacerItem *verticalSpacerDown;
    QSpacerItem *verticalSpacerUp;
    QSpacerItem *horizontalSpacerRight;
    QSpacerItem *horizontalSpacerLeft;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        MainWindow->setMinimumSize(QSize(800, 600));
        MainWindow->setStyleSheet(QString::fromUtf8("#MainWindow {\n"
"	border-image: url(\"client_src/menuSprites/background.png\");\n"
"}\n"
"\n"
"QWidget {\n"
"    background: transparent;\n"
"}"));
        backgroundwidget = new QWidget(MainWindow);
        backgroundwidget->setObjectName(QString::fromUtf8("backgroundwidget"));
        backgroundwidget->setAutoFillBackground(false);
        gridLayout = new QGridLayout(backgroundwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(-1, 0, -1, -1);
        stackedWidget = new QStackedWidget(backgroundwidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy);
        mainPage = new QWidget();
        mainPage->setObjectName(QString::fromUtf8("mainPage"));
        sizePolicy.setHeightForWidth(mainPage->sizePolicy().hasHeightForWidth());
        mainPage->setSizePolicy(sizePolicy);
        gridLayout_2 = new QGridLayout(mainPage);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(-1, 0, -1, -1);
        internalWidget = new QWidget(mainPage);
        internalWidget->setObjectName(QString::fromUtf8("internalWidget"));
        sizePolicy.setHeightForWidth(internalWidget->sizePolicy().hasHeightForWidth());
        internalWidget->setSizePolicy(sizePolicy);
        internalWidget->setMinimumSize(QSize(600, 600));
        internalWidget->setAutoFillBackground(false);
        internalWidget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_2 = new QVBoxLayout(internalWidget);
        verticalLayout_2->setSpacing(20);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(10, 0, 10, 20);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, -1, -1, 20);
        titleHorizontalSpacerLeft = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        horizontalLayout->addItem(titleHorizontalSpacerLeft);

        titleLabel = new QLabel(internalWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        sizePolicy.setHeightForWidth(titleLabel->sizePolicy().hasHeightForWidth());
        titleLabel->setSizePolicy(sizePolicy);
        titleLabel->setMinimumSize(QSize(400, 187));
        titleLabel->setMaximumSize(QSize(1080, 505));
        QFont font;
        font.setFamily(QString::fromUtf8("Ubuntu Sans Mono"));
        font.setPointSize(48);
        font.setBold(true);
        titleLabel->setFont(font);
        titleLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
        titleLabel->setFrameShape(QFrame::NoFrame);
        titleLabel->setLineWidth(1);
        titleLabel->setTextFormat(Qt::AutoText);
        titleLabel->setScaledContents(true);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setWordWrap(false);
        titleLabel->setMargin(0);
        titleLabel->setIndent(0);

        horizontalLayout->addWidget(titleLabel);

        titleHorizontalSpacerRight = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        horizontalLayout->addItem(titleHorizontalSpacerRight);


        verticalLayout_2->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred);

        verticalLayout_2->addItem(verticalSpacer);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonLayout->setContentsMargins(10, 40, 10, 45);
        buttonsHorizontalSpacerLeft = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(buttonsHorizontalSpacerLeft);

        createButton = new QPushButton(internalWidget);
        createButton->setObjectName(QString::fromUtf8("createButton"));
        sizePolicy.setHeightForWidth(createButton->sizePolicy().hasHeightForWidth());
        createButton->setSizePolicy(sizePolicy);
        createButton->setMinimumSize(QSize(150, 119));
        createButton->setMaximumSize(QSize(150, 119));
        createButton->setLayoutDirection(Qt::LeftToRight);
        createButton->setStyleSheet(QString::fromUtf8("#createButton {\n"
"	border-image: url(\"client_src/menuSprites/iniciar.png\");\n"
"}\n"
"\n"
"#createButton:hover {\n"
"   border-image: url(\"client_src/menuSprites/iniciarHover.png\");\n"
"}\n"
"\n"
"#createButton:pressed {\n"
"   border-image: url(\"client_src/menuSprites/iniciarPress.png\");\n"
"}\n"
""));
        createButton->setIconSize(QSize(160, 200));
        createButton->setFlat(false);

        buttonLayout->addWidget(createButton);

        continueButton = new QPushButton(internalWidget);
        continueButton->setObjectName(QString::fromUtf8("continueButton"));
        sizePolicy.setHeightForWidth(continueButton->sizePolicy().hasHeightForWidth());
        continueButton->setSizePolicy(sizePolicy);
        continueButton->setMinimumSize(QSize(150, 119));
        continueButton->setMaximumSize(QSize(150, 119));
        continueButton->setLayoutDirection(Qt::LeftToRight);
        continueButton->setStyleSheet(QString::fromUtf8("#continueButton {\n"
"	border-image: url(\"client_src/menuSprites/retomar.png\");\n"
"}\n"
"\n"
"#continueButton:hover {\n"
"   border-image: url(\"client_src/menuSprites/retomarHover.png\");\n"
"}\n"
"\n"
"#continueButton:pressed {\n"
"   border-image: url(\"client_src/menuSprites/retomarPress.png\");\n"
"}\n"
""));
        continueButton->setIconSize(QSize(160, 200));
        continueButton->setAutoDefault(false);

        buttonLayout->addWidget(continueButton);

        joinButton = new QPushButton(internalWidget);
        joinButton->setObjectName(QString::fromUtf8("joinButton"));
        sizePolicy.setHeightForWidth(joinButton->sizePolicy().hasHeightForWidth());
        joinButton->setSizePolicy(sizePolicy);
        joinButton->setMinimumSize(QSize(150, 119));
        joinButton->setMaximumSize(QSize(150, 119));
        joinButton->setStyleSheet(QString::fromUtf8("#joinButton {\n"
"	border-image: url(\"client_src/menuSprites/unirse.png\");\n"
"}\n"
"\n"
"#joinButton:hover {\n"
"   border-image: url(\"client_src/menuSprites/unirseHover.png\");\n"
"}\n"
"\n"
"#joinButton:pressed {\n"
"   border-image: url(\"client_src/menuSprites/unirsePress.png\");\n"
"}"));
        joinButton->setIconSize(QSize(160, 200));

        buttonLayout->addWidget(joinButton);

        exitButton = new QPushButton(internalWidget);
        exitButton->setObjectName(QString::fromUtf8("exitButton"));
        sizePolicy.setHeightForWidth(exitButton->sizePolicy().hasHeightForWidth());
        exitButton->setSizePolicy(sizePolicy);
        exitButton->setMinimumSize(QSize(150, 119));
        exitButton->setMaximumSize(QSize(150, 119));
        exitButton->setStyleSheet(QString::fromUtf8("#exitButton {\n"
"	border-image: url(\"client_src/menuSprites/salir.png\");\n"
"}\n"
"\n"
"#exitButton:hover {\n"
"   border-image: url(\"client_src/menuSprites/salirHover.png\");\n"
"}\n"
"\n"
"#exitButton:pressed {\n"
"   border-image: url(\"client_src/menuSprites/salirPress.png\");\n"
"}\n"
""));
        exitButton->setIconSize(QSize(160, 200));

        buttonLayout->addWidget(exitButton);

        buttonsHorizontalSpacerRight = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(buttonsHorizontalSpacerRight);


        verticalLayout_2->addLayout(buttonLayout);


        gridLayout_2->addWidget(internalWidget, 0, 0, 1, 1);

        stackedWidget->addWidget(mainPage);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        stackedWidget->addWidget(page_2);
        customizePage = new QWidget();
        customizePage->setObjectName(QString::fromUtf8("customizePage"));
        gridLayout_3 = new QGridLayout(customizePage);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(-1, 0, -1, 0);
        widget = new QWidget(customizePage);
        widget->setObjectName(QString::fromUtf8("widget"));
        gridLayout_4 = new QGridLayout(widget);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(-1, 0, -1, 0);
        customizeMenu = new QWidget(widget);
        customizeMenu->setObjectName(QString::fromUtf8("customizeMenu"));
        sizePolicy.setHeightForWidth(customizeMenu->sizePolicy().hasHeightForWidth());
        customizeMenu->setSizePolicy(sizePolicy);
        customizeMenu->setMinimumSize(QSize(500, 515));
        customizeMenu->setMaximumSize(QSize(740, 800));
        customizeMenu->setStyleSheet(QString::fromUtf8("#customizeMenu {\n"
"	border-image: url(\"client_src/menuSprites/woodTemplate.png\");\n"
"}"));

        gridLayout_4->addWidget(customizeMenu, 0, 2, 1, 1);

        backButtonLayout1 = new QVBoxLayout();
        backButtonLayout1->setObjectName(QString::fromUtf8("backButtonLayout1"));
        backButton1 = new QPushButton(widget);
        backButton1->setObjectName(QString::fromUtf8("backButton1"));
        backButton1->setMinimumSize(QSize(90, 50));
        backButton1->setMaximumSize(QSize(90, 50));
        backButton1->setLayoutDirection(Qt::LeftToRight);
        backButton1->setStyleSheet(QString::fromUtf8("#backButton1 {\n"
"   border-image: url(\"client_src/menuSprites/volver.png\");\n"
"}\n"
"\n"
"#backButton1:hover {\n"
"   border-image: url(\"client_src/menuSprites/volverHover.png\");\n"
"}\n"
"\n"
"#backButton1:pressed {\n"
"   border-image: url(\"client_src/menuSprites/volverPress.png\");\n"
"}"));
        backButton1->setIconSize(QSize(100, 70));

        backButtonLayout1->addWidget(backButton1);

        backButtonVSpacer1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        backButtonLayout1->addItem(backButtonVSpacer1);


        gridLayout_4->addLayout(backButtonLayout1, 0, 0, 1, 1);

        _hiddenlayout = new QVBoxLayout();
        _hiddenlayout->setObjectName(QString::fromUtf8("_hiddenlayout"));
        _hiddenlayout->setContentsMargins(0, -1, -1, -1);
        _hiddenbutton = new QPushButton(widget);
        _hiddenbutton->setObjectName(QString::fromUtf8("_hiddenbutton"));
        _hiddenbutton->setEnabled(false);
        _hiddenbutton->setMinimumSize(QSize(90, 50));
        _hiddenbutton->setMaximumSize(QSize(90, 50));
        _hiddenbutton->setStyleSheet(QString::fromUtf8("#_hiddenbutton {\n"
"	background-color: transparent;\n"
"}"));
        _hiddenbutton->setFlat(true);

        _hiddenlayout->addWidget(_hiddenbutton);


        gridLayout_4->addLayout(_hiddenlayout, 0, 5, 1, 1);

        customMenuHSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        gridLayout_4->addItem(customMenuHSpacer2, 0, 3, 1, 1);

        customMenuHSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        gridLayout_4->addItem(customMenuHSpacer1, 0, 1, 1, 1);


        gridLayout_3->addWidget(widget, 0, 0, 1, 1);

        stackedWidget->addWidget(customizePage);
        page_3 = new QWidget();
        page_3->setObjectName(QString::fromUtf8("page_3"));
        stackedWidget->addWidget(page_3);

        gridLayout->addWidget(stackedWidget, 1, 1, 1, 1);

        verticalSpacerDown = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout->addItem(verticalSpacerDown, 3, 1, 1, 1);

        verticalSpacerUp = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

        gridLayout->addItem(verticalSpacerUp, 0, 1, 1, 1);

        horizontalSpacerRight = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacerRight, 1, 2, 1, 1);

        horizontalSpacerLeft = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacerLeft, 1, 0, 1, 1);

        MainWindow->setCentralWidget(backgroundwidget);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        titleLabel->setText(QString());
        createButton->setText(QString());
        continueButton->setText(QString());
        joinButton->setText(QString());
        exitButton->setText(QString());
        backButton1->setText(QString());
        _hiddenbutton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
