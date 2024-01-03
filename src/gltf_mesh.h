#pragma once

#include <string>
#include "gltf_primitive.h"

class GLTFMesh {
    std::string name;
    std::vector<GLTFPrimitive> primitives;

public:
    GLTFMesh() = default;

    // Primitives should be moved into the GLTFMesh
    GLTFMesh(const std::string &name, std::vector<GLTFPrimitive> primitives);

    void build_render_pipeline(wgpu::Device &device,
                               const wgpu::ShaderModule &shader_module,
                               const std::vector<wgpu::ColorTargetState> &color_targets,
                               const wgpu::DepthStencilState &depth_state,
                               const std::vector<wgpu::BindGroupLayout> &bind_group_layouts);

    void render(wgpu::RenderPassEncoder &pass);

    const std::string &get_name() const;
};
