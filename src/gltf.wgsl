alias float2 = vec2<f32>;
alias float3 = vec3<f32>;
alias float4 = vec4<f32>;

struct VertexInput {
    @location(0) position: float3
    @location(1) uv: float2
}

struct VertexOutput {
    @builtin(position) position: float4,
    @location(0) world_pos: float3,
    @location(1) uv: float2,
}

struct ViewParams {
    view_proj: mat4x4<f32>,
}
@group(0) @binding(0)
var<uniform> view_params: ViewParams;

struct NodeParams {
    transform: mat4x4<f32>,
};
@group(1) @binding(0)
var<uniform> node_params: NodeParams;

@vertex
fn vertex_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = view_params.view_proj * node_params.transform * float4(in.position, 1.0);
    out.world_pos = in.position.xyz;
    out.uv = in.uv;
    return out;
};

@fragment
fn fragment_main(in: VertexOutput) -> @location(0) float4 {
    let dx = dpdx(in.world_pos);
    let dy = dpdy(in.world_pos);
    let n = normalize(cross(dx, dy));
    //return float4((n + 1.0) * 0.5, 1.0);
    return float4(in.uv, 0.0, 1.0);
}

