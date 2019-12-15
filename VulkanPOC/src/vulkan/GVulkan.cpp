#include "GVulkan.h"
#include "GvkSurfaceSetup.h"
#include "../tests/Tests.h"
#include "../window/myWindow.h"

namespace
{
	VkAllocationCallbacks *GvkAllocator = VK_NULL_HANDLE;

	uint32_t GvkInstanceExtensionCount;
	const char **GvkInstanceExtensions;
	uint32_t GvkInstancceLayerCount;
	const char **GvkInstancceLayers;
	uint32_t GvkDeviceExtensionCount;
	const char **GvkDeviceExtensions;

	VkInstance GvkInstance = VK_NULL_HANDLE;
	VkSurfaceKHR GvkSurface = VK_NULL_HANDLE;

	VkPhysicalDevice GvkPhysicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits GvkMSAABit = VK_SAMPLE_COUNT_1_BIT;
	int GvkQueueFamilyIndices[2] = {};
	VkQueue GvkQueueGraphics = VK_NULL_HANDLE;
	VkQueue GvkQueuePresent = VK_NULL_HANDLE;
	VkDevice GvkDevice = VK_NULL_HANDLE;

	VkCommandPool GvkCommandPool = VK_NULL_HANDLE;

	uint32_t GvkSwapchainImageSize = 0;
	VkSwapchainKHR GvkSwapchain = VK_NULL_HANDLE;
	VkImage* GvkSwapchainImages = nullptr;
	VkImageView* GvkSwapchainImageViews = nullptr;
	VkFramebuffer* GvkFrameBuffer = nullptr;
	VkCommandBuffer* GvkCommandBuffer = nullptr;

	VkSurfaceCapabilitiesKHR GvkSurfaceCapabilities;
	VkSurfaceFormatKHR GvkSwapchainSurfaceFormat = {};
	VkPresentModeKHR GvkSwapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	VkExtent2D GvkExtent2D = {};
	VkExtent3D GvkExtent3D = {};
	VkFormat GvkZBufferFormat = VK_FORMAT_UNDEFINED;

	VkRenderPass GvkRenderpass = VK_NULL_HANDLE;
	VkImage GvkMSAAImage = VK_NULL_HANDLE;
	VkImageView GvkMSAAImageView = VK_NULL_HANDLE;
	VkDeviceMemory GvkMSAADeviceMemory = VK_NULL_HANDLE;
	VkImage GvkZBufferImage = VK_NULL_HANDLE;
	VkImageView GvkZBufferImageView = VK_NULL_HANDLE;
	VkDeviceMemory GvkZBufferDeviceMemory = VK_NULL_HANDLE;

	VkSemaphore *GvkSemaphoreImageAvailable;
	VkSemaphore *GvkSemaphoreRenderFinished;
	VkFence *GvkFenceRendering;

	uint32_t GvkCurrentFrame = 0;
	myWindow *window;
	VoidFuncPtr *userCreatePipeline;
	VoidFuncPtr *userDestroyPipeline;
}

uint32_t reset_swapchain(const char& _wndEvent);

uint32_t gvk_setup_allocator(void** _allocator)
{
	GvkAllocator = static_cast<VkAllocationCallbacks*>(*_allocator);
	return 0;
}

