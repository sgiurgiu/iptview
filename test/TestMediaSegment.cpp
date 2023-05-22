#include "mediasegment.h"

#include <QTest>

class TestMediaSegment : public QObject
{
    Q_OBJECT
private slots:
    void TestEmptyMediaSegmentObject();
    void TestMediaSegmentObjectNoAttributes();
    void TestMediaSegmentObject();
};

void TestMediaSegment::TestEmptyMediaSegmentObject()
{
    MediaSegment segment;
    QCOMPARE(segment.GetTitle(), QString());
    QCOMPARE(segment.GetUri(), QString());
    QCOMPARE(segment.GetDuration(), 0.0f);
    QCOMPARE(segment.GetAttributeNames().size(), 0);
}

void TestMediaSegment::TestMediaSegmentObjectNoAttributes()
{
    MediaSegment segment("a", "b", 1.5f);
    QCOMPARE(segment.GetUri(), "a");
    QCOMPARE(segment.GetTitle(), "b");
    QCOMPARE(segment.GetDuration(), 1.5f);
    QCOMPARE(segment.GetAttributeNames().size(), 0);
}

void TestMediaSegment::TestMediaSegmentObject()
{
    MediaSegment segment("a", "b", 1.5f, {{"a","b"}});
    QCOMPARE(segment.GetUri(), "a");
    QCOMPARE(segment.GetTitle(), "b");
    QCOMPARE(segment.GetDuration(), 1.5f);
    QCOMPARE(segment.GetAttributeNames().size(), 1);
    QCOMPARE(segment.GetAttributeValue("a"), "b");
    segment.DeleteAllAttributes();
    QCOMPARE(segment.GetAttributeNames().size(), 0);
}


QTEST_MAIN(TestMediaSegment)
#include "TestMediaSegment.moc"
