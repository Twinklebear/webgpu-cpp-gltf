#pragma once

#include <string>
#include "gltf_mesh.h"
#include "webgpu_cpp.h"
#include <glm/glm.hpp>

class GLTFNode {
    std::string name;
    glm::mat4 transform;
    // Reference to the shared set of meshes
    GLTFMesh *mesh = nullptr;

    wgpu::Buffer node_params_buf;
    wgpu::BindGroupLayout node_params_bg_layout;
    wgpu::BindGroup node_params_bg;

public:
    GLTFNode() = default;

    GLTFNode(const std::string &name, const glm::mat4 &transform, GLTFMesh *mesh);

    void build_render_pipeline(wgpu::Device &device,
                               const wgpu::ShaderModule &shader_module,
                               const std::vector<wgpu::ColorTargetState> &color_targets,
                               const wgpu::DepthStencilState &depth_state,
                               const std::vector<wgpu::BindGroupLayout> &bind_group_layouts);

    void render(wgpu::RenderPassEncoder &pass);

    const std::string &get_name() const;
};
