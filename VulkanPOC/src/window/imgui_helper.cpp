#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"
#include "imgui_helper.h"
#include "../vulkan/GVulkanGlobal.h"
#include <stdlib.h>

#ifdef _DEBUG
	#include <iostream>
	#include <stdio.h>
#endif

namespace {
	static VkDescriptorPool imgui_descriptor_pool;
	static VkPipelineCache imgui_pipelineCache;
	static VkAllocationCallbacks *imgui_allocator;
	static ImGui_ImplVulkanH_Window imgui_window;
	static VkCommandPool imgui_commandpool;
	static VkCommandBuffer imgui_commandBuffer;
	static VkRenderPass imgui_renderpass;
}

#ifdef _DEBUG
VkDebugReportCallbackEXT imgui_debug_report;
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
void setup_allocator();
#endif

//Prototype Declarations
static void check_vk_result(VkResult err);
void setup_descriptor_pool();
void imgui_setup_vulkan_init();

//Initialization
bool imgui_init_context()
{
	//Setup ImGui Context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.WantCaptureMouse = true;
	io.WantCaptureKeyboard = true;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	//Setup Dear ImGui style
	ImGui::StyleColorsDark();
	return false;
}
bool imgui_init()
{
	//Setup Platform/Renderer Bindings
	ImGui_ImplWin32_Init(vkStruct.winHandle);
	imgui_setup_vulkan_init();

	return false;
}
void imgui_set_wnd_proc(void* _functionPtr)
{
	ImGui_ImplWin32_SetWndProc(_functionPtr);
}

//getters
void imgui_get_command_pool(void **_outCommandPool)
{
	if (_outCommandPool)
		*_outCommandPool = imgui_commandpool;
}
void imgui_get_command_buffer(void **_outCommandBuffer)
{
	if (_outCommandBuffer)
		*_outCommandBuffer = imgui_commandBuffer;
}

//Update & Render
ImVec4 clear_color = ImVec4(0.0f, 0.2f, 0.4f, 1.0f);
void imgui_process_frame(void** _outClearColor)
{
	//Return  the clear color
	*_outClearColor = &clear_color;

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void imgui_end_frame()
{
	VkCommandBuffer cbuffer = static_cast<VkCommandBuffer>(vkStruct.vkSwapchainCommandBuffer);
	ImGui::Render();
	ImGui::EndFrame();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cbuffer);
}

bool imgui_back_button()
{
	ImGui::Begin("Back");
	bool ret = ImGui::Button("<-");
	ImGui::End();
	return ret;
}

static float f = 0.0f;
static int counter = 0;
static bool show_demo_window = true;
static bool show_another_window = false;
void imgui_test_draw(void *_currentCommandBuffer)
{
	VkCommandBuffer cbuffer = static_cast<VkCommandBuffer>(_currentCommandBuffer);

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cbuffer);
}

//Cleanup
void imgui_cleanup()
{
	ImGui_ImplVulkanH_DestroyWindow(vkStruct.vkInstance, vkStruct.vkDevice, &imgui_window, imgui_allocator);
	ImGui_ImplVulkan_Shutdown();
}
void imgui_cleanup_vulkan()
{
	vkDestroyDescriptorPool(vkStruct.vkDevice, imgui_descriptor_pool, imgui_allocator);
	vkFreeCommandBuffers(vkStruct.vkDevice, imgui_commandpool, 1, &imgui_commandBuffer);
	vkDestroyCommandPool(vkStruct.vkDevice, imgui_commandpool, imgui_allocator);
#ifdef _DEBUG
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkStruct.vkInstance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(vkStruct.vkInstance, imgui_debug_report, imgui_allocator);
#endif
}

//Prototype Definitions
void imgui_setup_vulkan_init()
{
	setup_descriptor_pool();
#ifdef _DEBUG
	setup_allocator();
#endif

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vkStruct.vkInstance;
	init_info.PhysicalDevice = vkStruct.vkPhysicalDevice;
	init_info.Device = vkStruct.vkDevice;
	init_info.QueueFamily = vkStruct.vkQueueFamilyIndexGraphics[0];
	init_info.Queue = vkStruct.vkQueueGraphics;
	init_info.PipelineCache = imgui_pipelineCache;
	init_info.DescriptorPool = imgui_descriptor_pool;
	init_info.Allocator = imgui_allocator;
	init_info.MinImageCount = vkStruct.vkSurfaceCapabilities.minImageCount;
	init_info.ImageCount = vkStruct.vkImageCount;
	init_info.CheckVkResultFn = check_vk_result;
	init_info.MSAASamples = vkStruct.vkMSAABit;
	imgui_renderpass = vkStruct.vkSwapchainRenderPass;
	ImGui_ImplVulkan_Init(&init_info, imgui_renderpass);

	//Setup Command Pool and Buffer;
	VkCommandPoolCreateInfo cpool_create_info = {};
	cpool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpool_create_info.queueFamilyIndex = vkStruct.vkQueueFamilyIndexGraphics[0];
	cpool_create_info.flags = VK_NULL_HANDLE;
	cpool_create_info.pNext = VK_NULL_HANDLE;
	vkCreateCommandPool(vkStruct.vkDevice, &cpool_create_info, imgui_allocator, &imgui_commandpool);

	//Allocate Command buffer Information
	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = imgui_commandpool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;

	vkAllocateCommandBuffers(vkStruct.vkDevice, &command_buffer_allocate_info, &imgui_commandBuffer);

	// Upload Fonts
	{
		// Use any command queue

		VkResult err = vkResetCommandPool(vkStruct.vkDevice, imgui_commandpool, 0);
		check_vk_result(err);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(imgui_commandBuffer, &begin_info);
		check_vk_result(err);

		ImGui_ImplVulkan_CreateFontsTexture(imgui_commandBuffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &imgui_commandBuffer;
		err = vkEndCommandBuffer(imgui_commandBuffer);
		check_vk_result(err);
		err = vkQueueSubmit(vkStruct.vkQueueGraphics, 1, &end_info, VK_NULL_HANDLE);
		check_vk_result(err);

		err = vkDeviceWaitIdle(vkStruct.vkDevice);
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}
static void check_vk_result(VkResult err)
{
	if (err == 0) return;
#ifdef _DEBUG
	printf("VkResult %d\n", err);
#endif
	if (err < 0)
		abort();
}
void setup_descriptor_pool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VkResult err = vkCreateDescriptorPool(vkStruct.vkDevice, &pool_info, imgui_allocator, &imgui_descriptor_pool);
	check_vk_result(err);
}

#ifdef _DEBUG
void setup_allocator()
{
	// Get the function pointer (required for any extensions)
	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkStruct.vkInstance, "vkCreateDebugReportCallbackEXT");
	IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

	// Setup the debug report callback
	VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
	debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debug_report_ci.pfnCallback = debug_report;
	debug_report_ci.pUserData = NULL;
	VkResult err = vkCreateDebugReportCallbackEXT(vkStruct.vkInstance, &debug_report_ci, imgui_allocator, &imgui_debug_report);
	check_vk_result(err);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[Vulkan] ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif
