#version 330 core
precision highp float;

out vec4 color;

in vec3 pos_screen;

uniform float time;
uniform float zoom;
uniform float deplt_x;
uniform float deplt_y;

uniform float width;
uniform float height;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 eye;

const int NUM_MAX_ITERATION = 100;
const float MAX_DISTANCE = 1000.f;
int numOctaves = 6;
float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}
float fbm3d(in vec3 x, in float H)
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
float sdf_sphere(vec3 p, vec3 c, float r) {
    return length(p - c) - r;
}

float sdf_mandelbulb(vec3 r, int n) {
    vec3 zn = r.xyz;
	float rad = 0.0;
	float hit = 0.0;
	float p = 8.0;
	float d = 1.0;
	for( int i = 0; i < 1000; i++ )
	{
        rad = length(zn);

        if(rad > 2.0) {	
            hit = 0.5 * log(rad) * rad / d;
            break;
        } else {
            float th = atan( length( zn.xy ), zn.z );
            float phi = atan( zn.y, zn.x );		
            float rado = pow(rad,8.0);
            d = pow(rad, 7.0) * 7.0 * d + 1.0;

            float sint = sin( th * p );
            zn.x = rado * sint * cos( phi * p );
            zn.y = rado * sint * sin( phi * p );
            zn.z = rado * cos( th * p ) ;
            zn += r;
        }
	}
	
	return hit;
}

vec4 ray_marching(vec3 p_viewport) {
    vec4 p_homogenous_clip = vec4(p_viewport.xy, -1.f, 1.f);
    vec4 p_eye = inverse(projection) * p_homogenous_clip;
    p_eye = vec4(p_eye.xy, -1.f, 0.f);
    vec3 dir = (inverse(view) * p_eye).xyz;
    dir = normalize(dir);

    float depth = 0.01f;
    for (int step = 0; step < NUM_MAX_ITERATION; step++) {
        vec3 p = eye + depth * dir;
        float dist = sdf_sphere(p, vec3(0.0f), 1.f);

        if (dist < 1e-3) {
            // In the surface
            return vec4(vec3(p), 1.f);
        }

        depth = depth + dist;

        if (depth > MAX_DISTANCE) {
            return vec4(0.f, 0.f, 0.f, 1.f);
        }
    }

    return vec4(0.f);
}



float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

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
    vec2 q = vec2(fbm(x + vec2(4.0, 0.2)*time/3.f, 1.f),
                  fbm(x + vec2(2.2, 2.3)*time/3.f, 1.f));

    color = mix(vec4(50/255.f, 50/255.f, 100/255.f, 1.f), vec4(200/255.f, 200/255.f, 15/255.f, 1.f), q.x);
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

    int N = 500;

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
    //p.y /= (width/height);
    
    //float factor = warp_third(p*5)/3.f * (sin(time/2.f)*0.5 + 0.5);

    //vec2 h = vec2(fbm(p + time*vec2(0.6, 0.8), 1.0f), fbm(p + time*vec2(-5.6, 8.8), 1.0f));
    /*float factor = in_mandelbrot_set((0.001f + 0.0005f*sin(time/2.f))*p + vec2(-0.06783611264225832, 0.6617460391250546));

    vec4 c0 = vec4(0/255.f, 0/255.f, 0/255.f, 1.f);
    vec4 c1 = vec4(57/255.f, 136/255.f, 243/255.f, 1.f);
    vec4 c2 = vec4(233/255.f, 162/255.f, 0/255.f, 1.f);
    vec4 c3 = vec4(15/255.f, 55/255.f, 240/255.f, 1.f);

    color = mix(c0, c1, smoothstep(0.f, 0.33f, factor));
    color = mix(color, c2, smoothstep(0.33f, 0.66f, factor));
    color = mix(color, c3, smoothstep(0.66f, 1.f, factor));
    */
    color = ray_marching(pos_screen);
}