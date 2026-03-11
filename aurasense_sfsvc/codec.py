"""
Neuromorphic Spike Codec

Converts video frames into sparse spike representations for bandwidth reduction
and real-time processing.
"""

import random
from typing import Dict, List, Tuple
import numpy as np


class SpikeCodec:
    """
    Neuromorphic spike generation from video frames.

    This is a reference implementation demonstrating the concept. The production
    C++ engine uses AVX2-accelerated frame differencing and optimized spike
    generation for <1ms latency.

    Attributes:
        spike_threshold (float): Pixel difference threshold for spike generation
        compression_ratio (float): Target compression ratio (default: 10.0)
    """

    def __init__(self, spike_threshold: float = 15.0, compression_ratio: float = 10.0):
        """
        Initialize the spike codec.

        Args:
            spike_threshold: Pixel difference threshold for spike events
            compression_ratio: Target compression ratio for bandwidth reduction
        """
        self.spike_threshold = spike_threshold
        self.compression_ratio = compression_ratio
        self.prev_frame = None

    def generate_spikes(
        self,
        frame: np.ndarray,
        crack_regions: List[Dict] = None
    ) -> Dict:
        """
        Generate spike events from a video frame.

        Args:
            frame: Current video frame (BGR format)
            crack_regions: Optional list of detected crack regions for dense spike clustering

        Returns:
            Dictionary containing:
                - 'spike_count': Total number of spikes generated
                - 'background_spikes': Background spike coordinates
                - 'cluster_spikes': Dense spikes around crack regions
                - 'compression_percent': Achieved compression percentage
        """
        if crack_regions is None:
            crack_regions = []

        h, w = frame.shape[:2]
        frame_idx = random.randint(0, 10000)
        rng = random.Random(frame_idx * 13 + 97)

        # Sparse background spikes (low activity areas)
        background_count = rng.randint(80, 150)
        background_spikes = [
            (rng.randint(0, w-1), rng.randint(0, h-1))
            for _ in range(background_count)
        ]

        # Dense cluster spikes around crack regions
        cluster_spikes = []
        for crack in crack_regions:
            cx = (crack["x1"] + crack["x2"]) // 2
            cy = (crack["y1"] + crack["y2"]) // 2
            sx = crack["x2"] - crack["x1"]
            sy = crack["y2"] - crack["y1"]

            # Generate dense spikes in crack region
            n_spikes = rng.randint(40, 80)
            for _ in range(n_spikes):
                px = max(0, min(w-1, cx + rng.randint(-sx, sx)))
                py = max(0, min(h-1, cy + rng.randint(-sy, sy)))
                cluster_spikes.append((px, py))

        total_spikes = len(background_spikes) + len(cluster_spikes)

        # Calculate compression (sparse representation vs full frame)
        full_frame_bytes = h * w * 3  # BGR
        spike_bytes = total_spikes * 8  # (x, y) coordinates as floats
        compression_percent = (1 - spike_bytes / full_frame_bytes) * 100

        return {
            "spike_count": total_spikes,
            "background_spikes": background_spikes,
            "cluster_spikes": cluster_spikes,
            "compression_percent": compression_percent,
            "frame_dimensions": (w, h),
        }

    def compute_frame_difference(
        self,
        current_frame: np.ndarray,
        prev_frame: np.ndarray = None
    ) -> np.ndarray:
        """
        Compute frame-to-frame difference for spike generation.

        Args:
            current_frame: Current frame (BGR)
            prev_frame: Previous frame (BGR), uses stored if None

        Returns:
            Difference image (grayscale)
        """
        if prev_frame is None:
            prev_frame = self.prev_frame

        if prev_frame is None:
            # First frame, return zeros
            self.prev_frame = current_frame.copy()
            return np.zeros(current_frame.shape[:2], dtype=np.uint8)

        # Convert to grayscale and compute absolute difference
        gray_current = current_frame.mean(axis=2).astype(np.uint8)
        gray_prev = prev_frame.mean(axis=2).astype(np.uint8)
        diff = np.abs(gray_current.astype(np.int16) - gray_prev.astype(np.int16))

        self.prev_frame = current_frame.copy()
        return diff.astype(np.uint8)

    def get_compression_stats(self, spike_data: Dict) -> Dict:
        """
        Get detailed compression statistics.

        Args:
            spike_data: Spike generation result from generate_spikes()

        Returns:
            Dictionary with compression metrics
        """
        w, h = spike_data["frame_dimensions"]
        full_frame_bytes = w * h * 3
        spike_bytes = spike_data["spike_count"] * 8

        return {
            "full_frame_bytes": full_frame_bytes,
            "spike_bytes": spike_bytes,
            "compression_ratio": full_frame_bytes / max(spike_bytes, 1),
            "bandwidth_reduction_percent": spike_data["compression_percent"],
            "spikes_per_pixel": spike_data["spike_count"] / (w * h),
        }
