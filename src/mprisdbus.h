#ifndef MPRISDBUS_H
#define MPRISDBUS_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QStringList>
#include <QGuiApplication>

class DBusTracklist : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.TrackList")
    Q_PROPERTY(QStringList Tracks READ Tracks WRITE SetTracks)
    Q_PROPERTY(bool CanEditTracks READ CanEditTracks CONSTANT)
public:
    DBusTracklist(QObject *parent);
    QList<QMap<QString,QDBusVariant>> GetTracksMetadata(const QList<QString>& trackIds) const;
    void AddTrack(const QString& uri, const QString& afterTrack, bool setCurrent);
    void RemoveTrack(const QString& track);
    void GoTo(const QString& track);

    bool CanEditTracks() const
    {
        return false;
    }
    QStringList Tracks() const
    {
        return tracks;
    }
    void SetTracks(const QStringList& tracks)
    {
        this->tracks = tracks;
    }
signals:
    void TrackListReplaced(const QList<QString>& trackIds, const QString& currentTrack);
    void TrackAdded(const QMap<QString,QDBusVariant>& metadata, const QString& afterTrack);
    void TrackRemoved(const QString& track);
    void TrackMetadataChanged(const QString& track, const QMap<QString,QDBusVariant>& metadata);
private:
    QStringList tracks;
};


class MPRISDBus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool CanQuit READ CanQuit CONSTANT)
    Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE SetFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen CONSTANT)
    Q_PROPERTY(bool CanRaise READ CanRaise CONSTANT)
    Q_PROPERTY(bool HasTrackList READ HasTrackList CONSTANT)
    Q_PROPERTY(QString Identity READ Identity CONSTANT)
    Q_PROPERTY(QString DesktopEntry READ DesktopEntry CONSTANT)
    Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes CONSTANT)
    Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes CONSTANT)

    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus WRITE SetPlaybackStatus)
    Q_PROPERTY(double Rate READ Rate WRITE SetRate)
    Q_PROPERTY(QVariantMap Metadata READ Metadata WRITE SetMetadata)
    Q_PROPERTY(double Volume READ Volume WRITE SetVolume)
    Q_PROPERTY(int Position READ Position)
    Q_PROPERTY(double MinimumRate READ MinimumRate CONSTANT)
    Q_PROPERTY(double MaximumRate READ MaximumRate CONSTANT)
    Q_PROPERTY(bool CanGoNext READ CanGoNext WRITE SetCanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious WRITE SetCanGoPrevious)
    Q_PROPERTY(bool CanPlay READ CanPlay WRITE SetCanPlay)
    Q_PROPERTY(bool CanPause READ CanPause WRITE SetCanPause)
    Q_PROPERTY(bool CanSeek READ CanSeek CONSTANT)
    Q_PROPERTY(bool CanControl READ CanControl CONSTANT)

public:
    explicit MPRISDBus(QObject *parent = nullptr);
public slots:
    void SetFullscreen(bool flag);
    void PlayingChannel(int64_t);
    void SelectedChannel(int64_t);
    void EnableSkipForward(bool);
    void EnableSkipBack(bool);
signals:
    void showingFullScreen(bool);
    void skipBack();
    void skipForward();
private slots:
    void propertyChanged(QString name, QVariantMap map, QStringList list);

private:
    void Raise();
    void Quit();
    void Next();
    void Previous();
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(int microSeconds);
    void SetPosition(const QDBusObjectPath& id, int position);
    void OpenUri(const QString& uri);

    bool CanQuit() const
    {
        return true;
    }
    bool CanSetFullscreen() const
    {
        return true;
    }
    bool Fullscreen() const
    {
        return fullScreen;
    }

    bool CanRaise() const
    {
        return true;
    }
    bool HasTrackList() const
    {
        return false;
    }
    QString Identity() const
    {
        return QGuiApplication::applicationDisplayName();
    }
    QString DesktopEntry() const
    {
        return QGuiApplication::desktopFileName();
    }
    QStringList SupportedUriSchemes() const
    {
        return {};
    }
    QStringList SupportedMimeTypes() const
    {
        return {};
    }
    double MinimumRate() const
    {
        return 1.0;
    }
    double MaximumRate() const
    {
        return 1.0;
    }
    double Rate() const
    {
        return 1.0;
    }
    void SetRate(double)
    {}
    QVariantMap Metadata() const
    {
        return metadata;
    }
    void SetMetadata(const QVariantMap& m)
    {
        metadata = m;
    }
    int Position() const
    {
        return position;
    }
    QString PlaybackStatus() const
    {
        return playbackStatus;
    }
    void SetPlaybackStatus(const QString& status)
    {
        playbackStatus = status;
    }
    double Volume() const
    {
        return volume;
    }
    void SetVolume(double v)
    {
        volume = v;
    }
    bool CanControl() const
    {
        return true;
    }
    bool CanSeek() const
    {
        return false;
    }
    bool CanGoNext() const
    {
        return canGoNext;
    }
    void SetCanGoNext(bool flag)
    {
        canGoNext = flag;
    }
    bool CanGoPrevious() const
    {
        return canGoPrevious;
    }
    void SetCanGoPrevious(bool flag)
    {
        canGoPrevious = flag;
    }
    bool CanPlay() const
    {
        return canPlay;
    }
    void SetCanPlay(bool flag)
    {
        canPlay = flag;
    }
    bool CanPause() const
    {
        return canPause;
    }
    void SetCanPause(bool flag)
    {
        canPause = flag;
    }
private:
    DBusTracklist* tracklist = nullptr;

    friend class DBusManagerAdaptor;
    friend class DBusPlayerAdaptor;

    bool fullScreen = false;
    QString playbackStatus;
    double volume = 0.0;
    int position = 0;
    QVariantMap metadata;
    bool canGoNext = false;
    bool canGoPrevious = false;
    bool canPlay = false;
    bool canPause = false;
};

#endif // MPRISDBUS_H
