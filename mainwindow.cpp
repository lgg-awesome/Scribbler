#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    preferencesDialog = new PreferencesDialog();

    connect(ui->actionAbout_Scribbler, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));
    connect(ui->actionLicenses_and_Credits, SIGNAL(triggered()),
            this, SLOT(showLicensesBox()));
    connect(ui->actionLoad_Font, SIGNAL(triggered()),
            this, SLOT(loadFont()));
    connect(ui->actionRender, SIGNAL(triggered()),
            this, SLOT(render()));
    connect(ui->actionPreferences, SIGNAL(triggered()),
            preferencesDialog, SLOT(exec()));
    connect(preferencesDialog, SIGNAL(settingsChanged()),
            ui->svgView, SLOT(loadSettingsFromFile()));
    connect(ui->actionSave_All_Sheets, SIGNAL(triggered()),
            this, SLOT(saveAllSheets()));
    connect(ui->actionSave_Current_Sheet_as, SIGNAL(triggered()),
            this, SLOT(saveSheet()));
    connect(ui->actionPrint_Current_Sheet, SIGNAL(triggered()),
            this, SLOT(printSheet()));

    connect(ui->actionShow_ToolBar, SIGNAL(triggered(bool)),
            ui->toolBar, SLOT(setVisible(bool)));
    connect(ui->toolBar, SIGNAL(visibilityChanged(bool)),
            ui->actionShow_ToolBar, SLOT(setChecked(bool)));

    connect(ui->toolBar->addAction(QIcon("://render.ico"),"Render"), SIGNAL(triggered(bool)),
            this, SLOT(render()));
    connect(ui->toolBar->addAction(QIcon("://printer.ico"),"Print Current Sheet"), SIGNAL(triggered(bool)),
            this, SLOT(printSheet()));
    connect(ui->toolBar->addAction(QIcon("://save.ico"),"Save Current Sheet as Image"), SIGNAL(triggered(bool)),
            this, SLOT(saveSheet()));
    ui->toolBar->addSeparator();
    connect(ui->toolBar->addAction(QIcon("://right.ico"), "Next Sheet"), SIGNAL(triggered(bool)),
            this, SLOT(renderNextSheet()));
    connect(ui->toolBar->addAction(QIcon("://left.ico"), "Previous Sheet"), SIGNAL(triggered(bool)),
            this, SLOT(renderPreviousSheet()));

    ui->toolBar->actions()[4]->setDisabled(true);
    ui->toolBar->actions()[5]->setDisabled(true);
    sheetPointers.push_back(0);
    currentSheetNumber = 0;
    version = "0.2 alpha";

    preferencesDialog->loadSettingsFromFile();

    /* This is a hack to avoid a bug. When program starts, it needs some time
     * (at least 1 ms on my configuration, but I set delay to 100 ms just to be sure
     * that it will work on weaker machines) before it can write settings to file,
     * otherwise ui->colorButton->palette().background().color() will return
     * default buttons background color, which will be written to settings
     * file at once program launches.
     */

    QTime dieTime = QTime::currentTime().addMSecs(100);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    preferencesDialog->loadSettingsToFile();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete preferencesDialog;
}

void MainWindow::showAboutBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("About") + " Scribbler");
    aboutBox.setIconPixmap(QPixmap("://aboutIcon.png"));
    aboutBox.setText(tr("I'm one-eyed Blot and this is my favourite Scribbler in the universe. <br><br>"
                        "<strong>Scribbler</strong> ") + version);
    aboutBox.setInformativeText("<p>" + tr("Distributed under The MIT License. See License and Credist page.") +
                                "<br><br><a href=https://github.com/aizenbit/Scribbler>https://github.com/aizenbit/Scribbler<a></p>");
    aboutBox.exec();
}

void MainWindow::showLicensesBox()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("Licenses and Credits"));
    aboutBox.setText(tr("<strong>Scribbler</strong> ") + version);
    aboutBox.setInformativeText(tr("<p>The MIT License (MIT)<br><br>"
                                   "Copyright © 2016 <a href=https://github.com/aizenbit>aizenbit</a><br><br>"
                                   "Permission is hereby granted, free of charge, to any person obtaining a copy "
                                   "of this software and associated documentation files (the \"Software\"), to deal "
                                   "in the Software without restriction, including without limitation the rights "
                                   "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                                   "copies of the Software, and to permit persons to whom the Software is "
                                   "furnished to do so, subject to the following conditions:<br><br>"

                                   "The above copyright notice and this permission notice shall be included in all "
                                   "copies or substantial portions of the Software.<br><br>"

                                   "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
                                   "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
                                   "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
                                   "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
                                   "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
                                   "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
                                   "SOFTWARE.<br><br>"

                                   "<strong>Credits:</strong><br>"
                                   "General Icon (\"one-eyed Blot\") made by <a href=https://virink.com/NH>Nuclear Hound</a> "
                                   "is licensed by <a href=https://www.gnu.org/licenses/gpl-3.0.html>GNU GPLv3</a>.<br>"
                                   "Tool Bar Icons made by <a href=http://www.flaticon.com/authors/picol title=Picol>Picol</a>, "
                                   "<a href=http://www.freepik.com title=Freepik>Freepik</a> from "
                                   "<a href=http://www.flaticon.com title=Flaticon>www.flaticon.com</a> "
                                   "are licensed by <a href=http://creativecommons.org/licenses/by/3.0/ "
                                   "title=Creative Commons BY 3.0>CC BY 3.0</a>.<br><br>"

                                   "Thanks to <a href=https://virink.com/domerk>Daniel Domerk</a> for a help."));
    aboutBox.exec();
}

