#include "0-Triangle.h"
#include "vulkan/vulkan.h"
#include "../../vulkan/GVulkanHelper.h"
#include <string.h>

namespace {
	//For Vertex
	static const uint8_t POSITION = 0;
	static const uint8_t COLOR = 1;

	//For Shaders
	static const uint8_t VERTEX = 0;
	static const uint8_t FRAGMENT = 1;

	struct Vertex {
		float position[3];
		float color[3];

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

			attribute_description[COLOR].binding = 0;
			attribute_description[COLOR].location = COLOR;
			attribute_description[COLOR].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_description[COLOR].offset = 12;

			return attribute_description;
		}
	};
	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	};
	namespace {
		Vertex hTriangle[3] = {};
		Buffer hVertexBuffer = {};
		VkPipelineLayout hPipelineLayout = {};
		VkPipeline hPipeline = {};
	}
}

//Helper function prototypes
void vkCreateTriangleVB(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkCommandPool& _commandPool, const VkQueue& _queueGraphics, const uint32_t& vertex_size, Vertex* _vertex, Buffer* _buffer);
void vkCreateTrianglePipeline(const VkDevice& _device, const VkExtent2D& _swapchainExtent2D, const VkRenderPass& _swapchainRenderPass, const VkSampleCountFlagBits &_msaa);

//Init - Draw - Cleanup
void vkCreateTriangle(void *_physicalDevice, void *_device, void *_commandPool, void *_queueGraphics, void *_swapchainExtent2D, void *_swapchainRenderPass, void *_msaaBit)
{
	//Static Casting
	VkPhysicalDevice physical_device = static_cast<VkPhysicalDevice>(_physicalDevice);
	VkDevice device = static_cast<VkDevice>(_device);
	VkCommandPool command_pool = static_cast<VkCommandPool>(_commandPool);
	VkQueue queue_graphics = static_cast<VkQueue>(_queueGraphics);
	VkExtent2D extent2d = *(static_cast<VkExtent2D*>(_swapchainExtent2D));
	VkRenderPass render_pass = static_cast<VkRenderPass>(_swapchainRenderPass);
	VkSampleCountFlagBits msaa = *(static_cast<VkSampleCountFlagBits*>(_msaaBit));

	//create the triangle
	hTriangle[0].position[1] = -0.5f;
	hTriangle[0].color[0] = 1.0f;

	hTriangle[1].position[0] = 0.5f;
	hTriangle[1].position[1] = 0.5f;
	hTriangle[1].color[2] = 1.0f;

	hTriangle[2].position[0] = -0.5f;
	hTriangle[2].position[1] = 0.5f;
	hTriangle[2].color[1] = 1.0f;


	//setup vertex buffer
	vkCreateTriangleVB(physical_device, device, command_pool, queue_graphics, 3, hTriangle, &hVertexBuffer);

	//setup simple graphics pipeline
	vkCreateTrianglePipeline(device, extent2d, render_pass, msaa);
}
void vkDrawTriangle(void *_commandBuffer)
{
	//Static Casting
	VkCommandBuffer command_buffer = static_cast<VkCommandBuffer>(_commandBuffer);

	//Draw Triangle
	VkDeviceSize offset[] = { 0 };
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, hPipeline);
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &hVertexBuffer.buffer, offset);
	vkCmdDraw(command_buffer, 3, 1, 0, 0);
}
void vkCleanupTriangle(void *_device)
{
	//Static Casting
	VkDevice device = static_cast<VkDevice>(_device);

	if (hVertexBuffer.buffer)
		vkDestroyBuffer(device, hVertexBuffer.buffer, VK_NULL_HANDLE);
	if (hVertexBuffer.memory)
		vkFreeMemory(device, hVertexBuffer.memory, VK_NULL_HANDLE);
	if (hPipelineLayout)
		vkDestroyPipelineLayout(device, hPipelineLayout, VK_NULL_HANDLE);
	if (hPipeline)
		vkDestroyPipeline(device, hPipeline, VK_NULL_HANDLE);
}

