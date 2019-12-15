# Vulkan Proof of Concept
This is simply my Proof of Concept that I will be playing with for Gateware. Will Update it to adjust for Linux and Mac.

## Notes: 
There will be a couple of things that I will adjust, since this is supposed to simply be a "Proof of Concept" for Gateware. This will include the following:
* __Windowing Management__: "Win32 Window" -> GWindow. This will change this project from a "Windows-only" solution to multi-platform (Windows, Linux and MacOS). This will also allow me to test GWindow and ensure it is working as intended.
* __File Structure__: "Visual Studio Solution" -> 3 separate folders (Windows, Linux, Mac) -> CMake. The whole reason for the split process is simply due to priorities. When I finish Windows side, thats when i create the 3 structures. From there, I finish the other two, and making sure they all work on the same file, so i can finalize everything with CMake.
* __ImGui:__ Dropping. To my dislike in this decision, it is best due to the purpose of this project. One design aspect is to make it as lightweight as possible, while if i do have a library (GLM for instance), it is replacable by any library of choice. Currently, the only librarys that this will be using is: Gwindow (Dependant), GInput (Independant), GLM (Independant).

## Continuation with my Framework
If you wish to see how all of this started, I do have a repository that will continue this framework. you can access it here:
https://github.com/dgramirez/dgramirezVulkanTestFramework

## Goals:
1. Drop ImGui
	* Instantly draw the triangle. No Menu!
	* Implement Keyboard Controls.
		* Using Gateware's Input Library
		* Implement "Modes":
			* Triangle Mode
			* Vertex/Texture Mode
		* Enter Key: Swap Modes
			* Hold Ctrl: Swap Vertex/Texture Mode
			* Else: Swap (Triangle Mode) (Vertex/Texture Mode).
		* Triangle Mode
			* Left/Right Arrow: Translate Triangle
			* Left/Right Arrow + Ctrl: Rotate Triangle
			* Left/Right Arrow + Alt: Scale Triangle
			* Left/Right Arrow + Shift: Change UV (w/ Texture Mode Only!) 
		* Vertex Mode:
			* Q: Vertex 1
			* W: Vertex 2
			* E: Vertex 3
			* 1/2: Red Channel (+/-)
			* 3/4: Green Channel (+/-)
			* 5/6: Blue Channel (+/-)
	* No Dynamic Image (Will use GXI Picture). This does mean stb_image will dissapear too :(
2. Integrate GWindow
	* Simply Ensure that the window events are handled properly and the swapchain is resized properly.
3. File Structure Change #1: Separate Platforms
	* Structure to the following:
		* Windows (Visual Studio)
		* Linux (G++)
		* Mac (XCode Hopefully)
4. Work on Linux
	* Make sure It works on both Windows and Linux.
5. Work on Mac
	* Make sure it works on all 3 Platform
6. File Structure Change #2: Cmake
	* Again, make sure it works on all 3 platforms, with a single directory structure.
	
## References:
* Gateware: [Insert Gateware Link Here]
* Dear ImGui: https://github.com/ocornut/imgui
* Stb: https://github.com/nothings/stb
* GLM: https://glm.g-truc.net/0.9.9/index.html
