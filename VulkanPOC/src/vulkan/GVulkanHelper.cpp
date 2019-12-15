#include "vulkan/vulkan.h"
#include "GVulkanHelper.h"
#include <string.h>
#include <cstdio>

//WooWoo! Optional Class! 
//This is created to simulate the "optional class" in C++17
template <typename T>
class optional
{
public:
	optional() { m_Value = nullptr; }
	optional(T p_Value) { *this = p_Value; }
	~optional() { reset(); }

	void operator=(T v) { if (!m_Value) m_Value = new T; *m_Value = v; }
	T operator->() { return *m_Value; }
	T operator*() { return *m_Value; }
	const T operator->() const { return *m_Value; }
	const T operator*() const { return *m_Value; }

	const bool has_value()  const { return m_Value; }
	const T value() const { return *m_Value; }

	void reset() { if (m_Value) delete m_Value; }

private:
	T* m_Value;
};

//Internal Helper Functions
template <typename T>
static T min(const T &x, const T &y) { return x < y ? x : y;}
static int64_t gpu_compatibility(const VkPhysicalDevice &_physicalDevice, const VkSurfaceKHR &_surface, const uint32_t & _deviceExtensionCount, const char** _deviceExtensions)
{
	//Gather all device extensions
	uint32_t extension_count = 0;
	VkExtensionProperties* all_extensions = VK_NULL_HANDLE;
	VkResult r = get_device_extensions(_physicalDevice, &extension_count, &all_extensions);

	//Setup Checks
	int64_t score = 0;
	bool is_compatible = false;

	//First check the true requirement
	const char* swapchain_ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	for (uint32_t i = 0; i < extension_count; ++i)
		if (!strcmp(all_extensions[i].extensionName, swapchain_ext))
		{
			is_compatible = true;
			break;
		}

	//Check if compatible
	if (!is_compatible)
		return static_cast<int64_t>(0xC000000000000000);

	//Check Swapchain Support (Format Count, Present Mode Count Checks)
	uint32_t format_count;
	r = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &format_count, VK_NULL_HANDLE);
	if (r)
		return static_cast<int64_t>(0xC000000000000000);

	uint32_t present_mode_count;
	r = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &present_mode_count, VK_NULL_HANDLE);
	if (r)
		return static_cast<int64_t>(0xC000000000000000);

	if (!(format_count && present_mode_count))
		return static_cast<int64_t>(0xC000000000000000);

	//Device Queue Compatibility
	int qf[2] = {-1, -1};
	int* pqf = qf;
	VkBool32 can_compute;
	get_best_queue_family_indices(_physicalDevice, _surface, &pqf, &can_compute);

	if (qf[0] == -1 || qf[1] == -1)
		return static_cast<int64_t>(0xC000000000000000);

	//Score if can compute
	score += can_compute ? 100000000 : 0;

	//Check other extensions that was sent.
	for (uint32_t i = 0; i < extension_count; ++i)
		for (uint32_t j = 0; j < _deviceExtensionCount; ++j)
			if (!strcmp(all_extensions[i].extensionName, _deviceExtensions[j]))
			{
				score += 1000000000;
				break;
			}

	//Passes everything. Completely Compatible
	score += static_cast<int64_t>(0x4000000000000000);
	delete[] all_extensions;
	return score;
}
static int64_t gpu_device_type(const VkPhysicalDevice &_physicalDevice)
{
	//Gather Physical Device Properties
	VkPhysicalDeviceProperties all_properties;
	vkGetPhysicalDeviceProperties(_physicalDevice, &all_properties);
	VkPhysicalDeviceType current_type = all_properties.deviceType;

	//Score up based on Device Type
	int64_t score = 0;
	switch (current_type)
	{
	case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		score += static_cast<int64_t>(0x80000000000000);
		break;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		score += static_cast<int64_t>(0xC0000000000000);
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		score += static_cast<int64_t>(0xF0000000000000);
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		score += static_cast<int64_t>(0xE0000000000000);
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		break;
	}

	//Return Score
	return score;
}
static int64_t gpu_features(const VkPhysicalDevice &physical_device)
{
	//Get the Device Features
	VkPhysicalDeviceFeatures device_feature;
	vkGetPhysicalDeviceFeatures(physical_device, &device_feature);

	//Score up the features
	int64_t score = 0;
	score += device_feature.tessellationShader * 100000000000;
	score += device_feature.geometryShader * 100000000000;
	score += device_feature.samplerAnisotropy * 50000000000;
	score += device_feature.sampleRateShading * 50000000000;
	score += device_feature.fillModeNonSolid * 10000000000;

	return score;
}
static int64_t gpu_memory(const VkPhysicalDevice &_physicalDeviceBest, const VkPhysicalDevice &_physicalDeviceCurrent)
{
	//Get Memory Properties for current best GPU
	VkPhysicalDeviceMemoryProperties memory_properties_best = {};
	vkGetPhysicalDeviceMemoryProperties(_physicalDeviceBest, &memory_properties_best);

	//Get Memory Properties for current GPU
	VkPhysicalDeviceMemoryProperties memory_properties_current = {};
	vkGetPhysicalDeviceMemoryProperties(_physicalDeviceCurrent, &memory_properties_current);

	//Get the Memory Size for current best GPU
	uint64_t best_size = 0;
	for (uint32_t i = 0; i < memory_properties_best.memoryHeapCount; ++i)
		if (memory_properties_best.memoryHeaps[i].flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			best_size = memory_properties_best.memoryHeaps[i].size;
			break;
		}

	//Get the Memory Size for current GPU
	uint64_t current_size = 0;
	for (uint32_t i = 0; i < memory_properties_current.memoryHeapCount; ++i)
		if (memory_properties_best.memoryHeaps[i].flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			current_size = memory_properties_current.memoryHeaps[i].size;
			break;
		}

	if (current_size <= best_size)
		return -1;

	return 0;
}

