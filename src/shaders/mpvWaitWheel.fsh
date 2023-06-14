// stolen and adapted from https://www.shadertoy.com/view/Xdc3WX
// this is a simple loading circle(s)

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
