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

static const char *vertexShader=
        R"(
#version 330 core
        attribute highp vec4 position;
        void main()
        {
            gl_Position = position;
        }
        )";
// stolen and adapted from https://www.shadertoy.com/view/3djGRm
// this is a full screen of circles moving
static const char *fragmentShader=
        R"(
    #version 330 core
#ifdef GL_ES
precision highp float;
#endif
        uniform vec2      iResolution;
        uniform float   iTime;
#define AA_SIZE 0.0015

// Sampling
int g_seed;

int Hash(int x)
{
    x = (x ^ 61) ^ (x >> 16);
    x = x + (x << 3);
    x = x ^ (x >> 4);
    x = x * 0x27d4eb2d;
    x = x ^ (x >> 15);
    x = 1103515245 * x + 12345;
    return x;
}

// Time
float GetTime()
{
    return iTime;
}

void Rand_Init(int seed)
{
    g_seed = seed;
}

float Rand_GetFloat01()
{
    g_seed = Hash(g_seed);
    return float(g_seed) * 2.3283064365386963e-10 * 0.5 + 0.5;
}

float Rand_Range(float min, float max)
{
    return Rand_GetFloat01() * (max - min) + min;
}

vec2 Rand_Sample2D()
{
    return vec2(Rand_GetFloat01(), Rand_GetFloat01());
}

vec3 Rand_Sample3D()
{
    return vec3(Rand_GetFloat01(), Rand_GetFloat01(), Rand_GetFloat01());
}

float Circle(vec2 point, float radius, float slope, vec2 uv)
{
    return smoothstep(radius + slope, radius - slope, length(uv - point));
}

float Rectangle(vec2 point, vec2 size, vec2 uv)
{
    return smoothstep(size.x + AA_SIZE, size.x - AA_SIZE, length(uv.x - point.x))
        * smoothstep(size.y + AA_SIZE, size.y - AA_SIZE, length(uv.y - point.y));
}

float Sin01(float x)
{
    return sin(x) * 0.5 + 0.5;
}

float Cos01(float x)
{
    return cos(x) * 0.5 + 0.5;
}

vec3 DrawZero(vec2 point, vec2 uv)
{
    vec3 result = vec3(0.0);
    for (int i = 0; i < 50; ++i)
    {
        float start = Rand_GetFloat01();
        float t = GetTime();
        float r1 = Rand_GetFloat01();
        float r2 = Rand_GetFloat01();
        float r3 = Rand_GetFloat01();
        float r4 = Rand_GetFloat01();
        float x = cos(t * r1 + start) * r3;
        float y = sin(t * r2 + start) * r4;
        vec3 color = vec3(Sin01(Rand_GetFloat01() * GetTime() + Rand_GetFloat01()), Cos01(Rand_GetFloat01() * GetTime()),Sin01(Rand_GetFloat01() * GetTime())) * Rand_Range(0.25, 0.5);
        float radius = Rand_GetFloat01() * 0.2 + 0.01;
        float slope = Rand_Range(0.001, 0.5) * radius;
        result += max(color * Circle(Rand_Sample2D() * 2.0 - 1.0 + vec2(x, y), radius, slope, uv), 0.0);
    }

    for (int i = 0; i < 10; ++i)
    {
        float start = Rand_GetFloat01();
        float t = GetTime();
        float r1 = Rand_GetFloat01();
        float r2 = Rand_GetFloat01();
        float r3 = Rand_GetFloat01() * 1.5;
        float r4 = Rand_GetFloat01() * 1.5;
        float x = cos(t * r1 + start) * r3;
        float y = sin(t * r2 + start) * r4;
        vec3 color = Rand_Sample3D() * Rand_Range(0.1, 0.25);
        float radius = 0.3;//Rand_GetFloat01() * 0.2 + 0.01;
        float slope = Rand_Range(0.001, 0.5);
        result += max(color * Circle(Rand_Sample2D() * 2.0 - 1.0 + vec2(x, y), radius, slope, uv), 0.0);
    }

    for (int i = 0; i < 10; ++i)
    {
        float start = Rand_GetFloat01();
        float t = GetTime();
        float r1 = Rand_GetFloat01();
        float r2 = Rand_GetFloat01();
        float r3 = Rand_GetFloat01() * 1.5;
        float r4 = Rand_GetFloat01() * 1.5;
        float x = cos(t * r1 + start) * r3;
        float y = sin(t * r2 + start) * r4;
        vec3 color = Rand_Sample3D() * Rand_Range(0.1, 0.25);
        float radius = 0.7;//Rand_GetFloat01() * 0.2 + 0.01;
        float slope = Rand_Range(0.001, 0.5);
        result += max(color * Circle(Rand_Sample2D() * 2.0 - 1.0 + vec2(x, y), radius, slope, uv), 0.0);
    }

    return max(result, 0.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    Rand_Init(int(32));
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    uv -= vec2(0.5);
    uv.x *= iResolution.x / iResolution.y;

    // Time varying pixel color
    vec3 col = DrawZero(vec2(0.0, 0.0), uv);

    // Output to screen
    fragColor = vec4(col,1.0);
}
        void main()
        {
            vec4 color = vec4(0.0,0.0,0.0,1.0);
            mainImage(color, gl_FragCoord.xy);
            color.w = 1.0;
            gl_FragColor = color;
        }
        )";

