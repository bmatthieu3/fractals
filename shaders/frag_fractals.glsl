#version 330 core
precision highp float;

out vec4 color;

in vec3 pos_screen;

uniform float time;
uniform float zoom;
uniform float deplt_x;
uniform float deplt_y;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

int numOctaves = 10;
float fbm(in vec2 x, in float H)
{    
    float G = exp2(-H);
    float f = 1.0;
    float a = 1.0;
    float t = 0.0;
    for(int i=0; i<numOctaves; i++) {
        t += a*noise(f*x);
        f *= 2.0;
        a *= G;
    }
    return t;
}

float warp_second(in vec2 x) {
    vec2 q = vec2(fbm(x + vec2(0.0, 0.2)*time/2.f, 1.f),
                  fbm(x + vec2(2.2, 0.3)*time/2.f, 1.f));
    return fbm(x + 4.0f*q, 1.f);
}

float warp_third(in vec2 x) {
    vec2 q = vec2(fbm(x + vec2(5.0, 7.2), 1.f),
                  fbm(x + vec2(-2.5, 5.3), 1.f));

    return warp_second(x + 4.0f*q);
}

float in_mandelbrot_set(in vec2 x) {
    float re_c = x.x;
    float im_c = x.y;
    
    float re_z = 0.f;
    float im_z = 0.f;

    int N = 100;

    float factor = 1.f;
    for(int n = 0; n < N; n++) {
        float re_z_next = re_z*re_z - im_z*im_z + re_c;
        im_z = im_c + 2.f*re_z*im_z;
        re_z = re_z_next;

        float r = length(vec2(re_z, im_z));

        if(r > 2.f) {
            factor = float(n)/(N - 1);
            break;
        }
    }

    return factor;
}

void main() {
    vec2 p = pos_screen.xy;
    //float factor = warp_third(p*10)/3.f;

    //vec2 h = vec2(fbm(p + time*vec2(0.6, 0.8), 1.0f), fbm(p + time*vec2(-5.6, 8.8), 1.0f));
    float factor = 5*in_mandelbrot_set(p);

    vec4 c0 = vec4(10/255.f, 10/255.f, 100/255.f, 1.f);
    vec4 c1 = vec4(10/255.f, 10/255.f, 130/255.f, 1.f);
    vec4 c2 = vec4(50/255.f, 10/255.f, 176/255.f, 1.f);
    vec4 c3 = vec4(10/255.f, 10/255.f, 0/255.f, 1.f);

    color = mix(c0, c1, smoothstep(0.f, 0.33f, factor));
    color = mix(color, c2, smoothstep(0.33f, 0.66f, factor));
    color = mix(color, c3, smoothstep(0.66f, 1.f, factor));
}