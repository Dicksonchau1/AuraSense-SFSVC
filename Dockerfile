# AuraSense SFSVC SDK - Docker Image
# Multi-stage build for optimized image size

FROM python:3.11-slim as base

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    libgl1-mesa-glx \
    libglib2.0-0 \
    libsm6 \
    libxext6 \
    libxrender-dev \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 aurasense && \
    chown -R aurasense:aurasense /app

# Switch to non-root user
USER aurasense

# Set Python environment
ENV PYTHONUNBUFFERED=1 \
    PYTHONDONTWRITEBYTECODE=1 \
    PATH="/home/aurasense/.local/bin:${PATH}"

#
# SDK Installation Stage
#
FROM base as sdk

# Copy package files
COPY --chown=aurasense:aurasense pyproject.toml setup.py README.md ./
COPY --chown=aurasense:aurasense aurasense_sfsvc/ ./aurasense_sfsvc/
COPY --chown=aurasense:aurasense benchmarks/ ./benchmarks/
COPY --chown=aurasense:aurasense examples/ ./examples/

# Install SDK in editable mode with all extras
RUN pip install --user --no-cache-dir -e ".[all]"

#
# Runtime Stage - SDK Only
#
FROM sdk as runtime-sdk

# Copy demo video if exists
COPY --chown=aurasense:aurasense demo.mp4 ./demo.mp4 2>/dev/null || true

# Default command: show help
CMD ["python", "-m", "aurasense_sfsvc", "--help"]

#
# Runtime Stage - Streamlit App
#
FROM sdk as runtime-streamlit

# Copy Streamlit app and config
COPY --chown=aurasense:aurasense streamlit_app.py ./
COPY --chown=aurasense:aurasense .streamlit/ ./.streamlit/
COPY --chown=aurasense:aurasense demo.mp4 ./demo.mp4 2>/dev/null || true

# Expose Streamlit port
EXPOSE 8501

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD python -c "import requests; requests.get('http://localhost:8501/_stcore/health')"

# Run Streamlit app
CMD ["streamlit", "run", "streamlit_app.py", \
     "--server.port=8501", \
     "--server.address=0.0.0.0", \
     "--server.headless=true", \
     "--browser.gatherUsageStats=false"]

#
# Development Stage
#
FROM sdk as development

USER root

# Install development tools
RUN apt-get update && apt-get install -y \
    git \
    vim \
    && rm -rf /var/lib/apt/lists/*

USER aurasense

# Install development dependencies
RUN pip install --user --no-cache-dir \
    ipython \
    jupyter \
    pytest \
    pytest-cov \
    black \
    flake8 \
    mypy

# Default to bash for development
CMD ["/bin/bash"]
