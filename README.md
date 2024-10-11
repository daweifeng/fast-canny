# Fast Canny

Fast and furious Canny edge detection.

## Pre-requisites

- Cmake >= 3.22
- GCC >= 8.5.0

## Build

```bash

cmake -B build
cmake --build build

```
NOTE: When building the whole project, especifally if you are building the opencv benchmark, CMake will download the opencv source code and build it. This will take a while.

## Run

### Running OpenCV benchmark

To run the OpenCV benchmark, you will first need to download the images by running the python script `download_coco_images.py`. This script will download the images from the COCO dataset and save them in the `coco_images` folder.

For local development, you should set up and run the Python virtual environment. When running the benchmark on the ECE machines, we don't need to create the virtual environment. We can run the script directly:

```bash

pipenv shell # Only for local development
python download_coco_images.py
```
