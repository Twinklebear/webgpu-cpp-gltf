#include "gltf_mesh.h"

GLTFMesh::GLTFMesh(const std::string &name, std::vector<GLTFPrimitive> primitives)
    : name(name), primitives(std::move(primitives))
{
}

void GLTFMesh::build_render_pipeline(
    wgpu::Device &device,
    const wgpu::ShaderModule &shader_module,
    const std::vector<wgpu::ColorTargetState> &color_targets,
    const wgpu::DepthStencilState &depth_state,
    const std::vector<wgpu::BindGroupLayout> &bind_group_layouts)
{
    for (auto &p : primitives) {
        p.build_render_pipeline(
            device, shader_module, color_targets, depth_state, bind_group_layouts);
    }
}

void GLTFMesh::render(wgpu::RenderPassEncoder &pass)
{
    for (auto &p : primitives) {
        p.render(pass);
    }
}

const std::string &GLTFMesh::get_name() const
{
    return name;
}
