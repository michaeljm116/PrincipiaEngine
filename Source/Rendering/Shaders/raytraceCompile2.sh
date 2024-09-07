glslangvalidator -V raytrace-main.comp -o ../../../../ShinyAfroMan/Bin/Assets/Shaders/raytracing.comp.spv

if [ $? -ne 0 ]; then
	cmd /k
fi
