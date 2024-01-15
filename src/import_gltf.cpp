#include "import_gltf.h"
#include <iostream>
#include <memory>
#include "gltf_accessor.h"
#include "gltf_primitive.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

std::unique_ptr<GLTFRenderData> import_gltf(const uint8_t *glb, const size_t glb_size)
{
    auto gltf_model = std::make_unique<GLTFRenderData>();

    tinygltf::TinyGLTF context;
    std::string err, warn;
    bool ret = false;
    ret = context.LoadBinaryFromMemory(&gltf_model->model, &err, &warn, glb, glb_size);
    if (!warn.empty()) {
        std::cout << "Warning loading GLB: " << warn << "\n";
    }
    if (!ret || !err.empty()) {
        std::cerr << "Error loading GLB: " << err << "\n";
    }

    std::cout << "GLTF file loaded\n";

    auto &model = gltf_model->model;
    // Create GLTFBufferViews for all the buffer views in the file
    for (auto &bv : model.bufferViews) {
        const auto &buf = model.buffers[bv.buffer];
        gltf_model->buffers.emplace_back(&bv, buf);
    }

    std::cout << "# of meshes: " << model.meshes.size() << "\n";
    for (const auto &m : model.meshes) {
        std::cout << "Mesh name: " << m.name << " has " << m.primitives.size()
                  << " primitives\n";
        std::vector<GLTFPrimitive> primitives;
        for (const auto &p : m.primitives) {
            if (p.mode != TINYGLTF_MODE_TRIANGLES && p.mode != TINYGLTF_MODE_TRIANGLE_STRIP) {
                std::cout << "Skipping non-triangle primitive in " << m.name << "\n";
                continue;
            }

            GLTFAccessor indices;
            if (p.indices != -1) {
                const auto &acc = model.accessors[p.indices];
                auto &bv = gltf_model->buffers[acc.bufferView];
                indices = GLTFAccessor(&bv, &acc);

                bv.needs_upload = true;
                bv.add_usage(wgpu::BufferUsage::Index);
            }

            GLTFAccessor positions;
            for (const auto &attr : p.attributes) {
                if (attr.first == "POSITION") {
                    const auto &acc = model.accessors[attr.second];
                    auto &bv = gltf_model->buffers[acc.bufferView];
                    positions = GLTFAccessor(&bv, &acc);

                    bv.needs_upload = true;
                    bv.add_usage(wgpu::BufferUsage::Vertex);
                }
            }

            if (positions.size() == 0) {
                std::cerr << "Primitive in " << m.name << " has no positions!\n";
                continue;
            }

            primitives.emplace_back(positions, indices, &p);
        }
        gltf_model->meshes.emplace_back(m.name, std::move(primitives));
    }

    // Flatten the GLTF nodes
    if (model.defaultScene == -1) {
        model.defaultScene = 0;
    }
    flatten_gltf(model);

    // Load the GLTF Nodes for the default scene
    for (const auto &nid : model.scenes[model.defaultScene].nodes) {
        const auto &n = model.nodes[nid];
        if (n.mesh != -1) {
            const glm::mat4 xfm = read_node_transform(n);
            auto *mesh = &gltf_model->meshes[n.mesh];
            gltf_model->nodes.emplace_back(n.name, xfm, mesh);
        }
    }
    return gltf_model;
}