//Extension, Layers and Enumeration Support
VkResult check_instance_extension_name(const char* _extension)
{
	//Gather all the instance extensions
	uint32_t count;
	VkExtensionProperties* available_extensions = VK_NULL_HANDLE;
	if (get_instance_extensions(&count, &available_extensions))
		return VK_RESULT_MAX_ENUM;

	//compare all instance extensions with the parameter
	for (uint32_t i = 0; i < count; ++i)
	{
		if (!strcmp(available_extensions[i].extensionName, _extension))
		{
			delete[] available_extensions;
			return VK_SUCCESS;
		}
	}

	//Failed to find extension. Return MAX_ENUM
	delete[] available_extensions;
	return VK_RESULT_MAX_ENUM;
}
VkResult check_instance_layer_name(const char* _layer)
{
	//Gather all Instance Layers
	uint32_t count;
	VkLayerProperties* available_layers = VK_NULL_HANDLE;
	if (get_instance_layers(&count, &available_layers))
		return VK_RESULT_MAX_ENUM;

	//Compare all instance layers with the parameter
	for (uint32_t i = 0; i < count; ++i)
	{
		if (!strcmp(available_layers[i].layerName, _layer))
		{
			delete[] available_layers;
			return VK_SUCCESS;
		}
	}

	//Failed to find extension. Return MAX_ENUM
	delete[] available_layers;
	return VK_RESULT_MAX_ENUM;
}
VkResult check_device_extension_name(const VkPhysicalDevice &_physicalDevice, const char* _extension)
{
	//Gather all the device extensions
	uint32_t count;
	VkExtensionProperties* available_extensions = VK_NULL_HANDLE;
	if (get_device_extensions(_physicalDevice, &count, &available_extensions))
		return VK_RESULT_MAX_ENUM;

	//compare all device extensions with the parameter
	for (uint32_t i = 0; i < count; ++i)
	{
		if (!strcmp(available_extensions[i].extensionName, _extension))
		{
			delete[] available_extensions;
			return VK_SUCCESS;
		}
	}

	//Failed to find extension. Return MAX_ENUM
	delete[] available_extensions;
	return VK_RESULT_MAX_ENUM;
}
VkResult get_instance_extensions(uint32_t* _outCount, VkExtensionProperties **_outExtensionProperties)
{
	//Gather All Extensions
	VkExtensionProperties* out;
	VkResult r = vkEnumerateInstanceExtensionProperties(nullptr, _outCount, VK_NULL_HANDLE);
	
	if (*_outCount < 1)
	{
		//There are no extensions. Abort
		r = VK_RESULT_MAX_ENUM;
		return r;
	}
	if (r)
		return r; //Failed to retrieve extensions. Abort.

	//Create a new array to hold all the extensions
	out = new VkExtensionProperties[*_outCount];
	r = vkEnumerateInstanceExtensionProperties(nullptr, _outCount, out);
	if (r)
	{
		//Is NOT VK_SUCCESS! Delete the newly created array and set to null handle
		delete[] out;
		out = VK_NULL_HANDLE; 
	}

	//Give the new extension array to the parameter
	*_outExtensionProperties = out;

	//Return Result
	return r;
}
VkResult get_instance_layers(uint32_t* _outCount, VkLayerProperties **_outLayerProperties)
{
	//Gather All Layers
	VkLayerProperties* out;
	VkResult r = vkEnumerateInstanceLayerProperties(_outCount, VK_NULL_HANDLE);
	
	if (*_outCount < 1)
	{
		//There are no Layers. Abort
		r = VK_RESULT_MAX_ENUM;
		return r;
	}
	if (r)
		return r; //Failed to retrieve Layers. Abort.

	//Create a new array to hold all the Layers
	out = new VkLayerProperties[*_outCount];
	r = vkEnumerateInstanceLayerProperties(_outCount, out);
	if (r)
	{
		//Is NOT VK_SUCCESS! Delete the newly created array and set to nullptr
		delete[] out;
		out = VK_NULL_HANDLE;
	}

	//Give the new Layer array to the parameter
	*_outLayerProperties = out;

	//Return Result
	return r;
}
VkResult get_physical_devices(const VkInstance &_instance, uint32_t *_outCount, VkPhysicalDevice **_outPhysicalDevice)
{
	//Gather All Devices
	VkPhysicalDevice* out;
	VkResult r = vkEnumeratePhysicalDevices(_instance, _outCount, VK_NULL_HANDLE);

	if (*_outCount < 1)
	{
		//There are no devices. Abort
		r = VK_RESULT_MAX_ENUM;
		return r;
	}
	if (r)
		return r; //Failed to retrieve devices. Abort.

	//Create a new array to hold all the devices
	out = new VkPhysicalDevice[*_outCount];
	r = vkEnumeratePhysicalDevices(_instance, _outCount, out);
	if (r)
	{
		//Is NOT VK_SUCCESS! Delete the newly created array and set to null handle
		delete[] out;
		out = VK_NULL_HANDLE;
	}

	//Give the new device array to the parameter
	*_outPhysicalDevice = out;

	//Return Result
	return r;
}
VkResult get_device_extensions(const VkPhysicalDevice &_physicalDevice, uint32_t *_outCount, VkExtensionProperties **_outExtensionProperties)
{
	//Gather All Extensions
	VkExtensionProperties* out;
	VkResult r = vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, _outCount, VK_NULL_HANDLE);
	
	if (*_outCount < 1)
	{
		//There are no extensions. Abort
		r = VK_RESULT_MAX_ENUM;
		return r;
	}
	if (r)
		return r; //Failed to retrieve extensions. Abort.

	//Create a new array to hold all the extensions
	out = new VkExtensionProperties[*_outCount];
	r = vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, _outCount, out);
	if (r)
	{
		//Is NOT VK_SUCCESS! Delete the newly created array and set to null handle
		delete[] out;
		out = VK_NULL_HANDLE;
	}

	//Give the new extension array to the parameter
	*_outExtensionProperties = out;

	//Return Result
	return r;
}

