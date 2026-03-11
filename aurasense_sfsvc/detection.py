"""
Crack Detection Module

Simulated crack detection with severity classification and confidence scoring.
"""

import random
from typing import Dict, List, Tuple


class CrackDetector:
    """
    Crack detection with severity classification.

    This is a simulated implementation for demonstration. The production engine
    uses computer vision (edge detection, contour analysis) and optional ML models
    (YOLO, custom CNN) for real crack detection.

    Attributes:
        confidence_threshold (float): Minimum confidence for detection (0.0-1.0)
        severity_levels (List[str]): Available severity classifications
    """

    SEVERITY_LEVELS = ["Low", "Medium", "High"]
    SEVERITY_COLORS = {
        "High": (0, 0, 255),    # Red (BGR)
        "Medium": (0, 165, 255),  # Orange
        "Low": (0, 255, 0),     # Green
    }

    def __init__(self, confidence_threshold: float = 0.85):
        """
        Initialize the crack detector.

        Args:
            confidence_threshold: Minimum confidence for crack detection (0.0-1.0)
        """
        self.confidence_threshold = confidence_threshold
        self.detection_history = []

    def detect_cracks(
        self,
        frame_width: int,
        frame_height: int,
        frame_idx: int = 0
    ) -> List[Dict]:
        """
        Detect cracks in a frame region.

        Args:
            frame_width: Frame width in pixels
            frame_height: Frame height in pixels
            frame_idx: Frame index for deterministic simulation

        Returns:
            List of crack dictionaries with keys:
                - x1, y1, x2, y2: Bounding box coordinates
                - severity: Classification (Low/Medium/High)
                - confidence: Detection confidence (0.0-1.0)
                - width_mm: Estimated crack width in mm
                - length_mm: Estimated crack length in mm
        """
        rng = random.Random(frame_idx * 7 + 31)
        cracks = []

        # Simulate 0-5 cracks per frame
        num_cracks = rng.randint(0, 5)

        for _ in range(num_cracks):
            # Random bounding box
            x1 = rng.randint(50, max(51, frame_width - 220))
            y1 = rng.randint(50, max(51, frame_height - 170))
            box_width = rng.randint(60, 200)
            box_height = rng.randint(30, 100)
            x2 = x1 + box_width
            y2 = y1 + box_height

            # Random severity and confidence
            severity = rng.choice(self.SEVERITY_LEVELS)
            confidence = rng.uniform(0.80, 0.99)

            # Only include if above threshold
            if confidence >= self.confidence_threshold:
                # Estimate crack dimensions (simplified)
                width_mm = rng.uniform(0.5, 5.0)  # 0.5-5mm width
                length_mm = ((box_width**2 + box_height**2)**0.5) * 0.05  # pixels to mm

                crack = {
                    "x1": x1,
                    "y1": y1,
                    "x2": x2,
                    "y2": y2,
                    "severity": severity,
                    "confidence": confidence,
                    "width_mm": width_mm,
                    "length_mm": length_mm,
                }
                cracks.append(crack)

        self.detection_history.append({
            "frame_idx": frame_idx,
            "num_cracks": len(cracks),
            "severities": [c["severity"] for c in cracks],
        })

        return cracks

    def classify_severity(
        self,
        crack_width_mm: float,
        crack_length_mm: float
    ) -> str:
        """
        Classify crack severity based on dimensions.

        Args:
            crack_width_mm: Crack width in millimeters
            crack_length_mm: Crack length in millimeters

        Returns:
            Severity level: "Low", "Medium", or "High"
        """
        # Simple rule-based classification
        if crack_width_mm > 3.0 or crack_length_mm > 100:
            return "High"
        elif crack_width_mm > 1.5 or crack_length_mm > 50:
            return "Medium"
        else:
            return "Low"

    def get_detection_stats(self) -> Dict:
        """
        Get statistics from detection history.

        Returns:
            Dictionary with detection metrics:
                - total_frames: Number of frames processed
                - total_cracks: Total cracks detected
                - avg_cracks_per_frame: Average detections per frame
                - severity_counts: Count by severity level
        """
        if not self.detection_history:
            return {
                "total_frames": 0,
                "total_cracks": 0,
                "avg_cracks_per_frame": 0.0,
                "severity_counts": {"Low": 0, "Medium": 0, "High": 0},
            }

        total_frames = len(self.detection_history)
        total_cracks = sum(h["num_cracks"] for h in self.detection_history)

        severity_counts = {"Low": 0, "Medium": 0, "High": 0}
        for history in self.detection_history:
            for severity in history["severities"]:
                severity_counts[severity] += 1

        return {
            "total_frames": total_frames,
            "total_cracks": total_cracks,
            "avg_cracks_per_frame": total_cracks / total_frames,
            "severity_counts": severity_counts,
        }

    def reset_history(self):
        """Clear detection history."""
        self.detection_history = []
