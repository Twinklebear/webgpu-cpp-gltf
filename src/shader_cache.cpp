#include "shader_cache.h"
#include <iostream>

ShaderCache::ShaderCache(wgpu::Device device) : device(device) {}

wgpu::ShaderModule ShaderCache::getShader(ShaderConfigMask config_mask)
{
    auto fnd = cache.find(config_mask);
    if (fnd != cache.end()) {
        return fnd->second;
    }

    const auto wgsl = generateShader(config_mask);

    wgpu::ShaderModuleWGSLDescriptor shader_module_wgsl;
    shader_module_wgsl.code = wgsl.c_str();

    wgpu::ShaderModuleDescriptor shader_module_desc;
    shader_module_desc.nextInChain = &shader_module_wgsl;
    auto shader_module = device.CreateShaderModule(&shader_module_desc);

    shader_module.GetCompilationInfo(
        [](WGPUCompilationInfoRequestStatus status, WGPUCompilationInfo const *info, void *) {
            if (info->messageCount != 0) {
                std::cout << "Shader compilation info:\n";
                for (uint32_t i = 0; i < info->messageCount; ++i) {
                    const auto &m = info->messages[i];
                    std::cout << m.lineNum << ":" << m.linePos << ": ";
                    switch (m.type) {
                    case WGPUCompilationMessageType_Error:
                        std::cout << "error";
                        break;
                    case WGPUCompilationMessageType_Warning:
                        std::cout << "warning";
                        break;
                    case WGPUCompilationMessageType_Info:
                        std::cout << "info";
                        break;
                    default:
                        break;
                    }

                    std::cout << ": " << m.message << "\n";
                }
            }
        },
        nullptr);

    cache[config_mask] = shader_module;
    return shader_module;
}

std::string ShaderCache::generateShader(ShaderConfigMask config_mask)
{
    const static std::string aliases =
        R"(alias float2 = vec2<f32>;
           alias float3 = vec3<f32>;
           alias float4 = vec4<f32>;)";


    return "";
}
