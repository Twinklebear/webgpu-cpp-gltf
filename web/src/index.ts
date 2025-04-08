import { loadApp } from "@twinklebear/webgpu_cpp_gltf"

(async () => {
    const canvas = document.getElementById("webgpu-canvas")
    if (navigator.gpu === undefined) {
        canvas.setAttribute("style", "display:none;");
        document.getElementById("no-webgpu").setAttribute("style", "display:block;");
        return;
    }

    // Block right click so we can use right click + drag to pan
    canvas.addEventListener("contextmenu", (evt) => {
        evt.preventDefault();
    });

    const app = await loadApp();

    try {
        app.callMain("#webgpu-canvas");
    } catch (e) {
        console.error(e.stack);
    }

    const loadingText = document.getElementById("loading-text");

    // Setup listener to upload new GLB files now that the app is running
    document.getElementById("uploadGLB").onchange = (evt: Event) => {
        // When we get a new file we read it into an array buffer, then allocate room in the Wasm
        // memory and copy the array buffer in to pass it to the C++ code
        const picker = evt.target as HTMLInputElement;
        if (picker.files.length === 0) {
            return;
        }
        loadingText.hidden = false;

        const reader = new FileReader();
        reader.onerror = () => { throw Error(`Error reading file ${picker.files[0].name}`); };

        reader.onload = () => {
            const start = performance.now();
            const buf = new Uint8Array(reader.result as ArrayBuffer);

            app.loadGLTFBuffer(buf);

            loadingText.hidden = true;

            const end = performance.now();
            console.log(`Import took ${end - start}ms`);
        };

        reader.readAsArrayBuffer(picker.files[0]);
    };
})();

