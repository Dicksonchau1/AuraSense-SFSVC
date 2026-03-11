"""Setup script for AuraSense SFSVC SDK."""

from setuptools import setup, find_packages
from pathlib import Path

# Read README for long description
readme_file = Path(__file__).parent / "README.md"
long_description = readme_file.read_text(encoding="utf-8") if readme_file.exists() else ""

setup(
    name="aurasense-sfsvc",
    version="0.1.0",
    author="AuraSense HK",
    author_email="DicksonChau@aurasensehk.com",
    description="Neuromorphic video codec layer for drone inspections with real-time crack detection",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://www.aurasensehk.com",
    project_urls={
        "Bug Reports": "https://github.com/Dicksonchau1/AuraSense-SFSVC/issues",
        "Source": "https://github.com/Dicksonchau1/AuraSense-SFSVC",
    },
    packages=find_packages(include=["aurasense_sfsvc", "aurasense_sfsvc.*", "benchmarks", "benchmarks.*"]),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering :: Image Recognition",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
    python_requires=">=3.8",
    install_requires=[
        "numpy>=1.24.0",
        "opencv-python-headless>=4.8.0",
        "Pillow>=10.0.0",
    ],
    extras_require={
        "dev": [
            "pytest>=7.0.0",
            "pytest-cov>=4.0.0",
            "black>=23.0.0",
            "flake8>=6.0.0",
            "mypy>=1.0.0",
        ],
        "streamlit": [
            "streamlit>=1.30.0",
            "python-dateutil>=2.8.2",
        ],
    },
    entry_points={
        "console_scripts": [
            "aurasense-benchmark=benchmarks.run_benchmark:main",
        ],
    },
    include_package_data=True,
    keywords="neuromorphic codec crack-detection drone-inspection computer-vision",
)
