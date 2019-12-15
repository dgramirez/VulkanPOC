@echo off
SET shdr=triangle

c:\VulkanSDK\1.1.121.2\Bin\glslangValidator.exe -V %shdr%.vert
c:\VulkanSDK\1.1.121.2\Bin\glslangValidator.exe -V %shdr%.frag

@echo off
move ./vert.spv ./%shdr%.vert.spv
move ./frag.spv ./%shdr%.frag.spv
PAUSE