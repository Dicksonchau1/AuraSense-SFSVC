# AuraSense SFSVC SDK Documentation

## Overview

The AuraSense SFSVC SDK provides a Python interface for neuromorphic video codec and crack detection capabilities. This SDK is a reference implementation demonstrating the core concepts. For production deployments, see the C++ engine documentation.

## Installation

### From Source (Development)

```bash
# Clone the repository
git clone https://github.com/Dicksonchau1/AuraSense-SFSVC.git
cd AuraSense-SFSVC

# Install in editable mode
pip install -e .

# Install with optional dependencies
pip install -e ".[streamlit]"  # For Streamlit demo
pip install -e ".[dev]"         # For development tools
pip install -e ".[all]"         # Everything
```

### Using Docker

```bash
# Build the SDK image
docker build --target runtime-sdk -t aurasense-sfsvc:sdk .

# Run example
docker run --rm -v $(pwd)/demo.mp4:/app/demo.mp4 \
    aurasense-sfsvc:sdk \
    python examples/simple_neurocodec_python.py --input demo.mp4

# Build and run Streamlit app
docker build --target runtime-streamlit -t aurasense-sfsvc:streamlit .
docker run -p 8501:8501 aurasense-sfsvc:streamlit
```

### Using Docker Compose

```bash
# Run Streamlit app
docker-compose up streamlit

# Run SDK examples
docker-compose --profile sdk up sdk

# Development environment
docker-compose --profile dev up -d dev
docker-compose exec dev bash
```

## Quick Start

### Basic Usage

```python
from aurasense_sfsvc import VideoProcessor

# Initialize processor
processor = VideoProcessor(
    video_path="demo.mp4",
    spike_threshold=15.0,
    confidence_threshold=0.85
)

# Get video metadata
metadata = processor.get_video_metadata()
print(f"Video: {metadata['width']}x{metadata['height']} @ {metadata['fps']}fps")

# Process video
summary = processor.process_video(max_frames=100)
print(f"Processed {summary['total_frames']} frames")
print(f"Average latency: {summary['avg_processing_time_ms']:.2f}ms")
print(f"Compression: {summary['avg_compression_percent']:.1f}%")
```

### Using Individual Components

```python
from aurasense_sfsvc import SpikeCodec, CrackDetector
import cv2

# Initialize components
codec = SpikeCodec(spike_threshold=15.0)
detector = CrackDetector(confidence_threshold=0.85)

# Read a frame
frame = cv2.imread("frame.jpg")
h, w = frame.shape[:2]

# Detect cracks
cracks = detector.detect_cracks(w, h, frame_idx=0)
print(f"Found {len(cracks)} cracks")

# Generate spikes
spike_data = codec.generate_spikes(frame, cracks)
print(f"Generated {spike_data['spike_count']} spikes")
print(f"Compression: {spike_data['compression_percent']:.1f}%")
```

## Command-Line Tools

### Run Example Script

```bash
# Process video with default settings
python examples/simple_neurocodec_python.py --input demo.mp4

# Process with custom parameters
python examples/simple_neurocodec_python.py \
    --input demo.mp4 \
    --output output.mp4 \
    --spike-threshold 20.0 \
    --confidence 0.90 \
    --max-frames 500
```

### Run Benchmarks

```bash
# Run all benchmarks
python benchmarks/run_benchmark.py --input demo.mp4

# Run specific benchmark
python benchmarks/run_benchmark.py \
    --input demo.mp4 \
    --benchmark latency \
    --num-frames 100

# Throughput benchmark
python benchmarks/run_benchmark.py \
    --input demo.mp4 \
    --benchmark throughput \
    --duration 10
```

### Run Streamlit Demo

```bash
streamlit run streamlit_app.py
```

## API Reference

### VideoProcessor

High-level video processing pipeline.

```python
VideoProcessor(
    video_path: str,
    spike_threshold: float = 15.0,
    confidence_threshold: float = 0.85
)
```

**Methods:**
- `get_video_metadata()` → Dict: Get video properties (fps, dimensions, etc.)
- `process_frame(frame, frame_idx)` → Dict: Process single frame
- `process_video(output_path, frame_callback, max_frames)` → Dict: Process entire video
- `reset_stats()`: Reset processing statistics

### SpikeCodec

Neuromorphic spike generation.

