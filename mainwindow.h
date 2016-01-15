#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "manuscript.h"
#include "preferencesdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showAboutBox();
    void test_render();

private:
    Ui::MainWindow *ui;
    Manuscript* manuscript;
    PreferencesDialog * preferencesDialog;

signals:
};

#endif // MAINWINDOW_H