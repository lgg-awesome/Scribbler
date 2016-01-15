#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QWheelEvent>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    manuscript = new Manuscript();
    preferencesDialog = new PreferencesDialog();

    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));
    connect(ui->actionTest_rendering, SIGNAL(triggered()),
            this, SLOT(test_render()));
    connect(ui->actionPreferences, SIGNAL(triggered()),
            preferencesDialog, SLOT(exec()));

    //ui->graphicsView->setScene(manuscript);
    //ui->graphicsView->setMaximumSize(manuscript->sheetSize);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete manuscript;
    delete preferencesDialog;
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About"));
    aboutBox.setText(tr("It's Scribbler. I can't say anything esle."));
    aboutBox.exec();
}

void MainWindow::test_render()
{
    ui->graphicsView->renderText(ui->textEdit->toPlainText());
}