//Surface & Swapchain Information
VkResult get_surface_formats(const VkPhysicalDevice &_physicalDevice, const VkSurfaceKHR &_surface, uint32_t *_outCount, VkSurfaceFormatKHR **_outSurfaceFormats)
{
	//Gather all the surface formats
	VkSurfaceFormatKHR *out = *_outSurfaceFormats;
	VkResult r = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, _outCount, VK_NULL_HANDLE);
	
	if (*_outCount < 1)
	{
		//There are no surface formats. Abort
		r = VK_RESULT_MAX_ENUM;
		return r;
	}
	if (r)
		return r; //Failed to retrieve surface formats. Abort.

	//Create a new array to hold all the surface formats
	out = new VkSurfaceFormatKHR[*_outCount];
	r = vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, _outCount, out);
	if (r)
	{
		//Is NOT VK_SUCCESS! Delete the newly created array and set to null handle
		delete[] out;
		out = VK_NULL_HANDLE;
	}

	//Give the new extension array to the parameter
	*_outSurfaceFormats = out;

	//Return Result
	return r;
}
VkResult get_surface_present_modes(const VkPhysicalDevice &_physicalDevice, const VkSurfaceKHR &_surface, uint32_t *_outCount, VkPresentModeKHR **_outPresentMode)
{
	//Gather all the surface present modes
	VkPresentModeKHR* out = *_outPresentMode;
	VkResult r = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, _outCount, nullptr);
	
	if (*_outCount < 1)
	{
		//There are no surface present modes. Abort
		r = VK_RESULT_MAX_ENUM;
		return r;
	}
	if (r)
		return r; //Failed to retrieve surface formats. Abort.

	//Create a new array to hold all the surface formats
	out = new VkPresentModeKHR[*_outCount];
	r = vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, _outCount, out);
	if (r)
	{
		//Is NOT VK_SUCCESS! Delete the newly created array and set to null handle
		delete[] out;
		out = VK_NULL_HANDLE;
	}

	//Give the new extension array to the parameter
	*_outPresentMode = out;

	//Return Result
	return r;
}
VkResult get_best_surface_formats(const VkPhysicalDevice &_physicalDevice, const VkSurfaceKHR &_surface, VkSurfaceFormatKHR *_outSurfaceFormat)
{
	//Gather all the surface formats
	uint32_t count;
	VkSurfaceFormatKHR *surface_formats = VK_NULL_HANDLE;
	VkResult r = get_surface_formats(_physicalDevice, _surface, &count, &surface_formats);
	if (count < 1 || r)
	{
		//Failed to find surface formats. Abort.
		if (surface_formats) delete[] surface_formats;
		return r;
	}

	//If the count is 1 and the format is unknown, set a default format as best
	if (count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED)
	{
		*_outSurfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		delete[] surface_formats;
		return r;
	}

	//Loop through all formats and find the best format.
	for (uint32_t i = 0; i < count; ++i)
	{
		if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			*_outSurfaceFormat = surface_formats[i];
			delete[] surface_formats;
			return r;
		}
	}

	if (surface_formats)
	{
		*_outSurfaceFormat = surface_formats[0];
		delete[] surface_formats;
	}
	return r;
}
VkResult get_best_surface_present_mode(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, VkPresentModeKHR *_outPresentMode)
{
	//Gather all surface present modes
	uint32_t count;
	VkPresentModeKHR *present_modes = VK_NULL_HANDLE;
	VkResult r = get_surface_present_modes(_physicalDevice, _surface, &count, &present_modes);
	if (count < 1 || r)
	{
		//Failed to find present modes. Abort.
		if (present_modes) delete[] present_modes;
		return r;
	}

	//Find the best mode (best: Mailbox, runner-up: Immediate, Default: FIFO)
	VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			best_mode = present_modes[i];
			*_outPresentMode = best_mode;
			break;
		}
		else if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			best_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	if (present_modes)
	{
		delete[] present_modes;
		*_outPresentMode = best_mode;
	}
	return r;
}
VkResult get_surface_extent(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, const VkExtent2D &_winSize, VkExtent2D *_outExtent2D)
{
	//Gather all surface capabilities
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkResult r = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surface_capabilities);
	if (r)
		return r;

	//If Capabilities's extent is not MAX, Set to those extents
	if (surface_capabilities.currentExtent.width != 0xFFFFFFFF)
		*_outExtent2D = surface_capabilities.currentExtent;
	else //Otherwise set it to window's width and height.
		*_outExtent2D = _winSize;
	
	return r;
}
VkResult find_depth_format(const VkPhysicalDevice& _physicalDevice, const VkImageTiling& _tiling, const VkFormatFeatureFlags& _formatFeatureFlags, const VkFormat *_formats, VkFormat* _outDepthFormat)
{
	//Setup Formats (Based on either input or default)
	VkFormat depth_formats[3];
	if (_formats)
	{
		depth_formats[0] = _formats[0];
		depth_formats[1] = _formats[1];
		depth_formats[2] = _formats[2];
	}
	else
	{
		depth_formats[0] = VK_FORMAT_D32_SFLOAT;
		depth_formats[1] = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depth_formats[2] = VK_FORMAT_D24_UNORM_S8_UINT;
	}

	//Find the best compatible format for Depth
	for (uint32_t i = 0; i < 3; ++i)
	{
		VkFormatProperties format_properties;
		vkGetPhysicalDeviceFormatProperties(_physicalDevice, depth_formats[i], &format_properties);

		if (_tiling == VK_IMAGE_TILING_LINEAR &&
			(format_properties.linearTilingFeatures & _formatFeatureFlags) == _formatFeatureFlags)
		{
			*_outDepthFormat = depth_formats[i];
			return VK_SUCCESS;
		}
		else if (_tiling == VK_IMAGE_TILING_OPTIMAL &&
			(format_properties.optimalTilingFeatures & _formatFeatureFlags) == _formatFeatureFlags)
		{
			*_outDepthFormat = depth_formats[i];
			return VK_SUCCESS;
		}
	}

	//Format not found. Set to Undefined
	*_outDepthFormat = VK_FORMAT_UNDEFINED;

	//Return Max Enum
	return VK_RESULT_MAX_ENUM;
}