```python
SpikeCodec(
    spike_threshold: float = 15.0,
    compression_ratio: float = 10.0
)
```

**Methods:**
- `generate_spikes(frame, crack_regions)` → Dict: Generate spikes from frame
- `compute_frame_difference(current_frame, prev_frame)` → np.ndarray: Compute frame diff
- `get_compression_stats(spike_data)` → Dict: Calculate compression metrics

### CrackDetector

Crack detection with severity classification.

```python
CrackDetector(
    confidence_threshold: float = 0.85
)
```

**Methods:**
- `detect_cracks(frame_width, frame_height, frame_idx)` → List[Dict]: Detect cracks
- `classify_severity(crack_width_mm, crack_length_mm)` → str: Classify severity
- `get_detection_stats()` → Dict: Get detection statistics
- `reset_history()`: Clear detection history

## Performance Targets

### Python Reference Implementation (This SDK)
- **Latency:** ~0.5-2ms per frame (unoptimized Python)
- **Throughput:** Varies by hardware, typically 100-500fps on modern CPUs
- **Compression:** 94-98% bandwidth reduction

### Production C++ Engine (Private SDK)
- **Latency:** <0.5ms P95, <0.8ms P99 (AVX2-optimized)
- **Throughput:** 125+ fps sustained on 720p video
- **Compression:** 94% bandwidth reduction (10× ratio)
- **Reliability:** 99.9% uptime, zero frame drops over 72hr

## Docker Commands

### Build Images

```bash
# SDK runtime
docker build --target runtime-sdk -t aurasense-sfsvc:sdk .

# Streamlit app
docker build --target runtime-streamlit -t aurasense-sfsvc:streamlit .

# Development environment
docker build --target development -t aurasense-sfsvc:dev .
```

### Run Containers

```bash
# Run benchmark
docker run --rm -v $(pwd)/demo.mp4:/app/demo.mp4 \
    aurasense-sfsvc:sdk \
    python benchmarks/run_benchmark.py --input demo.mp4

# Run example
docker run --rm -v $(pwd)/demo.mp4:/app/demo.mp4 \
    aurasense-sfsvc:sdk \
    python examples/simple_neurocodec_python.py --input demo.mp4

# Run Streamlit (interactive)
docker run -p 8501:8501 aurasense-sfsvc:streamlit

# Development shell
docker run -it --rm -v $(pwd):/app aurasense-sfsvc:dev
```

## Directory Structure

```
AuraSense-SFSVC/
├── aurasense_sfsvc/          # SDK package
│   ├── __init__.py           # Public API
│   ├── codec.py              # Spike codec
│   ├── detection.py          # Crack detection
│   └── processor.py          # Video processor
├── benchmarks/               # Benchmark utilities
│   ├── __init__.py
│   └── run_benchmark.py      # Benchmark script
├── examples/                 # Example scripts
│   └── simple_neurocodec_python.py
├── docs/                     # Documentation
├── streamlit_app.py          # Streamlit demo
├── setup.py                  # Package setup
├── pyproject.toml            # Package metadata
├── Dockerfile                # Docker build
├── docker-compose.yml        # Docker Compose config
├── requirements.txt          # Python dependencies
└── demo.mp4                  # Demo video

```

## Environment Variables

- `PYTHONUNBUFFERED=1`: Disable Python output buffering
- `PYTHONDONTWRITEBYTECODE=1`: Prevent .pyc file creation

## Troubleshooting

### Import Errors

```bash
# Ensure SDK is installed
pip install -e .

# Verify installation
python -c "import aurasense_sfsvc; print(aurasense_sfsvc.__version__)"
```

### Docker Build Issues

```bash
# Clean Docker cache
docker system prune -af

# Rebuild without cache
docker build --no-cache -t aurasense-sfsvc:sdk .
```

### Performance Issues

The Python reference implementation is not optimized for production. For production use:
1. Contact AuraSense HK for access to the C++ production engine
2. Use AVX2-optimized builds
3. Deploy on dedicated hardware with GPU acceleration

## Support

- **Website:** https://www.aurasensehk.com
- **Email:** DicksonChau@aurasensehk.com
- **GitHub:** https://github.com/Dicksonchau1/AuraSense-SFSVC

## License

Proprietary - Contact AuraSense HK for licensing information.
