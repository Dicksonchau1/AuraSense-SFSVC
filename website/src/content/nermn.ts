import type { ProductPageContent } from '../types/content';

export const nermnContent: ProductPageContent = {
  hero: {
    headline: 'NERMN — Neuromorphic Edge Robotic Navigation Module',
    subheadline: 'Edge-Native Navigation for Autonomous Inspection Platforms',
    description:
      'NERMN is an edge-native navigation stack for robotic inspection platforms operating in GPS-denied, degraded, or confined environments. It ingests event camera and LiDAR data through a spike-encoded sensor pipeline, produces bounded-latency path decisions under hard time budgets, and enforces watchdog-supervised safety constraints with degraded-safe fallback behavior. Designed for tunnel crawlers, pipeline robots, structural inspection drones, and confined-space autonomy.',
    primaryCta: { label: 'Request NERMN Briefing', href: '/contact' },
    secondaryCta: { label: 'View All Products', href: '/products' },
  },
  sections: [
    {
      id: 'navigation-stack',
      eyebrow: 'Architecture',
      title: 'Navigation Stack',
      description:
        'NERMN implements a layered navigation stack designed for robotic platforms that operate under real-world constraints — limited compute, unreliable GPS, degraded visibility, and confined traversal paths. The stack processes sensor data through a spike-encoded pipeline that produces navigation commands within bounded latency, with safety supervision at every stage.',
      items: [
        {
          title: 'Sensor Ingestion Layer',
          description:
            'Ingests event camera streams (DVS/DAVIS format), LiDAR point clouds, IMU data, and odometry telemetry. Sensor data is timestamped at ingestion and routed to the spike encoding stage. The ingestion layer handles clock synchronization, data format normalization, and sensor health monitoring.',
        },
        {
          title: 'Spike Encoding Stage',
          description:
            'Raw sensor data is transformed into spike-encoded representations — sparse, temporal event signals that capture scene dynamics while suppressing static background. Event camera data is natively spike-compatible. LiDAR and IMU data are encoded through rate-coding and temporal differencing schemes.',
        },
        {
          title: 'Path Planning Engine',
          description:
            'Spike-encoded environmental representations are processed by the path planning engine, which produces navigation commands — heading, velocity, and obstacle avoidance decisions — within hard latency bounds. The planner operates over a local occupancy representation updated at sensor frame rate.',
        },
        {
          title: 'Command Output Interface',
          description:
            'Navigation commands are emitted to the robotic platform control interface through a deterministic output stage. Command format, update rate, and safety bounds are configurable per platform. The output interface supports MAVLink, ROS2 topics, and direct serial/CAN bus communication.',
        },
      ],
    },
    {
      id: 'sensor-to-decision',
      eyebrow: 'Pipeline',
      title: 'Sensor-to-Decision Pipeline',
      description:
        'The NERMN sensor-to-decision pipeline transforms raw sensor inputs into navigation commands through a sequence of deterministic stages. Every stage operates within bounded latency. Every intermediate result is logged for replay verification. The pipeline produces consistent navigation decisions regardless of execution timing or thread scheduling.',
      items: [
        {
          title: 'Event Camera Processing',
          description:
            'Dynamic Vision Sensor (DVS) events are ingested as asynchronous pixel-level brightness changes. Events are accumulated into temporal windows and processed as spike frames. This preserves microsecond temporal resolution while enabling batch processing at configurable frame rates — typically 100–500Hz for navigation.',
        },
        {
          title: 'LiDAR Integration',
          description:
            'LiDAR point clouds are projected into the local occupancy grid and spike-encoded through temporal differencing — new obstacles trigger spike events, stable geometry is suppressed. This produces a sparse, change-driven environmental representation that aligns with the event camera pipeline.',
        },
        {
          title: 'Spike-Encoded Inference',
          description:
            'The combined spike representation is processed by the neuromorphic inference engine, which computes obstacle proximity, traversability, and heading correction using spike-timing-dependent computations. Inference runs on CPU with SIMD acceleration — no GPU, no neural accelerator dependency.',
        },
        {
          title: 'Decision Arbitration',
          description:
            'The decision arbitration stage resolves conflicts between path planning, obstacle avoidance, and mission objectives. Priority ordering is: safety constraint satisfaction → obstacle avoidance → mission waypoint tracking. Arbitration decisions are logged with full context for post-mission review.',
        },
      ],
    },
    {
      id: 'safety-supervision',
      eyebrow: 'Safety',
      title: 'Safety and Supervision',
      description:
        'NERMN enforces safety constraints through a watchdog supervision architecture. The safety system operates independently of the navigation pipeline and can override navigation commands when safety invariants are violated. Degraded-safe fallback behavior is pre-configured — if the navigation pipeline fails, stalls, or produces out-of-bounds commands, the safety system activates a controlled stop or retreat.',
      items: [
        {
          title: 'Watchdog Supervision',
          description:
            'An independent watchdog process monitors the navigation pipeline for liveness, latency violations, and output bounds compliance. If the pipeline fails to produce a valid command within the configured deadline, the watchdog triggers the degraded-safe fallback. The watchdog runs on a separate CPU core with hard real-time priority.',
        },
        {
          title: 'Degraded-Safe Fallback',
          description:
            'When the navigation pipeline cannot produce reliable commands — due to sensor failure, excessive latency, or confidence degradation — the fallback policy activates. Default fallback behavior is controlled deceleration and station-keeping. Fallback policies are configurable per deployment context: hover, retreat-to-last-waypoint, or controlled landing.',
        },
        {
          title: 'Command Bounds Enforcement',
          description:
            'Every navigation command is validated against configured bounds before transmission to the platform controller. Velocity, acceleration, heading rate, and altitude commands must fall within per-platform safety envelopes. Out-of-bounds commands are clamped and logged as safety events.',
        },
        {
          title: 'Safety Event Logging',
          description:
            'Every safety intervention — watchdog trigger, fallback activation, command clamping, sensor degradation — is recorded as a structured safety event in the evidence chain. Safety events carry the same chain-sealed integrity as classification telemetry and are retained for post-mission safety review.',
        },
      ],
    },
    {
      id: 'edge-runtime',
      eyebrow: 'Runtime',
      title: 'Edge Runtime Profile',
      description:
        'NERMN runs on commodity edge hardware with no GPU or specialized accelerator requirements. The runtime is designed for the compute, power, and thermal constraints of robotic platforms — inspection drones, ground robots, and rail-mounted crawlers.',
      items: [
        {
          title: 'Compute Requirements',
          description:
            'Minimum 4-core ARM64 or x86_64 CPU with NEON or AVX2 SIMD. 2GB RAM minimum, 4GB recommended for simultaneous navigation and inspection (NERMN + SFSVC). Power budget: 5–15W typical for ARM64 embedded platforms.',
        },
        {
          title: 'Real-Time Scheduling',
          description:
            'The navigation pipeline runs under real-time scheduling priority (SCHED_FIFO) with CPU core affinity. Sensor ingestion, spike encoding, path planning, and command output each run on dedicated cores when available. Lock-free inter-stage communication eliminates priority inversion.',
        },
        {
          title: 'Latency Characteristics',
          description:
            'Sensor-to-command latency: P95 under 5ms, P99 under 10ms for the full pipeline from sensor ingestion to command output. Individual path planning iterations complete in under 2ms. These bounds hold under full sensor load on target hardware.',
        },
        {
          title: 'Offline Operation',
          description:
            'NERMN operates without network connectivity. All navigation models, safety parameters, and mission configurations are loaded at startup from local storage. Evidence retention is local. Post-mission data retrieval occurs via physical media connection or selective uplink at a connectivity point.',
        },
      ],
    },
    {
      id: 'robotics-integration',
      eyebrow: 'Integration',
      title: 'Robotics and Drone Integration',
      description:
        'NERMN integrates with robotic platforms through a configurable command interface. The module supports common robotics communication protocols and is validated against representative inspection platforms.',
      items: [
        {
          title: 'MAVLink Integration',
          description:
            'Native MAVLink v2 support for autopilot integration. NERMN sends POSITION_TARGET_LOCAL_NED and ATTITUDE_TARGET messages. Supports offboard mode engagement with configurable heartbeat and failsafe behavior. Validated against PX4 and ArduPilot firmware.',
        },
        {
          title: 'ROS2 Integration',
          description:
            'ROS2 node with configurable topic mapping for navigation commands, sensor inputs, and telemetry outputs. Supports standard message types: geometry_msgs/Twist, nav_msgs/Path, sensor_msgs/PointCloud2. Compatible with ROS2 Humble and Iron distributions.',
        },
        {
          title: 'Direct Serial/CAN Interface',
          description:
            'For platforms without MAVLink or ROS2, NERMN supports direct serial (UART) and CAN bus command output with configurable message formats. Suitable for custom robotic platforms, rail-mounted crawlers, and legacy inspection systems.',
        },
        {
          title: 'Co-Deployment with SFSVC',
          description:
            'NERMN and SFSVC can run simultaneously on the same edge platform. NERMN handles navigation while SFSVC performs surface inspection. Both modules share the evidence retention layer and produce coordinated telemetry for post-mission review.',
        },
      ],
    },
    {
      id: 'deployment-model',
      eyebrow: 'Deployment',
      title: 'Deployment Model',
      description:
        'NERMN is deployed as a native binary with configuration files and model bundles. Deployment follows the NEPA governance pipeline with versioned bundles, human approval gates, and offline activation.',
      items: [
        {
          title: 'Platform Provisioning',
          description:
            'Each robotic platform is provisioned with a hardware-specific NERMN configuration that includes sensor mapping, safety bounds, communication interface settings, and mission parameters. Provisioning uses a declarative configuration format validated at startup.',
        },
        {
          title: 'Mission Configuration',
          description:
            'Mission-specific parameters — waypoints, inspection zones, survey patterns, return-to-home coordinates — are loaded from mission configuration files. Missions can be pre-programmed or dynamically updated through the command interface within safety bounds.',
        },
        {
          title: 'Model and Safety Bundle Updates',
          description:
            'Navigation models and safety parameters are packaged as versioned bundles with SHA-256 integrity verification. Updates follow the NEPA governance pipeline: development → validation → staging → production. No autonomous bundle promotion — human approval is required at each gate.',
        },
        {
          title: 'Post-Mission Data Handling',
          description:
            'After mission completion, evidence logs, navigation telemetry, safety event records, and inspection data are available for retrieval. Data is retained on the platform in chain-sealed format. Retrieval occurs via USB, ethernet, or wireless uplink depending on platform configuration.',
        },
      ],
    },
  ],
  specs: [
    { label: 'Sensor-to-Command Latency (P95)', value: '< 5ms' },
    { label: 'Sensor-to-Command Latency (P99)', value: '< 10ms' },
    { label: 'Path Planning Iteration', value: '< 2ms' },
    { label: 'Sensor Update Rate', value: '100–500Hz (configurable)' },
    { label: 'Minimum Compute', value: '4-core ARM64/x86_64 with SIMD' },
    { label: 'Minimum RAM', value: '2GB (4GB recommended)' },
    { label: 'Power Budget', value: '5–15W typical (ARM64)' },
    { label: 'Communication Protocols', value: 'MAVLink v2, ROS2, Serial, CAN' },
    { label: 'Safety Supervision', value: 'Independent watchdog with fallback' },
    { label: 'License Verification', value: 'Ed25519 offline activation' },
  ],
};
