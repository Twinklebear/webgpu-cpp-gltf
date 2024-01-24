#include "gltf_primitive.h"
#include <iostream>
#include <vector>
#include "gltf_accessor.h"
#include "webgpu_cpp.h"

GLTFPrimitive::GLTFPrimitive(const GLTFAccessor &positions,
                             const GLTFAccessor &indices,
                             const GLTFAccessor &texcoords,
                             const tinygltf::Primitive *primitive)
    : primitive(primitive), positions(positions), indices(indices), texcoords(texcoords)
{
    if (texcoords.size() == 0) {
        std::cerr << "TODO/Temp: Texcoords are currently required\n";
    }
}

void GLTFPrimitive::build_render_pipeline(
    wgpu::Device &device,
    const wgpu::ShaderModule &shader_module,
    const std::vector<wgpu::ColorTargetState> &color_targets,
    const wgpu::DepthStencilState &depth_state,
    const std::vector<wgpu::BindGroupLayout> &bind_group_layouts)
{
    std::vector<wgpu::VertexBufferLayout> vertex_buffers;

    // Positions attribute and buffer
    wgpu::VertexAttribute position_attrib;
    position_attrib.format = positions.vertex_format();
    position_attrib.offset = 0;
    position_attrib.shaderLocation = 0;
    {
        wgpu::VertexBufferLayout vbl;
        vbl.arrayStride = positions.stride();
        vbl.attributeCount = 1;
        vbl.attributes = &position_attrib;
        vertex_buffers.push_back(vbl);
    }

    // UVs attribute and buffer
    // TODO: should have some array/fixed size buffer of these to make it a bit simpler
    wgpu::VertexAttribute texcoords_attrib;
    if (texcoords.size() > 0) {
        texcoords_attrib.format = texcoords.vertex_format();
        texcoords_attrib.offset = 0;
        texcoords_attrib.shaderLocation = 1;

        wgpu::VertexBufferLayout vbl;
        vbl.arrayStride = texcoords.stride();
        vbl.attributeCount = 1;
        vbl.attributes = &texcoords_attrib;
        vertex_buffers.push_back(vbl);
    }

    wgpu::VertexState vertex_state;
    vertex_state.module = shader_module;
    vertex_state.entryPoint = "vertex_main";
    vertex_state.bufferCount = vertex_buffers.size();
    vertex_state.buffers = vertex_buffers.data();

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
    if (texcoords.size() > 0) {
        pass.SetVertexBuffer(
            1, texcoords.view->gpu_buffer, texcoords.offset(), texcoords.byte_length());
    }
    pass.SetIndexBuffer(indices.view->gpu_buffer,
                        indices.index_format(),
                        indices.offset(),
                        indices.byte_length());
    pass.DrawIndexed(indices.size());
}
