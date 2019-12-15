//Includes for ALL Platforms
#include "./window/myWindow.h"
#include "vulkan/GVulkan.h"
#include "./tests/Tests.h"
#include "./tests/0-Triangle/0-Triangle.h"
#include "./tests/0-Triangle/TestTriangle.h"

//Win32 Only Defines (Memory Leak Detection, Warning Disable [enum class]
#ifdef _WIN32
	#pragma warning(disable:26812)
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>
		#include <iostream>
	#endif
#endif

//Global Variables
namespace {
	myWindow *window;
	TestMenu *test_menu = nullptr;
	Test *current_test;
	void *voidTemp;
	void *instance_handle, *window_handle, *wnd_proc;
	unsigned int graphics_queue_index, min_swapchain_image_count, swapchain_image_count;
}

bool init();
bool init_vulkan();
bool setup_variables();
void run();
void cleanup();
void show_message(const char* msg, bool stop);

int main()
{
	#ifdef _WIN32
	//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//	_CrtSetBreakAlloc(212);
	#endif
	
	if (init())
	{
		show_message("Failed to Initialize the program!", true);
		return 0;
	}

	run();

	cleanup();
	return 0;
}

bool init()
{
	//Setup ImGui Context
	Test::imgui_setup();

	//Setup and initialize the window
	window = new myWindow();

	//Get the WndProc Function Pointer
	window->getWinProc(&wnd_proc);
	Test::imgui_set_wnd_proc(wnd_proc);

	const char *msg;
	if (window->init(&instance_handle, &window_handle, &msg))
	{
		show_message(msg, false);
		delete window;
		return true;
	}

	init_vulkan();
	setup_variables();
	Test::imgui_init();

	return false;
}
bool init_vulkan()
{
	//Initialize Vulkan Surface
	const char** inst_ext, ** lyr_ext, ** dev_ext;

#ifdef _DEBUG
	uint32_t inst_ext_count = 3;
	inst_ext = new const char* [inst_ext_count];
	inst_ext[0] = "VK_KHR_surface";
	inst_ext[1] = "VK_KHR_win32_surface";
	inst_ext[2] = "VK_EXT_debug_report"; //This is for ImGui

	uint32_t inst_layer_count = 1;
	lyr_ext = new const char* [inst_layer_count];
	lyr_ext[0] = "VK_LAYER_LUNARG_standard_validation";
#else
	uint32_t inst_ext_count = 2;
	inst_ext = new const char* [inst_ext_count];
	inst_ext[0] = "VK_KHR_surface";
	inst_ext[1] = "VK_KHR_win32_surface";

	uint32_t inst_layer_count = 0;
	lyr_ext = nullptr;
#endif

	uint32_t device_ext_count = 1;
	dev_ext = new const char* [device_ext_count];
	dev_ext[0] = "VK_KHR_swapchain";

	if (gvk_init(&window, inst_layer_count, &lyr_ext, inst_ext_count, &inst_ext, device_ext_count, &dev_ext))
	{
		show_message("Failed to initialize Vulkan!", false);
		delete window;
		return true;
	}

	if (inst_ext) delete[] inst_ext;
	if (lyr_ext) delete[] lyr_ext;
	if (dev_ext) delete[] dev_ext;

	return 0;
}
bool setup_variables()
{
	//Setup the void pointers
	void *instance, *physical_device, *device, *command_pool, *queue_graphics, *swapchain_extent2d, *renderpass, *swapchain, *msaa_bit, *fence;
	int *qfg;

	get_instance(&instance);
	get_physical_device(&physical_device);
	get_msaa_bit(&msaa_bit);
	get_device(&device);
	get_command_pool(&command_pool);
	get_queue_graphics(&queue_graphics);
	get_swapchain_extent2d(&swapchain_extent2d);
	get_renderpass(&renderpass);
	get_swapchain(&swapchain);
	get_graphics_queue_index(graphics_queue_index);
	get_min_image(min_swapchain_image_count);
	get_image_count(swapchain_image_count);
	get_surface_capabilities(&voidTemp);
	get_family_queue_indices(&qfg);
	get_fence(0, &fence);

	Test::init_vulkan_test(nullptr, window_handle, instance, physical_device, nullptr, *(int*)msaa_bit, queue_graphics, nullptr, device, command_pool, nullptr, swapchain_extent2d,
		renderpass, nullptr, nullptr, nullptr, fence, qfg, voidTemp, nullptr, 0, swapchain_image_count);

	return 0;
}

void run()
{
	//Create the ImGui Test Menu
	test_menu = new TestMenu(current_test);
	current_test = test_menu;
	test_menu->RegisterTest<Test0_Triangle>("Triangle Test");

	//Create the Triangle
	float *clearColor = new float[4];
	clearColor[0] = 0.0f; clearColor[1] = 0.2f; clearColor[2] = 0.4f; clearColor[3] = 0.0f;

	//Run the main loop
	unsigned char events;
	bool change_test = false;
	unsigned int framenum;
	gvk_set_current_pipelines(&Test::createPipelinePtr, &Test::destroyPipelinePtr);
	while (window->run)
	{
		//Get Events & Do Updates
		window->getEvents(events);
		current_test->Update(0);
		Test::imgui_start_frame((void**)(&clearColor));

		//Start Draw
		gvk_start_frame(&clearColor, events);
		get_swapchain_command_buffer(&voidTemp);
		get_current_image_index(framenum);
		Test::vulkan_update(voidTemp, nullptr, nullptr, nullptr, framenum);

		//Render
		current_test->Render();
		current_test->RenderImGui();
		if (current_test != test_menu && Test::imgui_back_button())
			change_test = true;

		//End Draw & Present
		Test::imgui_end_frame();
		gvk_end_frame();

		if (change_test)
		{
			delete current_test;
			current_test = test_menu;
			clearColor[0] = 0.0f; clearColor[1] = 0.2f; clearColor[2] = 0.4f; clearColor[3] = 0.0f;
			change_test = false;
		}
	}
}

void cleanup()
{
	gvk_cleanup_start();

	Test::imgui_cleanup_vulkan();
	Test::imgui_cleanup();
	
	gvk_cleanup_end();
	delete test_menu;
	delete window;
}
void show_message(const char* msg, bool stop)
{
#ifdef _DEBUG
	std::cout << msg << std::endl;
	if (stop)
	{
		std::cout << "Press the enter key to continue . . .";
		std::cin.get();
	}
#endif
}
