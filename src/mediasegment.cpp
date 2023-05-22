#include "mediasegment.h"

#include <limits>

MediaSegment::MediaSegment()
{
}
MediaSegment::MediaSegment(QString uri, QString title, float duration)
    : uri{std::move(uri)},
      title{std::move(title)},
      duration{duration}
{
}
MediaSegment::MediaSegment(QString uri, QString title, float duration, QMap<QString, QString> attributes)
    : uri{std::move(uri)},
      title{std::move(title)},
      duration{duration},
      attributes{std::move(attributes)}
{
}
bool MediaSegment::operator==(const MediaSegment& other) const
{
    return std::abs(this->duration - other.duration) < std::numeric_limits<float>::epsilon() &&
        this->uri == other.uri && this->title == other.title && this->attributes == other.attributes;
}
QString MediaSegment::GetUri() const
{
    return uri;
}
void MediaSegment::SetUri(QString uri)
{
    this->uri = std::move(uri);
}

QString MediaSegment::GetTitle() const
{
    return title;
}
void MediaSegment::SetTitle(QString title)
{
    this->title = std::move(title);
}

float MediaSegment::GetDuration() const
{
    return duration;
}
void MediaSegment::SetDuration(float duration)
{
    this->duration = duration;
}

void MediaSegment::AddAttribute(QString name, QString value)
{
    attributes.insert(std::move(name), std::move(value));
}
QList<QString> MediaSegment::GetAttributeNames() const
{
    return attributes.keys();
}
std::optional<QString> MediaSegment::GetAttributeValue(const QString& name) const
{
    if(attributes.contains(name))
    {
        return attributes[name];
    }
    return std::nullopt;
}
void MediaSegment::DeleteAllAttributes()
{
    attributes.clear();
}
