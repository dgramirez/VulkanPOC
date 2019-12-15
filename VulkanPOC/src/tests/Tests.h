#ifndef TESTS_H
#define TESTS_H

#include <vector>
#include <functional>
typedef void (*VoidFuncPtr)();

class Test {
public:
	Test() {}
	virtual ~Test() {}

	virtual void Update(const float& _deltaTime) {}
	virtual void Render() {}
	virtual void RenderImGui() {}

	static VoidFuncPtr createPipelinePtr;
	static VoidFuncPtr destroyPipelinePtr;
	virtual VoidFuncPtr SendCreatePipeline() = 0;
	virtual VoidFuncPtr SendDestroyPipeline() = 0;

	static void init_vulkan_test(void* winInst, void* winWindow, void* inst, void* pdev, void* surf, const int& msaa, void* qgraph, void* qpres, void* dev, void* cpool,
		void* schain, void* ext, void* rpass, void* cbuff, void* semaia, void* semarf, void* fence, int* qfamidxgraph,
		void* surfcap, void* surfform, const int& presmode, const int& imgcount);

	static void vulkan_update(void *commandBuffer, void *semaphoreIA, void *semaphoreRF, void *fenceR, const uint32_t& framenum);
	static void vulkan_reset(void* swapChain, void* renderPass, void* commandBuffer);

	//ImGui Init
	static void imgui_setup();
	static void imgui_set_wnd_proc(void* _functionPtr);
	static void imgui_init();

	//Update & Render
	static void imgui_start_frame(void** _outClearColor);
	static void imgui_end_frame();
	static bool imgui_back_button();
	static void imgui_test_draw();

	//Cleanup
	static void imgui_cleanup();
	static void imgui_cleanup_vulkan();

	void *m_CommandPool;
	void *m_CommandBuffer;
};

class TestMenu : public Test {
public:
	TestMenu(Test*& _currentTestPtr);
	~TestMenu();

	void RenderImGui() override;
	virtual VoidFuncPtr SendCreatePipeline();
	virtual VoidFuncPtr SendDestroyPipeline();

	template<typename T>
	void RegisterTest(const char* test_name)
	{
		m_Tests.push_back(std::make_pair(test_name, []() { return new T(); }));
	}

private:
	float *m_clearColor;
	Test *&m_CurrentTest;
	std::vector<std::pair<const char*, std::function<Test*()>>> m_Tests;
};

#endif