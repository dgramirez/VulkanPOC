#include "GvkSurfaceSetup.h"
#include "GVulkanHelper.h"

#ifdef _WIN32
	#include <windows.h>
	#include <WinUser.h>
	#include "vulkan/vulkan_win32.h"
#endif

#ifdef __linux__
	#include <X11/Xlib.h>
	#include "vulkan/vulkan_xlib.h"
#endif

#ifdef __APPLE__
	#include "vulkan/vulkan_macos.h"
#endif

static VkAllocationCallbacks *Allocator;
VkResult set_allocator(VkAllocationCallbacks *_allocator)
{
	Allocator = _allocator;
	return VK_SUCCESS;
}

VkResult get_extensions_win32(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions);
VkResult get_extensions_linux(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions);
VkResult get_extensions_macOS(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions);

VkResult get_important_extensions(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions) {
	//Separate this into 3 functions: Windows, Linux, MacOS
	#ifdef _WIN32
		return get_extensions_win32(_instanceExtensionCount, _instanceExtensions, _deviceExtensionCount, _deviceExtensions);
	#elif __linux__
		return get_extensions_linux(_instanceExtensionCount, _instanceExtensions, _deviceExtensionCount, _deviceExtensions);
	#elif __APPLE__
		return get_extensions_macOS(_instanceExtensionCount, _instanceExtensions, _deviceExtensionCount, _deviceExtensions);
	#endif
}
VkResult get_extensions_win32(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions) {
#ifdef _WIN32
	//First: Error Check the Crap out of this:
	if (!_instanceExtensions)
		return VK_RESULT_MAX_ENUM;
	if (!_deviceExtensions)
		return VK_RESULT_MAX_ENUM;

	//Second: Create Variable and set it to the dereferenced of the parameters
	const char** instance_extensions = *_instanceExtensions;
	const char** device_extensions = *_deviceExtensions;

	//Third: Check out the Instance Extension
	if (!instance_extensions)
	{
		_instanceExtensionCount = 2;
		const char ** new_ext = new const char* [_instanceExtensionCount];

		new_ext[0] = "VK_KHR_surface";
		new_ext[1] = "VK_KHR_win32_surface";

		*_instanceExtensions = new_ext;
	}
	else
	{
		//Setup what is needed
		const uint32_t TOTAL_NEED = 2;
		const char* needed_extensions[TOTAL_NEED] = { "VK_KHR_surface" , "VK_KHR_win32_surface" };
		bool need_extension[TOTAL_NEED] = { true, true };

		//Loop to find the needed extensions
		const char** current_extensions = instance_extensions;
		for (uint32_t i = 0; i < TOTAL_NEED; ++i)
			for (uint32_t j = 0; j < _instanceExtensionCount; ++j)
				if (!strcmp(needed_extensions[i], current_extensions[j]))
					need_extension[i] = false;

		//Have a count adder (without this, i can't properly allocate a new array)
		uint32_t added_count = 0;
		for (uint32_t i = 0; i < TOTAL_NEED; ++i)
			if (need_extension[i])
				++added_count;

		//If there is anything that is needed, we need to make a new array
		if (added_count > 0)
		{
			//Gather the total array size and create a new array
			uint32_t total_count = _instanceExtensionCount + added_count;
			const char ** new_ext = new const char*[total_count];

			//Fill in the new array with the previous array
			for (uint32_t i = 0; i < _instanceExtensionCount; ++i)
				new_ext[i] = current_extensions[i];

			//Fill in Whatever is needed
			for (uint32_t i = 0; i < TOTAL_NEED; ++i)
				if (need_extension[i])
				{
					--total_count;
					new_ext[total_count] = needed_extensions[i];
				}

			_instanceExtensionCount += added_count;
			delete[] current_extensions;
			*_instanceExtensions = new_ext;
		}
	}

	//Check out the Device Extensions
	if (!device_extensions)
	{
		_deviceExtensionCount = 1;
		const char **new_ext = new const char*[_deviceExtensionCount];

		new_ext[0] = "VK_KHR_swapchain";
		*_deviceExtensions = new_ext;
	}
	else
	{
		//Setup what is needed
		const uint32_t TOTAL_NEED = 1;
		const char* needed_extensions[TOTAL_NEED] = { "VK_KHR_swapchain" };
		bool need_extension[TOTAL_NEED] = { true };

		//Loop to find the needed extensions
		const char** current_extensions = device_extensions;
		for (uint32_t i = 0; i < TOTAL_NEED; ++i)
			for (uint32_t j = 0; j < _deviceExtensionCount; ++j)
				if (!strcmp(needed_extensions[i], current_extensions[j]))
					need_extension[i] = false;

		//Have a count adder (without this, i can't properly allocate a new array)
		uint32_t added_count = 0;
		for (uint32_t i = 0; i < TOTAL_NEED; ++i)
			if (need_extension[i])
				++added_count;

		//If there is anything that is needed, we need to make a new array
		if (added_count > 0)
		{
			//Gather the total array size and create a new array
			uint32_t total_count = _deviceExtensionCount + added_count;
			const char** new_ext = new const char* [total_count];

			//Fill in the new array with the previous array
			for (uint32_t i = 0; i < _instanceExtensionCount; ++i)
				new_ext[i] = current_extensions[i];

			//Fill in Whatever is needed
			for (uint32_t i = 0; i < TOTAL_NEED; ++i)
				if (need_extension[i])
				{
					--total_count;
					new_ext[total_count] = needed_extensions[i];
				}

			_deviceExtensionCount += added_count;
			delete[] current_extensions;
			*_deviceExtensions = new_ext;
		}
	}
#endif // _DEBUG

	return VK_SUCCESS;
}
VkResult get_extensions_linux(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions) {
#ifdef __linux__
#endif

	return VK_SUCCESS;
}
VkResult get_extensions_macOS(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions) {
#ifdef __APPLE__
#endif

	return VK_SUCCESS;
}

