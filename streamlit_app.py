"""
AuraSense SFSVC - Live Video Stream with Real-Time Crack Detection
www.aurasensehk.com

Continuous video playback with live crack detection overlay.
"""

import streamlit as st
import numpy as np
import random
from pathlib import Path

# Page config
st.set_page_config(
    page_title="AuraSense SFSVC - Live Demo",
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
st.markdown("# 🎥 AuraSense SFSVC - Live Video Detection")
st.markdown("**Real-time neuromorphic crack detection**")
st.markdown("[www.aurasensehk.com](https://www.aurasensehk.com) | Contact: dickson@aurasense.ai")

st.divider()

# ==============================================================================
# SECTION 1: VIDEO PLAYBACK
# ==============================================================================

# Find the demo video
possible_paths = [
    Path(__file__).parent / "demo.mp4",
    Path("demo.mp4"),
]

demo_video_path = None
for path in possible_paths:
    if path.exists():
        demo_video_path = path
        break

if demo_video_path:
    st.markdown("## 📺 Inspection Video Stream")

    # Use st.video() for native browser playback (works reliably)
    video_file = open(demo_video_path, "rb")
    video_bytes = video_file.read()
    video_file.close()
    st.video(video_bytes, format="video/mp4")

    st.divider()

    # ==============================================================================
    # SECTION 2: SIMULATED REAL-TIME DETECTION METRICS
    # ==============================================================================

    st.markdown("## 📊 Detection Metrics Dashboard")
    st.caption("Simulated detection results — production system processes spike events in real time.")

    col_conf, col_spike = st.columns(2)
    with col_conf:
        confidence_threshold = st.slider("Confidence Threshold", 0.50, 0.99, 0.85, 0.01)
    with col_spike:
        enable_spike_detection = st.checkbox("🔌 Enable Spike Detection", value=True)

    # Generate simulated detection results (seeded per session for consistency)
    if "detection_seed" not in st.session_state:
        st.session_state.detection_seed = random.randint(0, 100000)

    rng = random.Random(st.session_state.detection_seed)

    if st.button("🔄 Re-run Detection Simulation"):
        st.session_state.detection_seed = random.randint(0, 100000)
        st.rerun()

    # Simulate detections across frames
    num_cracks = rng.randint(2, 6)
    cracks = []
    for _ in range(num_cracks):
        severity = rng.choice(["Low", "Medium", "High"])
        confidence = rng.uniform(0.80, 0.99)
        length_mm = rng.uniform(1.5, 25.0)
        if confidence >= confidence_threshold:
            cracks.append({
                "severity": severity,
                "confidence": confidence,
                "length_mm": length_mm,
            })

    # Metrics row
    m1, m2, m3, m4, m5 = st.columns(5)

    with m1:
        st.markdown(f"""
        <div class="metric-box">
            <div>Cracks Detected</div>
            <div class="metric-value">{len(cracks)}</div>
        </div>
        """, unsafe_allow_html=True)

    with m2:
        avg_conf = np.mean([c["confidence"] for c in cracks]) if cracks else 0
        st.markdown(f"""
        <div class="metric-box">
            <div>Avg Confidence</div>
            <div class="metric-value">{avg_conf*100:.1f}%</div>
        </div>
        """, unsafe_allow_html=True)

    with m3:
        num_high = sum(1 for c in cracks if c["severity"] == "High")
        st.markdown(f"""
        <div class="metric-box">
            <div>High Severity</div>
            <div class="metric-value">{num_high}</div>
        </div>
        """, unsafe_allow_html=True)

    with m4:
        latency = rng.uniform(0.70, 0.95)
        st.markdown(f"""
        <div class="metric-box">
            <div>Latency</div>
            <div class="metric-value">{latency:.2f}ms</div>
        </div>
        """, unsafe_allow_html=True)

    with m5:
        reduction = 94.0 - rng.uniform(0, 2)
        st.markdown(f"""
        <div class="metric-box">
            <div>Data Reduction</div>
            <div class="metric-value">{reduction:.1f}%</div>
        </div>
        """, unsafe_allow_html=True)

    # Detected defects list
    if cracks:
        st.markdown("### 🔴 Detected Defects")
        for i, crack in enumerate(cracks, 1):
            severity = crack["severity"]
            confidence = crack["confidence"]
            length = crack["length_mm"]

            badge_class = {"High": "crack-high", "Medium": "crack-medium", "Low": "crack-low"}[severity]

            st.markdown(f"""
            <div style="padding: 0.5rem; background: #f5f5f5; border-radius: 0.3rem; margin: 0.3rem 0; font-size: 0.9rem;">
                <b>Crack #{i}</b>
                <span class="detection-badge {badge_class}">{severity}</span>
                | Length: {length:.1f}mm | Confidence: {confidence*100:.1f}%
            </div>
            """, unsafe_allow_html=True)

        if enable_spike_detection:
            spike_count = rng.randint(120, 400)
            st.info(f"⚡ Spike detection active — {spike_count} spike events processed this frame")
    else:
        st.success("✅ No cracks detected above confidence threshold")

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
    st.error("❌ demo.mp4 not found")
    st.info("Place `demo.mp4` in the repository root alongside `streamlit_app.py`.")
