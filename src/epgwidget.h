#ifndef EPGWIDGET_H
#define EPGWIDGET_H

#include "channeltreeitem.h"
#include <QDateTime>
#include <QList>
#include <QToolBar>

class QNetworkAccessManager;
class QJsonDocument;
class QTimer;
class QLabel;
class QAction;
class QScrollArea;

class EPGWidget : public QToolBar
{
    Q_OBJECT
public:
    explicit EPGWidget(QNetworkAccessManager* networkManager,
                       QWidget* parent = nullptr);
    void SetChannel(ChannelTreeItem* channel);
    void ClearChannel();
signals:
private slots:
    void retrieveEpgListings();
    void showPreviousListing();
    void showNextListing();

private:
    struct EpgListing
    {
        QString id;
        QString epgId;
        QString title;
        QString lang;
        QString description;
        QDateTime start;
        QDateTime end;
        QString channelId;
    };
    void buildEpgListings(const QJsonDocument& doc);
    void displayCurrentListing();
    void resetCurrentListingIndex();

private:
    QNetworkAccessManager* networkManager = nullptr;
    QTimer* refreshListingsTimer = nullptr;
    std::unique_ptr<ChannelTreeItem> selectedChannel = { nullptr };
    QList<EpgListing> listings;
    QLabel* titleLabel = nullptr;
    int64_t serverTimezoneUTCDifference = 0;
    QAction* previousListingAction = nullptr;
    QAction* nextListingAction = nullptr;
    int currentListingIndex = 0;
};

#endif // EPGWIDGET_H
