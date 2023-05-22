#include "m3ulist.h"
#include <QMutexLocker>
#include <QTextStream>
#include <limits>
#include <QFile>

M3UList::M3UList()
{
}
M3UList::M3UList(QList<MediaSegment> segments):segments{std::move(segments)}
{
    QMutexLocker locker(&listMutex);
    for(const auto& segment : this->segments)
    {
        for(const auto& attribute : segment.GetAttributeNames())
        {
            attributes.insert(attribute, attribute);
        }
        if(segment.GetDuration() > 0.0f)
        {
            totalTime += segment.GetDuration();
        }
    }
}
M3UList::M3UList(const M3UList& other) :
    segments{other.segments},
    totalTime{other.totalTime},
    attributes{other.attributes}
{
}
M3UList& M3UList::operator=(const M3UList& other)
{
    if (this != &other)
    {
        segments = other.segments;
        totalTime = other.totalTime;
        attributes = other.attributes;
    }
    return *this;
}
M3UList::M3UList(M3UList&& other) noexcept :
    segments{std::move(other.segments)},
    totalTime{other.totalTime},
    attributes{std::move(other.attributes)}
{
    other.totalTime = 0.f;
}
M3UList& M3UList::operator=(M3UList&& other) noexcept
{
    if (this != &other)
    {
        segments = std::move(other.segments);
        totalTime = other.totalTime;
        attributes = std::move(other.attributes);
        other.totalTime = 0.f;
    }
    return *this;
}

bool M3UList::operator==(const M3UList& other) const
{    
    return std::abs(this->totalTime - other.totalTime) < std::numeric_limits<float>::epsilon() &&
        this->attributes == other.attributes && this->segments == other.segments;
}

void M3UList::AddSegment(MediaSegment segment)
{
    for(const auto& attribute : segment.GetAttributeNames())
    {
        attributes.insert(attribute, attribute);
    }
    if(segment.GetDuration() > 0.0f)
    {
        totalTime += segment.GetDuration();
    }
    segments.append(std::move(segment));
}
qsizetype M3UList::GetSegmentsCount() const
{
    return segments.size();
}
const MediaSegment& M3UList::GetSegmentAt(qsizetype index) const
{
    return segments.at(index);
}
void M3UList::DeleteSegmentAt(qsizetype index)
{
    auto it = std::next(segments.begin(), index);
    if(it->GetDuration() > 0.0f)
    {
        totalTime -= it->GetDuration();
    }
    segments.erase(it);
}
void M3UList::DeleteAllSegments()
{
    segments.clear();
    attributes.clear();
    totalTime = 0.0f;
}
const QMap<QString,QString>& M3UList::GetAttributes() const
{
    return attributes;
}
float M3UList::GetTotalTime() const
{
    return totalTime;
}
float M3UList::GetTotalTimeSafe() const
{
    QMutexLocker locker(&listMutex);
    return totalTime;
}
void M3UList::AddSegmentsSafe(const QList<MediaSegment>& segments)
{
    QMutexLocker locker(&listMutex);
    for(const auto& segment : segments)
    {
        for(const auto& attribute : segment.GetAttributeNames())
        {
            attributes.insert(attribute, attribute);
        }
        if(segment.GetDuration() > 0.0f)
        {
            totalTime += segment.GetDuration();
        }
        AddSegment(segment);
    }
}
void M3UList::AddSegmentSafe(MediaSegment segment)
{
    QMutexLocker locker(&listMutex);
    AddSegment(std::move(segment));
}
qsizetype M3UList::GetSegmentsCountSafe() const
{
    QMutexLocker locker(&listMutex);
    return GetSegmentsCount();
}
MediaSegment M3UList::GetSegmentAtSafe(qsizetype index) const
{
    QMutexLocker locker(&listMutex);
    return GetSegmentAt(index);
}
void M3UList::DeleteSegmentAtSafe(qsizetype index)
{
    QMutexLocker locker(&listMutex);
    DeleteSegmentAt(index);
}
void M3UList::DeleteAllSegmentsSafe()
{
    QMutexLocker locker(&listMutex);
    DeleteAllSegments();
}
QMap<QString,QString> M3UList::GetAttributesSafe() const
{
    QMutexLocker locker(&listMutex);
    return GetAttributes();
}

void M3UList::SaveToFile(const QString& fileName) const
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if(file.isOpen())
    {
        QTextStream outStream(&file);
        outStream << "#EXTM3U\r\n";
        for(const auto& segment : segments)
        {
            outStream << "#EXTINF:";
            outStream << segment.GetDuration();
            for(const auto& name : segment.GetAttributeNames())
            {
                auto value = segment.GetAttributeValue(name);
                if(value)
                {
                    outStream << " " << name <<"="<<"\""<<value.value()<<"\"";
                }
            }
            outStream << "," << segment.GetTitle() << "\r\n";
            outStream << segment.GetUri() << "\r\n";
        }
    }
}
