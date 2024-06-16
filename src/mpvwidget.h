#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <atomic>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class QOpenGLShaderProgram;
class QTimer;
class QOpenGLTexture;

class MpvWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    MpvWidget(QWidget* parent = nullptr);
    ~MpvWidget();
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    QSize sizeHint() const override
    {
        return QSize(640, 480);
    }
    void clearScreen();
    void stopRenderingMedia();
    void startRenderingMedia();
signals:
    void durationChanged(int value);
    void positionChanged(int value);
    void wheelScrolled(QPoint delta);
    void mouseMoved(QMouseEvent* event);
    void fileLoaded();
    void doubleClicked();
    void fileLoadingError(QString message);
    void fileUnknownFormatError(QString message);
    void unsupportedSystemError(QString message);
    void outputInitializationError(QString message);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
private slots:
    void onMpvEvents();
    void maybeUpdate();
    void updateTimeSinceShowingSpinner();

private:
    void handleMpvEvent(mpv_event* event);
    static void onUpdate(void* ctx);
    void drawSpinner();
    void notifyOfErrors(int errorCode);

    mpv_handle* mpv;
    mpv_render_context* mpv_gl;
    double volume = 100.0;
    std::atomic_bool shouldRenderMedia = false;
    QTimer* updateTimer = nullptr;
    QTimer* updateTimeSinceshowingSpinner = nullptr;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QByteArray vertexShader;
    QByteArray fragmentShader;
    QOpenGLShaderProgram* program = nullptr;
    std::chrono::steady_clock::time_point showingSpinnerStartTime;
};

#endif // MPVWIDGET_H
