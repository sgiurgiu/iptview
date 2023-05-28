#include "iptviewmainwindow.h"
#include <QCoreApplication>
#include <QApplication>
#include <QSettings>
#include <QScreen>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QProgressDialog>

#include "iptviewmainwidget.h"
#include "m3uparsercontroller.h"

IPTViewMainWindow::IPTViewMainWindow(QWidget *parent)
    : QMainWindow{parent}
{
    setWindowIcon(QIcon(":/icons/iptview-icon.png"));
    setMinimumSize(640,480);
    setWindowTitle(QCoreApplication::applicationName());
    centerOnScreen();
    createActions();
    createMenus();
    addStatusBar();
    addWindowWidgets();
}
void IPTViewMainWindow::createActions()
{
    quitApplicationAction = new QAction(this);
    quitApplicationAction->setText( tr("&Quit") );
    quitApplicationAction->setShortcuts(QKeySequence::Quit);
    quitApplicationAction->setToolTip(tr("Quit application"));
    quitApplicationAction->setIcon(QIcon(":/icons/exit.svg"));
    quitApplicationAction->setStatusTip(quitApplicationAction->toolTip());
    connect(quitApplicationAction, SIGNAL(triggered()), SLOT(close()));

    fileOpenAction = new QAction(QIcon(":/icons/open.svg"), tr("&Open"), this);
    fileOpenAction->setShortcuts(QKeySequence::Open);
    fileOpenAction->setToolTip(tr("Open playlist"));
    fileOpenAction->setStatusTip(quitApplicationAction->toolTip());
    connect(fileOpenAction, SIGNAL(triggered()), SLOT(openPlaylist()));

}
void IPTViewMainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu( tr("&File") );
    fileMenu->addAction(fileOpenAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitApplicationAction);

    viewMenu = menuBar()->addMenu(tr("View"));
}
void IPTViewMainWindow::addStatusBar()
{
    statusBar()->showMessage("");
}
void IPTViewMainWindow::addWindowWidgets()
{
   mainWidget = new IPTViewMainWidget(this);
   setCentralWidget(mainWidget);
}
void IPTViewMainWindow::centerOnScreen()
{
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width()-width()) / 2;
    int y = (screenGeometry.height()-height()) / 2;
    move(x, y);
}
void IPTViewMainWindow::closeEvent(QCloseEvent * event)
{
    QSettings settings;
    settings.setValue("mainwindow/geometry", saveGeometry());
    settings.setValue("mainwindow/state", saveState());
    QMainWindow::closeEvent(event);
}

void IPTViewMainWindow::openPlaylist()
{
    QSettings settings;
    QString defaultFolder = settings.value("lastOpenFolder").value<QString>();

    auto fileName = QFileDialog::getOpenFileName(this,tr("Open M3U"),defaultFolder,
                                                 tr("M3U Files (*.m3u *.m3u8);;All Files (*)"));
    if(fileName.isEmpty()) return;
    QFileInfo file(fileName);
    if(file.exists())
    {
        settings.setValue("lastOpenFolder",file.absoluteDir().absolutePath());
        loadPlaylist(fileName);
    }
    else
    {
        QMessageBox::critical(this,tr("Error loading file"),tr("File does not exist"));
    }
}

void IPTViewMainWindow::loadPlaylist(const QString& fileName)
{
    M3UParserController* parserController = new M3UParserController(this);
    QProgressDialog* progress = new QProgressDialog(this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setLabelText("Loading "+fileName);
    progress->setMinimum(0);
    progress->setAutoClose(true);
    connect(parserController, &M3UParserController::streamSize, this, [progress](qint64 size){
        progress->setMaximum(size);
    });
    connect(parserController, &M3UParserController::updatePos, this, [progress](qint64 pos) {
        progress->setValue(pos);
    });
    connect(progress, &QProgressDialog::canceled, parserController,&M3UParserController::cancel);
    connect(parserController, &M3UParserController::listReady, this, [this, progress, parserController](M3UList list) {
        mainWidget->ImportPlaylist(std::move(list));
        progress->reset();
        progress->deleteLater();
        parserController->deleteLater();
    });
    connect(parserController, &M3UParserController::errorParsing, this, [this, progress, parserController](const QString& error) {
        QMessageBox::critical(this, tr("Error loading file"), error);
        progress->reset();
        progress->deleteLater();
        parserController->deleteLater();
    });
    parserController->ParseList(fileName);
}
