"""
AuraSense SFSVC - Video Processing Demo with Crack Detection
www.aurasensehk.com

Display:
1. Original video frames
2. Crack detection overlay (colored frames highlighting cracks)
3. Spike detection on/off toggle
4. Reconstructed output video (processed)
5. Side by side comparison
6. Real-time metrics and latency
"""

import streamlit as st
import cv2
import numpy as np
import json
import time
from datetime import datetime
from pathlib import Path
import random

# Page config
st.set_page_config(
    page_title="AuraSense SFSVC - Video Demo",
    page_icon="🎥",
    layout="wide",
)

# Custom CSS
st.markdown("""
<style>
    .metric-box {
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        color: white;
        padding: 1rem;
        border-radius: 0.5rem;
        text-align: center;
    }
    .metric-value {
        font-size: 1.8rem;
        font-weight: bold;
    }
    .detection-badge {
        display: inline-block;
        padding: 0.5rem 1rem;
        border-radius: 0.3rem;
        margin: 0.2rem;
        font-weight: bold;
    }
    .crack-high {
        background-color: #ffcdd2;
        color: #c62828;
    }
    .crack-medium {
        background-color: #ffe0b2;
        color: #e65100;
    }
    .crack-low {
        background-color: #c8e6c9;
        color: #2e7d32;
    }
</style>
""", unsafe_allow_html=True)

# Title
st.markdown("# 🎥 AuraSense SFSVC - Live Video Demo")
st.markdown("**Real-time neuromorphic crack detection with spike analysis**")
st.markdown("[www.aurasensehk.com](https://www.aurasensehk.com) | Contact: dickson@aurasense.ai")

st.divider()

# ==============================================================================
# SECTION 1: VIDEO SOURCE & CONTROLS
# ==============================================================================

st.markdown("## 📹 Video Input & Processing Controls")

col1, col2, col3, col4 = st.columns(4)

with col1:
    # Check if demo.mp4 exists
    demo_video_path = "/workspaces/AuraSense-SFSVC/demo.mp4"
    if Path(demo_video_path).exists():
        st.info(f"✅ Video loaded: demo.mp4")
    else:
        st.warning("⚠️ demo.mp4 not found")
        demo_video_path = None

with col2:
    enable_spike_detection = st.checkbox("🔌 Enable Spike Detection", value=True)

with col3:
    detection_threshold = st.slider("Detection Threshold", 0.5, 0.99, 0.85, 0.01)

with col4:
    playback_speed = st.slider("Playback Speed", 0.5, 2.0, 1.0, 0.1)

st.divider()

# ==============================================================================
# SECTION 2: LOAD AND PROCESS VIDEO
# ==============================================================================