// stolen and adapted from https://www.shadertoy.com/view/Xdc3WX
// this is a simple loading circle(s)
static const char *fragmentShader1=
        R"(
        #version 330 core
        #ifdef GL_ES
        precision highp float;
        #endif
                uniform vec2      iResolution;
                uniform float   iTime;
        #define PI 3.14159265

        void mainImage( out vec4 fragColor, in vec2 fragCoord )
        {
            float time = iTime;
            float mx = max(iResolution.x, iResolution.y);
            vec2 scrs = iResolution.xy/mx;
            vec2 uv = vec2(fragCoord.x, iResolution.y-fragCoord.y)/mx;

            vec3 col = vec3(0.0);
            float x,y = 0.0;
            float radius = 0.02;
            const float dotsnb = 10.0;

            for(float i = 0.0 ; i < dotsnb ; i++){
                x = 0.1*cos(2.0*PI*i/dotsnb+time*(i+3.0)/3.0);
                y = 0.1*sin(2.0*PI*i/dotsnb+time*(i+3.0)/3.0);

                col += vec3(smoothstep(radius, radius-0.01, distance(uv, scrs/2.0 + vec2(x,y)) ) * (sin(i/dotsnb+time+2.0*PI/3.0)+1.0)/2.0,
                            smoothstep(radius, radius-0.01, distance(uv, scrs/2.0 + vec2(x,y)) ) * (sin(i/dotsnb+time+4.0*PI/3.0)+1.0)/2.0,
                            smoothstep(radius, radius-0.01, distance(uv, scrs/2.0 + vec2(x,y)) ) * (sin(i/dotsnb+time+6.0*PI/3.0)+1.0)/2.0);
            }

            fragColor = vec4(col,1.0);
        }
        void main()
        {
            vec4 color = vec4(0.0,0.0,0.0,1.0);
            mainImage(color, gl_FragCoord.xy);
            color.w = 1.0;
            gl_FragColor = color;
        }
        )";

}
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
    program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader1);
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
    //auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - showingSpinnerStartTime);
    auto duration = std::chrono::duration_cast<std::chrono::duration<float,std::ratio<1,1> >>(std::chrono::steady_clock::now() - showingSpinnerStartTime);

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
    case MPV_EVENT_FILE_LOADED:
        startRenderingMedia();
        emit fileLoaded();
        break;
    default: ;
        // Ignore uninteresting or unknown events.
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
