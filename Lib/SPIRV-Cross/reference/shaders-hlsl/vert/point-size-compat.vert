static float4 gl_Position;
static float gl_PointSize;
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = float4(1.0f, 1.0f, 1.0f, 1.0f);
    gl_PointSize = 10.0f;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
