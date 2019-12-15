#ifndef __H_H__
#define __H_H__

void vkCreateTriangle(void *_physicalDevice, void *_device, void *_commandPool, void *_queueGraphics, void *_swapchainExtent2D, void *_swapchainRenderPass, void* _msaaBit);
void vkDrawTriangle(void *_commandBuffer);
void vkCleanupTriangle(void *_device);

#endif