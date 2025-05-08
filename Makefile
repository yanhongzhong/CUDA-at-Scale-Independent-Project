# Directories
SRCDIR := src
BINDIR := bin
DATADIR := data

# Files
SRC := $(SRCDIR)/noisy_image_smooth_npp.cpp
TARGET := noisy_image_smooth_npp

# Compiler
NVCC := nvcc

# NPP includes
EXTERNDIR := extern/cuda-samples/Common/UtilNPP
INCLUDES := -I$(EXTERNDIR)

# OpenCV flags
OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4)
OPENCV_LIBS   := $(shell pkg-config --libs opencv4)

# CUDA/NPP libs
CUDA_LIBS := -lnppig -lnppif -lnppidei -lnppisu -lnppicc -lcudart -lfreeimage

# Build all
all: $(BINDIR)/$(TARGET)

# Compile target
$(BINDIR)/$(TARGET): $(SRC)
	@mkdir -p $(BINDIR)
	$(NVCC) -std=c++17 $(INCLUDES) $(OPENCV_CFLAGS) $< -o $@ \
		$(OPENCV_LIBS) $(CUDA_LIBS)

# Clean artifacts
clean:
	rm -rf $(BINDIR)/$(TARGET)