#ifndef GVK_SURFACE_SETUP_H
#define GVK_SURFACE_SETUP_H

#pragma warning(disable:26812)
#include "vulkan/vulkan.h"
#include "vulkan/vk_platform.h"

VkResult set_allocator(VkAllocationCallbacks* _allocator);
VkResult get_important_extensions(uint32_t &_instanceExtensionCount, const char ***_instanceExtensions, uint32_t &_deviceExtensionCount, const char ***_deviceExtensions);
VkResult create_instance(const char *_wintitle, const uint32_t &_instextcount, const char ***_instext, const uint32_t &_instlayercount, const char ***_instlayers, VkInstance* _out);
VkResult create_surface(const VkInstance &_instance, void *_instanceHandle, void *_windowHandle, VkSurfaceKHR *_out);
VkResult find_physical_device(const VkInstance &_instance, const VkSurfaceKHR &_surface, const uint32_t &_deviceExtensionCount, const char ***_deviceExtensions, VkPhysicalDevice *_outPhysicalDevice, int *_outQueueIndices, VkSampleCountFlagBits *_outMSAABit);
VkResult create_device(const VkInstance &_instance, const VkSurfaceKHR &_surface, const VkPhysicalDevice &_physicalDevice, const uint32_t &_deviceExtensionCount, const char ***_deviceExtensions, VkQueue *_outQueueGraphics, VkQueue *_outQueuePresent, VkDevice *_outDevice);
VkResult gather_best_swapchain_info(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkSurfaceKHR& _surface, const uint32_t& _winWidth, const uint32_t& _winHeight,
	VkSurfaceCapabilitiesKHR* _outSurfaceCapabilities, VkSurfaceFormatKHR* _outSurfaceFormat, VkPresentModeKHR* _outSurfacePresentMode, VkFormat* _outDepthFormat, VkExtent3D* _outExtent3D);
VkResult create_command_pool(const VkDevice& _device, const uint32_t& _queueGraphicsIndex, VkCommandPool* _outCommandPool);
VkResult create_swapchain(const VkPhysicalDevice& _physicaDevice, const VkDevice& _device, const VkSurfaceKHR& _surface, const VkSurfaceCapabilitiesKHR& _surfaceCapabilities, const VkSurfaceFormatKHR& _surfaceFormat, const VkPresentModeKHR& _presentMode, const VkExtent3D _extent3D, uint32_t* _outImageCount, VkSwapchainKHR* _outSwapchain, VkImage** _outSwapchainImage, VkImageView** _outSwapchainImageView);
VkResult create_renderpass(const VkDevice& _device, const VkFormat& _format, const VkFormat& _depthFormat, const VkSampleCountFlagBits& _msaaBit, VkRenderPass *_outRenderPass);
VkResult create_msaa_buffer(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent3D, const VkSampleCountFlagBits& _msaaBit, const VkQueue& _queueGraphics, const VkCommandPool& _commandPool, const VkFormat& _format, VkImage* _outMSAAImage, VkImageView* _outMSAAImageView, VkDeviceMemory* _outMSAADeviceMemory);
VkResult create_depth_buffer(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent3D, const VkSampleCountFlagBits& _msaaBit, const VkQueue& _queueGraphics, const VkCommandPool& _commandPool, const VkFormat& _format, VkImage* _outDepthImage, VkImageView* _outDepthImageView, VkDeviceMemory* _outDepthDeviceMemory);
VkResult create_frame_buffer(const VkPhysicalDevice& _physicalDevice, const VkDevice& _device, const VkExtent3D& _extent3D, const VkFormat& _format, const VkRenderPass& _renderPass, const VkImageView& _msaaImageView, const VkImageView& _depthImageView, const uint32_t& _imageCount, VkImageView* _swapchainImageViews, VkFramebuffer** _outFrameBuffers);
VkResult create_command_buffer(const VkDevice& _device, const VkCommandPool& _commandPool, const uint32_t& _imageCount, VkCommandBuffer** _outCommandBuffers);
VkResult create_fence_and_semaphores(const VkDevice& _device, const uint32_t &image_count, VkFence **_outFenceRender, VkSemaphore **_outSemaphoreImageAvailable, VkSemaphore **_outSemaphoreRenderFinished);

VkResult setup_validation_layer(const VkInstance &_instance, const uint32_t &_instLayerCount, const char ***_instLayers);
#endif