//Physical Device Information
VkResult get_best_gpu(const VkSurfaceKHR & _surface, const uint32_t& _totalDevices, VkPhysicalDevice* _allPhysicalDevices, const uint32_t &_totalDeviceExtensions, const char **_deviceExtensions, uint32_t *outIndex)
{
	/*
	The Idea:
	So, The purpose of this function is ideally to find the best GPU.
	The hard part, is that I cannot find a way to check clock speeds.
	So instead, i will base this on Compatibility, Device Type, Features and Memory.
		- Compatibility has the biggest impact. If its not compatible, then its a negative score.
		- Device Types will have a major impact. Best in order: Discrete GPU -> Virtual GPU -> Integrated GPU -> CPU
		- Features has minor impact. All features combined will have a great impact.
		- Memory will only come into play when scores are the same.
	There is absolutely no guarantee this is the best, but i want to be at least 95% accurate.
	*/

	//Gather Best GPU Index. Default is 0
	uint32_t best_index = 0;
	int64_t best_score = 0;
	for (uint32_t i = 0; i < _totalDevices; ++i)
	{
		int64_t score = 0;
		score += gpu_compatibility(_allPhysicalDevices[i], _surface, _totalDeviceExtensions, _deviceExtensions);
		score += gpu_device_type(_allPhysicalDevices[i]);
		score += gpu_features(_allPhysicalDevices[i]);
		
		if (score == best_score)
			score += gpu_memory(_allPhysicalDevices[best_index], _allPhysicalDevices[i]);

		if (score >= best_score)
		{
			best_index = i;
			best_score = score;
		}
	}

	*outIndex = best_index;
	return VK_SUCCESS;
}
VkResult get_best_queue_family_indices(const VkPhysicalDevice &_physicalDevice, const VkSurfaceKHR &_surface, int **_outIndices, VkBool32 *_outCanCompute)
{
	//Gather Queue Family Properties
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, nullptr);
	VkQueueFamilyProperties* queue_family_properties = new VkQueueFamilyProperties[count];
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &count, queue_family_properties);

	//Setup the Optional (Graphics and Present) Queue
	optional<int32_t> graphics;
	optional<int32_t> present;

	//Find Absolute Best: With Compute Shader
	for (uint32_t i = 0; i < count; ++i)
	{
		VkQueueFamilyProperties* cur = &queue_family_properties[i];
		if (cur->queueCount > 0 && cur->queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
		{
			graphics = i;
			*_outCanCompute = VK_TRUE;

			VkBool32 present_support = VK_FALSE;
			VkResult r = vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &present_support);
			if (r)
				continue;

			if (present_support)
				present = i;

			if (present.has_value())
			{
				(*_outIndices)[0] = i;
				(*_outIndices)[1] = i;
				delete[] queue_family_properties;
				return r;
			}
		}
	}

	//No Compute Shader!
	_outCanCompute = VK_FALSE;
	for (uint32_t i = 0; i < count; ++i)
	{
		VkQueueFamilyProperties* cur = &queue_family_properties[i];
		if ( (cur->queueCount > 0) && (cur->queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			graphics = i;

			VkBool32 present_support = VK_FALSE;
			VkResult r = vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &present_support);
			if (r)
				continue;

			if (present_support)
				present = i;

			if (present.has_value())
			{
				(*_outIndices)[0] = i;
				(*_outIndices)[1] = i;
				delete[] queue_family_properties;
				return r;
			}
		}
	}

	(*_outIndices)[0] = -1;
	(*_outIndices)[1] = -1;
	delete[] queue_family_properties;
	return VK_RESULT_MAX_ENUM;
}
VkResult get_best_msaa_format(const VkPhysicalDevice& _physicalDevice, const VkSampleCountFlagBits&_idealMSAAFlag, VkSampleCountFlagBits *_outMSAAFlag)
{
	//Gather all physical device
	VkPhysicalDeviceProperties physical_device_properties;
	vkGetPhysicalDeviceProperties(_physicalDevice, &physical_device_properties);

	//Set a flag based on the minimum
	VkSampleCountFlagBits flags = static_cast<VkSampleCountFlagBits>( min(
		static_cast<int32_t>(physical_device_properties.limits.framebufferColorSampleCounts),
		static_cast<int32_t>(physical_device_properties.limits.framebufferDepthSampleCounts)
	) );

	//Find the highest MSAA
	for (VkSampleCountFlagBits bit = VK_SAMPLE_COUNT_64_BIT; bit > VK_SAMPLE_COUNT_1_BIT; bit = static_cast<VkSampleCountFlagBits>(bit >> 1))
	{
		if ((flags & bit))
		{
			*_outMSAAFlag = bit;

			if (bit >= _idealMSAAFlag)
				return VK_SUCCESS;
			else
				return VK_RESULT_MAX_ENUM;
		}
	}

	//Couldn't find MSAA, set default
	*_outMSAAFlag = VK_SAMPLE_COUNT_1_BIT;
	return VK_RESULT_MAX_ENUM;
}

