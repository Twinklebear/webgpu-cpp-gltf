import WGPUApp from "./cpp/wgpu_app.js";

(async () =>
{
    let canvas = document.getElementById("webgpu-canvas")
    if (navigator.gpu === undefined) {
        canvas.setAttribute("style", "display:none;");
        document.getElementById("no-webgpu").setAttribute("style", "display:block;");
        return;
    }

    // Block right click so we can use right click + drag to pan
    canvas.addEventListener("contextmenu", (evt) =>
    {
        evt.preventDefault();
    });

    // Get a GPU device to render with
    let adapter = await navigator.gpu.requestAdapter();
    let device = await adapter.requestDevice();

    // We set -sINVOKE_RUN=0 when building and call main ourselves because something
    // within the promise -> call directly chain was gobbling exceptions
    // making it hard to debug
    let app = await WGPUApp({
        preinitializedWebGPUDevice: device,
    });

    try {
        app.callMain();
    } catch (e) {
        console.error(e.stack);
    }

    let loadingText = document.getElementById("loading-text");

    let loadGLTFBuffer = app.cwrap("load_gltf_buffer", null, ["number", "number"]);

    // Setup listener to upload new GLB files now that the app is running
    document.getElementById("uploadGLB").onchange = (evt: Event) =>
    {
        // When we get a new file we read it into an array buffer, then allocate room in the Wasm
        // memory and copy the array buffer in to pass it to the C++ code
        let picker = evt.target as HTMLInputElement;
        if (picker.files.length === 0) {
            return;
        }
        loadingText.hidden = false;

        let reader = new FileReader();
        reader.onerror = () => {throw Error(`Error reading file ${picker.files[0].name}`);};

        reader.onload = () =>
        {
            let start = performance.now();
            let buf = reader.result as ArrayBuffer;

            // Allocate room in the Wasm memory to write the glb buffer
            let ptr = app._malloc(buf.byteLength);

            app.HEAPU8.set(new Uint8Array(buf), ptr);

            loadGLTFBuffer(ptr, buf.byteLength);

            // Release the memory we allocated in the Wasm, it's no longer needed
            app._free(ptr);
            loadingText.hidden = true;

            let end = performance.now();
            console.log(`Import took ${end - start}ms`);
        };

        reader.readAsArrayBuffer(picker.files[0]);
    };
})();