VkResult create_instance(const char *_wintitle, const uint32_t &_instextcount, const char ***_instext, const uint32_t &_instlayercount, const char ***_instlayers, VkInstance* _out)
{
	//Application Information
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_1;
	app_info.pApplicationName = _wintitle;
	app_info.applicationVersion = 1;
	app_info.pEngineName = "Gateware";
	app_info.engineVersion = 0;
	app_info.pNext = nullptr;

	//Application Create Info [Basics]
	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	//Extensions
	create_info.enabledExtensionCount = _instextcount;
	create_info.ppEnabledExtensionNames = *_instext;
	create_info.enabledLayerCount = _instlayercount;
	create_info.ppEnabledLayerNames = *_instlayers;

	VkResult r = vkCreateInstance(&create_info, Allocator, _out);
	return r;
}
VkResult create_surface(const VkInstance &_instance, void *_instanceHandle, void *_windowHandle, VkSurfaceKHR *_out)
{
	VkResult r;

#ifdef _WIN32
	HINSTANCE hInst = static_cast<HINSTANCE>(_instanceHandle);
	HWND hWnd = static_cast<HWND>(_windowHandle);

	VkWin32SurfaceCreateInfoKHR create_info;
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.hinstance = hInst;
	create_info.hwnd = hWnd;
	create_info.flags = 0;
	create_info.pNext = 0;

	r = vkCreateWin32SurfaceKHR(_instance, &create_info, Allocator, _out);
#elif __linux__
	void *display = static_cast<void*>(_instanceHandle);
	void *window = static_cast<void*>(_windowHandle);

	VkXlibSurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.dpy = display;
	create_info.window = window;
	create_info.flags = 0;
	create_info.pNext = 0;

	r = vkCreateXlibSurfaceKHR(_instance, &create_info, Allocator, _out);
#elif __APPLE__
	VkMacOSSurfaceCreateInfoMVK create_info = {};
	void *view = static_cast<void*>(_windowHandle);
	create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	create_info.pView = view;
	create_info.flags = 0;
	create_info.pNext = 0;

	vkCreateMacOSSurfaceMVK(_instance, &create_info, Allocator, _out);
#endif
	return r;
}
VkResult find_physical_device(const VkInstance &_instance, const VkSurfaceKHR &_surface, const uint32_t &_deviceExtensionCount, const char ***_deviceExtensions, VkPhysicalDevice *_outPhysicalDevice, int *_outQueueIndices, VkSampleCountFlagBits *_outMSAABit)
{
	//Get ALL GPUs
	uint32_t device_count = 0;
	VkPhysicalDevice* all_physical_devices = VK_NULL_HANDLE;
	VkResult r = get_physical_devices(_instance, &device_count, &all_physical_devices);

	//Pick Best GPU
	uint32_t best_gpu_index = 0;
	get_best_gpu(_surface, device_count, all_physical_devices, _deviceExtensionCount, *_deviceExtensions, &best_gpu_index); //<---Needs to change to some form of algorithm to best suit
	*_outPhysicalDevice = all_physical_devices[best_gpu_index];

	//Get Family Queue based on best GPU
	VkBool32 canCompute = VK_FALSE;
	r = get_best_queue_family_indices(all_physical_devices[best_gpu_index], _surface, &_outQueueIndices, &canCompute);

	//Get Best MSAA Based on best GPU
	r = get_best_msaa_format(all_physical_devices[best_gpu_index], VK_SAMPLE_COUNT_8_BIT, _outMSAABit);

	delete[] all_physical_devices;
	return VK_SUCCESS;
}
VkResult create_device(const VkInstance &_instance, const VkSurfaceKHR &_surface, const VkPhysicalDevice &_physicalDevice, const uint32_t &_deviceExtensionCount, const char ***_deviceExtensions, VkQueue *_outQueueGraphics, VkQueue *_outQueuePresent, VkDevice *_outDevice)
{
	//Setup Queue Families for Create Info
	int qf[2] = { -1, -1 };
	int* pqf = qf;
	VkBool32 can_compute;
	VkResult r = get_best_queue_family_indices(_physicalDevice, _surface, &pqf, &can_compute);
	VkDeviceQueueCreateInfo* queue_create_info_array = new VkDeviceQueueCreateInfo[2];

	uint32_t qf_createsize = 0;
	if (qf[0] == qf[1])
		qf_createsize = 1;
	else
		qf_createsize = 2;

	//Set up Create Info for all unique queue families
	float priority = 1.0f;
	for (uint32_t i = 0; i < qf_createsize; ++i)
	{
		VkDeviceQueueCreateInfo create_info = {};

		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		create_info.queueFamilyIndex = qf[i];
		create_info.queueCount = 1;
		create_info.pQueuePriorities = &priority;
		queue_create_info_array[i] = create_info;
	}

	//Setup device features (Any Additional Features SHOULD BE CHECKED in DeviceCompatibility Function)
	VkPhysicalDeviceFeatures all_device_features;
	vkGetPhysicalDeviceFeatures(_physicalDevice, &all_device_features);

	VkPhysicalDeviceFeatures device_features = {};
	if (all_device_features.tessellationShader)	device_features.tessellationShader = VK_TRUE;
	if (all_device_features.geometryShader)		device_features.geometryShader = VK_TRUE;
	if (all_device_features.samplerAnisotropy)	device_features.samplerAnisotropy = VK_TRUE;
	if (all_device_features.sampleRateShading)	device_features.sampleRateShading = VK_TRUE;
	if (all_device_features.fillModeNonSolid)	device_features.fillModeNonSolid = VK_TRUE;

	//Setup Logical device create info
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_info_array;
	create_info.queueCreateInfoCount = qf_createsize;
	create_info.pEnabledFeatures = &device_features;

	create_info.enabledExtensionCount = _deviceExtensionCount;
	create_info.ppEnabledExtensionNames = *_deviceExtensions;

	//Create the Surface (With Results) [VK_SUCCESS = 0]
	r = vkCreateDevice(_physicalDevice, &create_info, Allocator, _outDevice);

	//If Device has been created, Setup the Device Queue for graphics and present family
	vkGetDeviceQueue(*_outDevice, qf[0], 0, _outQueueGraphics);
	vkGetDeviceQueue(*_outDevice, qf[1], 0, _outQueuePresent);

	//Device has been created successfully!
	delete[] queue_create_info_array;
	return r;
}
VkResult gather_best_swapchain_info(const VkPhysicalDevice &_physicalDevice, const VkDevice &_device, const VkSurfaceKHR &_surface, const uint32_t &_winWidth, const uint32_t &_winHeight,
	VkSurfaceCapabilitiesKHR * _outSurfaceCapabilities, VkSurfaceFormatKHR *_outSurfaceFormat, VkPresentModeKHR *_outSurfacePresentMode, VkFormat *_outDepthFormat, VkExtent3D *_outExtent3D)
{
	VkSurfaceCapabilitiesKHR surface_capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surface_capabilities);
	*_outSurfaceCapabilities = surface_capabilities;

	VkSurfaceFormatKHR surface_format;
	VkResult r = get_best_surface_formats(_physicalDevice, _surface, &surface_format);
	*_outSurfaceFormat = surface_format;

	VkPresentModeKHR present_mode;
	r = get_best_surface_present_mode(_physicalDevice, _surface, &present_mode);
	*_outSurfacePresentMode = present_mode;

	VkExtent2D extent;
	r = get_surface_extent(_physicalDevice, _surface, { _winWidth, _winHeight }, &extent);
	*_outExtent3D = { extent.width, extent.height, 1 };
	
	VkFormat depth_format;
	r = find_depth_format(_physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_NULL_HANDLE, &depth_format);
	*_outDepthFormat = depth_format;
	
	return r;
}
VkResult create_command_pool(const VkDevice& _device, const uint32_t& _queueGraphicsIndex, VkCommandPool *_outCommandPool)
{
	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = _queueGraphicsIndex;

	VkResult r = vkCreateCommandPool(_device, &create_info, Allocator, _outCommandPool);
	return r;
}
VkResult create_swapchain(const VkPhysicalDevice& _physicaDevice, const VkDevice& _device, const VkSurfaceKHR& _surface, const VkSurfaceCapabilitiesKHR &_surfaceCapabilities, const VkSurfaceFormatKHR &_surfaceFormat, const VkPresentModeKHR &_presentMode, const VkExtent3D _extent3D, uint32_t *_outImageCount, VkSwapchainKHR *_outSwapchain, VkImage **_outSwapchainImage, VkImageView **_outSwapchainImageView)
{
	//Gather Swapchain Count
	uint32_t image_count = 2u; //EXPERIMENTAL! [Working: image_count  = surface_capabilities.minImageCount + 1;]
	if (_surfaceCapabilities.maxImageCount > 0 && image_count > _surfaceCapabilities.maxImageCount)
		image_count = _surfaceCapabilities.maxImageCount;
 
	VkExtent2D extent = {_extent3D.width, _extent3D.height};

	//Create Info for SwapchainKHR [Part 1]
	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = _surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = _surfaceFormat.format;
	create_info.imageColorSpace = _surfaceFormat.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	create_info.preTransform = _surfaceCapabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = _presentMode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	
	//Create Info for Swapchain KHR [Part 2: Queue Families]
	//TODO: IT IS POSSIBLE FOR IT TO BE CONCURRENT!!!!!!!!
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult r = vkCreateSwapchainKHR(_device, &create_info, Allocator, _outSwapchain);

 	//Swapchain Image Setup
	VkSwapchainKHR swapchain = *_outSwapchain;
	r = vkGetSwapchainImagesKHR(_device, swapchain, _outImageCount, nullptr);
	image_count = *_outImageCount;

	VkImage *swapchain_images = new VkImage[image_count];
	r = vkGetSwapchainImagesKHR(_device, swapchain, _outImageCount, swapchain_images);

	VkImageView* swapchain_imageview = new VkImageView[image_count];
 	for (uint32_t i = 0; i < image_count; ++i)
 		create_image_view(_device, swapchain_images[i], _surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, Allocator, &swapchain_imageview[i]);

	*_outSwapchainImage = swapchain_images;
	*_outSwapchainImageView = swapchain_imageview;
	return VK_SUCCESS;
}
VkResult create_renderpass(const VkDevice& _device, const VkFormat& _format, const VkFormat& _depthFormat, const VkSampleCountFlagBits& _msaaBit, VkRenderPass* _outRenderPass)
{
	VkAttachmentDescription color_attachment_description = {};
	color_attachment_description.format = _format;
	color_attachment_description.samples = _msaaBit;
	color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_attachment_reference = {};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription color_attachment_resolve = {};
	color_attachment_resolve.format = _format;
	color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_resolve_reference = {};
	color_attachment_resolve_reference.attachment = 2;
	color_attachment_resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment_description = {};
	depth_attachment_description.format = _depthFormat;
	depth_attachment_description.samples = _msaaBit;
	depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_reference = {};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;
	subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
	subpass_description.pResolveAttachments = &color_attachment_resolve_reference;

	VkSubpassDependency subpass_dependency = {};
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = 0;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[3] = { color_attachment_description, depth_attachment_description, color_attachment_resolve };
	VkRenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 3;
	render_pass_create_info.pAttachments = attachments;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass_description;
	render_pass_create_info.dependencyCount = 1;
	render_pass_create_info.pDependencies = &subpass_dependency;

	VkRenderPass render_pass;
	VkResult r = vkCreateRenderPass(_device, &render_pass_create_info, nullptr, &render_pass);

	*_outRenderPass = render_pass;

	return r;
}
VkResult create_msaa_buffer(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent3D, const VkSampleCountFlagBits& _msaaBit, const VkQueue& _queueGraphics, const VkCommandPool& _commandPool, const VkFormat& _format, VkImage* _outMSAAImage, VkImageView* _outMSAAImageView, VkDeviceMemory* _outMSAADeviceMemory)
{
	//Create the image and image view for MSAA
	VkResult r = create_image(_physicalDevice, _device, _extent3D, 1, _msaaBit, _format, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Allocator, _outMSAAImage, _outMSAADeviceMemory);

	if (r)
	{
		if (*_outMSAAImage)
			vkDestroyImage(_device, *_outMSAAImage, VK_NULL_HANDLE);
		
		*_outMSAAImage = VK_NULL_HANDLE;
		return r;
	}

	r = create_image_view(_device, *_outMSAAImage, _format, VK_IMAGE_ASPECT_COLOR_BIT, 1, Allocator, _outMSAAImageView);
	if (r)
	{
		if (*_outMSAAImageView)
			vkDestroyImageView(_device, *_outMSAAImageView, VK_NULL_HANDLE);
		if (*_outMSAAImage)
			vkDestroyImage(_device, *_outMSAAImage, VK_NULL_HANDLE);

		*_outMSAAImageView = VK_NULL_HANDLE;
		*_outMSAAImage = VK_NULL_HANDLE;
		return r;
	}

	//Transition the image layout from Undefined to Color Attachment (Optimal)
	r = transition_image_layout(_device, _commandPool, _queueGraphics, 1,
		*_outMSAAImage, _format,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	return r;
}
VkResult create_depth_buffer(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent3D, const VkSampleCountFlagBits& _msaaBit, const VkQueue& _queueGraphics, const VkCommandPool& _commandPool, const VkFormat &_format, VkImage* _outDepthImage, VkImageView* _outDepthImageView, VkDeviceMemory* _outDepthDeviceMemory)
{
	//Create the image and image view for Depth Buffer
	VkResult r = create_image(_physicalDevice, _device, _extent3D, 1, _msaaBit, _format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Allocator, _outDepthImage, _outDepthDeviceMemory);

	if (r)
	{
		if (*_outDepthImage)
			vkDestroyImage(_device, *_outDepthImage, VK_NULL_HANDLE);
		if (*_outDepthDeviceMemory)
			vkFreeMemory(_device, *_outDepthDeviceMemory, VK_NULL_HANDLE);

		*_outDepthImage = VK_NULL_HANDLE;
		*_outDepthDeviceMemory = VK_NULL_HANDLE;
		return r;
	}

	r = create_image_view(_device, *_outDepthImage, _format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, Allocator, _outDepthImageView);
	if (r)
	{
		if (*_outDepthImageView)
			vkDestroyImageView(_device, *_outDepthImageView, VK_NULL_HANDLE);
		if (*_outDepthImage)
			vkDestroyImage(_device, *_outDepthImage, VK_NULL_HANDLE);
		if (*_outDepthDeviceMemory)
			vkFreeMemory(_device, *_outDepthDeviceMemory, VK_NULL_HANDLE);

		*_outDepthImageView = VK_NULL_HANDLE;
		*_outDepthImage = VK_NULL_HANDLE;
		return r;
	}

	//Transition the image layout from Undefined to Color Attachment (Optimal)
	r = transition_image_layout(_device, _commandPool, _queueGraphics, 1,
		*_outDepthImage, _format,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	return r;
}
VkResult create_frame_buffer(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent3D, const VkFormat& _format, const VkRenderPass& _renderPass, const VkImageView& _msaaImageView, const VkImageView& _depthImageView, const uint32_t& _imageCount, VkImageView* _swapchainImageViews, VkFramebuffer** _outFrameBuffers)
{
	VkFramebuffer* frame_buffers = new VkFramebuffer[_imageCount];
	VkResult r = VK_SUCCESS;
	//Loop through the Swapchain Frame Buffers and set their create info
	for (unsigned int i = 0; i < _imageCount; ++i) {
		// Create an array of image attachments for create info (NOTE: There is only 1 Color Image View and Depth Buffer!)
		VkImageView image_attachments[3] = {
			_msaaImageView,
			_depthImageView,
			_swapchainImageViews[i]
		};

		//Frame Buffer's Create Info
		VkFramebufferCreateInfo frame_buffer_create_info = {};
		frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_create_info.renderPass = _renderPass;
		frame_buffer_create_info.attachmentCount = 3;
		frame_buffer_create_info.pAttachments = image_attachments;
		frame_buffer_create_info.width = _extent3D.width;
		frame_buffer_create_info.height = _extent3D.height;
		frame_buffer_create_info.layers = 1;


		//Create the Surface (With Results) [VK_SUCCESS = 0]
		r = vkCreateFramebuffer(_device, &frame_buffer_create_info, nullptr, &frame_buffers[i]);
	}

	*_outFrameBuffers = frame_buffers;
	return r;
}
VkResult create_command_buffer(const VkDevice &_device, const VkCommandPool &_commandPool, const uint32_t &_imageCount, VkCommandBuffer **_outCommandBuffers)
{
	VkCommandBuffer* command_buffers = new VkCommandBuffer[_imageCount];

	//Allocate Command buffer Information
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = _commandPool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = _imageCount;

	VkResult r = vkAllocateCommandBuffers(_device, &command_buffer_allocate_info, command_buffers);
	
	*_outCommandBuffers = command_buffers;
	return r;
}
VkResult create_fence_and_semaphores(const VkDevice& _device, const uint32_t& image_count, VkFence **_outFenceRender, VkSemaphore **_outSemaphoreImageAvailable, VkSemaphore **_outSemaphoreRenderFinished)
{
	//Resize based on the max frames
	VkSemaphore *image_available_semaphore = new VkSemaphore[image_count];
	VkSemaphore *render_finished_semaphore = new VkSemaphore[image_count];
	VkFence *fences = new VkFence[image_count];

	//Semaphore Info Create
	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	//Fence Info Create
	VkFenceCreateInfo fence_create_info = {};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	//Create all the Semaphores and Fences needed
	for (unsigned int i = 0; i < image_count; ++i)
	{
		vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &image_available_semaphore[i]);
		vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &render_finished_semaphore[i]);
		vkCreateFence(_device, &fence_create_info, nullptr, &fences[i]);
	}

	*_outFenceRender = fences;
	*_outSemaphoreImageAvailable = image_available_semaphore;
	*_outSemaphoreRenderFinished = render_finished_semaphore;

	//Semaphores and Fences has been created successfully!
	return VK_SUCCESS;
}


