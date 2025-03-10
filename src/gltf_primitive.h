#pragma once

#include <vector>
#include "gltf_accessor.h"
#include "tiny_gltf.h"
#include <webgpu/webgpu_cpp.h>

class GLTFPrimitive {
    const tinygltf::Primitive *primitive = nullptr;
    GLTFAccessor positions;
    GLTFAccessor indices;

    wgpu::RenderPipeline render_pipeline;

public:
    GLTFPrimitive() = default;

    GLTFPrimitive(const GLTFAccessor &positions,
                  const GLTFAccessor &indices,
                  const tinygltf::Primitive *primitive);

    void build_render_pipeline(wgpu::Device &device,
                               const wgpu::ShaderModule &shader_module,
                               const std::vector<wgpu::ColorTargetState> &color_targets,
                               const wgpu::DepthStencilState &depth_state,
                               const std::vector<wgpu::BindGroupLayout> &bind_group_layouts);

    void render(wgpu::RenderPassEncoder &pass);
};
