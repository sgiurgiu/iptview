#include "epgwidget.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDateTime>
#include <QLocale>
#include <QTimer>
#include <QTimeZone>
#include <QLabel>

#include "databaseprovider.h"
#include "database.h"

EPGWidget::EPGWidget(QNetworkAccessManager* networkManager,QWidget *parent)
    : QToolBar{parent}, networkManager{networkManager}
{
    refreshListingsTimer = new QTimer(this);
    refreshListingsTimer->setSingleShot(true);
    connect(refreshListingsTimer,SIGNAL(timeout()), this, SLOT(retrieveEpgListings()));

    titleLabel = new QLabel();
    previousListingAction = new QAction();
    nextListingAction = new QAction();
    previousListingAction->setIcon(QIcon{":/icons/chevron-back.png"});
    previousListingAction->setCheckable(false);
    previousListingAction->setEnabled(false);
    nextListingAction->setIcon(QIcon{":/icons/chevron-forward.png"});
    nextListingAction->setCheckable(false);
    nextListingAction->setEnabled(false);

    connect(previousListingAction, SIGNAL(triggered(bool)), this, SLOT(showPreviousListing()));
    connect(nextListingAction, SIGNAL(triggered(bool)), this, SLOT(showNextListing()));

    addAction(previousListingAction);
    addWidget(titleLabel);
    addAction(nextListingAction);

    setFloatable(false);
    setOrientation(Qt::Orientation::Horizontal);
    previousListingAction->setVisible(false);
    nextListingAction->setVisible(false);
   // setVisible(false);
}

void EPGWidget::SetChannel(int64_t channelId)
{
    ClearChannel();
    selectedChannel.reset(DatabaseProvider::GetDatabase()->GetChannel(channelId));

    if(selectedChannel)
    {
        QString xstreamServerTimezone = DatabaseProvider::GetDatabase()->GetXStreamServerTimezone(selectedChannel->getXStreamServerId());
        if(!xstreamServerTimezone.isEmpty())
        {
            QTimeZone serversTimezone{xstreamServerTimezone.toUtf8()};
            if(serversTimezone.isValid())
            {
                QDateTime local(QDateTime::currentDateTime());
                serverTimezoneUTCDifference = local.toTimeZone(serversTimezone).offsetFromUtc();
            }
        }
        retrieveEpgListings();
    }
}
void EPGWidget::ClearChannel()
{
    selectedChannel.reset();
    refreshListingsTimer->stop();
    serverTimezoneUTCDifference = 0;
    listings.clear();
    titleLabel->clear();
    previousListingAction->setEnabled(false);
    nextListingAction->setEnabled(false);
}
void EPGWidget::retrieveEpgListings()
{
    setVisible(false);
    if(!selectedChannel->getEpgChannelUri().isEmpty())
    {
        QNetworkRequest request;
        request.setUrl(selectedChannel->getEpgChannelUri());
        request.setHeader(QNetworkRequest::UserAgentHeader, "IPTView 1.0");
        auto reply = networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [reply, this]()
        {
            if(reply->error())
            {
                reply->deleteLater();
                displayCurrentListing();
                return;
            }
            auto response = reply->readAll();
            reply->deleteLater();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            buildEpgListings(doc);
            resetCurrentListingIndex();
            displayCurrentListing();
        });
    }
}
void EPGWidget::buildEpgListings(const QJsonDocument& doc)
{
    auto jsonObject = doc.object();
    auto epgListingArray = jsonObject.value("epg_listings").toArray();
    if(!epgListingArray.isEmpty())
    {
        listings.clear();
    }
    for(const auto& listing : epgListingArray)
    {
        auto listingObject = listing.toObject();
        auto startTimestampString = listingObject.value("start_timestamp").toString("");
        auto endTimestampString = listingObject.value("stop_timestamp").toString("");
        if(startTimestampString.isEmpty() || endTimestampString.isEmpty()) continue;
        bool ok = false;
        qint64 startTime = startTimestampString.toLongLong(&ok);
        if(!ok) continue;
        qint64 endTime = endTimestampString.toLongLong(&ok);
        if(!ok) continue;
        startTime -= serverTimezoneUTCDifference;
        endTime -= serverTimezoneUTCDifference;
        auto titleEncoded = listingObject.value("title").toString("");
        if(titleEncoded.isEmpty()) return;
        EpgListing epgListing;
        epgListing.title = QByteArray::fromBase64(titleEncoded.toUtf8(), QByteArray::Base64Encoding);
        epgListing.start = QDateTime::fromSecsSinceEpoch(startTime);
        epgListing.end = QDateTime::fromSecsSinceEpoch(endTime);
        epgListing.id = listingObject.value("id").toString("");
        epgListing.epgId = listingObject.value("epg_id").toString("");
        auto descriptionEncoded = listingObject.value("description").toString("");
        epgListing.description = QByteArray::fromBase64(descriptionEncoded.toUtf8(), QByteArray::Base64Encoding);
        epgListing.channelId = listingObject.value("channel_id").toString("");
        epgListing.lang = listingObject.value("lang").toString("");
        listings.append(std::move(epgListing));
    }
}
void EPGWidget::resetCurrentListingIndex()
{
    auto now = QDateTime::currentDateTime();
    currentListingIndex = 0;
    for(const auto& listing : listings)
    {
        if(now < listing.start || now >= listing.end )
        {
            currentListingIndex++;
        }
        else
        {
            break;
        }
    }
}
void EPGWidget::displayCurrentListing()
{
    auto now = QDateTime::currentDateTime();
    auto systemLocale = QLocale::system();
    titleLabel->clear();
    if(currentListingIndex >= 0 && currentListingIndex < listings.size())
    {
        auto listing = listings.at(currentListingIndex);
        auto title = QString("%1-%2 %3").arg(
            systemLocale.toString(listing.start.time(),QLocale::ShortFormat),
            systemLocale.toString(listing.end.time(),QLocale::ShortFormat),
            listing.title);
        titleLabel->setText(title);
        titleLabel->setToolTip(listing.description);
        refreshListingsTimer->start(now.msecsTo(listing.end));
    }

    previousListingAction->setEnabled(currentListingIndex > 0);
    nextListingAction->setEnabled(currentListingIndex < (listings.size()-1));
    previousListingAction->setVisible(!listings.isEmpty());
    nextListingAction->setVisible(!listings.isEmpty());

    //setVisible(!listings.isEmpty());
}

void EPGWidget::showPreviousListing()
{
    currentListingIndex--;
    displayCurrentListing();
}
void EPGWidget::showNextListing()
{
    currentListingIndex++;
    displayCurrentListing();
}