//Command Help
VkResult signal_command_start(const VkDevice &_device, const VkCommandPool &_commandPool, VkCommandBuffer *_outCommandBuffer)
{
	//Command Buffer Allocate Info
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandPool = _commandPool;
	command_buffer_allocate_info.commandBufferCount = 1;

	//Create the command buffer and allocate the command buffer with create info
	VkResult r = vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, _outCommandBuffer);
	if (r)
		return r;

	//Start the command buffer's create info
	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin the Command Buffer's recording process
	r = vkBeginCommandBuffer(*_outCommandBuffer, &command_buffer_begin_info);
	
	//Command Buffer has been successfully started!
	return r;
}
VkResult signal_command_end(const VkDevice& _device, const VkQueue& _graphicsQueue, const VkCommandPool& _commandPool, VkCommandBuffer *_commandBuffer)
{
	//End the Command Buffer's recording Process
	VkResult r = vkEndCommandBuffer(*_commandBuffer);
	if (r)
		throw "VkResult is NOT VK_SUCCESS!";

	//Create the submit info
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = _commandBuffer;

	//Submit The Commands Recorded into the Queue. Then wait for the Graphics Queue to be idle
	r = vkQueueSubmit(_graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
	if (r)
	{
		throw "VkResult is NOT VK_SUCCESS!";
	}

	r = vkQueueWaitIdle(_graphicsQueue);
	if (r)
		throw "VkResult is NOT VK_SUCCESS!";

	//Free the command buffer from memory
	vkFreeCommandBuffers(_device, _commandPool, 1, _commandBuffer);

	//The Command Buffer has ended successfully!
	return r;
}

//Images Creation
VkResult create_image(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent, const uint32_t& _mipLevels, const VkSampleCountFlagBits& _msaaBit, 
	const VkFormat& _format, const VkImageTiling& _tiling, const VkImageUsageFlags& _usageFlags, const VkMemoryPropertyFlags& _memoryPropertyFlags, VkAllocationCallbacks* _allocator,
	VkImage *_outImage, VkDeviceMemory *_outImageMemory)
{
	//Create image info
	VkImageCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent = _extent;
	create_info.mipLevels = _mipLevels;
	create_info.arrayLayers = 1;
	create_info.format = _format;
	create_info.tiling = _tiling;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = _usageFlags;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.samples = _msaaBit;
	create_info.flags = 0;

	//Create the image
	VkResult r = vkCreateImage(_device, &create_info, _allocator, _outImage);
	if (r)
	{
		*_outImageMemory = 0u;
		return r;
	}

	//Create the memory required for any the image passed into this function
	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(_device, *_outImage, &memory_requirements);

	uint32_t memory_type_index = 0;
	find_memory_type(_physicalDevice, memory_requirements.memoryTypeBits, _memoryPropertyFlags, &memory_type_index);

	//Memory Allocate Info
	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = memory_type_index;

	//Allocate the memory created
	r = vkAllocateMemory(_device, &memory_allocate_info, _allocator, _outImageMemory);
	if (r)
	{
		vkDestroyImage(_device, *_outImage, _allocator);
		*_outImage = VK_NULL_HANDLE;
		_outImageMemory = 0u;
		return r;
	}

	//Bind the memory created
	r = vkBindImageMemory(_device, *_outImage, *_outImageMemory, 0);
	if (r)
	{
		vkDestroyImage(_device, *_outImage, VK_NULL_HANDLE);
		*_outImage = VK_NULL_HANDLE;
		vkFreeMemory(_device, *_outImageMemory, VK_NULL_HANDLE);
		return r;
	}

	//Image Creation has been successful!
	return r;
}
VkResult create_image_view(const VkDevice& _device, const VkImage& _image, const VkFormat& _format, const VkImageAspectFlags& _imageAspectFlags, const uint32_t& _mipLevels, VkAllocationCallbacks* _allocator, VkImageView *_outImageView)
{
	//Image View Create Info
	VkImageViewCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = _image;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = _format;
	create_info.subresourceRange.aspectMask = _imageAspectFlags;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = _mipLevels;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;

	//Create the Surface (With Results) [VK_SUCCESS = 0]
	VkResult r = vkCreateImageView(_device, &create_info, _allocator, _outImageView);

	//Image View has been created successfully, return it
	return r;
}
VkResult transition_image_layout(const VkDevice& _device, const VkCommandPool& _commandPool, const VkQueue& _graphicsQueue, const uint32_t& _mipLevel, const VkImage& _image, const VkFormat& _format, const VkImageLayout& _previousLayout, const VkImageLayout& _currentLayout)
{
	//Start the command buffer
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkResult r = signal_command_start(_device, _commandPool, &command_buffer);

	//Create the image memory barrier
	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.image = _image;
	image_memory_barrier.oldLayout = _previousLayout;
	image_memory_barrier.newLayout = _currentLayout;
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = _mipLevel;
	image_memory_barrier.subresourceRange.layerCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;

	//Setup the source and destination stage flags. Will be set based on the Old and New Layout set from outside
	VkPipelineStageFlags source_stage = VK_NULL_HANDLE;
	VkPipelineStageFlags destrination_stage = VK_NULL_HANDLE;

	if (_currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (_format == VK_FORMAT_D24_UNORM_S8_UINT || _format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			image_memory_barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	if (_previousLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		if (_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destrination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (_currentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destrination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (_currentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destrination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
	}
	else if (_previousLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destrination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(command_buffer, source_stage, destrination_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

	r = signal_command_end(_device, _graphicsQueue, _commandPool, &command_buffer);
	return r;
}
VkResult copy_buffer_to_image(const VkDevice& device, const VkCommandPool& command_pool, const VkQueue& graphics_queue, const VkBuffer& buffer, const VkImage& image, const VkExtent3D& extent)
{
	//Start Command Buffer
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;
	VkResult r = signal_command_start(device, command_pool, &command_buffer);

	//Setup The Buffer Image Copy
	VkBufferImageCopy buffer_image_copy = {};
	buffer_image_copy.bufferOffset = 0;
	buffer_image_copy.bufferRowLength = 0;
	buffer_image_copy.bufferImageHeight = 0;
	buffer_image_copy.imageSubresource.layerCount = 1;
	buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	buffer_image_copy.imageSubresource.mipLevel = 0;
	buffer_image_copy.imageSubresource.baseArrayLayer = 0;
	buffer_image_copy.imageOffset = { 0,0,0 };
	buffer_image_copy.imageExtent = extent;

	//Send Command to Copy Buffer to Image
	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

	//End the Command buffer
	r = signal_command_end(device, graphics_queue, command_pool, &command_buffer);

	//Return result
	return r;
}
VkResult create_image_set(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkCommandPool &_commandPool, const VkExtent3D& _extent, const VkQueue &_graphicsQueue, const uint32_t& _mipLevels, const VkSampleCountFlagBits& _msaaSample, const VkFormat& _format, 
	const VkImageTiling& _tiling, const VkImageUsageFlags& _usageFlags, const VkImageAspectFlags &_aspectFlags, const VkMemoryPropertyFlags& _memoryPropertyFlags, const VkImageLayout &_previousLayout, const VkImageLayout &_currentLayout, VkAllocationCallbacks* _allocator,
	VkImage *_outImage, VkImageView *_outImageView, VkDeviceMemory *_outDeviceMemory)
{
	//Create Image
	VkResult r = create_image(_physicalDevice, _device, _extent, _mipLevels, _msaaSample, _format, _tiling, _usageFlags, _memoryPropertyFlags, _allocator, _outImage, _outDeviceMemory);
	if (r)
		return r;

	//Create Image View
	r = create_image_view(_device, *_outImage, _format, _aspectFlags, _mipLevels, _allocator, _outImageView);
	if (r)
		return r;
	
	//Bind the image layouts
	r = transition_image_layout(_device, _commandPool, _graphicsQueue, _mipLevels, *_outImage, _format, _previousLayout, _currentLayout);
	return r;
}
VkResult create_mipmaps(const VkDevice& device, const VkCommandPool& command_pool, const VkQueue& graphics_queue, const VkImage& texture_image, const uint32_t& texture_width, const uint32_t& texture_height, const uint32_t& mip_levels)
{
	//Start the command buffer
	VkCommandBuffer command_buffer;
	VkResult r = signal_command_start(device, command_pool, &command_buffer);

	//Create the Image Memory Barrier for Mipmapping
	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.image = texture_image;
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;
	image_memory_barrier.subresourceRange.levelCount = 1;

	//Save the texture width and height for Mip levels
	int32_t mip_width = texture_width;
	int32_t mip_height = texture_height;

	//Loop for every Mip levels. NOTE: (i-1) is the current mip level, while (i) is the next mip level
	for (uint32_t i = 1; i < mip_levels; ++i) {
		//Setup the current mip level for blitting the image
		image_memory_barrier.subresourceRange.baseMipLevel = i - 1;
		image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		//Transfer the layout and Access Mask Information
		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr, 0, nullptr, 1, &image_memory_barrier);

		//Create the Blit Image. Src is (i-1), or current mip level. dst is (i), or next mip level.
		VkImageBlit image_blit = {};
		image_blit.srcSubresource.mipLevel = i - 1;
		image_blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_blit.srcSubresource.baseArrayLayer = 0;
		image_blit.srcSubresource.layerCount = 1;
		image_blit.srcOffsets[0] = { 0, 0, 0 };
		image_blit.srcOffsets[1] = { mip_width, mip_height, 1 };

		image_blit.dstSubresource.mipLevel = i;
		image_blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_blit.dstSubresource.baseArrayLayer = 0;
		image_blit.dstSubresource.layerCount = 1;
		image_blit.dstOffsets[0] = image_blit.srcOffsets[0];
		image_blit.dstOffsets[1] = { mip_width > 1 ? (mip_width >> 1) : 1 , mip_height > 1 ? (mip_height >> 1) : 1, 1 };

		//Blit the texture
		vkCmdBlitImage(command_buffer, texture_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &image_blit, VK_FILTER_LINEAR);

		//Set the layout and Access Mask (again) for the shader to read
		image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//Transfer the layout and Access Mask Information (Again, based on above values)
		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

		//Reduce the Mip level down by 1 level [By cutting width and height in half]
		if (mip_width > 1) { mip_width >>= 1; }
		if (mip_height > 1) { mip_height >>= 1; }
	}

	image_memory_barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

	//End the command
	r = signal_command_end(device, graphics_queue, command_pool, &command_buffer);

	//Mipmapping has been created successfully!
	return r;
}

//Other Important Functions
VkResult find_memory_type(const VkPhysicalDevice& _physicalDevice, const uint32_t& _filter, const VkMemoryPropertyFlags& _propertyFlags, uint32_t* _outMemoryType)
{
	//Get the Memory Properties from the Physical Device
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memory_properties);

	//Loop through the memory type count and see if there is a match with both the filter and property flags
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
	{
		if ((_filter & (1 << i)) &&
			(memory_properties.memoryTypes[i].propertyFlags & _propertyFlags) == _propertyFlags)
		{
			*_outMemoryType = i;
			return VK_SUCCESS;
		}
	}

	//Failed to find memory
	*_outMemoryType = 0x7FFFFFFF;
	return VK_RESULT_MAX_ENUM;
}

//Buffer Creation
VkResult create_buffer(const VkPhysicalDevice& physical_device, const VkDevice& device, const VkDeviceSize& size, const VkBufferUsageFlags& usage_flags, const VkMemoryPropertyFlags& property_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
	//Create Buffer Create Info
	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = size;
	buffer_create_info.usage = usage_flags;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.pNext = VK_NULL_HANDLE;

	//Create the buffer
	VkResult r = vkCreateBuffer(device, &buffer_create_info, nullptr, buffer);
	if (r)
		return r;

	//Get the memory required to allocate the buffer
	VkMemoryRequirements memory_requirement;
	vkGetBufferMemoryRequirements(device, *buffer, &memory_requirement);

	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirement.size;
	memory_allocate_info.pNext = VK_NULL_HANDLE;
	find_memory_type(physical_device, memory_requirement.memoryTypeBits, property_flags, &memory_allocate_info.memoryTypeIndex);

	//Allocate and Bind the buffer
	r = vkAllocateMemory(device, &memory_allocate_info, nullptr, buffer_memory);
	if (r)
		return r;

	r = vkBindBufferMemory(device, *buffer, *buffer_memory, 0);

	//Return the last result of that bind memory
	return r;
}
VkResult copy_buffer(const VkDevice& _device, const VkCommandPool& _commandPool, const VkQueue& _queueGraphics, const VkBuffer& _sourceBuffer, const VkBuffer& _destinationBuffer, const VkDeviceSize& _deviceSize)
{
	//Start the command buffer
	VkCommandBuffer command_buffer;
	VkResult r = signal_command_start(_device, _commandPool, &command_buffer);

	//Copy the buffer sent through
	VkBufferCopy buffer_copy = {};
	buffer_copy.srcOffset = 0;
	buffer_copy.dstOffset = 0;
	buffer_copy.size = _deviceSize;
	vkCmdCopyBuffer(command_buffer, _sourceBuffer, _destinationBuffer, 1, &buffer_copy);

	//End the command buffer
	r = signal_command_end(_device, _queueGraphics, _commandPool, &command_buffer);
	return r;
}
VkResult read_shader_file(const char* _fileName, uint64_t* _outShaderSize, char** _outShaderFile)
{
	//Open the File
	FILE *file = fopen(_fileName, "rb");
	if (!file)
		return VK_RESULT_MAX_ENUM; //File Failed to Open

	//Get the size of the file
	fseek(file, 0, SEEK_END);
	uint64_t shaderSize =  *_outShaderSize = ftell(file);

	//Copy all the contents of the file.
	rewind(file);
	char* shaderFile = *_outShaderFile = new char[shaderSize + 1];
	uint64_t readSize = fread(shaderFile, sizeof(char), shaderSize, file);
	shaderFile[shaderSize] = '\0';
	if (readSize != shaderSize)
	{
		delete[] shaderFile;	//Something went wrong with the write...
		shaderFile = nullptr;	//So delete and set to nullptr
	}

	//Close the file and return whether it was successful or not.
	fclose(file);
	return shaderFile ? VK_SUCCESS : VK_RESULT_MAX_ENUM;
}
VkResult create_shader_module(const VkDevice& _device, const uint64_t& _shaderSize, char* _shaderString, VkShaderModule* _outShaderModule)
{
	//Setup create info
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = _shaderSize;
	create_info.pCode = reinterpret_cast<const uint32_t*>(_shaderString);

	//Create the Shader Module and return the result of it.
	VkResult r = vkCreateShaderModule(_device, &create_info, nullptr, _outShaderModule);
	return r;
}
VkResult create_shader(const VkDevice& _device, const char* _fileName, const char* _entryPoint, const VkShaderStageFlagBits& _shaderType, VkShaderModule* _outShaderModule, VkPipelineShaderStageCreateInfo* _outStageInfo)
{
	//Read Shader
	uint64_t shader_size;
	char* shader_file = nullptr;
	VkResult r = read_shader_file(_fileName, &shader_size, &shader_file);
	if (r)
		return r;

	VkShaderModule shader_module = VK_NULL_HANDLE;
	r = create_shader_module(_device, shader_size, shader_file, &shader_module);
	if (r)
		return r;

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

	//return result
	return r;
}
