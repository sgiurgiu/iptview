#include "mediasegment.h"
#include <QSharedData>

#include <limits>

// https://doc.qt.io/qt-6/qshareddatapointer.html

class MediaSegmentData : public QSharedData
{
public:
    MediaSegmentData()
    {}
    MediaSegmentData(const MediaSegmentData &other)
        : QSharedData(other), uri(other.uri), title(other.title), duration(other.duration), attributes(other.attributes)
    {}
    MediaSegmentData& operator=(const MediaSegmentData& other) = delete;
    ~MediaSegmentData()
    {}

    QString uri;
    QString title;
    float duration = 0.0f;
    QMap<QString, QString> attributes;
};

MediaSegment::MediaSegment()
{
    d = new MediaSegmentData;
}
MediaSegment::MediaSegment(QString uri, QString title, float duration)
{
    d = new MediaSegmentData;
    SetUri(std::move(uri));
    SetTitle(std::move(title));
    SetDuration(std::move(duration));
}
MediaSegment::MediaSegment(QString uri, QString title, float duration, QMap<QString, QString> attributes)
{
    d = new MediaSegmentData;
    SetUri(std::move(uri));
    SetTitle(std::move(title));
    SetDuration(std::move(duration));
    d->attributes = std::move(attributes);
}
MediaSegment::~MediaSegment()
{}
MediaSegment::MediaSegment(const MediaSegment &other) : d(other.d)
{
}
MediaSegment& MediaSegment::operator=(const MediaSegment& other)
{
    this->d = other.d;
    return *this;
}
bool MediaSegment::operator==(const MediaSegment& other) const
{
    return std::abs(d->duration - other.GetDuration()) < std::numeric_limits<float>::epsilon() &&
        d->uri == other.GetUri() && d->title == other.GetTitle() && d->attributes == other.d->attributes;
}
QString MediaSegment::GetUri() const
{
    return d->uri;
}
void MediaSegment::SetUri(QString uri)
{
    d->uri = std::move(uri);
}

QString MediaSegment::GetTitle() const
{
    return d->title;
}
void MediaSegment::SetTitle(QString title)
{
    d->title = std::move(title);
}

float MediaSegment::GetDuration() const
{
    return d->duration;
}
void MediaSegment::SetDuration(float duration)
{
    d->duration = duration;
}

void MediaSegment::AddAttribute(QString name, QString value)
{
    d->attributes.insert(std::move(name), std::move(value));
}
QList<QString> MediaSegment::GetAttributeNames() const
{
    return d->attributes.keys();
}
std::optional<QString> MediaSegment::GetAttributeValue(const QString& name) const
{
    if(d->attributes.contains(name))
    {
        return d->attributes[name];
    }
    return std::nullopt;
}
void MediaSegment::DeleteAllAttributes()
{
    d->attributes.clear();
}
