#ifndef IMGUI_HELPER_H
#define IMGUI_HELPER_H

//Initialization
bool imgui_init_context();
void imgui_set_wnd_proc(void *_functionPtr);
bool imgui_init();

//Update & Render
void imgui_process_frame(void** _outClearColor);
void imgui_end_frame();
bool imgui_back_button();
void imgui_test_draw();

//Cleanup
void imgui_cleanup();
void imgui_cleanup_vulkan();

#endif