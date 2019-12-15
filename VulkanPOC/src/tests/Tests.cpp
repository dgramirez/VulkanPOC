#include "Tests.h"
#include "vulkan/vulkan.h"
#include "../vulkan/GVulkanGlobal.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

// ________                      __            __       __                               
//|        \                    |  \          |  \     /  \                              
// \$$$$$$$$______    _______  _| $$_         | $$\   /  $$  ______   _______   __    __ 
//   | $$  /      \  /       \|   $$ \        | $$$\ /  $$$ /      \ |       \ |  \  |  \
//   | $$ |  $$$$$$\|  $$$$$$$ \$$$$$$        | $$$$\  $$$$|  $$$$$$\| $$$$$$$\| $$  | $$
//   | $$ | $$    $$ \$$    \   | $$ __       | $$\$$ $$ $$| $$    $$| $$  | $$| $$  | $$
//   | $$ | $$$$$$$$ _\$$$$$$\  | $$|  \      | $$ \$$$| $$| $$$$$$$$| $$  | $$| $$__/ $$
//   | $$  \$$     \|       $$   \$$  $$      | $$  \$ | $$ \$$     \| $$  | $$ \$$    $$
//    \$$   \$$$$$$$ \$$$$$$$     \$$$$        \$$      \$$  \$$$$$$$ \$$   \$$  \$$$$$$ 

VoidFuncPtr Test::createPipelinePtr = nullptr;
VoidFuncPtr Test::destroyPipelinePtr = nullptr;

TestMenu::TestMenu(Test*& _currentTestPtr)
	: m_CurrentTest(_currentTestPtr) {
	m_clearColor = new float[4];
	m_clearColor[0] = 0.0f;
	m_clearColor[1] = 0.0f;
	m_clearColor[2] = 0.0f;
	m_clearColor[3] = 1.0f;
	
	m_CommandPool = nullptr;
	m_CommandBuffer = nullptr;
}
TestMenu::~TestMenu()
{
	delete[] m_clearColor;
}
void TestMenu::RenderImGui()
{
	ImGui::Begin("Main Menu");
	for (auto& i : m_Tests)
	{
		if (ImGui::Button(i.first))
		{
			m_CurrentTest = i.second();
			createPipelinePtr = m_CurrentTest->SendCreatePipeline();
			destroyPipelinePtr = m_CurrentTest->SendDestroyPipeline();
		}
	}
	ImGui::End();
}
VoidFuncPtr TestMenu::SendCreatePipeline()
{
	return nullptr;
}
VoidFuncPtr TestMenu::SendDestroyPipeline()
{
	return nullptr;
}

// ______                 ______             __         ___            __     __            __  __                          
//|      \               /      \           |  \       /   \          |  \   |  \          |  \|  \                         
// \$$$$$$ ______ ____  |  $$$$$$\ __    __  \$$      |  $$$\         | $$   | $$ __    __ | $$| $$   __  ______   _______  
//  | $$  |      \    \ | $$ __\$$|  \  |  \|  \       \$$ $$__       | $$   | $$|  \  |  \| $$| $$  /  \|      \ |       \ 
//  | $$  | $$$$$$\$$$$\| $$|    \| $$  | $$| $$      | \$$$/  \       \$$\ /  $$| $$  | $$| $$| $$_/  $$ \$$$$$$\| $$$$$$$\
//  | $$  | $$ | $$ | $$| $$ \$$$$| $$  | $$| $$      | $$\$$ $$        \$$\  $$ | $$  | $$| $$| $$   $$ /      $$| $$  | $$
// _| $$_ | $$ | $$ | $$| $$__| $$| $$__/ $$| $$      | $$_\$$\          \$$ $$  | $$__/ $$| $$| $$$$$$\|  $$$$$$$| $$  | $$
//|   $$ \| $$ | $$ | $$ \$$    $$ \$$    $$| $$       \$$  \$$\          \$$$    \$$    $$| $$| $$  \$$\\$$    $$| $$  | $$
// \$$$$$$ \$$  \$$  \$$  \$$$$$$   \$$$$$$  \$$        \$$$$ $$           \$      \$$$$$$  \$$ \$$   \$$ \$$$$$$$ \$$   \$$

