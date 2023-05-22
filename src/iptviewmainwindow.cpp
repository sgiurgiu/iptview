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

#include "iptviewmainwidget.h"

IPTViewMainWindow::IPTViewMainWindow(QWidget *parent)
    : QMainWindow{parent}
{
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
}
void IPTViewMainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu( tr("&File") );
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
