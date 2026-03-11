"""
Video Processor

High-level video processing pipeline combining codec and detection.
"""

import cv2
import numpy as np
from pathlib import Path
from typing import Optional, Callable, Dict
import time

from .codec import SpikeCodec
from .detection import CrackDetector


class VideoProcessor:
    """
    High-level video processing pipeline for neuromorphic crack detection.

    Combines spike codec and crack detection into a unified processing pipeline.

    Attributes:
        codec: SpikeCodec instance
        detector: CrackDetector instance
        video_path: Path to input video file
    """

    def __init__(
        self,
        video_path: str,
        spike_threshold: float = 15.0,
        confidence_threshold: float = 0.85
    ):
        """
        Initialize the video processor.

        Args:
            video_path: Path to input video file
            spike_threshold: Threshold for spike generation
            confidence_threshold: Minimum confidence for crack detection
        """
        self.video_path = Path(video_path)
        self.codec = SpikeCodec(spike_threshold=spike_threshold)
        self.detector = CrackDetector(confidence_threshold=confidence_threshold)
        self.processing_stats = {
            "total_frames": 0,
            "processing_times": [],
            "total_cracks": 0,
        }

    def get_video_metadata(self) -> Dict:
        """
        Get video metadata.

        Returns:
            Dictionary with video properties:
                - fps: Frames per second
                - total_frames: Total frame count
                - width: Frame width
                - height: Frame height
                - duration_seconds: Video duration
        """
        cap = cv2.VideoCapture(str(self.video_path))
        metadata = {
            "fps": cap.get(cv2.CAP_PROP_FPS) or 30.0,
            "total_frames": int(cap.get(cv2.CAP_PROP_FRAME_COUNT)),
            "width": int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)),
            "height": int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)),
        }
        metadata["duration_seconds"] = metadata["total_frames"] / metadata["fps"]
        cap.release()
        return metadata

    def process_frame(self, frame: np.ndarray, frame_idx: int = 0) -> Dict:
        """
        Process a single frame through the complete pipeline.

        Args:
            frame: Input frame (BGR format)
            frame_idx: Frame index for tracking

        Returns:
            Dictionary containing:
                - cracks: List of detected crack dictionaries
                - spike_data: Spike generation results
                - processing_time_ms: Frame processing time
        """
        start_time = time.perf_counter()

        h, w = frame.shape[:2]

        # Detect cracks
        cracks = self.detector.detect_cracks(w, h, frame_idx)

        # Generate spikes (with crack regions for dense clustering)
        spike_data = self.codec.generate_spikes(frame, cracks)

        processing_time_ms = (time.perf_counter() - start_time) * 1000

        # Update stats
        self.processing_stats["total_frames"] += 1
        self.processing_stats["processing_times"].append(processing_time_ms)
        self.processing_stats["total_cracks"] += len(cracks)

        return {
            "cracks": cracks,
            "spike_data": spike_data,
            "processing_time_ms": processing_time_ms,
        }

    def process_video(
        self,
        output_path: Optional[str] = None,
        frame_callback: Optional[Callable] = None,
        max_frames: Optional[int] = None
    ) -> Dict:
        """
        Process entire video through the pipeline.

        Args:
            output_path: Optional path to save processed video
            frame_callback: Optional callback(frame_idx, frame, results) called per frame
            max_frames: Maximum frames to process (None = all frames)

        Returns:
            Dictionary with processing summary:
                - total_frames: Frames processed
                - avg_processing_time_ms: Average per-frame latency
                - p95_latency_ms: 95th percentile latency
                - p99_latency_ms: 99th percentile latency
                - total_cracks: Total cracks detected
                - avg_compression_percent: Average bandwidth reduction
        """
        cap = cv2.VideoCapture(str(self.video_path))
        metadata = self.get_video_metadata()

        writer = None
        if output_path:
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            writer = cv2.VideoWriter(
                output_path,
                fourcc,
                metadata["fps"],
                (metadata["width"], metadata["height"])
            )

        frame_idx = 0
        compression_percents = []

        try:
            while True:
                ret, frame = cap.read()
                if not ret:
                    break

                if max_frames and frame_idx >= max_frames:
                    break

                # Process frame
                results = self.process_frame(frame, frame_idx)
                compression_percents.append(results["spike_data"]["compression_percent"])

                # Optional callback
                if frame_callback:
                    frame_callback(frame_idx, frame, results)

                # Write to output if requested
                if writer:
                    writer.write(frame)

                frame_idx += 1

        finally:
            cap.release()
            if writer:
                writer.release()

        # Calculate summary statistics
        times = self.processing_stats["processing_times"]
        times_sorted = sorted(times)
        n = len(times_sorted)

        summary = {
            "total_frames": self.processing_stats["total_frames"],
            "avg_processing_time_ms": np.mean(times) if times else 0,
            "p95_latency_ms": times_sorted[int(n * 0.95)] if n > 0 else 0,
            "p99_latency_ms": times_sorted[int(n * 0.99)] if n > 0 else 0,
            "total_cracks": self.processing_stats["total_cracks"],
            "avg_compression_percent": np.mean(compression_percents) if compression_percents else 0,
        }

        return summary

    def reset_stats(self):
        """Reset processing statistics."""
        self.processing_stats = {
            "total_frames": 0,
            "processing_times": [],
            "total_cracks": 0,
        }
        self.codec.prev_frame = None
        self.detector.reset_history()
