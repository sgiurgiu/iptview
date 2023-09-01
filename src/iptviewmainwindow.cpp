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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "iptviewmainwidget.h"
#include "m3uparsercontroller.h"
#include "xstreamcodewizard.h"

IPTViewMainWindow::IPTViewMainWindow(QWidget *parent)
    : QMainWindow{parent}, networkManager{new QNetworkAccessManager{this}}
{
    setWindowIcon(QIcon(":/icons/iptview-icon.png"));
    setMinimumSize(640,480);
    setWindowTitle(QCoreApplication::applicationName());
    centerOnScreen();
    createActions();
    createMenus();
    addStatusBar();
    addWindowWidgets();
    connect(this, &IPTViewMainWindow::importPlaylist, this, &IPTViewMainWindow::onImportPlaylist, Qt::QueuedConnection);    

}
void IPTViewMainWindow::createActions()
{
    quitApplicationAction = new QAction(this);
    quitApplicationAction->setText( tr("&Quit") );
    quitApplicationAction->setShortcuts(QKeySequence::Quit);
    quitApplicationAction->setToolTip(tr("Quit application"));
    quitApplicationAction->setIcon(QIcon(":/icons/exit.png"));
    quitApplicationAction->setStatusTip(quitApplicationAction->toolTip());
    connect(quitApplicationAction, SIGNAL(triggered()), SLOT(close()));

    fileOpenAction = new QAction(QIcon(":/icons/open.png"), tr("&Open"), this);
    fileOpenAction->setShortcuts(QKeySequence::Open);
    fileOpenAction->setToolTip(tr("Open playlist"));
    fileOpenAction->setStatusTip(fileOpenAction->toolTip());
    connect(fileOpenAction, SIGNAL(triggered()), SLOT(openPlaylist()));

    fileImportXstreamCodeAction= new QAction(tr("&Import Xstream Playlist"), this);
    fileImportXstreamCodeAction->setToolTip(tr("Import Xstream Playlist"));
    fileImportXstreamCodeAction->setStatusTip(fileImportXstreamCodeAction->toolTip());
    connect(fileImportXstreamCodeAction, SIGNAL(triggered()), SLOT(importXstreamCode()));

    fileSaveAction = new QAction(tr("&Save"), this);
    fileSaveAction->setShortcuts(QKeySequence::Save);
    fileSaveAction->setToolTip(tr("Save playlist"));
    fileSaveAction->setStatusTip(fileSaveAction->toolTip());
    connect(fileSaveAction, SIGNAL(triggered()), SLOT(savePlaylist()));


    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setToolTip(tr("About IPTV View"));
    aboutAction->setStatusTip(aboutAction->toolTip());
    connect(aboutAction, SIGNAL(triggered()), SLOT(about()));
}
void IPTViewMainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu( tr("&File") );
    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(fileSaveAction);
    fileMenu->addAction(fileImportXstreamCodeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitApplicationAction);

    auto helpMenu = menuBar()->addMenu( tr("&Help") );
    auto ipAction = helpMenu->addAction("Network Details");
    helpMenu->addAction(aboutAction);

    ipAction->setToolTip(tr("Show network details"));
    ipAction->setStatusTip(ipAction->toolTip());
    connect(ipAction, SIGNAL(triggered()), SLOT(ipDetails()));
}
void IPTViewMainWindow::addStatusBar()
{
    statusBar()->showMessage("");
}
void IPTViewMainWindow::addWindowWidgets()
{
   mainWidget = new IPTViewMainWidget(networkManager, this);
   connect(mainWidget, SIGNAL(showingFullScreen(bool)), this, SLOT(fullScreen(bool)));
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
void IPTViewMainWindow::onImportPlaylist(M3UList list)
{
    QProgressDialog* progress = new QProgressDialog(this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setLabelText("Saving channels ...");
    progress->setMinimum(0);
    progress->setAutoClose(true);
    progress->setMaximum(list.GetSegmentsCount());

    connect(mainWidget, &IPTViewMainWidget::updateImportedChannelIndex, this, [progress](qint64 index) {
        progress->setValue(index);
    });
    connect(progress, &QProgressDialog::canceled, mainWidget,&IPTViewMainWidget::cancelImportChannels);

    connect(mainWidget, &IPTViewMainWidget::channelsImported, this, [progress]() {
        progress->reset();
        progress->deleteLater();
    });
    mainWidget->ImportPlaylist(std::move(list));
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
        emit importPlaylist(list);

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

void IPTViewMainWindow::fullScreen(bool flag)
{
    if(flag)
    {
        contentMargins = this->contentsMargins();
        setContentsMargins(0,0,0,0);
        menuBar()->hide();
        statusBar()->hide();
    }
    else
    {
        setContentsMargins(contentMargins);
        menuBar()->show();
        statusBar()->show();
    }
}

void IPTViewMainWindow::importXstreamCode()
{
    auto xstreamInfo = XstreamCodeWizard::ImportXstreamCodes(this,networkManager);
    auto totalCategories = xstreamInfo.liveCategories.size()+xstreamInfo.vodCategories.size();
    if(totalCategories <= 0) return;
    QProgressDialog* progress = new QProgressDialog(this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setLabelText("Loading channels ...");
    progress->setMinimum(0);
    progress->setAutoClose(true);
    progress->setMaximum(xstreamInfo.liveCategories.size()+xstreamInfo.vodCategories.size());
    connect(progress, &QProgressDialog::canceled, mainWidget,&IPTViewMainWidget::cancelImportChannels);
    connect(mainWidget, &IPTViewMainWidget::updateImportedChannelIndex, this, [progress](qint64 index) {
        progress->setValue(index);
    });

    connect(mainWidget, &IPTViewMainWidget::channelsImported, this, [progress]() {
        progress->reset();
        progress->deleteLater();
    });

    mainWidget->ImportPlaylist(std::move(xstreamInfo));
}

void IPTViewMainWindow::savePlaylist()
{
    QSettings settings;
    QString defaultFolder = settings.value("lastOpenFolder").value<QString>();

    auto fileName = QFileDialog::getSaveFileName(this,tr("Save M3U"),defaultFolder,
                                                 tr("M3U Files (*.m3u *.m3u8);;All Files (*)"));
    if(fileName.isEmpty()) return;
    QFileInfo file(fileName);
    settings.setValue("lastOpenFolder",file.absoluteDir().absolutePath());
    if(file.completeSuffix().isEmpty())
    {
        fileName += ".m3u8";
    }
    exportPlaylist(fileName);
}

void IPTViewMainWindow::exportPlaylist(const QString& fileName)
{
    auto  list = mainWidget->GetM3UList();
    list.SaveToFile(fileName);
}

void IPTViewMainWindow::about()
{
    auto text = QString("<h3>About %1</h3><p>%2</p><p>Licensed under <a href=\"https://www.gnu.org/licenses/gpl-3.0.en.html\">GPLv3</a></p>"
                        "The latest version is available at <a href=\"https://github.com/sgiurgiu/iptview\">https://github.com/sgiurgiu/iptview</a>").arg(IPTVIEW_STRING, IPTVIEW_COPYRIGHT);
    QMessageBox::about(this, "About IPTV View",text);
}

void IPTViewMainWindow::ipDetails()
{
    QProgressDialog* progress = new QProgressDialog(this);
    progress->setLabelText("Retrieving network information");
    progress->setMinimum(0);
    progress->setMaximum(0);
    progress->setAutoClose(true);
    progress->setMinimumDuration(0);

    QNetworkRequest request;
    request.setUrl(QUrl("https://ipinfo.io"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
    request.setRawHeader("Accept", "application/json");
    auto reply = networkManager->get(request);
    connect(progress, &QProgressDialog::canceled, reply,&QNetworkReply::abort);

    connect(reply, &QNetworkReply::finished, this, [reply, progress, this](){
        reply->deleteLater();
        progress->reset();
        progress->deleteLater();
        if(reply->error())
        {
            QMessageBox::critical(this,"Error", QString("Error retrieving IP information: %1").arg(reply->errorString()));
            return;
        }
        auto response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        auto object = doc.object();
        auto text = QString("<h5>Network information</h5>"
                            "<table>"
                            "<tr><td>IP</td><td>%1</td></tr>"
                            "<tr><td>Host</td><td>%2</td></tr>"
                            "<tr><td>ISP</td><td>%3</td></tr>"
                            "<tr><td>City</td><td>%4</td></tr>"
                            "<tr><td>Region</td><td>%5</td></tr>"
                            "<tr><td>Country</td><td>%6</td></tr>"
                            "<tr><td>Timezone</td><td>%7</td></tr>"
                            "</table>")
                        .arg(object.value("ip").toString(""),
                             object.value("hostname").toString(""),
                             object.value("org").toString(""),
                             object.value("city").toString(""),
                             object.value("region").toString(""),
                             object.value("country").toString(""),
                             object.value("timezone").toString(""));

        QMessageBox::information(this, "Network", text);
    });
}
