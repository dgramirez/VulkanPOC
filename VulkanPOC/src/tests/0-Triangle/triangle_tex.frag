#version 450
#extension GL_ARB_separate_shader_objects : enable

//Binding Texture
layout(binding=1) uniform sampler2D uv_sampler;

//Object's info
layout(location=0) in vec2 inUV;		//Texture's UVs
layout(location=1) in vec2 inUVOffset;

//Return Value
layout(location=0) out vec4 out_color;

void main()
{
	//Get Object Color
	vec4 object_color = texture(uv_sampler, vec2(inUV.x + inUVOffset.x, inUV.y + inUVOffset.y) );
	out_color = object_color;
}