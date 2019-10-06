@pushd "%~dp0"
@%VULKAN_SDK%\Bin32\glslangValidator.exe -V ..\src\shader.vert -o ..\src\vert.spv
@%VULKAN_SDK%\Bin32\glslangValidator.exe -V ..\src\shader.frag -o ..\src\frag.spv
@%VULKAN_SDK%\Bin32\glslangValidator.exe -V ..\src\line_shader.vert -o ..\src\line_vert.spv
@%VULKAN_SDK%\Bin32\glslangValidator.exe -V ..\src\line_shader.frag -o ..\src\line_frag.spv
@g++ -I%VULKAN_SDK%\Include -L%VULKAN_SDK%\Lib32 ..\src\maths.c ..\src\graphics.c ..\src\rng.c ..\src\main.c -o ..\bin\dungeon_gen.exe -lvulkan-1
@popd
