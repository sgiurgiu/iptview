#include "mprisdbus.h"

#include <QtDBus>
#include <QApplication>
#include <QMessageBox>

#include "databaseprovider.h"
#include "database.h"
#include "channeltreeitem.h"

#include "mprisdbus_manager_adaptor.h"
#include "mprisdbus_player_adaptor.h"

MPRISDBus::MPRISDBus(QObject *parent)
    : QObject{parent}
{
    auto bus = QDBusConnection::sessionBus();
    if(!bus.isConnected())
    {
        // not connected to dbus, nothing we can do
        return;
    }
    auto pid = QApplication::instance()->applicationPid();
    QString serviceName = QString("org.mpris.MediaPlayer2.iptview.instance%1").arg(pid);
    if (!bus.registerService(serviceName))
    {
       // can't register service
       auto lastError = bus.lastError();
       if(lastError.type() == QDBusError::NoError) return;
       QString msg = QString("DBus register error: %1, %2, %3").arg(lastError.type()).arg(lastError.name()).arg(lastError.message());
       QMessageBox dialog;
       dialog.critical(NULL,"Error",msg);
       return;
    }

    //qDBusRegisterMetaType<QMap<QString,QDBusVariant>>();

    new DBusManagerAdaptor(this);
    new DBusPlayerAdaptor(this);

    bus.registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors);

    if (bus.connect(serviceName, "/org/mpris/MediaPlayer2",
                "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                SLOT(propertyChanged(QString, QVariantMap, QStringList)))) {
        qDebug() << "PropertiesChanged signal connected successfully to slot";
    } else {
        qDebug() << "PropertiesChanged signal connection was not successful";
    }
}
void MPRISDBus::propertyChanged(QString name, QVariantMap map, QStringList list)
{
    qDebug() << QString("properties of interface %1 changed").arg(name);
    for (QVariantMap::const_iterator it = map.cbegin(), end = map.cend(); it != end; ++it) {
        qDebug() << "property: " << it.key() << " value: " << it.value();
    }
    for (const auto& element : list) {
        qDebug() << "list element: " << element;
    }
}
void MPRISDBus::SetInitialVolume(int volume)
{
    this->volume = volume;
}
void MPRISDBus::SetFullscreen(bool flag)
{
    this->fullScreen = flag;
    notifyManagerChangedProperties("Fullscreen", flag);
}
void MPRISDBus::PlayingChannel(int64_t id)
{
    std::unique_ptr<ChannelTreeItem> channel{DatabaseProvider::GetDatabase()->GetChannel(id)};
    if(!channel) return;
    metadata.clear();
    metadata["mpris:trackid"] = QDBusObjectPath(QString("/iptview/track/%1").arg(id));
    if(!channel->getLogoUri().isEmpty())
    {
        metadata["mpris:artUrl"] = channel->getLogoUri();
    }
    metadata["xesam:url"] = channel->getUri();
    metadata["xesam:title"] = channel->getName();
    playbackStatus = "Playing";
    canPlay = false;
    canPause = true;
    notifyPlayerChangedProperties("Metadata", metadata);
    notifyPlayerChangedProperties("CanPause", canPause);
    notifyPlayerChangedProperties("CanPlay", canPlay);
    notifyPlayerChangedProperties("PlaybackStatus", playbackStatus);

}
void MPRISDBus::SelectedChannel(int64_t id)
{
    if(playbackStatus == "Playing") return;
    std::unique_ptr<ChannelTreeItem> channel{DatabaseProvider::GetDatabase()->GetChannel(id)};
    if(!channel) return;
    metadata.clear();
    metadata["mpris:trackid"] = QDBusObjectPath(QString("/iptview/track/%1").arg(id));
    if(!channel->getLogoUri().isEmpty())
    {
        metadata["mpris:artUrl"] = channel->getLogoUri();
    }
    metadata["xesam:url"] = channel->getUri();
    metadata["xesam:title"] = channel->getName();

    playbackStatus = "Stopped";
    canPlay = true;
    canPause = false;
    notifyPlayerChangedProperties("Metadata", metadata);
    notifyPlayerChangedProperties("CanPause", canPause);
    notifyPlayerChangedProperties("CanPlay", canPlay);
    notifyPlayerChangedProperties("PlaybackStatus", playbackStatus);
}
void MPRISDBus::EnableSkipForward(bool flag)
{
    canGoNext = flag;
    notifyPlayerChangedProperties("CanGoNext", flag);
}
void MPRISDBus::EnableSkipBack(bool flag)
{
    canGoPrevious = flag;
    notifyPlayerChangedProperties("CanGoPrevious", flag);
}

