#version 300 es
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

uniform mat4 view;
uniform mat4 projection;

out vec3 v_tex_coords;

void main()
{
    gl_Position = projection * view * vec4(a_position, 1.0);
    // interpolated cube vertex position match cubemap texture coordinates
    v_tex_coords = a_position;
}
