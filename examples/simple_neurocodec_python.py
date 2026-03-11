"""
Simple neuromorphic codec demonstration.

This example shows basic usage of the AuraSense SFSVC SDK for processing
video with neuromorphic spike generation and crack detection.
"""

import argparse
from pathlib import Path
import sys

# Add parent directory to path for local imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from aurasense_sfsvc import VideoProcessor


def main():
    parser = argparse.ArgumentParser(
        description="AuraSense SFSVC - Simple Neuromorphic Codec Demo"
    )
    parser.add_argument(
        "--input",
        type=str,
        required=True,
        help="Input video file path"
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="Output video file path (optional)"
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=None,
        help="Maximum frames to process (default: all)"
    )
    parser.add_argument(
        "--spike-threshold",
        type=float,
        default=15.0,
        help="Spike generation threshold (default: 15.0)"
    )
    parser.add_argument(
        "--confidence",
        type=float,
        default=0.85,
        help="Crack detection confidence threshold (default: 0.85)"
    )

    args = parser.parse_args()

    # Check input file exists
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"Error: Input file not found: {args.input}")
        return 1

    print(f"🎥 AuraSense SFSVC - Neuromorphic Codec Demo")
    print(f"=" * 60)
    print(f"Input: {args.input}")
    if args.output:
        print(f"Output: {args.output}")
    print(f"Spike threshold: {args.spike_threshold}")
    print(f"Confidence threshold: {args.confidence}")
    print(f"=" * 60)
    print()

    # Initialize processor
    processor = VideoProcessor(
        video_path=str(input_path),
        spike_threshold=args.spike_threshold,
        confidence_threshold=args.confidence
    )

    # Get video metadata
    metadata = processor.get_video_metadata()
    print(f"📊 Video Metadata:")
    print(f"  Resolution: {metadata['width']}x{metadata['height']}")
    print(f"  FPS: {metadata['fps']:.1f}")
    print(f"  Total frames: {metadata['total_frames']}")
    print(f"  Duration: {metadata['duration_seconds']:.2f}s")
    print()

    # Process video with progress callback
    def progress_callback(frame_idx, frame, results):
        if frame_idx % 30 == 0:  # Print every 30 frames
            n_cracks = len(results["cracks"])
            latency = results["processing_time_ms"]
            compression = results["spike_data"]["compression_percent"]
            print(f"Frame {frame_idx:5d}: {n_cracks} cracks, "
                  f"{latency:.2f}ms latency, {compression:.1f}% compression")

    print("🚀 Processing video...")
    summary = processor.process_video(
        output_path=args.output,
        frame_callback=progress_callback,
        max_frames=args.max_frames
    )

    # Print summary
    print()
    print(f"✅ Processing Complete!")
    print(f"=" * 60)
    print(f"📈 Performance Summary:")
    print(f"  Total frames: {summary['total_frames']}")
    print(f"  Avg latency: {summary['avg_processing_time_ms']:.2f}ms")
    print(f"  P95 latency: {summary['p95_latency_ms']:.2f}ms")
    print(f"  P99 latency: {summary['p99_latency_ms']:.2f}ms")
    print(f"  Total cracks detected: {summary['total_cracks']}")
    print(f"  Avg compression: {summary['avg_compression_percent']:.1f}%")
    print(f"=" * 60)

    if args.output:
        print(f"💾 Output saved to: {args.output}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
