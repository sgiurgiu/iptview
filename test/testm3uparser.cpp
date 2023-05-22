
#include "m3uparser.h"

#include <QTest>
#include <QByteArray>
#include <QTextStream>
#include <QSignalSpy>

class TestM3UParser : public QObject
{
    Q_OBJECT
private slots:
    void TestParseSimpletreams_data();
    void TestParseSimpletreams();
    void TestParseNonExistentFile();
    void TestParseInvalidList();
};

void TestM3UParser::TestParseSimpletreams_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<M3UList>("list");
    {
        QByteArray out;
        M3UList list;
        {
            QTextStream stream(&out);
            stream << "#EXTM3U";
        }
        QTest::newRow("header_only") << out << list;
    }

    {
        QByteArray out;
        M3UList list;
        list.AddSegment(MediaSegment{ "http://example.com/media.mp4", "Test", 5, { } });
        {
            QTextStream stream(&out);
            stream << "#EXTM3U" << "\n";
            stream << "#EXTINF:5,Test" << "\n";
            stream << "http://example.com/media.mp4" << "\n";
        }
        QTest::newRow("one media segment - no attributes") << out << list;
    }

    {
        QByteArray out;
        M3UList list;
        list.AddSegment(MediaSegment{ "http://example.com/media.mp4", "Test", 5, { {"id", "a"}, {"name", "test"} } });
        {
            QTextStream stream(&out);
            stream << "#EXTM3U" << "\n";
            stream << "#EXTINF:5 id=\"a\" name=\"test\",Test" << "\n";
            stream << "http://example.com/media.mp4" << "\n";
        }
        QTest::newRow("one media segment") << out << list;
    }

    {
        QByteArray out;
        M3UList list;
        list.AddSegment(MediaSegment{ "http://example.com/media.mp4", "Test", 5, { {"id", "a"}, {"name", "test"} } });
        list.AddSegment(MediaSegment{ "http://example.com/media.mp4", "Test Test", 1.12f, { {"id", "aa"}, {"name", "testtest"}, {"anotherone", "t t"}}});
        {
            QTextStream stream(&out);
            stream << "#EXTM3U" << "\n";
            stream << "#EXTINF:5 id=\"a\" name=\"test\",Test" << "\n";
            stream << "http://example.com/media.mp4" << "\n";
            stream << "#EXTINF:1.12 id=\"aa\" name=\"testtest\" anotherone=\"t t\",Test Test" << "\n";
            stream << "http://example.com/media.mp4" << "\n";
        }
        QTest::newRow("two media segments") << out << list;
    }

    {
        QByteArray out;
        M3UList list;
        list.AddSegment(MediaSegment{ "http://example.com/media.mp4", "Test", 5, { {"id-a", "a \"a\" a"}, {"name", "test"} } });
        {
            QTextStream stream(&out);
            stream << "#EXTM3U" << "\n";
            stream << "#EXTINF:5 id-a=\"a \"a\" a\" name=\"test\",Test" << "\n";
            stream << "http://example.com/media.mp4" << "\n";
        }
        QTest::newRow("media segment with quotes") << out << list;
    }
}

void TestM3UParser::TestParseSimpletreams()

{
    QFETCH(QByteArray, data);
    QFETCH(M3UList, list);
    M3UParser parser;
    QTextStream stream(data);
    auto parsedList = parser.ParseStream(stream);
    QCOMPARE(parsedList, list);    
}


void TestM3UParser::TestParseNonExistentFile()
{
    M3UParser parser;
    QSignalSpy spy(&parser, SIGNAL(errorParsing(QString)));
    parser.Parse("file_that_doesnt_exist");
    QCOMPARE(spy.count(), 1);
}

void TestM3UParser::TestParseInvalidList()
{
    QTextStream emptyStream("");
    M3UParser parser;
    QSignalSpy spy(&parser, SIGNAL(errorParsing(QString)));
    parser.Parse(emptyStream);
    QTextStream invalidStream("abc");
    parser.Parse(invalidStream);
    QCOMPARE(spy.count(), 2);
}

QTEST_MAIN(TestM3UParser)
#include "testm3uparser.moc"