namespace {
	static VkDescriptorPool imgui_descriptor_pool;
	static VkPipelineCache imgui_pipelineCache;
	static VkAllocationCallbacks* imgui_allocator;
	static ImGui_ImplVulkanH_Window imgui_window;
	static VkCommandPool imgui_commandpool;
	static VkCommandBuffer imgui_commandBuffer;
	static VkRenderPass imgui_renderpass;

	ImVec4 clear_color = ImVec4(0.0f, 0.2f, 0.4f, 1.0f);
	static float f = 0.0f;
	static int counter = 0;
	static bool show_demo_window = true;
	static bool show_another_window = false;
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

//Static Functions
void Test::init_vulkan_test(void *winInst, void *winWindow, void *inst, void *pdev, void *surf, const int &msaa, void *qgraph, void *qpres, void *dev, void *cpool, 
	void *schain, void *ext, void *rpass, void *cbuff, void *semaia, void *semarf, void *fence, int* qfamidxgraph,
	void *surfcap, void *surfform, const int &presmode, const int &imgcount)
{
	vkStruct.instHandle =			  winInst	? winInst : vkStruct.instHandle;
	vkStruct.winHandle =			winWindow	? winWindow : vkStruct.winHandle;
	vkStruct.vkInstance =				 inst	? static_cast<VkInstance>(inst) : vkStruct.vkInstance;
	vkStruct.vkPhysicalDevice =			 pdev	? static_cast<VkPhysicalDevice>(pdev) : vkStruct.vkPhysicalDevice;
	vkStruct.vkSurface =				 surf	? static_cast<VkSurfaceKHR>(surf) : vkStruct.vkSurface;
	vkStruct.vkMSAABit =				 msaa	? static_cast<VkSampleCountFlagBits>(msaa) : vkStruct.vkMSAABit;
	vkStruct.vkQueueGraphics =			 qgraph ? static_cast<VkQueue>(qgraph) : vkStruct.vkQueueGraphics;
	vkStruct.vkQueuePresent =			 qpres	? static_cast<VkQueue>(qpres) : vkStruct.vkQueuePresent;
	vkStruct.vkDevice =					 dev	? static_cast<VkDevice>(dev) : vkStruct.vkDevice;
	vkStruct.vkCommandPool =			 cpool	? static_cast<VkCommandPool>(cpool) : vkStruct.vkCommandPool;
	vkStruct.vkSwapchain =				 schain	? static_cast<VkSwapchainKHR>(schain) : vkStruct.vkSwapchain;
	vkStruct.vkSwapchainExtent =		 ext	? *static_cast<VkExtent2D*>(ext) : vkStruct.vkSwapchainExtent;
	vkStruct.vkSwapchainRenderPass =	 rpass	? static_cast<VkRenderPass>(rpass) : vkStruct.vkSwapchainRenderPass;
	vkStruct.vkSwapchainCommandBuffer =	 cbuff	? static_cast<VkCommandBuffer>(cbuff) : vkStruct.vkSwapchainCommandBuffer;
	vkStruct.vkSemaphoreImageAvailable = semaia	? static_cast<VkSemaphore>(semaia) : vkStruct.vkSemaphoreImageAvailable;
	vkStruct.vkSemaphoreRenderFinished = semarf	? static_cast<VkSemaphore>(semarf) : vkStruct.vkSemaphoreRenderFinished;
	vkStruct.vkFenceRendering =			 fence	? static_cast<VkFence>(fence) : vkStruct.vkFenceRendering;

	vkStruct.vkQueueFamilyIndexGraphics = qfamidxgraph ? qfamidxgraph : vkStruct.vkQueueFamilyIndexGraphics;
	vkStruct.vkSurfaceCapabilities = surfcap ? *static_cast<VkSurfaceCapabilitiesKHR*>(surfcap) : vkStruct.vkSurfaceCapabilities;
	vkStruct.vkSurfaceFormat = surfform ? *static_cast<VkSurfaceFormatKHR*>(surfform) : vkStruct.vkSurfaceFormat;
	vkStruct.vkSwapchainPresentMode = presmode ? static_cast<VkPresentModeKHR>(presmode) : VK_PRESENT_MODE_IMMEDIATE_KHR;
	vkStruct.vkImageCount = imgcount;
}
void Test::vulkan_update(void* commandBuffer, void* semaphoreIA, void* semaphoreRF, void* fenceR, const uint32_t& framenum)
{
	vkStruct.vkSwapchainCommandBuffer	= commandBuffer ? static_cast<VkCommandBuffer>(commandBuffer) : vkStruct.vkSwapchainCommandBuffer;
	vkStruct.vkSemaphoreImageAvailable	= semaphoreIA ? static_cast<VkSemaphore>(semaphoreIA) : vkStruct.vkSemaphoreImageAvailable;
	vkStruct.vkSemaphoreRenderFinished	= semaphoreRF ? static_cast<VkSemaphore>(semaphoreRF) : vkStruct.vkSemaphoreRenderFinished;
	vkStruct.vkFenceRendering			= fenceR ? static_cast<VkFence>(semaphoreRF) : vkStruct.vkFenceRendering;
	vkStruct.vkCurrentFrame				= framenum;
}
void Test::vulkan_reset(void* swapChain, void* renderPass, void* commandBuffer)
{
	vkStruct.vkSwapchain =				swapChain		? static_cast<VkSwapchainKHR>(swapChain)	  : vkStruct.vkSwapchain;
	vkStruct.vkSwapchainRenderPass =	renderPass		? static_cast<VkRenderPass>(renderPass)		  : vkStruct.vkSwapchainRenderPass;
	vkStruct.vkSwapchainCommandBuffer =	commandBuffer	? static_cast<VkCommandBuffer>(commandBuffer) : vkStruct.vkSwapchainCommandBuffer;
}

//Initialization
void Test::imgui_setup()
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
}
void Test::imgui_set_wnd_proc(void* _functionPtr)
{
	ImGui_ImplWin32_SetWndProc(_functionPtr);
}
void Test::imgui_init()
{
	//Setup Platform/Renderer Bindings
	ImGui_ImplWin32_Init(vkStruct.winHandle);
	imgui_setup_vulkan_init();
}
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
static void check_vk_result(VkResult err)
{
	if (err == 0) return;
#ifdef _DEBUG
	printf("VkResult %d\n", err);
#endif
	if (err < 0)
		abort();
}