uint32_t gvk_init(void* _window, uint32_t _instanceLayerCount, const char*** _instanceLayers, uint32_t _instanceExtensionCount, const char*** _instanceExtensions, uint32_t _deviceExtensionCount, const char*** _deviceExtensions)
{
	//Setup Result
	VkResult r = VK_SUCCESS;
	window = static_cast<myWindow*>(_window);
	void* _instanceHandle;
	window->getInstance(&_instanceHandle);
	void* _windowHandle;
	window->getWindow(&_windowHandle);

	int w = 0, h = 0;
	window->getRect(w, h);

	//Gather Important Extensions
	r = set_allocator(GvkAllocator);
	r = get_important_extensions(_instanceExtensionCount, _instanceExtensions, _deviceExtensionCount, _deviceExtensions);
	r = create_instance("Vulkan!", _instanceExtensionCount, _instanceExtensions, _instanceLayerCount, _instanceLayers, &GvkInstance);
	r = setup_validation_layer(GvkInstance, _instanceLayerCount, _instanceLayers);
	r = create_surface(GvkInstance, _instanceHandle, _windowHandle, &GvkSurface);
	r = find_physical_device(GvkInstance, GvkSurface, _deviceExtensionCount, _deviceExtensions, &GvkPhysicalDevice, GvkQueueFamilyIndices, &GvkMSAABit);
	r = create_device(GvkInstance, GvkSurface, GvkPhysicalDevice, _deviceExtensionCount, _deviceExtensions, &GvkQueueGraphics, &GvkQueuePresent, &GvkDevice);
	r = gather_best_swapchain_info(GvkPhysicalDevice, GvkDevice, GvkSurface, w, h, &GvkSurfaceCapabilities, &GvkSwapchainSurfaceFormat, &GvkSwapchainPresentMode, &GvkZBufferFormat, &GvkExtent3D);
	r = create_command_pool(GvkDevice, GvkQueueFamilyIndices[0], &GvkCommandPool);
	r = create_swapchain(GvkPhysicalDevice, GvkDevice, GvkSurface, GvkSurfaceCapabilities, GvkSwapchainSurfaceFormat, GvkSwapchainPresentMode, GvkExtent3D, &GvkSwapchainImageSize, &GvkSwapchain, &GvkSwapchainImages, &GvkSwapchainImageViews);
	r = create_renderpass(GvkDevice, GvkSwapchainSurfaceFormat.format, GvkZBufferFormat, GvkMSAABit, &GvkRenderpass);
	r = create_msaa_buffer(GvkPhysicalDevice, GvkDevice, GvkExtent3D, GvkMSAABit, GvkQueueGraphics, GvkCommandPool, GvkSwapchainSurfaceFormat.format, &GvkMSAAImage, &GvkMSAAImageView, &GvkMSAADeviceMemory);
	r = create_depth_buffer(GvkPhysicalDevice, GvkDevice, GvkExtent3D, GvkMSAABit, GvkQueueGraphics, GvkCommandPool, GvkZBufferFormat, &GvkZBufferImage, &GvkZBufferImageView, &GvkZBufferDeviceMemory);
	r = create_frame_buffer(GvkPhysicalDevice, GvkDevice, GvkExtent3D, GvkSwapchainSurfaceFormat.format, GvkRenderpass, GvkMSAAImageView, GvkZBufferImageView, GvkSwapchainImageSize, GvkSwapchainImageViews, &GvkFrameBuffer);
	r = create_command_buffer(GvkDevice, GvkCommandPool, GvkSwapchainImageSize, &GvkCommandBuffer);
	r = create_fence_and_semaphores(GvkDevice, GvkSwapchainImageSize, &GvkFenceRendering, &GvkSemaphoreImageAvailable, &GvkSemaphoreRenderFinished);
	return r;
}

void cleanup_swapchain()
{
	if (GvkMSAAImage)
	{
		vkDestroyImageView(GvkDevice, GvkMSAAImageView, GvkAllocator);
		vkDestroyImage(GvkDevice, GvkMSAAImage, GvkAllocator);
		vkFreeMemory(GvkDevice, GvkMSAADeviceMemory, GvkAllocator);
		GvkMSAAImage = VK_NULL_HANDLE;
	}

	if (GvkZBufferImage)
	{
		vkDestroyImageView(GvkDevice, GvkZBufferImageView, GvkAllocator);
		vkDestroyImage(GvkDevice, GvkZBufferImage, GvkAllocator);
		vkFreeMemory(GvkDevice, GvkZBufferDeviceMemory, GvkAllocator);
		GvkZBufferImage = VK_NULL_HANDLE;
	}

	if (GvkFrameBuffer)
	{
		for (uint32_t i = 0; i < GvkSwapchainImageSize; ++i)
			vkDestroyFramebuffer(GvkDevice, GvkFrameBuffer[i], GvkAllocator);

		delete[] GvkFrameBuffer;
		GvkFrameBuffer = nullptr;
	}

	if (GvkCommandBuffer)
	{
		vkFreeCommandBuffers(GvkDevice, GvkCommandPool, GvkSwapchainImageSize, GvkCommandBuffer);
		delete[] GvkCommandBuffer;
		GvkCommandBuffer = nullptr;
	}

	if (GvkRenderpass)
	{
		vkDestroyRenderPass(GvkDevice, GvkRenderpass, GvkAllocator);
		GvkRenderpass = VK_NULL_HANDLE;
	}

	if (GvkSwapchainImageViews)
	{
		for (unsigned int i = 0; i < GvkSwapchainImageSize; ++i)
			vkDestroyImageView(GvkDevice, GvkSwapchainImageViews[i], GvkAllocator);
		delete[] GvkSwapchainImageViews;
		GvkSwapchainImageViews = nullptr;
	}

	if (GvkSwapchain)
	{
		vkDestroySwapchainKHR(GvkDevice, GvkSwapchain, GvkAllocator);
		GvkSwapchain = nullptr;
		delete[] GvkSwapchainImages;
		GvkSwapchainImages = nullptr;
	}
}

