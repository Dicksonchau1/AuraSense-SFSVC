# 🚀 AuraSense SFSVC Pilot Onboarding Guide

**Duration:** 2 weeks | **Setup Time:** 15 minutes | **Support:** DicksonChau@aurasensehk.com

---

## 📋 Pre-Deployment Checklist

- [ ] **Hardware validated** - Run `pilot_benchmark.py --validate` ✓
- [ ] **Linux system** - Ubuntu 20.04+ or CentOS 8+
- [ ] **AVX2 support** - From validation report
- [ ] **2GB+ RAM** - For SFSVC processing lanes
- [ ] **50GB disk space** - For videos and logs

---

## 🔧 Installation (15 minutes)

### Step 1: Download SFSVC Package

```bash
# Option A: From GitHub
git clone https://github.com/aurasense/AuraSense-SFSVC.git
cd AuraSense-SFSVC

# Option B: From release
curl -L https://releases.aurasense.ai/sfsvc-v1.0.tar.gz | tar xz
cd sfsvc-v1.0
```

### Step 2: Build & Compile

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y cmake build-essential libopencv-dev

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Install to system
sudo make install
# ✓ Libraries installed to /usr/local/lib/
# ✓ Headers installed to /usr/local/include/
```

### Step 3: Verify Installation

```bash
# Test library
ldconfig -p | grep aurasense
# Output: libaurasense_rt_core.so.1 => /usr/local/lib/libaurasense_rt_core.so.1

# Run unit tests
./test_delta_spike_avx2
# ✓ All tests should pass
```

---

## 🔌 Integration with Existing Pipeline

### Scenario A: DJI Drones + ROS

If using ROS with DJI SDK:

```cpp
#include "aurasense/rt_core.h"

class DroneMonitor : public rclcpp::Node {
  private:
    aurasense::MultiRateEngine engine_;

  public:
    DroneMonitor() : rclcpp::Node("aurasense_monitor") {
        engine_.start(6); // 6 processing lanes

        // Subscribe to camera feed
        camera_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw",
            rclcpp::SensorDataQoS(),
            [this](const sensor_msgs::msg::Image::SharedPtr msg) {
                this->on_frame(msg);
            }
        );
    }

    void on_frame(const sensor_msgs::msg::Image::SharedPtr msg) {
        // Push frame to SFSVC
        engine_.push_frame(msg->data.data(), msg->height, msg->width);
    }
};
```

### Scenario B: Standalone Python Application

```python
import ctypes
import numpy as np

# Load SFSVC library
lib = ctypes.CDLL("libaurasense_rt_core.so")

# Define function signatures
lib.create_engine.argtypes = [ctypes.c_int]
lib.create_engine.restype = ctypes.c_void_p

lib.push_frame.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong, ctypes.c_int, ctypes.c_int, ctypes.c_int]

# Initialize engine (6 lanes)
engine = lib.create_engine(6)

# Process video
import cv2
cap = cv2.VideoCapture("drone_footage.mp4")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Convert to BGR if needed
    frame_bgr = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

    # Push to SFSVC engine
    lib.push_frame(
        engine,
        frame_bgr.ctypes.data_as(ctypes.POINTER(ctypes.c_ubyte)),
        frame.shape[0],  # height
        frame.shape[1],  # width
        0  # frame_id
    )
```

### Scenario C: Docker Container Deployment

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake build-essential libopencv-dev

COPY . /aurasense

WORKDIR /aurasense/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make -j4 && make install

ENTRYPOINT ["/aurasense/scripts/customer_demo.py"]
```

Deploy with Docker Compose:

```yaml
version: '3.8'
services:
  aurasense:
    build: .
    volumes:
      - ./videos:/data/videos
      - ./reports:/data/reports
    environment:
      - VIDEO_PATH=/data/videos/drone.mp4
      - FLEET_SIZE=30
```

---

## 📊 Configuration

Edit `/etc/aurasense/sfsvc.conf`:

```ini
# Core settings
[processing]
lanes = 6                    # Number of parallel processing lanes
frame_buffer_size = 3       # Frames queued before drop
mode = real_time            # real_time | batch

# Optimization
[optimization]
enable_avx2 = true          # Use AVX2 SIMD (auto-detect)
enable_simd_resize = true   # Custom SIMD bilinear resize
enable_prefetch = true      # L1 cache prefetch hints

# Performance tuning (for bare-metal PREEMPT_RT)
[tuning]
rt_priority = 50            # SCHED_FIFO priority (0-99)
cpu_affinity = "0,1,2,3"   # Pin to CPU cores
nice_level = -20            # CPU scheduling priority

# Monitoring
[monitoring]
enable_metrics = true
metrics_port = 9090
log_level = INFO
log_file = /var/log/aurasense/sfsvc.log
```

---

## 🧪 Validation Tests (Run Before Production)

### Test 1: Functionality Test (5 min)

```bash
# Run with sample video
python3 scripts/customer_demo.py --video ~/sample_drone.mp4

# ✅ Should show:
# - Video resolution & frame count
# - Bandwidth savings calculation
# - JSON export
```

### Test 2: Performance Test (10 min)

```bash
# Run benchmark for 1000 frames
./build/aurasense_benchmark ~/videos/test_1000fps.mp4

# ✅ Expected:
# - Throughput: > 100 fps
# - P95 latency: < 0.6ms (Codespaces) / < 0.3ms (bare-metal)
# - Zero frame drops
# - Stable crack detection
```

### Test 3: Reliability Test (30 min)