//Render Loop
void Test::imgui_start_frame(void** _outClearColor)
{
	//Return  the clear color
	*_outClearColor = &clear_color;

	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}
void Test::imgui_end_frame()
{
	VkCommandBuffer cbuffer = static_cast<VkCommandBuffer>(vkStruct.vkSwapchainCommandBuffer);
	ImGui::Render();
	ImGui::EndFrame();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cbuffer);
}
bool Test::imgui_back_button()
{
	ImGui::Begin("Back");
	bool ret = ImGui::Button("<-");
	ImGui::End();
	return ret;
}
void Test::imgui_test_draw()
{
	VkCommandBuffer cbuffer = static_cast<VkCommandBuffer>(vkStruct.vkSwapchainCommandBuffer);

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
void Test::imgui_cleanup()
{
	ImGui_ImplVulkanH_DestroyWindow(vkStruct.vkInstance, vkStruct.vkDevice, &imgui_window, imgui_allocator);
	ImGui_ImplVulkan_Shutdown();
}
void Test::imgui_cleanup_vulkan()
{
	vkDestroyDescriptorPool(vkStruct.vkDevice, imgui_descriptor_pool, imgui_allocator);
	vkFreeCommandBuffers(vkStruct.vkDevice, imgui_commandpool, 1, &imgui_commandBuffer);
	vkDestroyCommandPool(vkStruct.vkDevice, imgui_commandpool, imgui_allocator);
#ifdef _DEBUG
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkStruct.vkInstance, "vkDestroyDebugReportCallbackEXT");
	vkDestroyDebugReportCallbackEXT(vkStruct.vkInstance, imgui_debug_report, imgui_allocator);
#endif
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