uint32_t gvk_cleanup_end()
{
	cleanup_swapchain();

	if (GvkSemaphoreImageAvailable)
	{
		for (uint32_t i = 0; i < GvkSwapchainImageSize; ++i)
			vkDestroySemaphore(GvkDevice, GvkSemaphoreImageAvailable[i], GvkAllocator);
		delete[] GvkSemaphoreImageAvailable;
		GvkSemaphoreImageAvailable = nullptr;
	}

	if (GvkSemaphoreRenderFinished)
	{
		for (uint32_t i = 0; i < GvkSwapchainImageSize; ++i)
			vkDestroySemaphore(GvkDevice, GvkSemaphoreRenderFinished[i], GvkAllocator);
		delete[] GvkSemaphoreRenderFinished;
		GvkSemaphoreRenderFinished = nullptr;
	}

	if (GvkFenceRendering)
	{
		for (uint32_t i = 0; i < GvkSwapchainImageSize; ++i)
				vkDestroyFence(GvkDevice, GvkFenceRendering[i], GvkAllocator);
		delete[] GvkFenceRendering;
		GvkFenceRendering = nullptr;
	}

	if (GvkCommandPool)	{ vkDestroyCommandPool(GvkDevice, GvkCommandPool, GvkAllocator);GvkCommandPool = VK_NULL_HANDLE; }
	if (GvkDevice)		{ vkDestroyDevice(GvkDevice, GvkAllocator);						GvkDevice = VK_NULL_HANDLE; }
	if (GvkSurface)		{ vkDestroySurfaceKHR(GvkInstance, GvkSurface, GvkAllocator);	GvkSurface = VK_NULL_HANDLE; }
	if (GvkInstance)	{ vkDestroyInstance(GvkInstance, GvkAllocator);					GvkInstance = VK_NULL_HANDLE; }

	return 0;
}

uint32_t gvk_set_current_pipelines(VoidFuncPtr* _createPipeline, VoidFuncPtr* _destroyPipeline)
{
	userCreatePipeline = _createPipeline;
	userDestroyPipeline = _destroyPipeline;
	return 0;
}

uint32_t gvk_cleanup_start()
{
	vkWaitForFences(GvkDevice, 1, GvkFenceRendering, VK_TRUE, ~(static_cast<uint64_t>(0)));
	vkDeviceWaitIdle(GvkDevice);
	return 0;
}

uint32_t get_physical_device(void **_outPhysicalDevice)
{
	if (_outPhysicalDevice == nullptr)
		return (~0);

	*_outPhysicalDevice = GvkPhysicalDevice;

	return 0;
}

uint32_t get_instance(void **_outInstance)
{
	if (_outInstance == nullptr)
		return (~0);

	*_outInstance = GvkInstance;

	return 0;
}

uint32_t get_msaa_bit(void** _outMsaaBit)
{
	if (_outMsaaBit == nullptr)
		return (~0);

	*_outMsaaBit = &GvkMSAABit;

	return 0;
}

uint32_t get_device(void **_outDevice)
{
	if (_outDevice == nullptr)
		return (~0);

	*_outDevice = GvkDevice;

	return 0;
}

uint32_t get_command_pool(void **_outCommandPool)
{
	if (_outCommandPool == nullptr)
		return (~0);

	*_outCommandPool = GvkCommandPool;

	return 0;
}

uint32_t get_queue_graphics(void** _outQueueGraphics)
{
	if (_outQueueGraphics == nullptr)
		return (~0);

	*_outQueueGraphics = GvkQueueGraphics;

	return 0;
}

uint32_t get_swapchain_extent2d(void **_outSwapchainExtent2D)
{
	if (_outSwapchainExtent2D == nullptr)
		return (~0);

	GvkExtent2D = { GvkExtent3D.width, GvkExtent3D.height };
	*_outSwapchainExtent2D = &GvkExtent2D;

	return 0;
}

