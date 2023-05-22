#ifndef M3ULIST_H
#define M3ULIST_H

#include "mediasegment.h"

#include <QList>
#include <QMap>
#include <QMutex>

class M3UList
{
public:
    M3UList();
    M3UList(QList<MediaSegment> segments);
    // NOT thread safe
    M3UList(const M3UList& other);
    M3UList& operator=(const M3UList& other);
    M3UList(M3UList&& other) noexcept;
    M3UList& operator=(M3UList&& other) noexcept;

    void AddSegment(MediaSegment segment);
    qsizetype GetSegmentsCount() const;
    const MediaSegment& GetSegmentAt(qsizetype index) const;
    void DeleteSegmentAt(qsizetype index);
    void DeleteAllSegments();
    const QMap<QString,QString>& GetAttributes() const;
    float GetTotalTime() const;

    // thread safe
    void AddSegmentSafe(MediaSegment segment);
    void AddSegmentsSafe(const QList<MediaSegment>& segments);
    qsizetype GetSegmentsCountSafe() const;
    MediaSegment GetSegmentAtSafe(qsizetype index) const;
    void DeleteSegmentAtSafe(qsizetype index);
    void DeleteAllSegmentsSafe();
    QMap<QString,QString> GetAttributesSafe() const;
    float GetTotalTimeSafe() const;
    bool operator==(const M3UList&) const;

    void SaveToFile(const QString& fileName) const;
private:
    QList<MediaSegment> segments;
    float totalTime = 0.0f;
    QMap<QString,QString> attributes;
    mutable QMutex listMutex;
};

#endif // M3ULIST_H
