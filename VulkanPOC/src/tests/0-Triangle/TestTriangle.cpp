#include "TestTriangle.h"
#include "vulkan/vulkan.h"
#include "../../vulkan/GVulkanHelper.h"
#include "../../vulkan/GVulkanGlobal.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"
#include "glm.hpp"
#include "common.hpp"
#include "gtc/matrix_transform.hpp"
#define STBI_SUPPORT_ZLIB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>

namespace {
	//For Vertex
	static const uint8_t POSITION = 0;
	static const uint8_t UV = 1;

	//For Shaders
	static const uint8_t VERTEX = 0;
	static const uint8_t FRAGMENT = 1;

	struct Vertex {
		glm::vec3 pos;
		glm::vec2 uv;

		static VkVertexInputBindingDescription get_binding_description()
		{
			VkVertexInputBindingDescription vertex_input_binding_description = {};

			vertex_input_binding_description.binding = 0;
			vertex_input_binding_description.stride = sizeof(Vertex);
			vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return vertex_input_binding_description;
		}
		static VkVertexInputAttributeDescription* get_attribute_description()
		{
			VkVertexInputAttributeDescription* attribute_description = new VkVertexInputAttributeDescription[2];

			attribute_description[POSITION].binding = 0;
			attribute_description[POSITION].location = POSITION;
			attribute_description[POSITION].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_description[POSITION].offset = 0;

			attribute_description[UV].binding = 0;
			attribute_description[UV].location = UV;
			attribute_description[UV].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_description[UV].offset = 12;

			return attribute_description;
		}
	};
	const uint32_t vertex_size = 3;
	Vertex my_triangle[vertex_size];

	//Uniform Buffer Object
	struct UBO {
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 vert_colors = glm::mat4(1.0f);
		glm::vec2 uv_offset = {};
	};
	UBO ubo_struct;
	typedef void(*UniformFctn)(UBO& uniform_buffer_object);
	UniformFctn ubo_func;

	//Model Matrix
	glm::vec3 translation;
	glm::vec3 rotation;
	glm::vec3 scaling = {1.0f, 1.0f, 1.0f};

	//Color vs Texture
	bool use_texture = false;
	unsigned int failed_window = false;
	uint32_t tex_stage = 0;

	//Colors
	glm::vec4 v1 = { 1.0f, 0.0f, 0.0f, 1.0f };
	glm::vec4 v2 = { 0.0f, 1.0f, 0.0f, 1.0f };
	glm::vec4 v3 = { 0.0f, 0.0f, 1.0f, 1.0f };
	
	//Texturing
	char filename[256];
	glm::vec2 uv;
	const char *load_or_unload = "Load Texture";

	//Vulkan Buffers
	VkBuffer vkMyTriangleBuffer;
	VkDeviceMemory vkMyTriangleMemory;

	//Uniform Buffer
	std::vector<VkBuffer> vkMyUniformBuffer;
	std::vector<VkDeviceMemory> vkMyUniformMemory;

	//Vulkan Descriptor & Pipelines
	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	std::vector<VkDescriptorSet> descriptor_set;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	std::vector<VkDescriptorSet>& current_descriptor_set = descriptor_set;

	//Vulkan Descriptor & Pipelines
	VkDescriptorPool descriptor_pool_tex;
	VkDescriptorSetLayout descriptor_set_layout_tex;
	std::vector<VkDescriptorSet> descriptor_set_tex;
	VkPipelineLayout pipeline_layout_tex;
	VkPipeline graphics_pipeline_tex;

	//Vulkan Current Modes
	VkPipelineLayout *current_pipeline_layout;
	VkPipeline *current_pipeline;

	//Image
	int image_width, image_height, image_mips;
	unsigned char *myImage = nullptr;
	VkImage vkMyImage;
	VkImageView vkMyImageView;
	VkDeviceMemory vkMyImageMemory;
	VkSampler vkMyImageSampler;
}

//Buffers
void vkCreateTriangleVB();
void vkCreateUniformBuffer();

//Color
void vkCreateDescriptors();
void vkCreateTrianglePipeline();
void vkUpdateColorDescriptorSets();

//Texture
void vkCreateImage();
void vkCreateDescriptorsTexture();
void vkCreateTrianglePipelineTexture();
void cleanup_texture();

//Pipeline Creation and Destruction
void vkCreatePipelines();
void vkDestroyPipelines();

