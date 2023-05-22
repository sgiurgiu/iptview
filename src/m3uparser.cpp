#include "m3uparser.h"

#include <QFile>

#include <map>
#include <string>

namespace
{
    enum class ParsingState
    {
        None,
        Duration,
        AttributeValue,
        AttributeKey,
        Title
    };
}

M3UParser::M3UParser(QObject *parent)
    : QObject{parent}
{
}
void M3UParser::Cancel()
{
    cancelled = true;
}
qint64 M3UParser::GetCurrentStreamPosition() const
{
    return pos;
}
void M3UParser::Parse(QString fileName)
{
    QFile data(fileName);
    if(data.open(QFile::ReadOnly))
    {
        QTextStream input(&data);
        Parse(input);
    }
    else
    {
        emit errorParsing("Cannot open file "+fileName);
    }
}
void M3UParser::Parse(QTextStream &input)
{
    try
    {
        auto list = ParseStream(input);
        emit listReady(std::move(list));
    }
    catch(const M3UParserException& ex)
    {
        emit errorParsing(ex.what());
    }
}
M3UList M3UParser::ParseStream(QTextStream &input)
{
    this->inputStream = &input;
    if(input.device())
    {
        emit streamSize(input.device()->size());
    }
    {
        QString headerLine;
        if(!input.readLineInto(&headerLine))
        {
            throw M3UParserException("Cannot parse m3u, stream is empty");
        }
        if(headerLine != "#EXTM3U")
        {
            throw M3UParserException("Stream not an M3U file");
        }
    }

    M3UList list;
    qsizetype maxLineLength = 255;
    while (true)
    {
        if(cancelled) break;
        QString mediaSegmentDataLine;
        mediaSegmentDataLine.reserve(maxLineLength);
        if(!input.readLineInto(&mediaSegmentDataLine)) break;
        if(!mediaSegmentDataLine.startsWith("#EXTINF:")) continue;
        // update pos with the length of the line read + 1 to count for \n
        // the problem is we don't know if the file had a \r\n instead
        // so we may lose some bytes
        pos += mediaSegmentDataLine.length() + 1;
        if(maxLineLength < mediaSegmentDataLine.length())
        {
            maxLineLength = mediaSegmentDataLine.length() + 1;
        }
        MediaSegment segment;
        //parsing duration
        ParsingState state = ParsingState::Duration;
        qsizetype startIndex = 8; // just after the ':'
        auto duration = parseDuration(mediaSegmentDataLine, startIndex);
        if(duration)
        {
            segment.SetDuration(duration.value());
        }
        else
        {
            // cannot parse duration, bail out go to the next line
            continue;
        }
        qsizetype index = mediaSegmentDataLine.length() - 1;
        segment.SetTitle(parseTitle(mediaSegmentDataLine, startIndex, index));
        --index;
        if(startIndex < index)
        {
            // we got whatever we got as the title, we have attributes which we need to parse
            state = ParsingState::None;
            QString value;
            QString key;
            value.reserve(255);
            key.reserve(255);
            while(startIndex <= index)
            {
                auto ch = mediaSegmentDataLine.at(index);
                switch(state)
                {
                case ParsingState::None:
                    if(ch == '"')
                    {
                        state = ParsingState::AttributeValue; //we're gonna parse a value next
                    }
                    break;
                case ParsingState::AttributeValue:
                    if(ch == '"')
                    {
                        // ok, we see a '"', this can mean many things. If the next char is a '=', then we're done parsing
                        // a value and we start parse a key. Otherwise it's still a value
                        auto nextCh = mediaSegmentDataLine.at(index - 1);
                        if(nextCh == '=')
                        {
                            state = ParsingState::AttributeKey;
                            --index;
                        }
                        else
                        {
                            value.insert(0, ch);
                        }
                    }
                    else
                    {
                        value.insert(0, ch);
                    }
                    break;
                case ParsingState::AttributeKey:
                    if(ch == ' ')
                    {
                        state = ParsingState::None; // we're done
                        segment.AddAttribute(key, value);
                        key.clear();
                        value.clear();
                        value.reserve(255);
                        key.reserve(255);
                    }
                    else
                    {
                        key.insert(0, ch);
                    }
                    break;
                default:
                    break;
                }
                --index;
            }
        }

        QString mediaSegmentLine;
        mediaSegmentDataLine.reserve(maxLineLength);
        if(!input.readLineInto(&mediaSegmentLine)) break;
        if(maxLineLength < mediaSegmentLine.length())
        {
            maxLineLength = mediaSegmentLine.length() + 1;
        }

        pos += mediaSegmentLine.length() + 1;
        segment.SetUri(std::move(mediaSegmentLine));
        list.AddSegment(std::move(segment));
    }
    inputStream = nullptr;
    return list;
}


QString M3UParser::parseTitle(const QString& line, qsizetype minIndex, qsizetype& index /*int/out*/)
{
    // here we're going backwards, until we see a ',' and up to minIndex
    QString title;
    title.reserve(255);
    QChar ch;
    while ( (ch = line.at(index)) != ',' && index > minIndex)
    {
        title.insert(0, ch);
        --index;
    }

    return title;
}

std::optional<float> M3UParser::parseDuration(const QString& line, qsizetype& index)
{
    QString number;
    number.reserve(10); // arbitrary length
    QChar ch;
    while (true)
    {
        if(index >= line.length() -1) break;
        ch = line.at(index);
        if(ch == ' ' || ch == ',') break;
        number.append(ch);
        ++index;
    }
    bool ok = false;
    auto duration = number.toFloat(&ok);
    if(ok)
    {
        return std::optional<float>(duration);
    }
    else
    {
        return std::nullopt;
    }
}
