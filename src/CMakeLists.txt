embed_files(embedded_files ./gltf.wgsl ./DamagedHelmet.glb)

add_executable(
  wgpu_app
  main.cpp
  arcball_camera.cpp
  tiny_gltf.cpp
  gltf_buffer_view.cpp
  gltf_accessor.cpp
  gltf_primitive.cpp
  gltf_mesh.cpp
  gltf_node.cpp
  flatten_gltf.cpp
  import_gltf.cpp)

set_target_properties(wgpu_app PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED
                                                          ON)

target_link_libraries(wgpu_app PRIVATE glm embedded_files)

# target_compile_options(wgpu_app PRIVATE -pthread)

target_link_options(
  wgpu_app
  PRIVATE
  "SHELL: -sENVIRONMENT='web,worker'"
  # Not using threads so this can be deployed in GH pages where we can't set the
  # right headers to enable shader array buffer -pthread -sPTHREAD_POOL_SIZE=1
  -sEXPORT_ES6
  -sEXPORT_NAME=WGPUApp
  -sUSE_WEBGPU=1
  -sMODULARIZE=1
  -sEXPORT_ES6
  -sEXPORT_NAME=CPPApp
  -sALLOW_MEMORY_GROWTH=1
  -sASSERTIONS=1
  -sEXIT_RUNTIME=1
  -sINVOKE_RUN=0
  # Debug build flags
  $<$<CONFIG:DEBUG>:-fwasm-exceptions>
  $<$<CONFIG:DEBUG>:-g>
  $<$<CONFIG:DEBUG>:-O0>
  # Exported Emscripten runtime methods
  "SHELL: -sEXPORTED_RUNTIME_METHODS='[\"callMain\", \"cwrap\"]'"
  "SHELL: -sEXPORTED_FUNCTIONS='[\"_malloc\", \"_free\", \"_main\"]'")

option(ENABLE_SOURCE_MAP
       "Enable source map for debug builds (breaks Chrome DWARF debugging)" OFF)
if(ENABLE_SOURCE_MAP)
  target_link_options(wgpu_app PRIVATE $<$<CONFIG:DEBUG>:-gsource-map>)
endif()

option(ENABLE_MEMORY_PROFILER "Enable Emscripten's memory profiler" OFF)
if(ENABLE_MEMORY_PROFILER)
  target_link_options(wgpu_app PRIVATE --memoryprofiler)
endif()

option(ENABLE_CPU_PROFILER "Enable Emscripten's CPU profiler" OFF)
if(ENABLE_CPU_PROFILER)
  target_link_options(wgpu_app PRIVATE --cpuprofiler)
endif()

option(ENABLE_THREAD_PROFILER "Enable Emscripten's thread profiler" OFF)
if(ENABLE_THREAD_PROFILER)
  target_link_options(wgpu_app PRIVATE --threadprofiler)
endif()

if(ENABLE_MEMORY_PROFILER
   OR ENABLE_CPU_PROFILER
   OR ENABLE_THREAD_PROFILER)
  target_compile_options(wgpu_app PRIVATE $<$<CONFIG:RELEASE>:--profiling>)
  target_link_options(wgpu_app PRIVATE $<$<CONFIG:RELEASE>:--profiling>)
endif()

set(WEB_OUT_DIR ${PROJECT_SOURCE_DIR}/web/src/cpp)

set(COPY_SRC_MAP $<AND:$<BOOL:${ENABLE_SOURCE_MAP}>,$<CONFIG:DEBUG>>)

# The package.json is used as a CMake configure file to populate the library
# version from CMake
configure_file(${CMAKE_CURRENT_LIST_DIR}/ts/package.json
               ${CMAKE_CURRENT_BINARY_DIR}/package.json @ONLY)

set(NPM_PACKAGE_TS_DIR "${CMAKE_CURRENT_LIST_DIR}/ts/")
set(NPM_PACKAGE_TS_SRC
    ${NPM_PACKAGE_TS_DIR}/index.ts ${CMAKE_CURRENT_BINARY_DIR}/package.json
    ${NPM_PACKAGE_TS_DIR}/tsconfig.json ${NPM_PACKAGE_TS_DIR}/webpack.config.js
    ${NPM_PACKAGE_TS_DIR}/wasm.d.ts)

set(NPM_PACKAGE_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/npm_package/")
set(NPM_PACKAGE_BUILD_SRC
    ${NPM_PACKAGE_BUILD_DIR}/index.ts
    ${NPM_PACKAGE_BUILD_DIR}/package.json
    ${NPM_PACKAGE_BUILD_DIR}/tsconfig.json
    ${NPM_PACKAGE_BUILD_DIR}/webpack.config.js
    ${NPM_PACKAGE_BUILD_DIR}/wasm.d.ts
    ${NPM_PACKAGE_BUILD_DIR}/wgpu_app.wasm
    ${NPM_PACKAGE_BUILD_DIR}/wgpu_app.js)
# Command to copy the TS files and wasm to make the npm package build dir
add_custom_command(
  DEPENDS wgpu_app ${NPM_PACKAGE_TS_SRC}
  OUTPUT ${NPM_PACKAGE_BUILD_SRC}
  COMMAND cmake -E make_directory ${NPM_PACKAGE_BUILD_DIR}
  COMMAND cmake -E copy_if_different ${NPM_PACKAGE_TS_SRC}
          ${NPM_PACKAGE_BUILD_DIR}
  COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/wgpu_app.js
          ${CMAKE_CURRENT_BINARY_DIR}/wgpu_app.wasm ${NPM_PACKAGE_BUILD_DIR}
  COMMAND_EXPAND_LISTS)

# Command to build the npm package
add_custom_command(
  DEPENDS ${NPM_PACKAGE_BUILD_SRC}
  OUTPUT ${NPM_PACKAGE_BUILD_DIR}/dist/index.js
  WORKING_DIRECTORY ${NPM_PACKAGE_BUILD_DIR}
  COMMAND npm i
  COMMAND npm run build
  COMMAND cmake -E copy_if_different ${NPM_PACKAGE_BUILD_DIR}/package.json
          ${NPM_PACKAGE_BUILD_DIR}/dist/)
add_custom_target(
  npm_build ALL
  DEPENDS ${NPM_PACKAGE_BUILD_DIR}/dist/index.js
  COMMENT "npm i && npm run build")

# Custom command to run npm pack
add_custom_target(
  npm_pack
  DEPENDS npm_build
  WORKING_DIRECTORY ${NPM_PACKAGE_BUILD_DIR}/dist/
  COMMAND npm pack
  COMMENT "npm pack")

# Custom command to run npm publish
add_custom_target(
  npm_publish
  DEPENDS npm_build
  WORKING_DIRECTORY ${NPM_PACKAGE_BUILD_DIR}/dist/
  COMMAND npm publish
  COMMENT "npm publish")
