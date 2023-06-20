#include "m3ulist.h"
#include <QMutexLocker>
#include <QTextStream>
#include <limits>
#include <QFile>
#include <QSharedData>

class M3UListData : public QSharedData
{
public:
    M3UListData()
    {}
    M3UListData(const M3UListData &other)
        : QSharedData(other), segments(other.segments), totalTime(other.totalTime), attributes(other.attributes)
    {}
    M3UListData& operator=(const M3UListData& other) = delete;
    ~M3UListData()
    {}

    QList<MediaSegment> segments;
    float totalTime = 0.0f;
    QMap<QString,QString> attributes;
};

M3UList::M3UList()
{
    d = new M3UListData;
}
M3UList::M3UList(QList<MediaSegment> segments)
{
    d = new M3UListData;
    d->segments = std::move(segments);

    QMutexLocker locker(&listMutex);
    for(const auto& segment : d->segments)
    {
        for(const auto& attribute : segment.GetAttributeNames())
        {
            d->attributes.insert(attribute, attribute);
        }
        if(segment.GetDuration() > 0.0f)
        {
            d->totalTime += segment.GetDuration();
        }
    }
}
M3UList::~M3UList()
{}
M3UList::M3UList(const M3UList& other) : d(other.d)
{
}
M3UList& M3UList::operator=(const M3UList& other)
{
    this->d = other.d;
    return *this;
}
bool M3UList::operator==(const M3UList& other) const
{    
    return std::abs(d->totalTime - other.d->totalTime) < std::numeric_limits<float>::epsilon() &&
        d->attributes == other.d->attributes && d->segments == other.d->segments;
}

void M3UList::AddSegment(MediaSegment segment)
{
    for(const auto& attribute : segment.GetAttributeNames())
    {
        d->attributes.insert(attribute, attribute);
    }
    if(segment.GetDuration() > 0.0f)
    {
        d->totalTime += segment.GetDuration();
    }
    d->segments.append(std::move(segment));
}
qsizetype M3UList::GetSegmentsCount() const
{
    return d->segments.size();
}
const MediaSegment& M3UList::GetSegmentAt(qsizetype index) const
{
    return d->segments.at(index);
}
void M3UList::DeleteSegmentAt(qsizetype index)
{
    auto it = std::next(d->segments.begin(), index);
    if(it->GetDuration() > 0.0f)
    {
        d->totalTime -= it->GetDuration();
    }
    d->segments.erase(it);
}
void M3UList::DeleteAllSegments()
{
    d->segments.clear();
    d->attributes.clear();
    d->totalTime = 0.0f;
}
const QMap<QString,QString>& M3UList::GetAttributes() const
{
    return d->attributes;
}
float M3UList::GetTotalTime() const
{
    return d->totalTime;
}
float M3UList::GetTotalTimeSafe() const
{
    QMutexLocker locker(&listMutex);
    return d->totalTime;
}
void M3UList::AddSegmentsSafe(const QList<MediaSegment>& segments)
{
    QMutexLocker locker(&listMutex);
    for(const auto& segment : segments)
    {
        for(const auto& attribute : segment.GetAttributeNames())
        {
            d->attributes.insert(attribute, attribute);
        }
        if(segment.GetDuration() > 0.0f)
        {
            d->totalTime += segment.GetDuration();
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
        for(const auto& segment : d->segments)
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
