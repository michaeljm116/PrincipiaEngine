glslangvalidator -V raytrace_main.comp -o ../../../../ShinyAfroMan/Assets/Shaders/raytracing2.comp.spv

if [ $? -ne 0 ]; then
	cmd /k
fi