void MPRISDBus::Raise()
{

}
void MPRISDBus::Quit()
{
    QGuiApplication::quit();
}

void MPRISDBus::Next()
{
    emit skipForward();
}
void MPRISDBus::Previous()
{
    emit skipBack();
}
void MPRISDBus::Pause()
{
    emit pausePlayingSelectedChannel();
}
void MPRISDBus::PlayPause()
{
    emit playPauseSelectedChannel();
}
void MPRISDBus::Stop()
{
    emit stopPlayingSelectedChannel();
}
void MPRISDBus::Play()
{
    emit playSelectedChannel();
}
void MPRISDBus::Seek(int )
{

}
void MPRISDBus::SetPosition(const QDBusObjectPath& , int )
{

}
void MPRISDBus::OpenUri(const QString& uri)
{

}
void MPRISDBus::SetVolume(double v)
{
    volume = v;
    emit volumeChanged(static_cast<int>(volume*100.0));
}
void MPRISDBus::VolumeToggledExternal(bool)
{
    //volume
}
void MPRISDBus::VolumeChangedExternal(int newVolume)
{
    volume = static_cast<double>(newVolume) / 100.0;
    notifyPlayerChangedProperties("Volume", volume);
}

void MPRISDBus::emitManagerChangedPropertySignal()
{
    if(managerSignalingProperties.empty()) return;
    QDBusMessage signal = QDBusMessage::createSignal(
     "/org/mpris/MediaPlayer2",
     QStringLiteral("org.freedesktop.DBus.Properties"),
     QStringLiteral("PropertiesChanged"));
    signal << "org.mpris.MediaPlayer2";
    signal << managerSignalingProperties << QStringList();;
    QDBusConnection::sessionBus().send(signal);
    managerSignalingProperties.clear();
}
void MPRISDBus::notifyManagerChangedProperties(const QString& property, const QVariant& value)
{
    const bool firstChange = managerSignalingProperties.empty();
    managerSignalingProperties[property] = value;
    if(firstChange)
    {
        QMetaObject::invokeMethod(this, &MPRISDBus::emitManagerChangedPropertySignal, Qt::QueuedConnection);
    }
}
void MPRISDBus::emitPlayerChangedPropertySignal()
{
    if(playerSignalingProperties.empty()) return;
    QDBusMessage signal = QDBusMessage::createSignal(
     "/org/mpris/MediaPlayer2",
     QStringLiteral("org.freedesktop.DBus.Properties"),
     QStringLiteral("PropertiesChanged"));
    signal << "org.mpris.MediaPlayer2.Player";
    signal << playerSignalingProperties << QStringList();;
    QDBusConnection::sessionBus().send(signal);
    playerSignalingProperties.clear();
}
void MPRISDBus::notifyPlayerChangedProperties(const QString& property, const QVariant& value)
{
    const bool firstChange = playerSignalingProperties.empty();
    playerSignalingProperties[property] = value;
    if(firstChange)
    {
        QMetaObject::invokeMethod(this, &MPRISDBus::emitPlayerChangedPropertySignal, Qt::QueuedConnection);
    }
}


DBusTracklist::DBusTracklist(QObject *parent): QDBusAbstractAdaptor{parent}
{
    tracks.append("AAAA");
}
QList<QMap<QString,QDBusVariant>> DBusTracklist::GetTracksMetadata(const QList<QString>& trackIds) const
{
    QList<QMap<QString,QDBusVariant>> tracks;
    QMap<QString,QDBusVariant> track1;
    track1["mpris:trackid"] = QDBusVariant("aa");
    tracks.append(track1);
    return tracks;
}
void DBusTracklist::AddTrack(const QString& uri, const QString& afterTrack, bool setCurrent)
{

}
void DBusTracklist::RemoveTrack(const QString& track)
{

}
void DBusTracklist::GoTo(const QString& track)
{

}


