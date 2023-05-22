#include "m3ulist.h"

#include <QTest>

class TestM3UList : public QObject
{
    Q_OBJECT
private slots:
    void TestList();
};

void TestM3UList::TestList()
{
    M3UList list;
    QCOMPARE(list.GetSegmentsCount(), 0);
    list.AddSegment(MediaSegment());
    QCOMPARE(list.GetSegmentsCount(), 1);
    QCOMPARE(list.GetSegmentAt(0).GetUri(), "");
    QCOMPARE(list.GetTotalTime(), 0.f);
    list.AddSegment(MediaSegment("a","b",1.f));
    QCOMPARE(list.GetSegmentsCount(), 2);
    QCOMPARE(list.GetSegmentAt(1).GetUri(), "a");
    QCOMPARE(list.GetSegmentAt(1).GetTitle(), "b");
    QCOMPARE(list.GetSegmentAt(1).GetDuration(), 1.f);
    QCOMPARE(list.GetTotalTime(), 1.f);

    list.AddSegment(MediaSegment("a","b",1.f, {{"a","b"}}));
    QCOMPARE(list.GetSegmentsCount(), 3);
    QCOMPARE(list.GetTotalTime(), 2.f);
    QCOMPARE(list.GetSegmentAt(2).GetUri(), "a");
    QCOMPARE(list.GetSegmentAt(2).GetTitle(), "b");
    QCOMPARE(list.GetSegmentAt(2).GetDuration(), 1.f);
    QCOMPARE(list.GetSegmentAt(2).GetAttributeNames().size(), 1);
    QCOMPARE(list.GetSegmentAt(2).GetAttributeValue("a"), "b");
    QCOMPARE(list.GetAttributes().size(), 1);
    QCOMPARE(*(list.GetAttributes().begin()), "a");

    list.AddSegment(MediaSegment("a","b",1.f, {{"a","b"}}));
    QCOMPARE(list.GetSegmentsCount(), 4);
    QCOMPARE(list.GetTotalTime(), 3.f);
    QCOMPARE(list.GetSegmentAt(3).GetUri(), "a");
    QCOMPARE(list.GetSegmentAt(3).GetTitle(), "b");
    QCOMPARE(list.GetSegmentAt(3).GetDuration(), 1.f);
    QCOMPARE(list.GetSegmentAt(3).GetAttributeNames().size(), 1);
    QCOMPARE(list.GetSegmentAt(3).GetAttributeValue("a"), "b");
    QCOMPARE(list.GetAttributes().size(), 1);
    QCOMPARE(*(list.GetAttributes().begin()), "a");

    list.AddSegment(MediaSegment("a","b",1.f, {{"c","d"}}));
    QCOMPARE(list.GetSegmentsCount(), 5);
    QCOMPARE(list.GetTotalTime(), 4.f);
    QCOMPARE(list.GetSegmentAt(2).GetUri(), "a");
    QCOMPARE(list.GetSegmentAt(2).GetTitle(), "b");
    QCOMPARE(list.GetSegmentAt(2).GetDuration(), 1.f);
    QCOMPARE(list.GetSegmentAt(2).GetAttributeNames().size(), 1);
    QCOMPARE(list.GetSegmentAt(2).GetAttributeValue("a"), "b");
    QCOMPARE(list.GetAttributes().size(), 2);
    QCOMPARE(list.GetAttributes().contains("c"), true);

}

QTEST_MAIN(TestM3UList)
#include "TestM3UList.moc"
