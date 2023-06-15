#include "mpvwidget.h"
#include <stdexcept>
#include <QMetaObject>
#include <QOpenGLContext>
#include <QWheelEvent>
#include "mpvqthelper.hpp"
#include <QDebug>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QVector2D>
#include <chrono>
#include <QResource>

namespace
{
static const QVector4D BACKGROUND_COLOR = {0.3, 0.3, 0.3, 1.0f};
static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "onMpvEvents", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name)
{
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

} //anonymous namespace

MpvWidget::MpvWidget(QWidget *parent): QOpenGLWidget{parent}
{
    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_property_string(mpv, "terminal", "yes");
    mpv_set_property_string(mpv, "msg-level", "all=v");
    mpv_set_property_string(mpv, "sub-create-cc-track", "yes");
    mpv_set_property_string(mpv, "input-default-bindings", "no");
    mpv_set_property_string(mpv, "config", "no");
    mpv_set_property_string(mpv, "input-vo-keyboard", "no");
    mpv_observe_property(mpv, 0, "height", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "width", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "pause", MPV_FORMAT_FLAG);

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Request hw decoding, just for testing.
    //mpv::qt::set_option_variant(mpv, "hwdec", "auto");

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    double vol = 100.0;
    double volMax = 150.0;
    mpv_set_property(mpv,"volume",MPV_FORMAT_DOUBLE,&vol);
    mpv_set_property(mpv,"volume-max",MPV_FORMAT_DOUBLE,&volMax);

    mpv_set_wakeup_callback(mpv, wakeup, this);
    updateTimer = new QTimer(this);
    updateTimer->setInterval(33);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(update()));

    QResource vertexShaderResource{":/shaders/mpv.vsh"};
    QResource fragmentShaderResource{":/shaders/mpvWaitWheel.fsh"};

    vertexShader = vertexShaderResource.uncompressedData();
    fragmentShader = fragmentShaderResource.uncompressedData();
}

MpvWidget::~MpvWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_render_context_free(mpv_gl);
    mpv_terminate_destroy(mpv);
}
void MpvWidget::command(const QVariant& params)
{
    mpv::qt::command(mpv, params);
}

void MpvWidget::setProperty(const QString& name, const QVariant& value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MpvWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}
void MpvWidget::clearScreen()
{
    makeCurrent();
    auto functions = this->context()->functions();
    functions->glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
    functions->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    functions->glClearColor(BACKGROUND_COLOR.x(), BACKGROUND_COLOR.y(), BACKGROUND_COLOR.z(), BACKGROUND_COLOR.w());
    functions->glClear(GL_COLOR_BUFFER_BIT);
    functions->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    doneCurrent();
}
void MpvWidget::stopRenderingMedia()
{
    shouldRenderMedia = false;
    showingSpinnerStartTime = std::chrono::steady_clock::now();
    updateTimer->start();
}
void MpvWidget::startRenderingMedia()
{
    shouldRenderMedia = true;
    updateTimer->stop();
}

void MpvWidget::initializeGL()
{
    initializeOpenGLFunctions();

    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    if (mpv_render_context_create(&mpv_gl, mpv, params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_gl, MpvWidget::onUpdate, reinterpret_cast<void *>(this));


    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceCode(QOpenGLShader::Vertex,  vertexShader);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    program->link();
    program->bind();

    static const GLfloat vertices[] = {
        - 1.0, - 1.0,
        1.0, - 1.0,
        - 1.0, 1.0,

        1.0, - 1.0,
        1.0, 1.0,
        - 1.0, 1.0
    };
    vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer); // VBO
    vbo.create();
    vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));
    vao.create(); // create underlying OpenGL object
    vao.bind(); //

    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 2);

    vbo.release();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(0);
    clearScreen();
    updateTimer->start();
    showingSpinnerStartTime = std::chrono::steady_clock::now();
}

void MpvWidget::paintGL()
{
    int iwidth = width();
    int iheight = height();
    double dwidth = iwidth * devicePixelRatioF();
    double dheight = iheight * devicePixelRatioF();
    iwidth = static_cast<int>(dwidth);
    iheight = static_cast<int>(dheight);
    mpv_opengl_fbo mpfbo{static_cast<int>(defaultFramebufferObject()), iwidth, iheight , GL_RGBA };
    int flip_y{1};
    int skipRendering{shouldRenderMedia ? 0 : 1};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_SKIP_RENDERING, &skipRendering},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpv_gl, params);
    if(!shouldRenderMedia)
    {
        // display some spinning something
        drawSpinner();
    }

}

void MpvWidget::drawSpinner()
{
    using FloatSecond = std::chrono::duration<float,std::ratio<1,1>>;
    auto duration = std::chrono::duration_cast<FloatSecond>(std::chrono::steady_clock::now() - showingSpinnerStartTime);

    glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());


    program->bind();

    program->setUniformValue("iResolution", QVector2D{(float)(width() * devicePixelRatio()), (float)(height() * devicePixelRatio())});
    program->setUniformValue("iTime", duration.count());
    vao.bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);

    vao.release();
    program->release();


}

void MpvWidget::onMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handleMpvEvent(event);
    }
}

void MpvWidget::handleMpvEvent(mpv_event *event)
{    
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE:
    {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                emit positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                emit durationChanged(time);
            }        
        }
        break;
    }
    case MPV_EVENT_END_FILE:
    {
        mpv_event_end_file* end_file = static_cast<mpv_event_end_file*>(event->data);
        if(end_file->error < 0)
        {
            notifyOfErrors(end_file->error);
        }
    }
        break;
    case MPV_EVENT_NONE:
    case MPV_EVENT_IDLE:
        break;
    case MPV_EVENT_FILE_LOADED:
        startRenderingMedia();
        emit fileLoaded();
        break;
    default:
        qDebug() << "MpvWidget::handleMpvEvent : "<<event->event_id;
        break;
        // Ignore uninteresting or unknown events.
    }
}

void MpvWidget::notifyOfErrors(int errorCode)
{
    if(errorCode >= 0) return;
    switch(errorCode)
    {
    case MPV_ERROR_LOADING_FAILED:
        emit fileLoadingError(mpv_error_string(errorCode));
        break;
    case MPV_ERROR_UNKNOWN_FORMAT:
        emit fileUnknownFormatError(mpv_error_string(errorCode));
        break;
    case MPV_ERROR_UNSUPPORTED:
        emit unsupportedSystemError(mpv_error_string(errorCode));
        break;
    case MPV_ERROR_AO_INIT_FAILED:
        [[fallthrough]];
    case MPV_ERROR_VO_INIT_FAILED:
        emit outputInitializationError(mpv_error_string(errorCode));
        break;
    }
}

// Make Qt invoke mpv_render_context_render() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's render API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        doneCurrent();
    } else {
        update();
    }
}

void MpvWidget::onUpdate(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}

void MpvWidget::wheelEvent (QWheelEvent * event)
{
    QPoint delta = event->angleDelta();
    if(!delta.isNull())
    {
        emit wheelScrolled(std::move(delta));
    }
    event->accept();
}

void MpvWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit doubleClicked();
    }
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void MpvWidget::updateTimeSinceShowingSpinner()
{
    //++timeSinceShowingSpinner;
}
