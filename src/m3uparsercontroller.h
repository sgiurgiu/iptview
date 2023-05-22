#ifndef M3UPARSERCONTROLLER_H
#define M3UPARSERCONTROLLER_H

#include <QObject>
#include <QThread>
#include "m3ulist.h"

class M3UParser;
class QTimer;
class M3UParserController : public QObject
{
    Q_OBJECT
public:
    explicit M3UParserController(QObject *parent = nullptr);
    ~M3UParserController();
    void ParseList(const QString& fileName);
signals:
    void listReady(M3UList list);
    void errorParsing(QString message);
    void streamSize(qint64 size);
    void updatePos(qint64 size);
    void parseList(const QString& fileName);
public slots:
    void cancel();
private slots:
    void timerFired();
    void parserListReady(M3UList list);
private:
    QTimer* timer = nullptr;
    QThread workerThread;
    M3UParser* parser = nullptr;
};

#endif // M3UPARSERCONTROLLER_H
