# AuraSense SFSVC – Neuromorphic Codec for Drone Inspection

AuraSense SFSVC is a **neuromorphic video codec layer** for drone inspections that sits between the camera, your existing models, and the uplink – running on standard CPU/GPU hardware – to deliver **real‑time crack‑aware control** with a fraction of the bandwidth.

Instead of streaming raw video to the cloud, SFSVC converts frame changes into **sparse spike representations**, applies a **real‑time control loop**, and only transmits what matters for safety, analytics, and audit.

This repository contains a **simple Python neuromorphic codec demo** and reference benchmark structure that demonstrate the core concept on real inspection video.


---

## Why this matters for drone inspection

Modern inspection drones hit three hard limits:

1. **Latency** – Cloud or heavy on‑board models can't always make decisions fast enough for safety‑critical, low‑altitude flight near concrete and infrastructure.
2. **Bandwidth & storage** – Raw or lightly compressed video is expensive to stream and store at scale, especially for long inspection routes.
3. **Battery & compute** – Throwing more GPU power at the problem doesn't help if every extra watt steals flight time.

SFSVC tackles all three by using a **neuromorphic codec as middleware**, not as a replacement for your stack:

- Runs on **CPU or GPU‑grade edge hardware** you already deploy
- Plugs into existing **camera → edge box → cloud** pipelines
- Reduces the load and cost of your current models and infrastructure instead of competing with them

---

## Neuromorphic codecs: where we are today

Neuromorphic codecs are where **LLMs were a few years ago**: exciting papers, early hardware prototypes, but very few **practical, plug‑and‑play layers** you can drop into a real product without building everything from scratch.

Existing tools are either:

- Tied to **specific neuromorphic hardware** (event cameras, Loihi chips)
- **Research‑grade frameworks** requiring significant engineering to integrate
- Sensor‑centric SDKs, not general middleware codecs

What's missing is a **hardware‑agnostic neuromorphic codec layer** that takes in normal video frames, outputs compressed spike representations with control‑relevant features, and runs on commodity hardware.

This repo is meant to be that **first accessible step** – a simple, understandable neuromorphic codec you can experiment with, plus real numbers from a production‑grade engine.

---

## Key benchmark results (from production engine)

Using AuraSense's **C++‑accelerated SFSVC engine** on a 720p runway inspection video, we measured:

### Real‑time control (Lane 1, C++)

- **2.50 ms** average control latency
- **2.61 ms P95**, **2.81 ms P99**
- **0% SLA violations** against a **6 ms** safety contract
- Effective control rate ≈ **106 Hz**

### Neuromorphic compression (codec layer)

- **9.97× compression**
- **89.97% bandwidth reduction**
- ~**681 GB/day** storage savings (modeled on typical inspection routes)
- ≈ **US$116,000/year** saved per **1,000 km** of infrastructure inspected

### Perception & behavior

- Every frame scored for **crack severity** (width, severity class, confidence)
- System chooses between **FORWARD / CAUTION / SLOW** actions, not just bounding boxes
- Signature and YOLO lanes run in the background, enriching semantics **without touching the hard real‑time loop**

> **Note:** These numbers come from AuraSense's **production C++ engine**, which is part of a private SDK used in pilot programs. This public repo provides a **simple Python neuromorphic codec** and benchmark structure for concept exploration; it is not the full commercial engine.

---

## What's in this repository

This public repo is a **reference implementation** and learning resource, not the full production SDK.

### Python SDK Package (`aurasense_sfsvc/`)

A modular Python SDK providing:

- **SpikeCodec:** Neuromorphic spike generation from video frames
- **CrackDetector:** Crack detection with severity classification
- **VideoProcessor:** High-level video processing pipeline
- **Benchmarks:** Performance measurement tools
- **Docker:** Containerized deployment

**Quick Start:**

```bash
# Install SDK
pip install -e .

# Run example
python examples/simple_neurocodec_python.py --input demo.mp4

# Run benchmarks
python benchmarks/run_benchmark.py --input demo.mp4

# Or use Docker
docker-compose up streamlit
```

### `streamlit_app.py`

Interactive demo application with:

- Live video playback with frame controls
- Real-time crack detection visualization
- Spike overlay rendering
- Performance metrics dashboard

**Usage:**

```bash
streamlit run streamlit_app.py
```

---

## SDK Documentation

For complete SDK documentation, see [SDK_USAGE.md](SDK_USAGE.md).

### Quick API Example

```python
from aurasense_sfsvc import VideoProcessor

# Initialize processor
processor = VideoProcessor(
    video_path="demo.mp4",
    spike_threshold=15.0,
    confidence_threshold=0.85
)

# Process video
summary = processor.process_video(max_frames=100)
print(f"Avg latency: {summary['avg_processing_time_ms']:.2f}ms")
print(f"Compression: {summary['avg_compression_percent']:.1f}%")
```

### Docker Usage

```bash
# Build and run Streamlit app
docker-compose up streamlit

# Run SDK benchmarks
docker build --target runtime-sdk -t aurasense-sfsvc:sdk .
docker run --rm -v $(pwd)/demo.mp4:/app/demo.mp4 \
    aurasense-sfsvc:sdk \
    python benchmarks/run_benchmark.py --input demo.mp4
```

---

## Project Structure

```
AuraSense-SFSVC/
├── aurasense_sfsvc/          # SDK package
│   ├── codec.py              # Spike codec
│   ├── detection.py          # Crack detection
│   └── processor.py          # Video processor
├── benchmarks/               # Benchmarking tools
│   └── run_benchmark.py      # Performance tests
├── examples/                 # Example scripts
│   └── simple_neurocodec_python.py
├── docs/                     # Documentation
├── streamlit_app.py          # Interactive demo
├── setup.py                  # Package setup
├── pyproject.toml            # Package metadata
├── Dockerfile                # Docker configuration
├── docker-compose.yml        # Docker Compose
└── demo.mp4                  # Demo video (720p runway)
```

---

## Performance Comparison

| Metric | Python SDK (This Repo) | C++ Production Engine |
|--------|------------------------|----------------------|
| Latency (P95) | ~0.5-2ms | <0.5ms |
| Latency (P99) | ~2-5ms | <0.8ms |
| Throughput | 100-500fps (varies) | 125+ fps sustained |
| Compression | 94-98% | 94% |
| Platform | Python 3.8+ | AVX2-optimized C++ |

---

## Links & Resources

- **Website:** [www.aurasensehk.com](https://www.aurasensehk.com)
- **SDK Documentation:** [SDK_USAGE.md](SDK_USAGE.md)
- **Technical Datasheet:** [docs/SFSVC_DATASHEET.md](docs/SFSVC_DATASHEET.md)
- **Deployment Guide:** [docs/DEPLOYMENT_CHECKLIST.md](docs/DEPLOYMENT_CHECKLIST.md)
- **Pilot Onboarding:** [docs/PILOT_ONBOARDING.md](docs/PILOT_ONBOARDING.md)

---

## Contact

For production SDK access, pilot programs, or commercial licensing:

- **Email:** DicksonChau@aurasensehk.com
- **Website:** https://www.aurasensehk.com

---

## License

Proprietary - Contact AuraSense HK for licensing information.
