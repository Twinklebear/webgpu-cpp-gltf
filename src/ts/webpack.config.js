const path = require("path");
const packageInfo = require("./package.json");
const CopyWebpackPlugin = require("copy-webpack-plugin");

const rules = [
  {
    test: /\.(png|jpg|jpeg)$/i,
    type: "asset/resource",
  },
  {
    test: /\.tsx?$/,
    use: "ts-loader",
    exclude: /node_modules/,
  },
  {
    test: /\.wasm$/,
    type: "asset/resource",
  }
];

const resolve = {
  extensions: [".tsx", ".ts", ".js"],
};

// Maybe I don't need the copy webpack at this step
// for the wasm file? I was just copying the dwarf
// file which I won't ship
const plugins = [
  new CopyWebpackPlugin({
    patterns: [{
      from: "./*.dwarf",
      to() {
        return "[name][ext]";
      },
      noErrorOnMissing: true
    }]
  })
]

const browser_config = {
  entry: "./index.ts",
  mode: "development",
  devtool: "inline-source-map",
  target: "web",
  output: {
    filename: "index.js",
    path: path.resolve(__dirname, "dist"),
    globalObject: "this",
    library: {
      name: "webgpu_cpp_gltf",
      type: "umd",
    },
    // We need to force this to / so the package can work
    // in Google Colab. Other Jupyter environments (VSCode,
    // Jupyter Lab) support public path
    publicPath: "/",
  },
  module: {
    rules: rules,
  },
  resolve: resolve,
  plugins: plugins,
};

module.exports = [browser_config];

