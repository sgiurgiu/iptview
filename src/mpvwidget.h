#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <atomic>

class MpvWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    MpvWidget(QWidget *parent = nullptr);
    ~MpvWidget();
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    QSize sizeHint() const override { return QSize(640,480);}
    void clearScreen();
    void stopRendering();
    void restartRendering();
signals:
    void durationChanged(int value);
    void positionChanged(int value);
    void wheelScrolled(QPoint delta);
    void fileLoaded();
    void doubleClicked();
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void wheelEvent (QWheelEvent * event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private slots:
    void onMpvEvents();
    void maybeUpdate();
private:
    void handleMpvEvent(mpv_event *event);
    static void onUpdate(void *ctx);

    mpv_handle *mpv;
    mpv_render_context *mpv_gl;
    double volume = 100.0;
    std::atomic_bool shouldRender = false;
};

#endif // MPVWIDGET_H
