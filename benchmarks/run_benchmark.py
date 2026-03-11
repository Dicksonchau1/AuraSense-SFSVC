"""
Benchmark script for AuraSense SFSVC SDK.

Measures latency, throughput, and compression performance.
"""

import argparse
import sys
import time
from pathlib import Path
import numpy as np

# Add parent directory to path for local imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from aurasense_sfsvc import VideoProcessor, SpikeCodec, CrackDetector


def benchmark_latency(video_path: str, num_frames: int = 100) -> dict:
    """
    Benchmark per-frame processing latency.

    Args:
        video_path: Path to test video
        num_frames: Number of frames to benchmark

    Returns:
        Dictionary with latency statistics
    """
    print(f"⏱️  Benchmarking latency on {num_frames} frames...")

    processor = VideoProcessor(video_path)
    metadata = processor.get_video_metadata()

    # Run benchmark
    summary = processor.process_video(max_frames=num_frames)

    return {
        "avg_latency_ms": summary["avg_processing_time_ms"],
        "p50_latency_ms": np.median(processor.processing_stats["processing_times"]),
        "p95_latency_ms": summary["p95_latency_ms"],
        "p99_latency_ms": summary["p99_latency_ms"],
        "min_latency_ms": min(processor.processing_stats["processing_times"]),
        "max_latency_ms": max(processor.processing_stats["processing_times"]),
    }


def benchmark_throughput(video_path: str, duration_seconds: int = 10) -> dict:
    """
    Benchmark processing throughput (frames per second).

    Args:
        video_path: Path to test video
        duration_seconds: Benchmark duration

    Returns:
        Dictionary with throughput statistics
    """
    print(f"📊 Benchmarking throughput for {duration_seconds}s...")

    processor = VideoProcessor(video_path)
    metadata = processor.get_video_metadata()

    # Calculate target frames
    target_frames = int(metadata["fps"] * duration_seconds)
    target_frames = min(target_frames, metadata["total_frames"])

    start_time = time.perf_counter()
    summary = processor.process_video(max_frames=target_frames)
    elapsed_time = time.perf_counter() - start_time

    fps = summary["total_frames"] / elapsed_time

    return {
        "frames_processed": summary["total_frames"],
        "elapsed_time_s": elapsed_time,
        "throughput_fps": fps,
        "real_time_factor": fps / metadata["fps"],
    }


def benchmark_compression(video_path: str, num_frames: int = 100) -> dict:
    """
    Benchmark compression ratio and bandwidth reduction.

    Args:
        video_path: Path to test video
        num_frames: Number of frames to benchmark

    Returns:
        Dictionary with compression statistics
    """
    print(f"🗜️  Benchmarking compression on {num_frames} frames...")

    processor = VideoProcessor(video_path)
    compression_percents = []

    # Process frames and collect compression stats
    def collect_compression(frame_idx, frame, results):
        compression_percents.append(results["spike_data"]["compression_percent"])

    processor.process_video(max_frames=num_frames, frame_callback=collect_compression)

    return {
        "avg_compression_percent": np.mean(compression_percents),
        "min_compression_percent": np.min(compression_percents),
        "max_compression_percent": np.max(compression_percents),
        "std_compression_percent": np.std(compression_percents),
    }


def main():
    parser = argparse.ArgumentParser(
        description="AuraSense SFSVC SDK Benchmark Suite"
    )
    parser.add_argument(
        "--input",
        type=str,
        default="demo.mp4",
        help="Input video file path (default: demo.mp4)"
    )
    parser.add_argument(
        "--benchmark",
        type=str,
        choices=["latency", "throughput", "compression", "all"],
        default="all",
        help="Benchmark type to run (default: all)"
    )
    parser.add_argument(
        "--num-frames",
        type=int,
        default=100,
        help="Number of frames for latency/compression benchmarks (default: 100)"
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=10,
        help="Duration in seconds for throughput benchmark (default: 10)"
    )

    args = parser.parse_args()

    # Check input file
    input_path = Path(args.input)
    if not input_path.exists():
        print(f"❌ Error: Input file not found: {args.input}")
        return 1

    print("=" * 70)
    print("🚀 AuraSense SFSVC SDK Benchmark Suite")
    print("=" * 70)
    print(f"Input video: {args.input}")
    print(f"Benchmark type: {args.benchmark}")
    print("=" * 70)
    print()

    results = {}

    # Run benchmarks
    if args.benchmark in ["latency", "all"]:
        results["latency"] = benchmark_latency(str(input_path), args.num_frames)
        print()

    if args.benchmark in ["throughput", "all"]:
        results["throughput"] = benchmark_throughput(str(input_path), args.duration)
        print()

    if args.benchmark in ["compression", "all"]:
        results["compression"] = benchmark_compression(str(input_path), args.num_frames)
        print()

    # Print results
    print("=" * 70)
    print("📈 Benchmark Results")
    print("=" * 70)

    if "latency" in results:
        print("\n⏱️  Latency Performance:")
        print(f"  Average:    {results['latency']['avg_latency_ms']:.2f} ms")
        print(f"  Median:     {results['latency']['p50_latency_ms']:.2f} ms")
        print(f"  P95:        {results['latency']['p95_latency_ms']:.2f} ms")
        print(f"  P99:        {results['latency']['p99_latency_ms']:.2f} ms")
        print(f"  Min:        {results['latency']['min_latency_ms']:.2f} ms")
        print(f"  Max:        {results['latency']['max_latency_ms']:.2f} ms")

    if "throughput" in results:
        print("\n📊 Throughput Performance:")
        print(f"  Frames processed: {results['throughput']['frames_processed']}")
        print(f"  Elapsed time:     {results['throughput']['elapsed_time_s']:.2f} s")
        print(f"  Throughput:       {results['throughput']['throughput_fps']:.1f} fps")
        print(f"  Real-time factor: {results['throughput']['real_time_factor']:.2f}x")

    if "compression" in results:
        print("\n🗜️  Compression Performance:")
        print(f"  Average:    {results['compression']['avg_compression_percent']:.2f}%")
        print(f"  Min:        {results['compression']['min_compression_percent']:.2f}%")
        print(f"  Max:        {results['compression']['max_compression_percent']:.2f}%")
        print(f"  Std Dev:    {results['compression']['std_compression_percent']:.2f}%")

    print("=" * 70)

    # Compare to target specs (from production engine)
    if "latency" in results:
        target_p95 = 0.5  # ms (production C++ engine target)
        if results["latency"]["p95_latency_ms"] < 1.0:
            print(f"\n✅ P95 latency under 1ms! (Production target: <{target_p95}ms)")
        else:
            ratio = results["latency"]["p95_latency_ms"] / target_p95
            print(f"\n⚠️  Python reference is {ratio:.1f}x slower than C++ production engine")
            print(f"   (Expected: Python ~8-10ms, C++ <0.5ms)")

    return 0


if __name__ == "__main__":
    sys.exit(main())
