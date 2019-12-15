#ifndef GVULKANHELPER_H
#define GVULKANHELPER_H
#pragma warning(disable:6011)
#pragma warning(disable:26812)

//Extension, Layers and Enumeration Support (RETRUNS: VK_FALSE IS SUCCESS, VK_TRUE IS FAILURE)
VkResult check_instance_extension_name(const char* _extension);
VkResult check_instance_layer_name(const char* _layer);
VkResult check_device_extension_name(const VkPhysicalDevice& _physicalDevice, const char* _extension);
VkResult get_instance_extensions(uint32_t* _outCount, VkExtensionProperties** _outExtensionProperties);
VkResult get_instance_layers(uint32_t* _outCount, VkLayerProperties** _outLayerProperties);
VkResult get_physical_devices(const VkInstance& _instance, uint32_t* _outCount, VkPhysicalDevice** _outPhysicalDevice);
VkResult get_device_extensions(const VkPhysicalDevice& _physicalDevice, uint32_t* _outCount, VkExtensionProperties** _outExtensionProperties);

//Surface & Swapchain Information
VkResult get_surface_formats(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, uint32_t* _outCount, VkSurfaceFormatKHR** _outSurfaceFormats);
VkResult get_surface_present_modes(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, uint32_t* _outCount, VkPresentModeKHR** _outPresentMode);
VkResult get_best_surface_formats(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, VkSurfaceFormatKHR* _outSurfaceFormat);
VkResult get_best_surface_present_mode(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, VkPresentModeKHR* _outPresentMode);
VkResult get_surface_extent(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, const VkExtent2D& _winSize, VkExtent2D* _outExtent3D);
VkResult find_depth_format(const VkPhysicalDevice& _physicalDevice, const VkImageTiling& _tiling, const VkFormatFeatureFlags& _formatFeatureFlags, const VkFormat* _formats, VkFormat* _outDepthFormat);

//Physical Device Information
VkResult get_best_gpu(const VkSurfaceKHR& _surface, const uint32_t& _totalDevices, VkPhysicalDevice* _allPhysicalDevices, const uint32_t& _totalDeviceExtensions, const char** _deviceExtensions, uint32_t* outIndex);
VkResult get_best_queue_family_indices(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, int** _outIndices, VkBool32* _outCanCompute);
VkResult get_best_msaa_format(const VkPhysicalDevice& _physicalDevice, const VkSampleCountFlagBits& _idealMSAAFlag, VkSampleCountFlagBits* _outMSAAFlag);

//Command Help
VkResult signal_command_start(const VkDevice& _device, const VkCommandPool& _commandPool, VkCommandBuffer* _outCommandBuffer);
VkResult signal_command_end(const VkDevice& _device, const VkQueue& _graphicsQueue, const VkCommandPool& _commandPool, VkCommandBuffer* _commandBuffer);

//Images Creation
VkResult create_image(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent, const uint32_t& _mipLevels, const VkSampleCountFlagBits& _msaaBit,
	const VkFormat& _format, const VkImageTiling& _tiling, const VkImageUsageFlags& _usageFlags, const VkMemoryPropertyFlags& _memoryPropertyFlags, VkAllocationCallbacks* _allocator,
	VkImage* _outImage, VkDeviceMemory* _outImageMemory);
VkResult create_image_view(const VkDevice& _device, const VkImage& _image, const VkFormat& _format, const VkImageAspectFlags& _imageAspectFlags, const uint32_t& _mipLevels, VkAllocationCallbacks* _allocator, VkImageView* _outImageView);
VkResult transition_image_layout(const VkDevice& _device, const VkCommandPool& _commandPool, const VkQueue& _graphicsQueue, const uint32_t& _mipLevel, const VkImage& _image, const VkFormat& _format, const VkImageLayout& _previousLayout, const VkImageLayout& _currentLayout);
VkResult create_image_set(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkCommandPool &_commandPool, const VkExtent3D& _extent, const VkQueue &_graphicsQueue, const uint32_t& _mipLevels, const VkSampleCountFlagBits& _msaaSample, const VkFormat& _format, 
	const VkImageTiling& _tiling, const VkImageUsageFlags& _usageFlags, const VkImageAspectFlags &_aspectFlags, const VkMemoryPropertyFlags& _memoryPropertyFlags, const VkImageLayout &_previousLayout, const VkImageLayout &_currentLayout, VkAllocationCallbacks* _allocator,
	VkImage *_outImage, VkImageView *_outImageView, VkDeviceMemory *_outDeviceMemory);
VkResult copy_buffer_to_image(const VkDevice& device, const VkCommandPool& command_pool, const VkQueue& graphics_queue, const VkBuffer& buffer, const VkImage& image, const VkExtent3D& extent);
VkResult create_mipmaps(const VkDevice& device, const VkCommandPool& command_pool, const VkQueue& graphics_queue, const VkImage& texture_image, const uint32_t& texture_width, const uint32_t& texture_height, const uint32_t& mip_levels);

//Other useful functions
VkResult find_memory_type(const VkPhysicalDevice& _physicalDevice, const uint32_t& _filter, const VkMemoryPropertyFlags& _propertyFlags, uint32_t* _outMemoryType);
VkResult create_shader(const VkDevice& _device, const char* _fileName, const char* _entryPoint, const VkShaderStageFlagBits& _shaderType, VkShaderModule* _outShaderModule, VkPipelineShaderStageCreateInfo* _outStageInfo);

//Template Helper Functions
template <typename buffObj>
VkResult write_to_buffer(const VkDevice &device, const buffObj &buffer_object, VkDeviceMemory &memory)
{
	void* data;
	VkResult r = vkMapMemory(device, memory, 0, sizeof(buffObj), 0, &data);

	if (r)
		return r;

	memcpy(data, &buffer_object, sizeof(buffObj));
	vkUnmapMemory(device, memory);

	return r;
}

//Buffer Creation
VkResult create_buffer(const VkPhysicalDevice& physical_device, const VkDevice& device, const VkDeviceSize& size, const VkBufferUsageFlags& usage_flags, const VkMemoryPropertyFlags& property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
VkResult copy_buffer(const VkDevice& _device, const VkCommandPool& _commandPool, const VkQueue& _queueGraphics, const VkBuffer& _sourceBuffer, const VkBuffer& _destinationBuffer, const VkDeviceSize& _deviceSize);

//Shader Creation
VkResult read_shader_file(const char* _fileName, uint64_t *_outShaderSize, char** _outShaderFile);
VkResult create_shader_module(const VkDevice& _device, const uint64_t& _shaderSize, char* _shaderString, VkShaderModule* _outShaderModule);

#endif