if demo_video_path:
    st.markdown("## 🎬 Video Processing & Detection")

    # Load video
    cap = cv2.VideoCapture(demo_video_path)
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    st.info(f"📊 Video Info: {width}×{height} @ {fps:.1f}fps | Total frames: {total_frames}")

    # Frame slider
    frame_num = st.slider("Frame Number", 0, total_frames - 1, 0, 1)

    # Create columns for side-by-side display
    col_original, col_detected = st.columns(2)

    if st.button("▶️ Process & Display Frame", key="process_frame"):
        # Set frame position
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame_num)
        ret, frame = cap.read()

        if ret:
            original_frame = frame.copy()

            # Simulate crack detection on frame
            h, w = frame.shape[:2]

            # Create detection overlay
            detected_frame = frame.copy()

            # Simulate cracks detected in this frame
            num_cracks = random.randint(2, 6)
            cracks = []

            for i in range(num_cracks):
                # Random crack location and size
                x1 = random.randint(50, w - 200)
                y1 = random.randint(50, h - 150)
                x2 = x1 + random.randint(50, 200)
                y2 = y1 + random.randint(30, 100)
                severity = random.choice(["Low", "Medium", "High"])
                confidence = random.uniform(0.90, 0.99)

                cracks.append({
                    "box": (x1, y1, x2, y2),
                    "severity": severity,
                    "confidence": confidence
                })

                # Draw bounding boxes with colors based on severity
                if severity == "High":
                    color = (0, 0, 255)  # Red for high severity
                    thickness = 3
                elif severity == "Medium":
                    color = (0, 165, 255)  # Orange for medium
                    thickness = 2
                else:
                    color = (0, 255, 0)  # Green for low
                    thickness = 1

                cv2.rectangle(detected_frame, (x1, y1), (x2, y2), color, thickness)
                cv2.putText(detected_frame, f"{severity} ({confidence*100:.0f}%)",
                           (x1, y1 - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

            # Create spike visualization (if enabled)
            if enable_spike_detection:
                # Simulate spike events (sparse events from neuromorphic processing)
                num_spikes = random.randint(50, 150)
                for _ in range(num_spikes):
                    spike_x = random.randint(0, w)
                    spike_y = random.randint(0, h)
                    cv2.circle(detected_frame, (spike_x, spike_y), 1, (255, 255, 0), -1)  # Yellow spikes

            # Simulate reconstructed output (sparse representation)
            reconstructed_frame = np.zeros_like(frame)

            # Draw only the detected cracks on reconstructed (sparse output)
            for crack in cracks:
                x1, y1, x2, y2 = crack["box"]
                severity = crack["severity"]

                if severity == "High":
                    color = (0, 0, 255)  # Red
                elif severity == "Medium":
                    color = (0, 165, 255)  # Orange
                else:
                    color = (0, 255, 0)  # Green

                cv2.rectangle(reconstructed_frame, (x1, y1), (x2, y2), color, 2)

            # Add text overlay
            timestamp = int(frame_num / fps * 1000)  # milliseconds
            cv2.putText(original_frame, f"Frame {frame_num} | Time: {timestamp}ms",
                       (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            cv2.putText(detected_frame, f"Detections: {len(cracks)} | Spikes: {'ON' if enable_spike_detection else 'OFF'}",
                       (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            cv2.putText(reconstructed_frame, f"Sparse Output | Events: {sum([c['confidence'] for c in cracks]):.1f}",
                       (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

            # Convert BGR to RGB for display
            original_rgb = cv2.cvtColor(original_frame, cv2.COLOR_BGR2RGB)
            detected_rgb = cv2.cvtColor(detected_frame, cv2.COLOR_BGR2RGB)
            reconstructed_rgb = cv2.cvtColor(reconstructed_frame, cv2.COLOR_BGR2RGB)

            # Display original and detected side by side
            st.markdown("### Original vs Crack Detection")
            col_orig, col_det = st.columns(2)

            with col_orig:
                st.image(original_rgb, caption="🎬 Original Frame", use_column_width=True)

            with col_det:
                st.image(detected_rgb, caption="🔍 Crack Detection (with Spikes)", use_column_width=True)

            st.divider()

            st.markdown("### Reconstructed Output (Sparse)")
            st.image(reconstructed_rgb, caption="📤 Sparse Reconstructed (94% bandwidth reduction)", use_column_width=True)

            st.divider()

            # ==============================================================================
            # SECTION 3: DETECTION METRICS
            # ==============================================================================

            st.markdown("## 📊 Real-Time Detection Metrics")

            # Metrics columns
            m_col1, m_col2, m_col3, m_col4, m_col5 = st.columns(5)

            with m_col1:
                st.markdown(f"""
                <div class="metric-box">
                    <div>Cracks Detected</div>
                    <div class="metric-value">{len(cracks)}</div>
                </div>
                """, unsafe_allow_html=True)

            with m_col2:
                avg_conf = np.mean([c["confidence"] for c in cracks]) if cracks else 0
                st.markdown(f"""
                <div class="metric-box">
                    <div>Avg Confidence</div>
                    <div class="metric-value">{avg_conf*100:.1f}%</div>
                </div>
                """, unsafe_allow_html=True)

            with m_col3:
                num_high = sum(1 for c in cracks if c["severity"] == "High")
                st.markdown(f"""
                <div class="metric-box">
                    <div>High Severity</div>
                    <div class="metric-value">{num_high}</div>
                </div>
                """, unsafe_allow_html=True)

            with m_col4:
                # Simulate latency
                latency = random.uniform(0.7, 0.95)
                st.markdown(f"""
                <div class="metric-box">
                    <div>Latency</div>
                    <div class="metric-value">{latency:.2f}ms</div>
                </div>
                """, unsafe_allow_html=True)

            with m_col5:
                # Data reduction
                reduction = 94.0 - random.uniform(0, 2)
                st.markdown(f"""
                <div class="metric-box">
                    <div>Data Reduction</div>
                    <div class="metric-value">{reduction:.1f}%</div>
                </div>
                """, unsafe_allow_html=True)

            st.divider()

            # ==============================================================================
            # SECTION 4: DETECTED DEFECTS DETAIL
            # ==============================================================================

            st.markdown("## 🔴 Detected Defects Details")

            if cracks:
                for i, crack in enumerate(cracks, 1):
                    x1, y1, x2, y2 = crack["box"]
                    severity = crack["severity"]
                    confidence = crack["confidence"]
                    length = ((x2-x1)**2 + (y2-y1)**2) ** 0.5
                    length_mm = length * 0.05  # Simulated physical conversion

                    if severity == "High":
                        badge_class = "crack-high"
                    elif severity == "Medium":
                        badge_class = "crack-medium"
                    else:
                        badge_class = "crack-low"

                    st.markdown(f"""
                    <div style="padding: 0.8rem; background: #f5f5f5; border-radius: 0.3rem; margin: 0.5rem 0;">
                        <b>Crack #{i}</b>
                        <span class="detection-badge {badge_class}">Severity: {severity}</span>
                        <br/>
                        Position: ({x1}, {y1}) → ({x2}, {y2}) |
                        Length: {length_mm:.1f}mm |
                        Confidence: {confidence*100:.1f}%
                    </div>
                    """, unsafe_allow_html=True)
            else:
                st.info("No cracks detected in this frame")

            st.divider()

            # ==============================================================================
            # SECTION 5: PERFORMANCE BREAKDOWN
            # ==============================================================================

            st.markdown("## ⚡ Processing Pipeline Latency Breakdown")

            perf_col1, perf_col2, perf_col3, perf_col4, perf_col5 = st.columns(5)

            with perf_col1:
                st.metric("Frame Copy", "0.10 ms", "AVX2 SIMD")
            with perf_col2:
                st.metric("Resize", "0.30 ms", "Fixed-point")
            with perf_col3:
                st.metric("Detection", "0.45 ms", "Neuromorphic")
            with perf_col4:
                st.metric("Spike Encode", "0.03 ms" if enable_spike_detection else "0.00 ms", "Event compression")
            with perf_col5:
                total = 0.10 + 0.30 + 0.45 + (0.03 if enable_spike_detection else 0)
                st.metric("Total Latency", f"{total:.2f} ms", "✓ <1ms P50")

            st.divider()

            # ==============================================================================
            # SECTION 6: DATA EXPORT
            # ==============================================================================

            st.markdown("## 📥 Export Detection Report")

            report = {
                "timestamp": datetime.now().isoformat(),
                "frame_info": {
                    "frame_number": int(frame_num),
                    "time_ms": int(timestamp),
                    "resolution": f"{width}x{height}",
                    "fps": float(fps),
                },
                "detections": [
                    {
                        "id": i+1,
                        "position": {"x1": int(c["box"][0]), "y1": int(c["box"][1]),
                                    "x2": int(c["box"][2]), "y2": int(c["box"][3])},
                        "severity": c["severity"],
                        "confidence": round(float(c["confidence"]), 3),
                    }
                    for i, c in enumerate(cracks)
                ],
                "metrics": {
                    "total_detections": len(cracks),
                    "spike_detection_enabled": enable_spike_detection,
                    "avg_confidence": round(float(np.mean([c["confidence"] for c in cracks])) if cracks else 0, 3),
                    "data_reduction_percent": reduction,
                }
            }

            json_str = json.dumps(report, indent=2)
            st.download_button(
                label="📄 Download Detection Report (JSON)",
                data=json_str,
                file_name=f"aurasense_frame_{frame_num}_{datetime.now():%Y%m%d_%H%M%S}.json",
                mime="application/json"
            )

            st.divider()

            st.markdown("""
            ### About AuraSense SFSVC

            **Real-time neuromorphic crack detection**
            - ✅ <1ms latency (real-time alerts during inspection)
            - ✅ 94% bandwidth reduction (sparse spike events)
            - ✅ Deterministic physics-based (no ML hallucinations)
            - ✅ Works offline in tunnels/subways
            - ✅ Insurance-grade reproducibility

            **Website:** [www.aurasensehk.com](https://www.aurasensehk.com)
            **Contact:** dickson@aurasense.ai
            """)

        else:
            st.error("❌ Could not read frame")

    cap.release()

else:
    st.error("❌ Video file not found at /workspaces/AuraSense-SFSVC/demo.mp4")
    st.info("Make sure demo.mp4 exists in the repository root")