```bash
# Run sustained load test
for i in {1..10}; do
    echo "Run $i/10"
    ./build/aurasense_benchmark ~/videos/test_drone.mp4 --json results_run_$i.json
done

# Analyze results
python3 - << 'EOF'
import json, statistics
latencies = []
for i in range(1, 11):
    with open(f"results_run_{i}.json") as f:
        data = json.load(f)
        latencies.append(data["p95_latency_ms"])
print(f"P95 variance: {statistics.stdev(latencies):.2f}ms")
print(f"Mean P95: {statistics.mean(latencies):.2f}ms")
EOF

# ✅ Should show < 5% variance
```

### Test 4: Integration Test (Depends on your system)

```bash
# Test with your actual drone feed (H.265/H.264 input)
# Verify:
# 1. Frames are detected and processed
# 2. Crack detections are accurate
# 3. Output is sent upstream correctly
# 4. No frame drops under sustained load
```

---

## 📈 Monitoring During Pilot

### Check System Health

```bash
# Monitor CPU usage
top -p $(pgrep -f aurasense)

# Monitor memory
free -h

# Check for frame drops
tail -f /var/log/aurasense/sfsvc.log | grep -i "drop"

# Monitor network bandwidth
iftop -n

# Real-time metrics
curl http://localhost:9090/metrics | grep aurasense
```

### Expected Metrics During Pilot

| Metric | Expected | Alert If |
|--------|----------|----------|
| **CPU Usage** | 30-50% per core | > 80% continuous |
| **Memory** | 1-2GB per lane | > 3GB |
| **P95 Latency** | 0.3-0.6ms | > 1ms |
| **Throughput** | 100+ fps | < 90 fps |
| **Frame Drop Rate** | 0% | > 0.1% |
| **Crack Detection Match** | 98%+ vs golden | < 95% |

---

## 🆘 Troubleshooting

### Problem: "AVX2 not supported"
**Solution:** Your CPU doesn't support AVX2. SFSVC requires modern Intel/AMD CPUs (2013+).
```bash
grep avx2 /proc/cpuinfo
# If empty, hardware is incompatible
```

### Problem: "Frame drops detected"
**Solution:** Increase number of processing lanes or reduce video resolution.
```bash
# In config: lanes = 8  (up to 12)
systemctl restart aurasense
```

### Problem: "High latency spikes"
**Solution:** Enable PREEMPT_RT kernel (production systems).
```bash
# On bare-metal systems, uses PREEMPT_RT to reduce jitter
# P95 jitter should drop 10-50x
```

### Problem: "Library not found"
**Solution:** Library installation failed or not in system path.
```bash
# Verify installation
ls -la /usr/local/lib/libaurasense*

# If missing, reinstall:
cd build
sudo make install
sudo ldconfig
```

### Problem: "Segfault on startup"
**Solution:** Likely incompatible OpenCV version or missing SIMD features.
```bash
# Check OpenCV version
pkg-config --modversion opencv4

# Rebuild with verbose output
cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..
make
```

---

## 📞 Support During Pilot

**Slack Channel:** #aurasense-pilots
**Email:** DicksonChau@aurasensehk.com
**Response Time:** Within 2 hours (business hours)

Common support topics:
- Performance tuning
- Integration help
- Troubleshooting crashes
- Accuracy validation

---

## 🎯 Success Criteria for Pilot

✅ **Functional**
- Processes 100% of incoming frames without drops
- Detects cracks with >95% accuracy vs manual labeling
- Latency stable (P95 within 10% of baseline)

✅ **Performance**
- Bandwidth reduced by 90%+ vs H.265
- CPU usage <50% on target hardware
- Monthly savings calculated and verified

✅ **Integration**
- Works with existing camera feed
- Output feeds correctly upstream
- Monitoring dashboard shows live metrics

✅ **Reliability**
- 48-72hr continuous run without crashes
- Automatic recovery from errors
- 99%+ uptime during pilot

---

## 📋 Post-Pilot Deployment Checklist

Once pilot is successful:

- [ ] Update firewall rules for remote monitoring
- [ ] Configure PREEMPT_RT kernel (if bare-metal)
- [ ] Set up CI/CD for continuous deployment
- [ ] Train operations team on dashboards
- [ ] Document custom integrations
- [ ] Schedule fleet rollout (5% → 20% → 100%)

---

## 💡 Tips for Success

1. **Start Small:** Pilot on 1-2 drones before full fleet
2. **Monitor Continuously:** Daily health checks during pilot
3. **Document Issues:** Screenshot errors and latency spikes
4. **Keep Baselines:** Before/after bandwidth and CPU usage
5. **Involve Your Team:** Get operations team familiar with tools
6. **Plan Rollout:** Gradual deployment to catch issues early

---

## 🏁 Next Steps

1. Follow installation steps above
2. Run `pilot_benchmark.py --validate` to confirm compatibility
3. Run validation tests (Test 1-3 above)
4. Contact DicksonChau@aurasensehk.com once ready for integration
5. We'll help with custom integration if needed
6. Start 2-week pilot

---

**Ready to begin? Email DicksonChau@aurasensehk.com with:**
- Hardware specs (CPU model, RAM, OS)
- Current H.265 bitrate and format
- Fleet size
- Integration preference (Docker / ROS / Custom)

**Expected pilot timeline:**
- Day 1: Installation & validation
- Days 2-7: Integration & testing
- Days 8-14: Performance monitoring
- Day 15: Handoff to operations
- Week 3+: Fleet rollout
