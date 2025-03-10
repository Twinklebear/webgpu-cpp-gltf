#include "gltf_node.h"
#include <vector>
#include <glm/ext.hpp>

GLTFNode::GLTFNode(const std::string &name, const glm::mat4 &transform, GLTFMesh *mesh)
    : name(name), transform(transform), mesh(mesh)
{
}

void GLTFNode::build_render_pipeline(
    wgpu::Device &device,
    const wgpu::ShaderModule &shader_module,
    const std::vector<wgpu::ColorTargetState> &color_targets,
    const wgpu::DepthStencilState &depth_state,
    const std::vector<wgpu::BindGroupLayout> &bind_group_layouts)
{
    if (!mesh) {
        return;
    }

    // TODO: make the node params bg
    {
        wgpu::BufferDescriptor desc;
        desc.size = sizeof(glm::mat4);
        desc.usage = wgpu::BufferUsage::Uniform;
        desc.mappedAtCreation = true;

        node_params_buf = device.CreateBuffer(&desc);

        std::memcpy(
            node_params_buf.GetMappedRange(), glm::value_ptr(transform), sizeof(glm::mat4));

        node_params_buf.Unmap();
    }

    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.buffer.hasDynamicOffset = false;
        entry.buffer.type = wgpu::BufferBindingType::Uniform;
        entry.visibility = wgpu::ShaderStage::Vertex;

        wgpu::BindGroupLayoutDescriptor desc;
        desc.entryCount = 1;
        desc.entries = &entry;

        node_params_bg_layout = device.CreateBindGroupLayout(&desc);
    }

    {
        wgpu::BindGroupEntry entry;
        entry.binding = 0;
        entry.buffer = node_params_buf;
        entry.size = node_params_buf.GetSize();

        wgpu::BindGroupDescriptor desc;
        desc.layout = node_params_bg_layout;
        desc.entryCount = 1;
        desc.entries = &entry;

        node_params_bg = device.CreateBindGroup(&desc);
    }

    std::vector<wgpu::BindGroupLayout> bg_layouts = bind_group_layouts;
    bg_layouts.push_back(node_params_bg_layout);

    mesh->build_render_pipeline(device, shader_module, color_targets, depth_state, bg_layouts);
}

void GLTFNode::render(wgpu::RenderPassEncoder &pass)
{
    if (!mesh) {
        return;
    }
    // TODO: we bind the node params bg then pass on
    pass.SetBindGroup(1, node_params_bg);
    mesh->render(pass);
}

const std::string &GLTFNode::get_name() const
{
    return name;
}
