# NPP Noisy Image Smoothing

A C++ application that uses NVIDIA CUDA NPP and OpenCV to apply box or Gaussian filters on noisy grayscale images.

## Clone and initialize

Clone the repository **with** submodules in one step:

```bash
git clone --recurse-submodules https://github.com/yanhongzhong/CUDA-at-Scale-Independent-Project.git
cd CUDA-at-Scale-Independent-Project

If you already cloned without submodules:
```bash
git submodule update --init --recursive


## Features

- **Filters**: Box filter and Gaussian filter (window sizes 3, 5, 7, or 9).
- **Preprocessing**: Automatic conversion to 8-bit grayscale, with error handling.
- **Command-Line**: Choose filter type, window size, input/output paths, and display help.
- **Directory Structure**: 
  - **`src/`**: Source code (`noisy_smooth_npp.cpp`).
  - **`data/`**: Input images (default: `Noisy_Lenna_Gray.png`).   Outut images (default: `Smooth_Lenna_Gray.png`).
  - **`bin/`**: Build output (`noisy_smooth_npp` executable).

## Dependencies

- CUDA Toolkit with NPP (v12.x or later)
- OpenCV (4.x) with development headers
- C++17-compatible compiler
- `pkg-config` for OpenCV flags
- FreeImage

Check `INSTALL` to find the installation

## Build Instructions

```bash
# Ensure data directory has default image
cp /path/to/Noisy_Lenna_Gray.png data/

# Build
make

#Usage

bin/noisy_smooth_npp [options]

Options

-h, --help            Show help message.

-f, --filter TYPE     Filter type: box (default) or gauss.

-w, --window N        Window size (odd integer >1, default 5).

-i, --input FILE      Input image (default: data/Noisy_Lenna_Gray.png).

-o, --output FILE     Output image (default: data/Smooth_Lenna_Gray.png).

Example

bin/noisy_smooth_npp -f gauss -w 5 -i data/my_noisy.png -o data/my_smooth.png

