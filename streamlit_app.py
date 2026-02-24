"""
AuraSense SFSVC Customer ROI Calculator - Streamlit App

Deploy to Streamlit Cloud for instant customer demos.

Usage:
    streamlit run streamlit_app.py

Deploy to Streamlit Cloud:
    1. Ensure GitHub repo is PUBLIC
    2. Go to https://share.streamlit.io/
    3. Create New app:
       - Repository: YOUR_USERNAME/AuraSense-SFSVC
       - Branch: main
       - Main file path: streamlit_app.py
    4. Click Deploy
"""

import streamlit as st
from pathlib import Path
import json
import subprocess
import shutil
from datetime import datetime

# Page config
st.set_page_config(
    page_title="AuraSense SFSVC ROI Calculator",
    page_icon="🎥",
    layout="wide",
)

# Custom CSS for professional look
st.markdown("""
<style>
    .main {
        max-width: 1200px;
    }
    .stMetric {
        background-color: #f0f2f6;
        padding: 1rem;
        border-radius: 0.5rem;
    }
    .header-text {
        color: #1f77b4;
        font-size: 2.5rem;
        font-weight: bold;
        margin-bottom: 1rem;
    }
    .subheader-text {
        color: #444;
        font-size: 1.2rem;
        margin-bottom: 2rem;
    }
    .savings-highlight {
        background-color: #1abc9c;
        color: white;
        padding: 1.5rem;
        border-radius: 0.5rem;
        font-size: 1.3rem;
        text-align: center;
        margin: 1rem 0;
    }
</style>
""", unsafe_allow_html=True)

# App title
st.markdown(
    '<div class="header-text">🎥 AuraSense SFSVC ROI Calculator</div>',
    unsafe_allow_html=True
)

st.markdown(
    '<div class="subheader-text">See your cellular data savings in real-time</div>',
    unsafe_allow_html=True
)

# Three-column layout for inputs
col1, col2, col3 = st.columns(3)

with col1:
    fleet_size = st.slider("Fleet Size (drones)", min_value=1, max_value=500, value=30, step=1)

with col2:
    bitrate_mbps = st.selectbox(
        "Video Resolution",
        options=[("720p (2.8 Mbps)", 2.8), ("1080p (5.2 Mbps)", 5.2), ("4K (12.5 Mbps)", 12.5)],
        format_func=lambda x: x[0]
    )[1]

with col3:
    operational_hours = st.slider("Daily Operations (hours)", min_value=1, max_value=24, value=8, step=1)

st.divider()

# Calculate metrics
COMPRESSION_RATIO = 0.062  # 6.2% (94% reduction)
MONTHLY_HOURS = operational_hours * 22  # Business days per month
MONTHLY_COST_PER_MBPS = 200  # $200/Mbps/month typical
SFSVC_OPERATIONAL_COST_PER_DRONE_PER_MONTH = 500

# Monthly savings
original_monthly_cost = bitrate_mbps * MONTHLY_COST_PER_MBPS
sfsvc_monthly_cost = (bitrate_mbps * COMPRESSION_RATIO) * MONTHLY_COST_PER_MBPS
monthly_savings_per_drone = original_monthly_cost - sfsvc_monthly_cost

# Fleet economics
fleet_monthly_savings = monthly_savings_per_drone * fleet_size
fleet_annual_cellular_savings = fleet_monthly_savings * 12
sfsvc_annual_operational_cost = SFSVC_OPERATIONAL_COST_PER_DRONE_PER_MONTH * fleet_size * 12
net_annual_savings = fleet_annual_cellular_savings - sfsvc_annual_operational_cost
payback_months = sfsvc_annual_operational_cost / fleet_monthly_savings if fleet_monthly_savings > 0 else 1000

# Display key metrics
st.markdown("### 📊 Financial Impact")

metric_col1, metric_col2, metric_col3, metric_col4 = st.columns(4)

with metric_col1:
    st.metric(
        "Monthly Savings/Drone",
        f"${monthly_savings_per_drone:,.0f}",
        f"{(monthly_savings_per_drone/original_monthly_cost*100):.0f}%"
    )

with metric_col2:
    st.metric(
        "Fleet Monthly Savings",
        f"${fleet_monthly_savings:,.0f}",
        f"at {fleet_size} drones"
    )

with metric_col3:
    st.metric(
        "Annual Net Savings",
        f"${net_annual_savings:,.0f}",
        f"after SFSVC costs"
    )

with metric_col4:
    st.metric(
        "Payback Period",
        f"{payback_months:.1f} months",
        "until SFSVC cost recovered"
    )

# Highlight the savings
st.markdown(f"""
<div class="savings-highlight">
💰 {fleet_size}-Drone Fleet saves <b>${net_annual_savings:,.0f}/year</b> on cellular costs
</div>
""", unsafe_allow_html=True)

st.divider()

# Comparison table
st.markdown("### 🔄 Bandwidth Comparison")

