# HLS Downloader

This Python package provides a high-performance HLS downloader built with FFmpeg and Pybind11.

## Installation

pip install hls_downloader


## Usage

```python
import hls_downloader

hls_downloader.start_workers(4)
hls_downloader.add_task("http://example.com/stream1.m3u8", "output1.ts")
hls_downloader.stop_downloads()
```

## Development

Libraries:
sudo apt-get update
sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavdevice-dev libavfilter-dev
sudo apt-get install libspdlog-dev libfmt-dev

Python:
pip install setuptools
pip install pybind11
pip install setuptools wheel twine

chmod +x build.sh
./build.sh

python3 setup.py sdist bdist_wheel
