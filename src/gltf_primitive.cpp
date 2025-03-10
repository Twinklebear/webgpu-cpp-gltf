#include "gltf_primitive.h"
#include <vector>

GLTFPrimitive::GLTFPrimitive(const GLTFAccessor &positions,
                             const GLTFAccessor &indices,
                             const tinygltf::Primitive *primitive)
    : primitive(primitive), positions(positions), indices(indices)
{
}

void GLTFPrimitive::build_render_pipeline(
    wgpu::Device &device,
    const wgpu::ShaderModule &shader_module,
    const std::vector<wgpu::ColorTargetState> &color_targets,
    const wgpu::DepthStencilState &depth_state,
    const std::vector<wgpu::BindGroupLayout> &bind_group_layouts)
{
    wgpu::VertexAttribute vertex_attribute;
    vertex_attribute.format = positions.vertex_format();
    vertex_attribute.offset = 0;
    vertex_attribute.shaderLocation = 0;

    wgpu::VertexBufferLayout vertex_buf_layout;
    vertex_buf_layout.arrayStride = positions.stride();
    vertex_buf_layout.attributeCount = 1;
    vertex_buf_layout.attributes = &vertex_attribute;

    wgpu::VertexState vertex_state;
    vertex_state.module = shader_module;
    vertex_state.entryPoint = "vertex_main";
    vertex_state.bufferCount = 1;
    vertex_state.buffers = &vertex_buf_layout;

    wgpu::FragmentState fragment_state;
    fragment_state.module = shader_module;
    fragment_state.entryPoint = "fragment_main";
    fragment_state.targetCount = color_targets.size();
    fragment_state.targets = color_targets.data();

    wgpu::PipelineLayoutDescriptor pipeline_layout_desc = {};
    pipeline_layout_desc.bindGroupLayoutCount = bind_group_layouts.size();
    pipeline_layout_desc.bindGroupLayouts = bind_group_layouts.data();

    wgpu::PipelineLayout pipeline_layout = device.CreatePipelineLayout(&pipeline_layout_desc);

    wgpu::RenderPipelineDescriptor render_pipeline_desc;
    render_pipeline_desc.vertex = vertex_state;
    render_pipeline_desc.fragment = &fragment_state;
    render_pipeline_desc.layout = pipeline_layout;
    render_pipeline_desc.depthStencil = &depth_state;

    if (primitive->mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
        render_pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
        render_pipeline_desc.primitive.stripIndexFormat = indices.index_format();
    } else {
        render_pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    }

    render_pipeline = device.CreateRenderPipeline(&render_pipeline_desc);
}

void GLTFPrimitive::render(wgpu::RenderPassEncoder &pass)
{
    pass.SetPipeline(render_pipeline);

    pass.SetVertexBuffer(
        0, positions.view->gpu_buffer, positions.offset(), positions.byte_length());
    pass.SetIndexBuffer(indices.view->gpu_buffer,
                        indices.index_format(),
                        indices.offset(),
                        indices.byte_length());
    pass.DrawIndexed(indices.size());
}
