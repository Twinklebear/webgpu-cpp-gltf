// The wgpu_app.js file is generated by Emscript
import App from "./wgpu_app.js";

type WasmModule = Awaited<ReturnType<typeof App>>;

export class WGPUApp {
  wasm: WasmModule;

  constructor(wasm: WasmModule) {
    this.wasm = wasm;
  }

  public callMain(canvasId: string, fixed_dpi: number = 0) {
    this.wasm.callMain([canvasId, `${fixed_dpi}`]);
  }

  public loadGLTFBuffer(data: Uint8Array) {
    // Allocate memory for the data and copy it in
    const ptr = this.wasm._malloc(data.byteLength);
    this.wasm.HEAPU8.set(data, ptr);
    this.wasm.loadGLTFBuffer(ptr, data.byteLength);
    // Release the memory we allocated in the Wasm, it's no longer needed
    this.wasm._free(ptr);
  }
}

export async function loadApp(args: any = {}) {
  const adapter = await navigator.gpu.requestAdapter();
  const device = await adapter.requestDevice();

  const app = await App({
    preinitializedWebGPUDevice: device,
    ...args
  });

  app.loadGLTFBuffer = app.cwrap("load_gltf_buffer", null, ["number", "number"]);

  return new WGPUApp(app);
}