void MainWindow::render()
{
    sheetPointers.clear();
    currentSheetNumber = 0;
    sheetPointers.push_back(0);
    QString text = ui->textEdit->toPlainText();
    ui->svgView->hideBorders(false);
    int endOfSheet = ui->svgView->renderText(QStringRef(&text));
    sheetPointers.push_back(endOfSheet);

    bool isThereMoreThanOneSheet = text.length() - 1 >= endOfSheet;
    ui->toolBar->actions()[4]->setEnabled(isThereMoreThanOneSheet);
    ui->toolBar->actions()[5]->setDisabled(true);
}

void MainWindow::renderNextSheet()
{
    QString text = ui->textEdit->toPlainText();
    currentSheetNumber++;
    int lettersToTheEnd = text.length() - sheetPointers.at(currentSheetNumber);
    ui->svgView->hideBorders(false);
    int endOfSheet = ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));
    endOfSheet += sheetPointers.at(currentSheetNumber);

    ui->toolBar->actions()[5]->setEnabled(true);

    if (endOfSheet >= text.length())
    {
        ui->toolBar->actions()[4]->setDisabled(true);
        return;
    }

    if (currentSheetNumber >= sheetPointers.count() - 1)
    {
        sheetPointers.push_back(endOfSheet);
    }
}

void MainWindow::renderPreviousSheet()
{
    QString text = ui->textEdit->toPlainText();
    ui->svgView->hideBorders(false);
    currentSheetNumber--;
    int lettersToTheEnd = sheetPointers.at(currentSheetNumber) - sheetPointers.at(currentSheetNumber + 1);
    ui->svgView->renderText(QStringRef(&text, sheetPointers.at(currentSheetNumber), lettersToTheEnd));

    ui->toolBar->actions()[4]->setEnabled(true);

    if (currentSheetNumber == 0)
    {
        ui->toolBar->actions()[5]->setDisabled(true);
        return;
    }
}

void MainWindow::loadFont()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Open"), "",
                                              tr("INI") +
                                                 "(*.ini);;" +
                                              tr("All Files") +
                                                 "(*.*)");
    if (fileName.isEmpty())
        return;

    ui->svgView->loadFont(fileName);
}

void MainWindow::saveSheet(QString fileName)
{
    if (fileName.isEmpty())
        fileName = QFileDialog::getSaveFileName(0, tr("Save"), "",
                                                  tr("PNG") +
                                                     "(*.png);;" +
                                                  tr("All Files") +
                                                     "(*.*)");
    ui->svgView->hideBorders(true);
    ui->svgView->saveRenderToImage().save(fileName);
}

void MainWindow::saveAllSheets()
{
    QString fileName = QFileDialog::getSaveFileName(0, tr("Save"), "",
                                              tr("PNG") +
                                                 "(*.png);;" +
                                              tr("All Files") +
                                                 "(*.*)");
    int indexOfExtension = fileName.indexOf(QRegularExpression("\\.\\w+$"), 0);
    if (indexOfExtension == -1)
        return;
    QString text = ui->textEdit->toPlainText();
    QString currentFileName;

    currentSheetNumber = -1;
    ui->toolBar->actions()[4]->setEnabled(true);

    while (ui->toolBar->actions()[4]->isEnabled())
    {
        renderNextSheet();
        currentFileName = fileName;
        currentFileName.insert(indexOfExtension, QString("_%1").arg(currentSheetNumber));
        saveSheet(currentFileName);
    }
    ui->svgView->hideBorders(false);
}

void MainWindow::printSheet()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer);
    if (dialog.exec() != QPrintDialog::Accepted)
        return;

    QSettings settings("Settings.ini", QSettings::IniFormat);
    settings.beginGroup("Settings");

    QSizeF paperSize(settings.value("sheet-width", 210.0).toInt(), settings.value("sheet-height", 297.0).toInt());
    bool isPortrait = settings.value("is-sheet-orientation-vertical", true).toBool();

    printer.setPaperSize(paperSize, QPrinter::Millimeter);
    printer.setResolution(settings.value("dpi", 300).toInt());
    printer.setOrientation(isPortrait ? QPrinter::Portrait : QPrinter::Landscape);

    settings.endGroup();

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing);

    ui->svgView->hideBorders(true);
    QImage image = ui->svgView->saveRenderToImage();
    ui->svgView->hideBorders(false);

    if (image.format() == QImage::Format_Invalid || !printer.isValid())
        return;

    painter.drawImage(0,0,image);
    painter.end();

}