uint32_t get_renderpass(void **_outRenderPass)
{
	if (_outRenderPass == nullptr)
		return (~0);

	*_outRenderPass = GvkRenderpass;

	return 0;
}

uint32_t get_swapchain(void **_outSwapchain)
{
	if (_outSwapchain == nullptr)
		return (~0);

	*_outSwapchain = GvkSwapchain;

	return 0;
}

uint32_t get_swapchain_command_buffer(void **_outCommandBuffer)
{
	if (_outCommandBuffer == nullptr)
		return (~0);

	*_outCommandBuffer = GvkCommandBuffer[GvkCurrentFrame];

	return 0;
}

uint32_t get_surface_capabilities(void** _outSurfaceCapabilities)
{
	if (!_outSurfaceCapabilities)
		return (~0);

	*_outSurfaceCapabilities = &GvkSurfaceCapabilities;

	return 0;
}

uint32_t get_family_queue_indices(int** _outFamilyQueueIndices)
{
	if (!_outFamilyQueueIndices)
		return (~0);

	*_outFamilyQueueIndices = GvkQueueFamilyIndices;

	return 0;
}

uint32_t get_graphics_queue_index(uint32_t& _outQueueGraphics)
{
	_outQueueGraphics = GvkQueueFamilyIndices[0];
	return 0;
}

uint32_t get_min_image(uint32_t& _outImageCount)
{

	_outImageCount = GvkSurfaceCapabilities.minImageCount;
	return 0;
}

uint32_t get_image_count(uint32_t& _outImageCount)
{
	_outImageCount = GvkSwapchainImageSize;
	return 0;
}

uint32_t get_current_image_index(uint32_t& _outCurrentImageIndex)
{
	_outCurrentImageIndex = GvkCurrentFrame;
	return 0;
}

uint32_t get_fence(const int32_t& index, void **_outFence)
{
	if (!_outFence)
		return (~0);
	if (index < -1)
		return (~0);
	if (index >= static_cast<int32_t>(GvkSwapchainImageSize))
		return (~0);

	if (index == -1)
		*_outFence = GvkFenceRendering[GvkCurrentFrame];
	else
		*_outFence = GvkFenceRendering[index];

	return 0;
}

