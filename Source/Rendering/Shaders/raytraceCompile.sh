glslangvalidator -V raytracing.comp -o raytracing.comp.spv

if [ $? -ne 0 ]; then
	cmd /k
fi
