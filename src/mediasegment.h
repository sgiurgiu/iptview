#ifndef MEDIASEGMENT_H
#define MEDIASEGMENT_H

#include <QMetaType>
#include <QString>
#include <QMap>
#include <optional>
#include <QSharedDataPointer>

class MediaSegmentData;

class MediaSegment
{
public:
    MediaSegment();
    MediaSegment(QString uri, QString title, float duration);
    MediaSegment(QString uri, QString title, float duration, QMap<QString, QString> attributes);

    MediaSegment(const MediaSegment &other);
    MediaSegment& operator=(const MediaSegment& other);
    ~MediaSegment();

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
    QSharedDataPointer<MediaSegmentData> d;
};

Q_DECLARE_TYPEINFO(MediaSegment, Q_MOVABLE_TYPE);

#endif // MEDIASEGMENT_H
