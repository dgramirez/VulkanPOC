#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

//Uniform
layout(binding = 0) uniform mvp_object {
	mat4 model;
    mat4 vert_col;
    vec2 uv_offset;
} mvp;

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inUV;

layout(location=0) out vec4 outColor;

void main()
{
    gl_Position = mvp.model * vec4(inPosition, 1.0f);
    outColor = mvp.vert_col[gl_VertexIndex];
}
