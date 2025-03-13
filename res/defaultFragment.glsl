#version 410

out vec4 out_color;
uniform vec3 light_direction = vec3(1, 1, 1);
uniform float light_intensity = 1.;
uniform vec3 light_color = vec3(1, 1, 1);
uniform vec4 color = vec4(1, 1, 1, 1);
uniform vec4[6] point_lights; // pos 3 max length 1
uniform vec4[6] point_lights_color; // color 3 intensity 1
uniform int light_count = 0;
uniform sampler2D texture_sample;
in vec3 vertex_position;
in vec2 uv;
in vec3 normal;
in vec3 position_ws;
in vec3 normal_ws;

void main() {
    int curr_light_count = light_count;
    vec4 light = vec4(0, 0, 0, 0);

    light.w = dot(normal_ws, normalize(light_direction)) * light_intensity;
    if(light.w < 0)
        light.w = 0;
    // dot_light += 0.3;

    for(int i; i < light_count; i++) {
        if(point_lights[i].w > 0) {
            vec3 point_light_direction = vec3(point_lights[i].x, point_lights[i].y, point_lights[i].z) - position_ws;
            float dot_point_light = dot(normalize(point_light_direction), normal_ws);
            float distance = (point_lights[i].w - length(point_light_direction));
            if(distance > 0) {
                dot_point_light *= distance;

                if(point_lights[i].w > 1)
                    dot_point_light /= point_lights[i].w;

            }

            dot_point_light *= point_lights_color[i].w;

            if(dot_point_light > light.w)
                light.w = dot_point_light;

            light.x += point_lights_color[i].x * point_lights_color[i].z;
            light.y += point_lights_color[i].y * point_lights_color[i].z;
            light.z += point_lights_color[i].z * point_lights_color[i].z;
        } else {
            curr_light_count--; // light doesnt contribute, remove it
        }
    }

    curr_light_count++;
    
    light.x = (light.x + light_color.x) /curr_light_count;
    light.y = (light.y + light_color.y) / curr_light_count;
    light.z = (light.z + light_color.z) / curr_light_count;

    vec4 tex_color = texture(texture_sample, vec2(uv.x, uv.y));
    tex_color.x *= light.w;
    tex_color.y *= light.w;
    tex_color.z *= light.w;

    out_color = vec4(color * tex_color);
}