Test0_Triangle::Test0_Triangle()
{
	//Create the Triangle
	my_triangle[0].pos = { 0.00f, -0.25f, 0.00f};
	my_triangle[0].uv  = {  0.50f,  0.00f};

	my_triangle[1].pos = { 0.25f,  0.25f, 0.00f};
	my_triangle[1].uv  = {  1.00f,  1.00f};

	my_triangle[2].pos = { -0.25f,  0.25f, 0.00f };
	my_triangle[2].uv  = {  0.00f,  1.00f};

	//setup vertex buffer
	vkCreateTriangleVB();

	//Setup Uniforms
	vkCreateUniformBuffer();
	
	//Create the pipelines (Color)
	vkCreateDescriptors();
	vkCreateTrianglePipeline();

	//Set Pipeline
	current_pipeline_layout = &pipeline_layout;
	current_pipeline = &graphics_pipeline;
	current_descriptor_set = descriptor_set;
}
Test0_Triangle::~Test0_Triangle() {
	//Cleanup Textures if necessary
	if (myImage)
	{
		stbi_image_free(myImage);
		myImage = nullptr;
		cleanup_texture();
	}
	else {
		//Wait for Queues to finish
		vkWaitForFences(vkStruct.vkDevice, 1, &vkStruct.vkFenceRendering, VK_TRUE, ~(static_cast<uint64_t>(0)));
		vkDeviceWaitIdle(vkStruct.vkDevice);
	}

	//Destroy Triangle Buffers
	vkDestroyBuffer(vkStruct.vkDevice, vkMyTriangleBuffer, nullptr);
	vkFreeMemory(vkStruct.vkDevice, vkMyTriangleMemory, nullptr);

	//Destroy Pipelines
	vkDestroyPipeline(vkStruct.vkDevice, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(vkStruct.vkDevice, pipeline_layout, nullptr);

	//Destroy Descriptor Set Layout
	vkDestroyDescriptorSetLayout(vkStruct.vkDevice, descriptor_set_layout, nullptr);

	//Destroy Descriptor Pool
	vkDestroyDescriptorPool(vkStruct.vkDevice, descriptor_pool, nullptr);

	//Destroy Uniform Buffers
	for (uint32_t i = 0; i < vkStruct.vkImageCount; ++i) {
		vkDestroyBuffer(vkStruct.vkDevice, vkMyUniformBuffer[i], nullptr);
		vkFreeMemory(vkStruct.vkDevice, vkMyUniformMemory[i], nullptr);
	}
}

void Test0_Triangle::Update(const float& _deltaTime)
{
	if (tex_stage & 1)
	{
		cleanup_texture();

		if (!(tex_stage & 2 )) //Stage 1 doesn't have the 2nd bit active. Stage 3 does
		{
			image_mips = static_cast<int>(log(glm::min(image_width, image_height)));
			vkCreateImage();
			vkCreateDescriptorsTexture();
			vkCreateTrianglePipelineTexture();
			current_pipeline = &graphics_pipeline_tex;
			current_pipeline_layout = &pipeline_layout_tex;
			current_descriptor_set = descriptor_set_tex;
		}
		else
		{
			vkUpdateColorDescriptorSets();
			load_or_unload = "Load Texture";
		}

		++tex_stage;
		tex_stage %= 4;
	}

	UBO new_ubo;
	const glm::mat4 id = glm::mat4(1.0f);
	const glm::vec3 xaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	const glm::vec3 yaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 zaxis = glm::vec3(0.0f, 0.0f, 1.0f);

	new_ubo.model = glm::translate(id, translation) * glm::rotate(id, rotation.z, zaxis) * glm::rotate(id, rotation.y, yaxis) * glm::rotate(id, rotation.x, xaxis) * glm::scale(id, scaling) * new_ubo.model;

	new_ubo.vert_colors[0] = v1;
	new_ubo.vert_colors[1] = v2;
	new_ubo.vert_colors[2] = v3;

	new_ubo.uv_offset = uv;

	write_to_buffer(vkStruct.vkDevice, new_ubo, vkMyUniformMemory[vkStruct.vkCurrentFrame]);
}
void Test0_Triangle::Render()
{
	VkDeviceSize offset[] = { 0 };
	vkCmdBindPipeline(vkStruct.vkSwapchainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *current_pipeline);
	vkCmdBindDescriptorSets(vkStruct.vkSwapchainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *current_pipeline_layout, 0, 1, &current_descriptor_set[vkStruct.vkCurrentFrame], 0, nullptr);
	vkCmdBindVertexBuffers(vkStruct.vkSwapchainCommandBuffer, 0, 1, &vkMyTriangleBuffer, offset);
	vkCmdDraw(vkStruct.vkSwapchainCommandBuffer, 3, 1, 0, 0);
}
void Test0_Triangle::RenderImGui()
{
	ImGui::Begin("Triangle Test");

	//Manipulating the Model Matrix
	ImGui::Text("Model Matrix");
	ImGui::SliderFloat3("Translation", reinterpret_cast<float*>(&translation), -1.0f, 1.0f);
	ImGui::SliderFloat3("Rotation", reinterpret_cast<float*>(&rotation), -3.1415927f, 3.1415927f);
	ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&scaling), 0.5f, 2.0f);

	//Setup Color vs Texture
	ImGui::Text("Model Colors");
	ImGui::Checkbox("Use Texture?", &use_texture);
	if (use_texture)
	{
		//Texture
		ImGui::InputText("Texture Filepath", filename, 260, 0, 0, nullptr);
		if (ImGui::Button(load_or_unload))
		{
			if (!tex_stage)
			{
				if (myImage) stbi_image_free(myImage);
				int n;
				myImage = stbi_load(filename, &image_width, &image_height, &n, 0);

				if (myImage)
				{
					++tex_stage;
					load_or_unload = "Unload Texture";
					failed_window = 0;
				}
				else
					++failed_window;
			}
			else if (tex_stage & 2)
				++tex_stage;

		}
		ImGui::SliderFloat2("UV Movement", (float*)&uv, -1.0f, 1.0f);
	}
	else
	{
		//Vertex Color
		ImGui::ColorEdit3("Vertex 1", reinterpret_cast<float*>(&v1));
		ImGui::ColorEdit3("Vertex 2", reinterpret_cast<float*>(&v2));
		ImGui::ColorEdit3("Vertex 3", reinterpret_cast<float*>(&v3));
	}
	ImGui::End();

	if (failed_window)
	{
		ImGui::Begin("Failure !");
		ImGui::Text("Failed to create image!");
		ImGui::Text("Counter: %d", failed_window);
		if (ImGui::Button("Close Window"))
			failed_window = 0;

		ImGui::End();
	}
}

