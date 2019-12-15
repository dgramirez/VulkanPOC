#pragma warning(disable:26812)
#include "vulkan/vk_platform.h"

typedef void (*VoidFuncPtr)();

uint32_t gvk_setup_allocator(void **_allocator);
uint32_t gvk_init(void* _window, uint32_t _instanceLayerCount, const char ***_instanceLayers, uint32_t _instanceExtensionCount, const char ***_instanceExtensions, uint32_t _deviceExtensionCount, const char ***_deviceExtensions);
uint32_t gvk_cleanup_start();
uint32_t gvk_cleanup_end();

uint32_t gvk_set_current_pipelines(VoidFuncPtr* _createPipeline, VoidFuncPtr* _destroyPipeline);

uint32_t get_instance(void **_outInstance);
uint32_t get_physical_device(void **_outPhysicalDevice);
uint32_t get_msaa_bit(void **_outMsaaBit);
uint32_t get_device(void **_outDevice);
uint32_t get_command_pool(void **_outCommandPool);
uint32_t get_queue_graphics(void **_outQueueGraphics);
uint32_t get_swapchain_extent2d(void **_outSwapchainExtent2D);
uint32_t get_renderpass(void **_outRenderPass);
uint32_t get_swapchain(void **_outSwapchain);
uint32_t get_swapchain_command_buffer(void **_outCommandBuffer);
uint32_t get_surface_capabilities(void **_outSurfaceCapabilities);
uint32_t get_family_queue_indices(int **_outFamilyQueueIndices);
uint32_t get_fence(const int32_t& index, void** _outFence);

uint32_t get_graphics_queue_index(uint32_t& _outQueueGraphics);
uint32_t get_min_image(uint32_t& _outImageCount);
uint32_t get_image_count(uint32_t &_outImageCount);
uint32_t get_current_image_index(uint32_t &_outCurrentImageIndex);

uint32_t gvk_start_frame(float **_clearColor, const char &_wndEvent);
uint32_t gvk_end_frame();