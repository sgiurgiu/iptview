#ifndef M3UPARSER_H
#define M3UPARSER_H

#include "m3ulist.h"

#include <QObject>
#include <QString>
#include <QTextStream>
#include <QException>

#include <optional>
#include <atomic>
#include <mutex>

class QTextStream;

class M3UParserException : public QException
{
public:
    M3UParserException(const QByteArray& message) throw() : message{message}
    {
    }
    M3UParserException(M3UParserException &&other) noexcept = default;
    M3UParserException(const M3UParserException &other) noexcept  = default;

    void raise() const override { throw *this; }
    M3UParserException *clone() const override { return new M3UParserException(*this); }
    virtual const char* what() const throw() override
    {
        return message.data();
    }
private:
    QByteArray message;
};

class M3UParser : public QObject
{
    Q_OBJECT
public:
    explicit M3UParser(QObject *parent = nullptr);
    // this method parses the stream and returns the result
    M3UList ParseStream(QTextStream &input);
    qint64 GetCurrentStreamPosition() const;
public slots:
    // these 2 methods parse the stream and signal the result
    // via listReady
    void Parse(QString fileName);
    void Parse(QTextStream &input);
    void Cancel();
signals:
    void listReady(M3UList list);
    void errorParsing(QString message);
    void streamSize(qint64 size);
private:
    std::optional<float> parseDuration(const QString& line, qsizetype& index /*in/out*/);
    QString parseTitle(const QString& line, qsizetype minIndex, qsizetype& index /*int/out*/);

private:
    std::atomic_bool cancelled = {false};
    QTextStream* inputStream = nullptr;
    std::atomic_int_fast64_t pos = {0};

};

#endif // M3UPARSER_H
