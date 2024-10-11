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

To run the OpenCV benchmark, you will need to unzip the images from the COCO dataset.

```bash
unzip coco_images.zip
```
### Running the image download script

The images in the zip file are downloaded by running the python script `download_coco_images.py`. This script will download the images from the COCO dataset and save them in the `coco_images` folder.

For local python development, you should set up and run the Python virtual environment. It will set up a Python 3.12 virtualenv. When running the benchmark on the ECE machines, we don't need to redownload the images.

```bash
 # Only for local development
pipenv shell
python download_coco_images.py
```