#if _DEBUG
#include <iostream>
static VkDebugUtilsMessengerEXT validation_layer;

VKAPI_ATTR VkBool32 VKAPI_CALL gvk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data)
{
#ifdef _DEBUG
	std::cerr << "Validation Layer: " << callback_data->pMessage << std::endl;
#endif
	return VK_FALSE;
}

void gvk_destroy_debug_utils_messenger_ext(const VkInstance& instance, const VkDebugUtilsMessengerEXT& debug_messenger, const VkAllocationCallbacks* allocator)
{
	auto address = (PFN_vkDestroyDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	if (address)
		address(instance, debug_messenger, allocator);
}

VkResult gvk_create_debug_utils_messenger_ext(
	const VkInstance& instance,
	const VkDebugUtilsMessengerCreateInfoEXT* create_info,
	const VkAllocationCallbacks* allocator,
	VkDebugUtilsMessengerEXT* debug_messenger)
{
	auto address = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (address)
		return address(instance, create_info, allocator, debug_messenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}
#endif
VkResult setup_validation_layer(const VkInstance &_instance, const uint32_t &_instLayerCount, const char ***_instLayers)
{
#if _DEBUG
	if (check_instance_layer_name("VK_LAYER_LUNARG_standard_validation"))
		return VK_RESULT_MAX_ENUM;

	bool is_valid = false;
	for (uint32_t i = 0; i < _instLayerCount; ++i)
		if (!strcmp(*_instLayers[i], "VK_LAYER_LUNARG_standard_validation"))
			is_valid = true;
	if (!is_valid)
		return VK_RESULT_MAX_ENUM;

	VkDebugUtilsMessengerCreateInfoEXT create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = gvk_debug_callback;
	create_info.pUserData = nullptr;

	VkResult r = gvk_create_debug_utils_messenger_ext(_instance, &create_info, nullptr, &validation_layer);
#endif
	return VK_SUCCESS;
}
