#ifndef GVULKANGLOBAL_H
#define GVULKANGLOBAL_H

#include "vulkan/vulkan.h"

//Class and Extern. Can be Commented If Needed
struct vkGlobalStruct {
	void *instHandle;
	void *winHandle;
	VkInstance vkInstance;
	VkPhysicalDevice vkPhysicalDevice;
	VkSurfaceKHR vkSurface;
	VkSampleCountFlagBits vkMSAABit;
	VkQueue vkQueueGraphics;
	VkQueue vkQueuePresent;
	VkDevice vkDevice;
	VkCommandPool vkCommandPool;
	VkSwapchainKHR vkSwapchain;
	VkExtent2D vkSwapchainExtent;
	VkRenderPass vkSwapchainRenderPass;
	VkCommandBuffer vkSwapchainCommandBuffer;	//x
	VkSemaphore vkSemaphoreImageAvailable;		//x
	VkSemaphore vkSemaphoreRenderFinished;		//x
	VkFence vkFenceRendering;					//x

	int32_t* vkQueueFamilyIndexGraphics;
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
	VkSurfaceFormatKHR vkSurfaceFormat;
	VkPresentModeKHR vkSwapchainPresentMode;
	uint32_t vkImageCount;
	uint32_t vkCurrentFrame;
} ;

extern vkGlobalStruct vkStruct;

#endif