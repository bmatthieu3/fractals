#version 330 core
precision highp float;

out vec4 color;

in vec3 pos_screen;

uniform float time;
uniform float zoom;
uniform float deplt_x;
uniform float deplt_y;

void main() {
    float off_re = -0.743643887037151;
    float off_im = 0.13182590420533;

    float zoom = min(exp(time), 30000.f);
    //float zoom = 1.f;
    float re_c = (pos_screen.x + deplt_x)/(zoom) + off_re;
    float im_c = (pos_screen.y + deplt_y)/(zoom) + off_im;

    float re_z = 0.f;
    float im_z = 0.f;

    int N = 200;
    float max_intensity = 0.5f;
    vec4 c0 = vec4(0/255.f, 0/255.f, 0/255.f, 1.f);
    vec4 c1 = vec4(18/255.f, 23/255.f, 76/255.f, 1.f);
    vec4 c2 = vec4(50/255.f, 65/255.f, 130/255.f, 1.f);

    float factor = 1.f;
    for(int n = 0; n < N; n++) {
        float re_z_next = re_z*re_z - im_z*im_z + re_c;
        im_z = im_c + 2.f*re_z*im_z;
        re_z = re_z_next;

        float r = length(vec2(re_z, im_z));

        if(r > 2.f) {
            factor = float(n)/float(N-1);
            //color = vec4(0.f, factor, 0.f, 1.f);
            break;
        }
    }

    color = mix(c0, c1, smoothstep(0.f, 0.5f, factor));
    color = mix(color, c2, smoothstep(0.5f, 1.f, factor));
}