#ifndef MEDIASEGMENT_H
#define MEDIASEGMENT_H

#include <QMetaType>
#include <QString>
#include <QMap>
#include <optional>

class MediaSegment
{
public:
    MediaSegment();
    MediaSegment(QString uri, QString title, float duration);
    MediaSegment(QString uri, QString title, float duration, QMap<QString, QString> attributes);

    QString GetUri() const;
    void SetUri(QString uri);

    QString GetTitle() const;
    void SetTitle(QString title);

    float GetDuration() const;
    void SetDuration(float duration);

    void AddAttribute(QString name, QString value);
    QList<QString> GetAttributeNames() const;
    std::optional<QString> GetAttributeValue(const QString& name) const;
    void DeleteAllAttributes();
    bool operator==(const MediaSegment&) const;
private:
    QString uri;
    QString title;
    float duration = 0.0f;
    QMap<QString, QString> attributes;
};

Q_DECLARE_METATYPE(MediaSegment)

#endif // MEDIASEGMENT_H
