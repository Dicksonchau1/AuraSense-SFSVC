"""
AuraSense SFSVC SDK - Neuromorphic Codec for Drone Inspection

A neuromorphic video codec layer for drone inspections that converts frame
changes into sparse spike representations with real-time crack detection.

Public API:
    - SpikeCodec: Neuromorphic spike generation from video frames
    - CrackDetector: Simulated crack detection with severity classification
    - VideoProcessor: High-level video processing pipeline
"""

__version__ = "0.1.0"
__author__ = "AuraSense HK"
__email__ = "DicksonChau@aurasensehk.com"

from .codec import SpikeCodec
from .detection import CrackDetector
from .processor import VideoProcessor

__all__ = [
    "SpikeCodec",
    "CrackDetector",
    "VideoProcessor",
    "__version__",
]
