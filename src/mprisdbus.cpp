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
void MPRISDBus::SetFullscreen(bool flag)
{
    this->fullScreen = flag;
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
    QDBusMessage signal = QDBusMessage::createSignal(
     "/org/mpris/MediaPlayer2",
    QStringLiteral("org.freedesktop.DBus.Properties"),
    QStringLiteral("PropertiesChanged"));
    signal << "org.mpris.MediaPlayer2.Player";
    QVariantMap changedProps;
    changedProps.insert("Metadata", metadata);
    changedProps.insert("CanPause", true);
    changedProps.insert("CanPlay", false);
    changedProps.insert("PlaybackStatus", "Playing");

    signal << changedProps<< QStringList();;
    QDBusConnection::sessionBus().send(signal);
}
void MPRISDBus::SelectedChannel(int64_t id)
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
    QDBusMessage signal = QDBusMessage::createSignal(
     "/org/mpris/MediaPlayer2",
     QStringLiteral("org.freedesktop.DBus.Properties"),
     QStringLiteral("PropertiesChanged"));
    signal << "org.mpris.MediaPlayer2.Player";
    QVariantMap changedProps;
    changedProps.insert("Metadata", metadata);
    changedProps.insert("CanPause", false);
    changedProps.insert("CanPlay", true);
    changedProps.insert("PlaybackStatus", "Stopped");

    signal << changedProps<< QStringList();;
    QDBusConnection::sessionBus().send(signal);
}
void MPRISDBus::EnableSkipForward(bool flag)
{
    QDBusMessage signal = QDBusMessage::createSignal(
     "/org/mpris/MediaPlayer2",
    QStringLiteral("org.freedesktop.DBus.Properties"),
    QStringLiteral("PropertiesChanged"));
    signal << "org.mpris.MediaPlayer2.Player";
    QVariantMap changedProps;
    changedProps.insert("CanGoNext", flag);
    signal << changedProps << QStringList();
    QDBusConnection::sessionBus().send(signal);
}
void MPRISDBus::EnableSkipBack(bool flag)
{
    QDBusMessage signal = QDBusMessage::createSignal(
     "/org/mpris/MediaPlayer2",
     QStringLiteral("org.freedesktop.DBus.Properties"),
     QStringLiteral("PropertiesChanged"));
    signal << "org.mpris.MediaPlayer2.Player";
    QVariantMap changedProps;
    changedProps.insert("CanGoPrevious", flag);
    signal << changedProps<< QStringList();;
    QDBusConnection::sessionBus().send(signal);
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

}
void MPRISDBus::PlayPause()
{

}
void MPRISDBus::Stop()
{

}
void MPRISDBus::Play()
{

}
void MPRISDBus::Seek(int microSeconds)
{

}
void MPRISDBus::SetPosition(const QDBusObjectPath& id, int position)
{

}
void MPRISDBus::OpenUri(const QString& uri)
{

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


