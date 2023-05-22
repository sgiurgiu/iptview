#include "m3uparsercontroller.h"

#include "m3uparser.h"
#include <QTimer>

M3UParserController::M3UParserController(QObject *parent)
    : QObject{parent}, timer{new QTimer(this)}
{
    connect(timer, &QTimer::timeout, this, &M3UParserController::timerFired);
    connect(&workerThread, &QThread::finished, this, &QObject::deleteLater);
    parser = new M3UParser();
    parser->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, parser, &QObject::deleteLater);
    connect(this, SIGNAL(parseList(const QString&)), parser, SLOT(Parse(const QString&)));
    connect(parser, &M3UParser::listReady, this, &M3UParserController::parserListReady);
    connect(parser, &M3UParser::errorParsing, this, &M3UParserController::errorParsing);
    connect(parser, &M3UParser::streamSize, this, &M3UParserController::streamSize);
    workerThread.start();
}

M3UParserController::~M3UParserController()
{
    timer->stop();
    workerThread.quit();
    workerThread.wait();
}

void M3UParserController::ParseList(const QString& fileName)
{
    timer->start(200);
    emit parseList(fileName);
}

void M3UParserController::timerFired()
{
    emit updatePos(parser->GetCurrentStreamPosition());
}
void M3UParserController::cancel()
{
    timer->stop();
    parser->Cancel();
}

void M3UParserController::parserListReady(M3UList list)
{
    timer->stop();
    emit listReady(std::move(list));
}