VoidFuncPtr Test0_Triangle::SendCreatePipeline()
{
	return vkCreatePipelines;
}
VoidFuncPtr Test0_Triangle::SendDestroyPipeline()
{
	return vkDestroyPipelines;
}

void vkCreateTriangleVB()
{
	VkDeviceSize buffer_size = sizeof(Vertex) * vertex_size;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	create_buffer(vkStruct.vkPhysicalDevice, vkStruct.vkDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&staging_buffer, &staging_buffer_memory);

	void* data;
	vkMapMemory(vkStruct.vkDevice, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, my_triangle, buffer_size);
	vkUnmapMemory(vkStruct.vkDevice, staging_buffer_memory);

	create_buffer(vkStruct.vkPhysicalDevice, vkStruct.vkDevice, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&vkMyTriangleBuffer, &vkMyTriangleMemory);

	copy_buffer(vkStruct.vkDevice, vkStruct.vkCommandPool, vkStruct.vkQueueGraphics, staging_buffer, vkMyTriangleBuffer, buffer_size);

	vkDestroyBuffer(vkStruct.vkDevice, staging_buffer, nullptr);
	vkFreeMemory(vkStruct.vkDevice, staging_buffer_memory, nullptr);
}
void vkCreateUniformBuffer()
{
	vkMyUniformBuffer.resize(vkStruct.vkImageCount);
	vkMyUniformMemory.resize(vkStruct.vkImageCount);
	for (uint32_t i = 0; i < vkStruct.vkImageCount; ++i)
		create_buffer(vkStruct.vkPhysicalDevice, vkStruct.vkDevice, sizeof(UBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vkMyUniformBuffer[i], &vkMyUniformMemory[i]);
}

void vkCreateDescriptors()
{
	//Descriptor Pool
	VkDescriptorPoolSize dps = {};
	dps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dps.descriptorCount = 0xFF;

	VkDescriptorPoolCreateInfo dp_create_info = {};
	dp_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dp_create_info.poolSizeCount = 1;
	dp_create_info.pPoolSizes = &dps;
	dp_create_info.maxSets = 0xFF;
	vkCreateDescriptorPool(vkStruct.vkDevice, &dp_create_info, nullptr, &descriptor_pool);

	//Descriptor Set Layout
	VkDescriptorSetLayoutBinding vs_ubo = {};
	vs_ubo.binding = 0;
	vs_ubo.descriptorCount = 1;
	vs_ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vs_ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo dsl_create_info = {};
	dsl_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dsl_create_info.bindingCount = 1;
	dsl_create_info.pBindings = &vs_ubo;
	vkCreateDescriptorSetLayout(vkStruct.vkDevice, &dsl_create_info, nullptr, &descriptor_set_layout);

	//Descriptor Sets
	descriptor_set.resize(vkStruct.vkImageCount);
	std::vector<VkDescriptorSetLayout> dsl_list(vkStruct.vkImageCount, descriptor_set_layout);
	VkDescriptorSetAllocateInfo ds_allocate_info = {};
	ds_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ds_allocate_info.descriptorSetCount = vkStruct.vkImageCount;
	ds_allocate_info.descriptorPool = descriptor_pool;
	ds_allocate_info.pSetLayouts = &dsl_list[0];
	vkAllocateDescriptorSets(vkStruct.vkDevice, &ds_allocate_info, descriptor_set.data());

	for (uint32_t i = 0; i < vkStruct.vkImageCount; ++i)
	{
		VkDescriptorBufferInfo dbi = {};
		dbi.buffer = vkMyUniformBuffer[i];
		dbi.offset = 0;
		dbi.range = sizeof(UBO);

		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = descriptor_set[i];
		wds.dstBinding = 0;
		wds.dstArrayElement = 0;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wds.descriptorCount = 1;
		wds.pBufferInfo = &dbi;
		wds.pImageInfo = nullptr;
		wds.pTexelBufferView = nullptr;
		wds.pNext = nullptr;

		vkUpdateDescriptorSets(vkStruct.vkDevice, 1, &wds, 0, nullptr);
	}
}
void vkCreateTrianglePipeline()
{
	//Vertex & Fragment Shader Modules & Stages
	VkShaderModule shader[2] = {};
	VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
	create_shader(vkStruct.vkDevice, "./src/tests/0-Triangle/triangle.vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &shader[VERTEX], &stage_create_info[VERTEX]);
	create_shader(vkStruct.vkDevice, "./src/tests/0-Triangle/triangle.frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &shader[FRAGMENT], &stage_create_info[FRAGMENT]);

	//Assembly State
	VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
	assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assembly_create_info.primitiveRestartEnable = false;

	//Vertex Input State
	VkVertexInputBindingDescription vertex_binding_description = Vertex::get_binding_description();
	VkVertexInputAttributeDescription* vertex_attribute_description = Vertex::get_attribute_description();

	VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
	input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	input_vertex_info.vertexBindingDescriptionCount = 1;
	input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
	input_vertex_info.vertexAttributeDescriptionCount = 2;
	input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;

	//Viewport State
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(vkStruct.vkSwapchainExtent.width);
	viewport.height = static_cast<float>(vkStruct.vkSwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = vkStruct.vkSwapchainExtent;

	VkPipelineViewportStateCreateInfo viewport_create_info = {};
	viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_create_info.viewportCount = 1;
	viewport_create_info.pViewports = &viewport;
	viewport_create_info.scissorCount = 1;
	viewport_create_info.pScissors = &scissor;

	//Rasterizer State
	VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_create_info.lineWidth = 1.0f;
	rasterization_create_info.cullMode = VK_CULL_MODE_NONE;
	rasterization_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_create_info.depthClampEnable = VK_FALSE;
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	rasterization_create_info.depthBiasClamp = 0.0f;
	rasterization_create_info.depthBiasConstantFactor = 0.0f;
	rasterization_create_info.depthBiasSlopeFactor = 0.0f;

	//Multisampling State
	VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.sampleShadingEnable = VK_FALSE;
	multisample_create_info.rasterizationSamples = vkStruct.vkMSAABit;
	multisample_create_info.minSampleShading = 1.0f;
	multisample_create_info.pSampleMask = VK_NULL_HANDLE;
	multisample_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_create_info.alphaToOneEnable = VK_FALSE;

	//Depth-Stencil State
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.depthTestEnable = VK_FALSE;
	depth_stencil_create_info.depthWriteEnable = VK_FALSE;
	depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_create_info.minDepthBounds = 0.0f;
	depth_stencil_create_info.maxDepthBounds = 1.0f;
	depth_stencil_create_info.stencilTestEnable = VK_FALSE;

	//Color Blending Attachment & State
	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
	color_blend_attachment_state.colorWriteMask = 0xF; //<-- RGBA Flags on... although blend is disabled
	color_blend_attachment_state.blendEnable = VK_FALSE;
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_create_info.logicOpEnable = VK_FALSE;
	color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_create_info.attachmentCount = 1;
	color_blend_create_info.pAttachments = &color_blend_attachment_state;
	color_blend_create_info.blendConstants[0] = 0.0f;
	color_blend_create_info.blendConstants[1] = 0.0f;
	color_blend_create_info.blendConstants[2] = 0.0f;
	color_blend_create_info.blendConstants[3] = 0.0f;

	//Dynamic State [DISABLED.... But still showing for tutorial reasons that it exists]
	VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
	dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_create_info.dynamicStateCount = 0;
	dynamic_create_info.pDynamicStates = VK_NULL_HANDLE;

	//Descriptor pipeline layout [NOTE: NEEDED FOR UNIFORM BUFFERS!, but for now not using.]
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;
	VkResult r = vkCreatePipelineLayout(vkStruct.vkDevice, &pipeline_layout_create_info, nullptr, &pipeline_layout);

	//////////////////////////////////////////////////
	//												//
	//		FINALLY: GRAPHICS PIPELINE CREATION!	//
	//												//
	//////////////////////////////////////////////////

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = stage_create_info;
	pipeline_create_info.pInputAssemblyState = &assembly_create_info;
	pipeline_create_info.pVertexInputState = &input_vertex_info;
	pipeline_create_info.pViewportState = &viewport_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_create_info;
	pipeline_create_info.pMultisampleState = &multisample_create_info;
	pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
	pipeline_create_info.pColorBlendState = &color_blend_create_info;
	pipeline_create_info.pDynamicState = VK_NULL_HANDLE;

	pipeline_create_info.layout = pipeline_layout;
	pipeline_create_info.renderPass = vkStruct.vkSwapchainRenderPass;
	pipeline_create_info.subpass = 0;

	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex = -1;

	vkCreateGraphicsPipelines(vkStruct.vkDevice, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics_pipeline);

	//Cleanup
	delete[] vertex_attribute_description;
	vkDestroyShaderModule(vkStruct.vkDevice, shader[VERTEX], nullptr);
	vkDestroyShaderModule(vkStruct.vkDevice, shader[FRAGMENT], nullptr);
}
void vkUpdateColorDescriptorSets()
{
	//Descriptor Sets
	descriptor_set.resize(vkStruct.vkImageCount);
	std::vector<VkDescriptorSetLayout> dsl_list(vkStruct.vkImageCount, descriptor_set_layout);
	VkDescriptorSetAllocateInfo ds_allocate_info = {};
	ds_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ds_allocate_info.descriptorSetCount = vkStruct.vkImageCount;
	ds_allocate_info.descriptorPool = descriptor_pool;
	ds_allocate_info.pSetLayouts = &dsl_list[0];
	vkAllocateDescriptorSets(vkStruct.vkDevice, &ds_allocate_info, descriptor_set.data());

	for (uint32_t i = 0; i < vkStruct.vkImageCount; ++i)
	{
		VkDescriptorBufferInfo dbi = {};
		dbi.buffer = vkMyUniformBuffer[i];
		dbi.offset = 0;
		dbi.range = sizeof(UBO);

		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = descriptor_set[i];
		wds.dstBinding = 0;
		wds.dstArrayElement = 0;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wds.descriptorCount = 1;
		wds.pBufferInfo = &dbi;
		wds.pImageInfo = nullptr;
		wds.pTexelBufferView = nullptr;
		wds.pNext = nullptr;

		vkUpdateDescriptorSets(vkStruct.vkDevice, 1, &wds, 0, nullptr);
	}
}

void vkCreateImage()
{
	//Get the image size for the texture
	VkDeviceSize image_size = static_cast<uint64_t>(image_width) * static_cast<uint64_t>(image_height) * sizeof(uint32_t);

	//Get the staging bugger and memory needed to allocate
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	//Create the staging buffer
	create_buffer(vkStruct.vkPhysicalDevice, vkStruct.vkDevice, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

	//Allocate the data into the buffer
	void* allocate_data = nullptr;
	vkMapMemory(vkStruct.vkDevice, staging_buffer_memory, 0, image_size, 0, &allocate_data);
	memcpy(allocate_data, myImage, (unsigned int)image_size);
	vkUnmapMemory(vkStruct.vkDevice, staging_buffer_memory);

	VkExtent3D extent = { static_cast<uint32_t>(image_width), static_cast<uint32_t>(image_height), 1 };

	//Create the image, using appropriate information (Mip Levels, Texture data, etc.)
	create_image(vkStruct.vkPhysicalDevice, vkStruct.vkDevice, extent, image_mips, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &vkMyImage, &vkMyImageMemory);

	//Transition, using memory barriers, from Undefined Layout to Transfer to Destination (Optimal)
	transition_image_layout(vkStruct.vkDevice, vkStruct.vkCommandPool, vkStruct.vkQueueGraphics, image_mips, vkMyImage,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//Copy the buffer to the image
	copy_buffer_to_image(vkStruct.vkDevice, vkStruct.vkCommandPool, vkStruct.vkQueueGraphics, staging_buffer, vkMyImage, extent);

	//Destroy memory created from staging buffer
	vkDestroyBuffer(vkStruct.vkDevice, staging_buffer, nullptr);
	vkFreeMemory(vkStruct.vkDevice, staging_buffer_memory, nullptr);

	//Create the mipmaps for texture
	create_mipmaps(vkStruct.vkDevice, vkStruct.vkCommandPool, vkStruct.vkQueueGraphics, vkMyImage, image_width, image_height, image_mips);

	//Image View Create Info
	VkImageViewCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = vkMyImage;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = image_mips;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;
	create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	create_info.components.a = VK_COMPONENT_SWIZZLE_A;

	//Create the Surface (With Results) [VK_SUCCESS = 0]
	vkCreateImageView(vkStruct.vkDevice, &create_info, nullptr, &vkMyImageView);

	//Create the sampler create info
	VkSamplerCreateInfo sampler_create_info = {};
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.magFilter = VK_FILTER_LINEAR;
	sampler_create_info.minFilter = VK_FILTER_LINEAR;
	sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	sampler_create_info.anisotropyEnable = VK_TRUE;
	sampler_create_info.maxAnisotropy = static_cast<float>(vkStruct.vkMSAABit);
	sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_create_info.unnormalizedCoordinates = VK_FALSE;
	sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create_info.mipLodBias = 0.0f;
	sampler_create_info.minLod = 0.0f;
	sampler_create_info.maxLod = static_cast<float>(image_mips);

	vkCreateSampler(vkStruct.vkDevice, &sampler_create_info, nullptr, &vkMyImageSampler);
}
void vkCreateDescriptorsTexture()
{
	//Descriptor Pool
	VkDescriptorPoolSize dps = {};
	dps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dps.descriptorCount = 0xFF;

	VkDescriptorPoolSize dps2 = {};
	dps2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	dps2.descriptorCount = 0xFF;

	const uint32_t TOTAL_DESCRIPTORS = 2;
	VkDescriptorPoolSize dpsarray[TOTAL_DESCRIPTORS] = { dps , dps2 };
	VkDescriptorPoolCreateInfo dp_create_info = {};
	dp_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dp_create_info.poolSizeCount = TOTAL_DESCRIPTORS;
	dp_create_info.pPoolSizes = dpsarray;
	dp_create_info.maxSets = 0xFF * TOTAL_DESCRIPTORS;
	vkCreateDescriptorPool(vkStruct.vkDevice, &dp_create_info, nullptr, &descriptor_pool_tex);

	//Descriptor Set Layout
	VkDescriptorSetLayoutBinding vs_ubo = {};
	vs_ubo.binding = 0;
	vs_ubo.descriptorCount = 1;
	vs_ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vs_ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding ps_img = {};
	ps_img.binding = 1;
	ps_img.descriptorCount = 1;
	ps_img.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ps_img.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding dslbindings[TOTAL_DESCRIPTORS] = { vs_ubo, ps_img };
	VkDescriptorSetLayoutCreateInfo dsl_create_info = {};
	dsl_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dsl_create_info.bindingCount = TOTAL_DESCRIPTORS;
	dsl_create_info.pBindings = dslbindings;
	vkCreateDescriptorSetLayout(vkStruct.vkDevice, &dsl_create_info, nullptr, &descriptor_set_layout_tex);

	//Descriptor Sets
	std::vector<VkDescriptorSetLayout> dsl_list(vkStruct.vkImageCount, descriptor_set_layout_tex);
	VkDescriptorSetAllocateInfo ds_allocate_info = {};
	ds_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ds_allocate_info.descriptorSetCount = vkStruct.vkImageCount;
	ds_allocate_info.descriptorPool = descriptor_pool_tex;
	ds_allocate_info.pSetLayouts = dsl_list.data();
	
	descriptor_set_tex.resize(vkStruct.vkImageCount);
	vkAllocateDescriptorSets(vkStruct.vkDevice, &ds_allocate_info, descriptor_set_tex.data());

	for (uint32_t i = 0; i < vkStruct.vkImageCount; ++i)
	{
		VkDescriptorBufferInfo dbi = {};
		dbi.buffer = vkMyUniformBuffer[i];
		dbi.offset = 0;
		dbi.range = sizeof(UBO);

		VkWriteDescriptorSet wds[TOTAL_DESCRIPTORS] = {};
		wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds[0].dstSet = descriptor_set_tex[i];
		wds[0].dstBinding = 0;
		wds[0].dstArrayElement = 0;
		wds[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wds[0].descriptorCount = 1;
		wds[0].pBufferInfo = &dbi;
		wds[0].pImageInfo = nullptr;
		wds[0].pTexelBufferView = nullptr;
		wds[0].pNext = nullptr;

		VkDescriptorImageInfo descriptor_image_info = {};
		descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptor_image_info.imageView = vkMyImageView;
		descriptor_image_info.sampler = vkMyImageSampler;

		wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds[1].dstSet = descriptor_set_tex[i];
		wds[1].dstBinding = 1;
		wds[1].dstArrayElement = 0;
		wds[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds[1].descriptorCount = 1;
		wds[1].pBufferInfo = nullptr;
		wds[1].pImageInfo = &descriptor_image_info;
		wds[1].pTexelBufferView = nullptr;
		wds[1].pNext = nullptr;

		vkUpdateDescriptorSets(vkStruct.vkDevice, TOTAL_DESCRIPTORS, wds, 0, nullptr);
	}
}
void vkCreateTrianglePipelineTexture()
{
	//Vertex & Fragment Shader Modules & Stages
	VkShaderModule shader[2] = {};
	VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
	create_shader(vkStruct.vkDevice, "./src/tests/0-Triangle/triangle_tex.vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &shader[VERTEX], &stage_create_info[VERTEX]);
	create_shader(vkStruct.vkDevice, "./src/tests/0-Triangle/triangle_tex.frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &shader[FRAGMENT], &stage_create_info[FRAGMENT]);

	//Assembly State
	VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
	assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assembly_create_info.primitiveRestartEnable = false;

	//Vertex Input State
	VkVertexInputBindingDescription vertex_binding_description = Vertex::get_binding_description();
	VkVertexInputAttributeDescription* vertex_attribute_description = Vertex::get_attribute_description();

	VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
	input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	input_vertex_info.vertexBindingDescriptionCount = 1;
	input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
	input_vertex_info.vertexAttributeDescriptionCount = 2;
	input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;

	//Viewport State
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(vkStruct.vkSwapchainExtent.width);
	viewport.height = static_cast<float>(vkStruct.vkSwapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = vkStruct.vkSwapchainExtent;

	VkPipelineViewportStateCreateInfo viewport_create_info = {};
	viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_create_info.viewportCount = 1;
	viewport_create_info.pViewports = &viewport;
	viewport_create_info.scissorCount = 1;
	viewport_create_info.pScissors = &scissor;

	//Rasterizer State
	VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_create_info.lineWidth = 1.0f;
	rasterization_create_info.cullMode = VK_CULL_MODE_NONE;
	rasterization_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_create_info.depthClampEnable = VK_FALSE;
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	rasterization_create_info.depthBiasClamp = 0.0f;
	rasterization_create_info.depthBiasConstantFactor = 0.0f;
	rasterization_create_info.depthBiasSlopeFactor = 0.0f;

	//Multisampling State
	VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.sampleShadingEnable = VK_FALSE;
	multisample_create_info.rasterizationSamples = vkStruct.vkMSAABit;
	multisample_create_info.minSampleShading = 1.0f;
	multisample_create_info.pSampleMask = VK_NULL_HANDLE;
	multisample_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_create_info.alphaToOneEnable = VK_FALSE;

	//Depth-Stencil State
	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.depthTestEnable = VK_FALSE;
	depth_stencil_create_info.depthWriteEnable = VK_FALSE;
	depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_create_info.minDepthBounds = 0.0f;
	depth_stencil_create_info.maxDepthBounds = 1.0f;
	depth_stencil_create_info.stencilTestEnable = VK_FALSE;

	//Color Blending Attachment & State
	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
	color_blend_attachment_state.colorWriteMask = 0xF; //<-- RGBA Flags on... although blend is disabled
	color_blend_attachment_state.blendEnable = VK_FALSE;
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_create_info.logicOpEnable = VK_FALSE;
	color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_create_info.attachmentCount = 1;
	color_blend_create_info.pAttachments = &color_blend_attachment_state;
	color_blend_create_info.blendConstants[0] = 0.0f;
	color_blend_create_info.blendConstants[1] = 0.0f;
	color_blend_create_info.blendConstants[2] = 0.0f;
	color_blend_create_info.blendConstants[3] = 0.0f;

	//Dynamic State [DISABLED.... But still showing for tutorial reasons that it exists]
	VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
	dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_create_info.dynamicStateCount = 0;
	dynamic_create_info.pDynamicStates = VK_NULL_HANDLE;

	//Descriptor pipeline layout [NOTE: NEEDED FOR UNIFORM BUFFERS!, but for now not using.]
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout_tex;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;
	VkResult r = vkCreatePipelineLayout(vkStruct.vkDevice, &pipeline_layout_create_info, nullptr, &pipeline_layout_tex);

	//////////////////////////////////////////////////
	//												//
	//		FINALLY: GRAPHICS PIPELINE CREATION!	//
	//												//
	//////////////////////////////////////////////////

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = stage_create_info;
	pipeline_create_info.pInputAssemblyState = &assembly_create_info;
	pipeline_create_info.pVertexInputState = &input_vertex_info;
	pipeline_create_info.pViewportState = &viewport_create_info;
	pipeline_create_info.pRasterizationState = &rasterization_create_info;
	pipeline_create_info.pMultisampleState = &multisample_create_info;
	pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
	pipeline_create_info.pColorBlendState = &color_blend_create_info;
	pipeline_create_info.pDynamicState = VK_NULL_HANDLE;

	pipeline_create_info.layout = pipeline_layout_tex;
	pipeline_create_info.renderPass = vkStruct.vkSwapchainRenderPass;
	pipeline_create_info.subpass = 0;

	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex = -1;

	vkCreateGraphicsPipelines(vkStruct.vkDevice, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics_pipeline_tex);

	//Cleanup
	delete[] vertex_attribute_description;
	vkDestroyShaderModule(vkStruct.vkDevice, shader[VERTEX], nullptr);
	vkDestroyShaderModule(vkStruct.vkDevice, shader[FRAGMENT], nullptr);
}
void cleanup_texture()
{
	//Set the pipeline back
	current_pipeline = &graphics_pipeline;
	current_pipeline_layout = &pipeline_layout;
	current_descriptor_set = descriptor_set;

	//Wait for Queues to finish
	vkWaitForFences(vkStruct.vkDevice, 1, &vkStruct.vkFenceRendering, VK_TRUE, ~(static_cast<uint64_t>(0)));
	vkDeviceWaitIdle(vkStruct.vkDevice);

	//Destroy Pipelines
	if (graphics_pipeline_tex)	{vkDestroyPipeline(vkStruct.vkDevice, graphics_pipeline_tex, nullptr);		graphics_pipeline_tex = VK_NULL_HANDLE; }
	if (pipeline_layout_tex)	{vkDestroyPipelineLayout(vkStruct.vkDevice, pipeline_layout_tex, nullptr);	pipeline_layout_tex = VK_NULL_HANDLE;   }

	//Destroy Descriptor Set Layout
	if (descriptor_set_layout_tex) {vkDestroyDescriptorSetLayout(vkStruct.vkDevice, descriptor_set_layout_tex, nullptr); descriptor_set_layout_tex = VK_NULL_HANDLE; }

	//Destroy Descriptor Pool
	if (descriptor_pool_tex) { vkDestroyDescriptorPool(vkStruct.vkDevice, descriptor_pool_tex, nullptr);	descriptor_pool_tex = VK_NULL_HANDLE; }
	//Destroy Sampler
	if (vkMyImageSampler) { vkDestroySampler(vkStruct.vkDevice, vkMyImageSampler, nullptr); vkMyImageSampler = VK_NULL_HANDLE; }

	//Free up memory
	if (vkMyImage) {vkDestroyImage(vkStruct.vkDevice, vkMyImage, nullptr); vkMyImage = VK_NULL_HANDLE;			  }
	if (vkMyImageView) {vkDestroyImageView(vkStruct.vkDevice, vkMyImageView, nullptr); vkMyImageView = VK_NULL_HANDLE;}
	if (vkMyImageMemory) {vkFreeMemory(vkStruct.vkDevice, vkMyImageMemory, nullptr); vkMyImageMemory = VK_NULL_HANDLE;  }
}

void vkCreatePipelines()
{
	vkCreateTrianglePipeline();
	if (tex_stage == 2)
		vkCreateTrianglePipelineTexture();
}
void vkDestroyPipelines()
{
	//Destroy Pipelines
	if (graphics_pipeline_tex) { vkDestroyPipeline(vkStruct.vkDevice, graphics_pipeline_tex, nullptr);		graphics_pipeline_tex = VK_NULL_HANDLE; }
	if (pipeline_layout_tex) { vkDestroyPipelineLayout(vkStruct.vkDevice, pipeline_layout_tex, nullptr);	pipeline_layout_tex = VK_NULL_HANDLE; }

	//Destroy Pipelines
	if (graphics_pipeline) { vkDestroyPipeline(vkStruct.vkDevice, graphics_pipeline, nullptr); graphics_pipeline = VK_NULL_HANDLE; }
	if (pipeline_layout) { vkDestroyPipelineLayout(vkStruct.vkDevice, pipeline_layout, nullptr); pipeline_layout = VK_NULL_HANDLE; }
}
