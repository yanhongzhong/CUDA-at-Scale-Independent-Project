
#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cuda_runtime.h>
#include <npp.h>
#include <nppi.h>
#include <ImageIO.h>
#include <ImagesCPU.h>
#include <ImagesNPP.h>

#define CUDA_CHECK(call) do { \
    cudaError_t err = call; \
    if (err != cudaSuccess) { \
        std::cerr << "CUDA error: " << cudaGetErrorString(err) \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return EXIT_FAILURE; \
    } \
} while (0)

#define NPP_CHECK(call) do { \
    NppStatus s = call; \
    if (s != NPP_SUCCESS) { \
        std::cerr << "NPP error: " << s \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return EXIT_FAILURE; \
    } \
} while (0)

void printHelp(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  -h, --help          Show this help message\n"
              << "  -f, --filter TYPE   Filter type: box or gauss (default: box)\n"
              << "  -w, --window N      Window size (odd integer, default: 5)\n"
              << "  -i, --input FILE    Input image path (default: Noisy_Lenna_Gray.png)\n"
              << "  -o, --output FILE   Output image path (default: Smooth_Lenna_Gray.png)\n";
}

int main(int argc, char* argv[]) {
    std::string filterType = "box";
    int window = 5;
    std::string inputFile  = "Noisy_Lenna_Gray.png";
    std::string outputFile = "Smooth_Lenna_Gray.png";

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return EXIT_SUCCESS;
        }
        else if ((arg == "-f" || arg == "--filter") && i + 1 < argc) {
            filterType = argv[++i];
        }
        else if ((arg == "-w" || arg == "--window") && i + 1 < argc) {
            window = std::stoi(argv[++i]);
        }
        else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            inputFile = argv[++i];
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            printHelp(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Validate window size
    if (window <= 1 || window % 2 == 0) {
        std::cerr << "Error: Window size must be an odd integer > 1\n";
        return EXIT_FAILURE;
    }

    // Load input image
    cv::Mat img = cv::imread(inputFile, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "Error: Could not open input image '" << inputFile << "'\n";
        return EXIT_FAILURE;
    }

    // Convert to grayscale 8-bit
    cv::Mat gray;
    if (img.channels() > 1) {
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = img;
    }

    cv::Mat img8u;
    if (gray.depth() != CV_8U) {
        gray.convertTo(img8u, CV_8U);
    } else {
        img8u = gray;
    }

    // Ensure data is contiguous
    if (!img8u.isContinuous()) {
        img8u = img8u.clone();
    }

    int width  = img8u.cols;
    int height = img8u.rows;

    // Prepare host NPP image
    npp::ImageCPU_8u_C1 hostSrc(width, height);
    for (int y = 0; y < height; ++y) {
        std::memcpy(
            hostSrc.data() + y * hostSrc.pitch(),
            img8u.ptr<uint8_t>(y),
            width * sizeof(uint8_t)
        );
    }

    // Upload to device
    npp::ImageNPP_8u_C1 devSrc(hostSrc);
    npp::ImageNPP_8u_C1 devDst(width, height);

    NppiSize oSizeROI  = { width, height };
    NppiSize oMaskSize = { window, window };
    NppiPoint oAnchor  = { window / 2, window / 2 };

    // Apply selected filter
    if (filterType == "box") {
        NPP_CHECK(nppiFilterBoxBorder_8u_C1R(
            devSrc.data(), devSrc.pitch(), oSizeROI, {0,0},
            devDst.data(), devDst.pitch(), oSizeROI,
            oMaskSize, oAnchor, NPP_BORDER_REPLICATE
        ));
    }
    else if (filterType == "gauss") {
        NppiMaskSize eMask;
        switch (window) {
            case 3:  eMask = NPP_MASK_SIZE_3_X_3;  break;
            case 5:  eMask = NPP_MASK_SIZE_5_X_5;  break;
            case 7:  eMask = NPP_MASK_SIZE_7_X_7;  break;
            case 9:  eMask = NPP_MASK_SIZE_9_X_9;  break;
            default:
                std::cerr << "Error: Gaussian filter only supports window 3,5,7,9\n";
                return EXIT_FAILURE;
        }
        NPP_CHECK(nppiFilterGaussBorder_8u_C1R(
            devSrc.data(), devSrc.pitch(), oSizeROI, {0,0},
            devDst.data(), devDst.pitch(), oSizeROI,
            eMask, NPP_BORDER_REPLICATE
        ));
    }
    else {
        std::cerr << "Error: Unsupported filter type '" << filterType << "'\n";
        return EXIT_FAILURE;
    }

    // Download result
    npp::ImageCPU_8u_C1 hostDst(width, height);
    devDst.copyTo(hostDst.data(), hostDst.pitch());

    // Save via OpenCV
    cv::Mat outMat(
        height, width, CV_8UC1,
        hostDst.data(), hostDst.pitch()
    );
    if (!cv::imwrite(outputFile, outMat)) {
        std::cerr << "Error: Failed to save '" << outputFile << "'\n";
        return EXIT_FAILURE;
    }

    std::cout << "Output saved to " << outputFile << "\n";
    return EXIT_SUCCESS;
}