//Helper function definitions
void vkCreateTriangleVB(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkCommandPool &_commandPool, const VkQueue &_queueGraphics, const uint32_t &vertex_size, Vertex *_vertex, Buffer* _buffer)
{
	VkDeviceSize buffer_size = sizeof(Vertex) * vertex_size;
	
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	create_buffer(_physicalDevice, _device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&staging_buffer, &staging_buffer_memory);

	void* data;
	vkMapMemory(_device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, _vertex, buffer_size);
	vkUnmapMemory(_device, staging_buffer_memory);

	create_buffer(_physicalDevice, _device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&hVertexBuffer.buffer, &hVertexBuffer.memory);

	copy_buffer(_device, _commandPool, _queueGraphics, staging_buffer, hVertexBuffer.buffer, buffer_size);

	vkDestroyBuffer(_device, staging_buffer, nullptr);
	vkFreeMemory(_device, staging_buffer_memory, nullptr);
}
void vkGraphicsPipelineCreateShader(const VkDevice &_device, const char *_fileName, const char *_entryPoint, const VkShaderStageFlagBits &_shaderType, VkShaderModule *_outShaderModule, VkPipelineShaderStageCreateInfo *_outStageInfo)
{
	//Read Shader
	uint64_t shader_size;
	char* shader_file = nullptr;
	VkResult r = read_shader_file(_fileName, &shader_size, &shader_file);
	VkShaderModule shader_module = VK_NULL_HANDLE;
	r = create_shader_module(_device, shader_size, shader_file, &shader_module);

	//Setup stage info
	VkPipelineShaderStageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	create_info.stage = _shaderType;
	create_info.module = shader_module;
	create_info.pName = _entryPoint;

	//Set the out
	*_outStageInfo = create_info;
	*_outShaderModule = shader_module;

	//free memory
	delete[] shader_file;
}
void vkCreateTrianglePipeline(const VkDevice &_device, const VkExtent2D &_swapchainExtent2D, const VkRenderPass &_swapchainRenderPass, const VkSampleCountFlagBits &_msaa)
{
	//Vertex & Fragment Shader Modules & Stages
	VkShaderModule shader[2] = {};
	VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
	vkGraphicsPipelineCreateShader(_device, "./src/tests/0-Triangle/triangle.vert.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &shader[VERTEX], &stage_create_info[VERTEX]);
	vkGraphicsPipelineCreateShader(_device, "./src/tests/0-Triangle/triangle.frag.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &shader[FRAGMENT], &stage_create_info[FRAGMENT]);

	//Assembly State
	VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
	assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assembly_create_info.primitiveRestartEnable = false;

	//Vertex Input State
	VkVertexInputBindingDescription vertex_binding_description = Vertex::get_binding_description();
	VkVertexInputAttributeDescription *vertex_attribute_description = Vertex::get_attribute_description();

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
	viewport.width = static_cast<float>(_swapchainExtent2D.width);
	viewport.height = static_cast<float>(_swapchainExtent2D.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = _swapchainExtent2D;

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
	multisample_create_info.rasterizationSamples = _msaa;
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
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pSetLayouts = nullptr;
	pipeline_layout_create_info.pushConstantRangeCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;
	VkResult r = vkCreatePipelineLayout(_device, &pipeline_layout_create_info, nullptr, &hPipelineLayout);

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

	pipeline_create_info.layout = hPipelineLayout;
	pipeline_create_info.renderPass = _swapchainRenderPass;
	pipeline_create_info.subpass = 0;

	pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex = -1;

	vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &hPipeline);

	//Cleanup
	delete[] vertex_attribute_description;
	vkDestroyShaderModule(_device, shader[VERTEX], nullptr);
	vkDestroyShaderModule(_device, shader[FRAGMENT], nullptr);
}