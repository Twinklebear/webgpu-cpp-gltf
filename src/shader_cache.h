#pragma once

#include "unordered_dense.h"
#include "webgpu_cpp.h"

enum ShaderConfig {
    ShaderConfig_None = 0,
    ShaderConfig_TexCoords = 1,
};

using ShaderConfigMask = uint32_t;

class ShaderCache {
    wgpu::Device device;
    ankerl::unordered_dense::map<ShaderConfigMask, wgpu::ShaderModule> cache;

public:
    ShaderCache() = default;

    ShaderCache(wgpu::Device device);

    wgpu::ShaderModule getShader(ShaderConfigMask config_mask);

private:
    std::string generateShader(ShaderConfigMask config_mask);
};
