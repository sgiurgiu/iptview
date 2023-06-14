
// stolen and adapted from https://www.shadertoy.com/view/3djGRm
// this is a full screen of circles moving

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