uint32_t gvk_start_frame(float **_clearValue, const char &_wndEvent)
{
	vkWaitForFences(GvkDevice, 1, &GvkFenceRendering[GvkCurrentFrame], VK_TRUE, ~(static_cast<uint64_t>(0)));

	VkResult frame_result = vkAcquireNextImageKHR(GvkDevice, GvkSwapchain, ~(static_cast<uint64_t>(0)),
		GvkSemaphoreImageAvailable[GvkCurrentFrame], VK_NULL_HANDLE, &GvkCurrentFrame);
	if (frame_result == VK_ERROR_OUT_OF_DATE_KHR || (_wndEvent & 14) )
		reset_swapchain(_wndEvent);
	else if (frame_result != VK_SUCCESS && frame_result != VK_SUBOPTIMAL_KHR)
	{
		throw "Cannot Do Frames!";
	}

	//Create the Command Buffer's Begin Info
	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	command_buffer_begin_info.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(GvkCommandBuffer[GvkCurrentFrame], &command_buffer_begin_info);

	float* color_array = *_clearValue;
	VkClearValue clear_value[2];
	clear_value[0].color = {{ color_array[0], color_array[1], color_array[2], color_array[3] }};
	clear_value[1].depthStencil = { 1.0f, 128 };

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = GvkRenderpass;
	render_pass_begin_info.framebuffer = GvkFrameBuffer[GvkCurrentFrame];
	render_pass_begin_info.renderArea.extent = {GvkExtent3D.width, GvkExtent3D.height};
	render_pass_begin_info.clearValueCount = 2;
	render_pass_begin_info.pClearValues = clear_value;

	vkCmdBeginRenderPass(GvkCommandBuffer[GvkCurrentFrame], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	return 0;
}

uint32_t gvk_end_frame()
{
	vkCmdEndRenderPass(GvkCommandBuffer[GvkCurrentFrame]);
	vkEndCommandBuffer(GvkCommandBuffer[GvkCurrentFrame]);
	//TODO: Record that the frame buffer has ended

	VkSemaphore wait_semaphores[] = { GvkSemaphoreImageAvailable[GvkCurrentFrame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signal_semaphore[] = { GvkSemaphoreRenderFinished[GvkCurrentFrame] };

	VkCommandBuffer pCommandBuffer[] = { GvkCommandBuffer[GvkCurrentFrame] };
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = pCommandBuffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphore;

	vkResetFences(GvkDevice, 1, &GvkFenceRendering[GvkCurrentFrame]);
	VkResult result;
	if (result = vkQueueSubmit(GvkQueueGraphics, 1, &submit_info,GvkFenceRendering[GvkCurrentFrame]))
	{
		throw "Queue Submit isn't Working!";
	}

	VkSwapchainKHR swapchains[] = { GvkSwapchain };

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphore; //<--- I don't get it! why signal
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &GvkCurrentFrame;
	present_info.pResults = nullptr;

	VkResult frame_result = vkQueuePresentKHR(GvkQueuePresent, &present_info);

	if (frame_result == VK_ERROR_OUT_OF_DATE_KHR || frame_result == VK_SUBOPTIMAL_KHR)
	{
		gvk_cleanup_start();
		cleanup_swapchain();
	}
	else if (frame_result != VK_SUCCESS)
	{
		throw "You've done fucked up!";
	}

	GvkCurrentFrame = (GvkCurrentFrame + 1) % GvkSwapchainImageSize;
	return 0;
}

uint32_t reset_swapchain(const char &_wndEvent)
{
	//Minimize Check
	int width = 0, height = 0;
	window->getRect(width, height);
	unsigned char wndEvent = 0;
	while (width == 0 || height == 0)
	{
		window->getEvents(wndEvent);
		window->getRect(width, height);
	}

	//Wait for Device to be ready
	vkDeviceWaitIdle(GvkDevice);

	//Destroy Pipeline
	if (*userDestroyPipeline)
		(*userDestroyPipeline)();

	//Destroy ImGui
	Test::imgui_cleanup();
	Test::imgui_cleanup_vulkan();

	//Cleanup Swapchain
	cleanup_swapchain();

	//Create Swapchain
	VkResult r = gather_best_swapchain_info(GvkPhysicalDevice, GvkDevice, GvkSurface, width, height, &GvkSurfaceCapabilities, &GvkSwapchainSurfaceFormat, &GvkSwapchainPresentMode, &GvkZBufferFormat, &GvkExtent3D);
	GvkExtent2D = { GvkExtent3D.width, GvkExtent3D.height };
	r = create_swapchain(GvkPhysicalDevice, GvkDevice, GvkSurface, GvkSurfaceCapabilities, GvkSwapchainSurfaceFormat, GvkSwapchainPresentMode, GvkExtent3D, &GvkSwapchainImageSize, &GvkSwapchain, &GvkSwapchainImages, &GvkSwapchainImageViews);
	r = create_renderpass(GvkDevice, GvkSwapchainSurfaceFormat.format, GvkZBufferFormat, GvkMSAABit, &GvkRenderpass);
	r = create_msaa_buffer(GvkPhysicalDevice, GvkDevice, GvkExtent3D, GvkMSAABit, GvkQueueGraphics, GvkCommandPool, GvkSwapchainSurfaceFormat.format, &GvkMSAAImage, &GvkMSAAImageView, &GvkMSAADeviceMemory);
	r = create_depth_buffer(GvkPhysicalDevice, GvkDevice, GvkExtent3D, GvkMSAABit, GvkQueueGraphics, GvkCommandPool, GvkZBufferFormat, &GvkZBufferImage, &GvkZBufferImageView, &GvkZBufferDeviceMemory);
	r = create_frame_buffer(GvkPhysicalDevice, GvkDevice, GvkExtent3D, GvkSwapchainSurfaceFormat.format, GvkRenderpass, GvkMSAAImageView, GvkZBufferImageView, GvkSwapchainImageSize, GvkSwapchainImageViews, &GvkFrameBuffer);
	r = create_command_buffer(GvkDevice, GvkCommandPool, GvkSwapchainImageSize, &GvkCommandBuffer);

	//Set Test Init & ImGui Init
	Test::init_vulkan_test(nullptr, nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr, GvkSwapchain, &GvkExtent2D, GvkRenderpass, GvkCommandBuffer[GvkCurrentFrame], nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, GvkSwapchainImageSize);
	Test::imgui_init();

	//Create Pipeline
	if (*userCreatePipeline)
		(*userCreatePipeline)();

	
	//Reset the Flags
	window->toggleEvent(_wndEvent);
	return 0;
}