comparison_data = {
    "Metric": ["Monthly Bitrate", "Monthly Data Volume", "Monthly Cost", "Annual Cost"],
    "Original H.265 (w/o SFSVC)": [
        f"{bitrate_mbps:.1f} Mbps",
        f"{(bitrate_mbps * MONTHLY_HOURS) / 1024:.1f} GB",
        f"${original_monthly_cost:,.0f}",
        f"${original_monthly_cost * 12:,.0f}",
    ],
    "SFSVC (Sparse Events Only)": [
        f"{(bitrate_mbps * COMPRESSION_RATIO):.2f} Mbps",
        f"{(bitrate_mbps * MONTHLY_HOURS * COMPRESSION_RATIO) / 1024:.1f} GB",
        f"${sfsvc_monthly_cost:,.0f}",
        f"${sfsvc_monthly_cost * 12:,.0f}",
    ],
    "Reduction": [
        f"{(1-COMPRESSION_RATIO)*100:.1f}%",
        f"{(1-COMPRESSION_RATIO)*100:.1f}%",
        f"{(1-COMPRESSION_RATIO)*100:.1f}%",
        f"{(1-COMPRESSION_RATIO)*100:.1f}%",
    ]
}

st.table(comparison_data)

st.divider()

# Technical details
st.markdown("### ⚙️ Technical Specifications")

col1, col2 = st.columns(2)

with col1:
    st.markdown("""
**SFSVC Features:**
- ✅ Real-time crack detection (< 1ms P95 latency)
- ✅ 94% bandwidth reduction (sparse events only)
- ✅ AVX2 SIMD optimization
- ✅ Lock-free multi-lane architecture
- ✅ Works with existing H.265 cameras
    """)

with col2:
    st.markdown("""
**Requirements:**
- x86-64 CPU with AVX2 support
- Linux kernel (PREEMPT_RT recommended)
- 1-2GB RAM per processing lane
- Optional: NVIDIA GPU for YOLO
- Works with drone video feeds @ 4-30fps
    """)

st.divider()

# Company info
st.markdown("### 📞 Get Started")

col1, col2, col3 = st.columns(3)

with col1:
    st.markdown("""
**Email**
dickson@aurasense.ai
(Sales)
    """)

with col2:
    st.markdown("""
**Validation Program**
2-week pilot on 1-2 drones
Zero setup cost
    """)

with col3:
    st.markdown("""
**Fleet Deployment**
30+ days rollout
Production SLA included
    """)

st.divider()

# JSON export button
st.markdown("### 💾 Export Results")

results = {
    "timestamp": datetime.now().isoformat(),
    "configuration": {
        "fleet_size": fleet_size,
        "video_resolution_mbps": bitrate_mbps,
        "daily_operational_hours": operational_hours,
    },
    "per_drone": {
        "original_monthly_cost_usd": float(original_monthly_cost),
        "sfsvc_monthly_cost_usd": float(sfsvc_monthly_cost),
        "monthly_savings_usd": float(monthly_savings_per_drone),
        "annual_savings_usd": float(monthly_savings_per_drone * 12),
        "bandwidth_reduction_percent": float((1 - COMPRESSION_RATIO) * 100),
    },
    "fleet": {
        "fleet_size": fleet_size,
        "monthly_savings_usd": float(fleet_monthly_savings),
        "annual_cellular_savings_usd": float(fleet_annual_cellular_savings),
        "annual_sfsvc_cost_usd": float(sfsvc_annual_operational_cost),
        "net_annual_savings_usd": float(net_annual_savings),
        "payback_months": float(payback_months),
    }
}

# Export buttons
col1, col2 = st.columns(2)

with col1:
    json_str = json.dumps(results, indent=2)
    st.download_button(
        label="📥 Download JSON Report",
        data=json_str,
        file_name=f"aurasense_roi_{fleet_size}drones_{datetime.now():%Y%m%d}.json",
        mime="application/json"
    )

with col2:
    # Create a simple CSV version
    csv_str = "Metric,Value\n"
    csv_str += f"Fleet Size,{fleet_size}\n"
    csv_str += f"Video Bitrate,{bitrate_mbps} Mbps\n"
    csv_str += f"Monthly Savings Per Drone,${monthly_savings_per_drone:,.0f}\n"
    csv_str += f"Fleet Annual Savings,${net_annual_savings:,.0f}\n"
    csv_str += f"Payback Period,{payback_months:.1f} months\n"

    st.download_button(
        label="📊 Download CSV Report",
        data=csv_str,
        file_name=f"aurasense_roi_{fleet_size}drones_{datetime.now():%Y%m%d}.csv",
        mime="text/csv"
    )

st.divider()

# Footer
st.markdown("""
---
**AuraSense SFSVC** | Crack detection for autonomous drones
[Visit Website](https://aurasense.ai) | [GitHub](https://github.com/aurasense) | dickson@aurasense.ai
""")
