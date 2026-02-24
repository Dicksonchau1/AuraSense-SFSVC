/* ═══════════════════════════════════════════════════════════════════════════
   AuraSense SFSVC — Website JavaScript
   Handles: navigation, animations, demo canvas, ROI calculator, form
   ═══════════════════════════════════════════════════════════════════════════ */

document.addEventListener('DOMContentLoaded', () => {
    lucide.createIcons();
    initNavbar();
    initScrollAnimations();
    initCounters();
    initStarfield();
    initDemo();
    initROI();
    initContactForm();
});

/* ── Navigation ────────────────────────────────────────────────────────────── */
function initNavbar() {
    const navbar = document.getElementById('navbar');
    const toggle = document.getElementById('navToggle');
    const links  = document.getElementById('navLinks');

    // Scroll effect
    window.addEventListener('scroll', () => {
        navbar.classList.toggle('scrolled', window.scrollY > 60);
    });

    // Mobile toggle
    toggle.addEventListener('click', () => {
        toggle.classList.toggle('active');
        links.classList.toggle('open');
    });

    // Close on link click
    links.querySelectorAll('a').forEach(a => {
        a.addEventListener('click', () => {
            toggle.classList.remove('active');
            links.classList.remove('open');
        });
    });
}

/* ── Scroll Animations ─────────────────────────────────────────────────────── */
function initScrollAnimations() {
    const observer = new IntersectionObserver((entries) => {
        entries.forEach((entry, idx) => {
            if (entry.isIntersecting) {
                // Stagger animation for sibling elements
                const parent = entry.target.parentElement;
                const siblings = parent ? [...parent.querySelectorAll('[data-aos]')] : [];
                const i = siblings.indexOf(entry.target);
                setTimeout(() => {
                    entry.target.classList.add('visible');
                }, i >= 0 ? i * 120 : 0);
                observer.unobserve(entry.target);
            }
        });
    }, { threshold: 0.1, rootMargin: '0px 0px -40px 0px' });

    document.querySelectorAll('[data-aos]').forEach(el => observer.observe(el));

    // Tilt effect on feature/use-case/perf cards
    document.querySelectorAll('.feature-card, .use-case-card, .perf-card').forEach(card => {
        card.addEventListener('mousemove', (e) => {
            const rect = card.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;
            const cx = rect.width / 2;
            const cy = rect.height / 2;
            const rotateX = (y - cy) / cy * -4;
            const rotateY = (x - cx) / cx * 4;
            card.style.transform = `perspective(800px) rotateX(${rotateX}deg) rotateY(${rotateY}deg) translateY(-6px)`;
        });
        card.addEventListener('mouseleave', () => {
            card.style.transform = '';
        });
    });
}

/* ── Counter Animation ─────────────────────────────────────────────────────── */
function initCounters() {
    const counterObserver = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                animateCounters(entry.target);
                counterObserver.unobserve(entry.target);
            }
        });
    }, { threshold: 0.3 });

    document.querySelectorAll('.hero-stats, .perf-grid').forEach(el => {
        counterObserver.observe(el);
    });
}

function animateCounters(container) {
    container.querySelectorAll('[data-count]').forEach(el => {
        const target = parseFloat(el.getAttribute('data-count'));
        const duration = 2000;
        const start = performance.now();
        const isDecimal = target % 1 !== 0;

        function update(now) {
            const elapsed = now - start;
            const progress = Math.min(elapsed / duration, 1);
            // easeOutExpo
            const eased = progress === 1 ? 1 : 1 - Math.pow(2, -10 * progress);
            const current = target * eased;

            if (target === 0) {
                el.textContent = '0';
            } else if (isDecimal) {
                el.textContent = current.toFixed(target < 1 ? 2 : 1);
            } else {
                el.textContent = Math.round(current).toLocaleString();
            }

            if (progress < 1) requestAnimationFrame(update);
        }
        requestAnimationFrame(update);
    });
}

/* ── Hero Typing Effect ─────────────────────────────────────────────────────── */
function initTypedEffect() {
    const el = document.getElementById('heroTyped');
    if (!el) return;

    const phrases = [
        'find every crack.',
        'cut your data by 94%.',
        'work without internet.',
        'replace manual inspections.',
        'save $29K/month.',
        'never miss a defect.',
    ];

    let phraseIdx = 0;
    let charIdx = 0;
    let deleting = false;
    let pauseTimer = 0;

    function tick() {
        const current = phrases[phraseIdx];

        if (!deleting) {
            el.textContent = current.substring(0, charIdx + 1);
            charIdx++;
            if (charIdx === current.length) {
                pauseTimer = 50; // pause at end
                deleting = true;
            }
        } else {
            if (pauseTimer > 0) { pauseTimer--; requestAnimationFrame(tick); return; }
            el.textContent = current.substring(0, charIdx);
            charIdx--;
            if (charIdx < 0) {
                deleting = false;
                charIdx = 0;
                phraseIdx = (phraseIdx + 1) % phrases.length;
            }
        }

        const speed = deleting ? 25 : (charIdx === 1 ? 600 : 55 + Math.random() * 40);
        setTimeout(() => requestAnimationFrame(tick), speed);
    }

    // Wait for entrance animation then start
    setTimeout(tick, 800);
}

/* ── Starfield Background ─────────────────────────────────────────────────── */
function initStarfield() {
    const canvas = document.getElementById('starfield');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    let W, H;
    const stars = [];
    const STAR_COUNT = 180;
    const NEBULA_COUNT = 4;
    const nebulae = [];

    function resize() {
        W = canvas.width = window.innerWidth;
        H = canvas.height = window.innerHeight;
    }
    resize();
    window.addEventListener('resize', resize);

    // Init stars
    for (let i = 0; i < STAR_COUNT; i++) {
        stars.push({
            x: Math.random() * W,
            y: Math.random() * H,
            r: 0.3 + Math.random() * 1.2,
            speed: 0.02 + Math.random() * 0.06,
            phase: Math.random() * Math.PI * 2,
            bright: 0.3 + Math.random() * 0.7
        });
    }

    // Init nebulae (large soft blobs)
    for (let i = 0; i < NEBULA_COUNT; i++) {
        nebulae.push({
            x: Math.random() * W,
            y: Math.random() * H,
            r: 200 + Math.random() * 300,
            hue: 200 + Math.random() * 40,
            alpha: 0.02 + Math.random() * 0.03,
            dx: (Math.random() - 0.5) * 0.15,
            dy: (Math.random() - 0.5) * 0.1
        });
    }

    let t = 0;
    function draw() {
        t++;
        ctx.fillStyle = '#020617';
        ctx.fillRect(0, 0, W, H);

        // Nebulae
        nebulae.forEach(n => {
            n.x += n.dx;
            n.y += n.dy;
            if (n.x < -n.r) n.x = W + n.r;
            if (n.x > W + n.r) n.x = -n.r;
            if (n.y < -n.r) n.y = H + n.r;
            if (n.y > H + n.r) n.y = -n.r;

            const grad = ctx.createRadialGradient(n.x, n.y, 0, n.x, n.y, n.r);
            grad.addColorStop(0, `hsla(${n.hue}, 80%, 50%, ${n.alpha})`);
            grad.addColorStop(1, 'transparent');
            ctx.fillStyle = grad;
            ctx.fillRect(n.x - n.r, n.y - n.r, n.r * 2, n.r * 2);
        });

        // Stars
        stars.forEach(s => {
            const twinkle = 0.5 + 0.5 * Math.sin(t * s.speed + s.phase);
            const alpha = s.bright * twinkle;
            ctx.beginPath();
            ctx.arc(s.x, s.y, s.r, 0, Math.PI * 2);
            ctx.fillStyle = `rgba(180,210,255,${alpha})`;
            ctx.fill();
        });

        requestAnimationFrame(draw);
    }
    draw();
}

/* ── Hero Neural Network Canvas ───────────────────────────────────────────── */
function initHeroCanvas() {
    const canvas = document.getElementById('heroCanvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    let W, H;
    const nodes = [];
    const connections = [];
    const MOUSE = { x: -1000, y: -1000 };
    const NODE_COUNT = 60;

    function resize() {
        W = canvas.width = canvas.offsetWidth;
        H = canvas.height = canvas.offsetHeight;
    }

    function init() {
        resize();
        nodes.length = 0;
        for (let i = 0; i < NODE_COUNT; i++) {
            nodes.push({
                x: Math.random() * W,
                y: Math.random() * H,
                vx: (Math.random() - 0.5) * 0.4,
                vy: (Math.random() - 0.5) * 0.4,
                r: 1.5 + Math.random() * 2,
                pulse: Math.random() * Math.PI * 2,
            });
        }
    }

    function animate() {
        ctx.clearRect(0, 0, W, H);

        // Update nodes
        nodes.forEach(n => {
            n.x += n.vx;
            n.y += n.vy;
            n.pulse += 0.02;

            // Bounce off edges
            if (n.x < 0 || n.x > W) n.vx *= -1;
            if (n.y < 0 || n.y > H) n.vy *= -1;

            // Mouse repulsion
            const dx = n.x - MOUSE.x;
            const dy = n.y - MOUSE.y;
            const dist = Math.sqrt(dx * dx + dy * dy);
            if (dist < 150) {
                const force = (150 - dist) / 150 * 0.03;
                n.vx += dx * force;
                n.vy += dy * force;
            }

            // Damping
            n.vx *= 0.995;
            n.vy *= 0.995;
        });

        // Draw connections
        for (let i = 0; i < nodes.length; i++) {
            for (let j = i + 1; j < nodes.length; j++) {
                const dx = nodes[i].x - nodes[j].x;
                const dy = nodes[i].y - nodes[j].y;
                const dist = Math.sqrt(dx * dx + dy * dy);

                if (dist < 160) {
                    const alpha = (1 - dist / 160) * 0.2;
                    ctx.strokeStyle = `rgba(96, 165, 250, ${alpha})`;
                    ctx.lineWidth = 0.8;
                    ctx.beginPath();
                    ctx.moveTo(nodes[i].x, nodes[i].y);
                    ctx.lineTo(nodes[j].x, nodes[j].y);
                    ctx.stroke();

                    // Occasional data pulse along the line
                    if (Math.sin(nodes[i].pulse * 3) > 0.97) {
                        const t = (Math.sin(nodes[i].pulse * 5) + 1) / 2;
                        const px = nodes[i].x + (nodes[j].x - nodes[i].x) * t;
                        const py = nodes[i].y + (nodes[j].y - nodes[i].y) * t;
                        ctx.fillStyle = `rgba(0, 242, 254, ${0.6 * (1 - dist / 160)})`;
                        ctx.beginPath();
                        ctx.arc(px, py, 2, 0, Math.PI * 2);
                        ctx.fill();
                    }
                }
            }
        }

        // Draw nodes with glow
        nodes.forEach(n => {
            const glow = 0.4 + Math.sin(n.pulse) * 0.3;
            ctx.fillStyle = `rgba(96, 165, 250, ${glow})`;
            ctx.shadowColor = 'rgba(96, 165, 250, 0.5)';
            ctx.shadowBlur = 8;
            ctx.beginPath();
            ctx.arc(n.x, n.y, n.r, 0, Math.PI * 2);
            ctx.fill();
            ctx.shadowBlur = 0;
        });

        requestAnimationFrame(animate);
    }

    canvas.addEventListener('mousemove', (e) => {
        const rect = canvas.getBoundingClientRect();
        MOUSE.x = e.clientX - rect.left;
        MOUSE.y = e.clientY - rect.top;
    });

    canvas.addEventListener('mouseleave', () => {
        MOUSE.x = -1000;
        MOUSE.y = -1000;
    });

    window.addEventListener('resize', () => { resize(); });

    init();
    animate();
}

/* ═══════════════════════════════════════════════════════════════════════════
   INTERACTIVE DEMO — Split-view crack detection with scenario picker
   ═══════════════════════════════════════════════════════════════════════════ */
function initDemo() {
    const rawCanvas = document.getElementById('demoCanvasRaw');
    const detCanvas = document.getElementById('demoCanvasDet');
    const rawCtx    = rawCanvas.getContext('2d');
    const detCtx    = detCanvas.getContext('2d');
    const playBtn   = document.getElementById('demoPlayBtn');
    const status    = document.getElementById('demoStatus');
    const progFill  = document.getElementById('demoProgress');
    const scLabel   = document.getElementById('demoScenarioLabel');
    const scenBtns  = document.querySelectorAll('.demo-scenario');

    let running = false;
    let frame = 0;
    let totalFrames = 480;
    let animId = null;

    /* Scenario definitions */
    const scenarios = {
        tunnel: {
            label: 'Tunnel Inspection',
            bgBase: '#080e1e', bgGrain: '#0d1a30', gridColor: 'rgba(59,130,246,0.06)',
            scanColor: [59,130,246], crackScale: 1.0, surfaceNoise: 350
        },
        bridge: {
            label: 'Bridge Deck Scan',
            bgBase: '#0a1018', bgGrain: '#141e2e', gridColor: 'rgba(6,182,212,0.05)',
            scanColor: [6,182,212], crackScale: 1.3, surfaceNoise: 250
        },
        runway: {
            label: 'Runway Survey',
            bgBase: '#0c0c14', bgGrain: '#1a1a28', gridColor: 'rgba(139,92,246,0.05)',
            scanColor: [139,92,246], crackScale: 0.8, surfaceNoise: 400
        },
        facade: {
            label: 'Facade Analysis',
            bgBase: '#0e0a18', bgGrain: '#1c1630', gridColor: 'rgba(236,72,153,0.05)',
            scanColor: [236,72,153], crackScale: 1.1, surfaceNoise: 300
        }
    };
    let currentScenario = 'tunnel';

    /* Scenario picker */
    scenBtns.forEach(btn => {
        btn.addEventListener('click', () => {
            scenBtns.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            currentScenario = btn.dataset.scenario;
            scLabel.textContent = scenarios[currentScenario].label;
            if (running) { cancelAnimationFrame(animId); running = false; }
            resetDemo();
        });
    });

    playBtn.addEventListener('click', () => {
        if (running) return;
        startDemo();
    });

    function resetDemo() {
        frame = 0;
        progFill.style.width = '0%';
        status.textContent = '● READY';
        status.style.color = '#10b981';
        playBtn.querySelector('span').textContent = 'Run Inspection';
        playBtn.classList.remove('running');
        clearCanvas(rawCtx, rawCanvas);
        clearCanvas(detCtx, detCanvas);
        ['dmCracks','dmLatency','dmSpikes','dmBandwidth','dmFPS'].forEach(id => {
            document.getElementById(id).textContent = '—';
            document.getElementById(id).classList.remove('highlight');
        });
    }

    function clearCanvas(ctx, cvs) {
        const sc = scenarios[currentScenario];
        ctx.fillStyle = sc.bgBase;
        ctx.fillRect(0, 0, cvs.width, cvs.height);
    }

    function startDemo() {
        running = true;
        status.textContent = '● SCANNING';
        status.style.color = '#22d3ee';
        playBtn.querySelector('span').textContent = 'Scanning...';
        playBtn.classList.add('running');
        frame = 0;
        requestAnimationFrame(render);
    }

    function render() {
        if (!running) return;
        const sc = scenarios[currentScenario];
        const W = rawCanvas.width;
        const H = rawCanvas.height;
        frame++;

        /* ── RAW feed (left pane) ─ */
        rawCtx.fillStyle = sc.bgBase;
        rawCtx.fillRect(0, 0, W, H);
        drawSurface(rawCtx, W, H, frame, sc);
        drawScanBeam(rawCtx, W, H, frame, sc);

        /* ── Detection output (right pane) ─ */
        detCtx.fillStyle = sc.bgBase;
        detCtx.fillRect(0, 0, W, H);
        drawSurface(detCtx, W, H, frame, sc);
        drawScanBeam(detCtx, W, H, frame, sc);

        const cracks = generateCracks(frame, W, H, sc);
        drawSpikeField(detCtx, W, H, frame, cracks);
        drawCrackBoxes(detCtx, cracks);
        drawHUD(detCtx, W, H, frame, cracks);

        /* Progress bar + metrics */
        const pct = (frame / totalFrames) * 100;
        progFill.style.width = pct + '%';
        updateDemoMetrics(frame, cracks);

        if (frame < totalFrames) {
            animId = requestAnimationFrame(render);
        } else {
            running = false;
            status.textContent = '● COMPLETE';
            status.style.color = '#f59e0b';
            playBtn.querySelector('span').textContent = 'Run Again';
            playBtn.classList.remove('running');
            document.getElementById('dmBandwidth').classList.add('highlight');
        }
    }

    /* Surface texture */
    function drawSurface(ctx, W, H, f, sc) {
        const rng = mulberry32(42);
        ctx.save();
        ctx.globalAlpha = 0.18;
        for (let i = 0; i < sc.surfaceNoise; i++) {
            const x = rng() * W;
            const y = rng() * H;
            const s = 0.5 + rng() * 2;
            ctx.fillStyle = sc.bgGrain;
            ctx.fillRect(x, y, s, s);
        }
        ctx.globalAlpha = 1;

        // Faint grid
        ctx.strokeStyle = sc.gridColor;
        ctx.lineWidth = 1;
        for (let x = 0; x < W; x += 40) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke(); }
        for (let y = 0; y < H; y += 40) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(W, y); ctx.stroke(); }
        ctx.restore();
    }

    /* Horizontal scan beam */
    function drawScanBeam(ctx, W, H, f, sc) {
        const scanY = (f * 2.5) % H;
        const [r, g, b] = sc.scanColor;
        const grad = ctx.createLinearGradient(0, scanY - 12, 0, scanY + 12);
        grad.addColorStop(0, `rgba(${r},${g},${b},0)`);
        grad.addColorStop(0.5, `rgba(${r},${g},${b},0.35)`);
        grad.addColorStop(1, `rgba(${r},${g},${b},0)`);
        ctx.fillStyle = grad;
        ctx.fillRect(0, scanY - 12, W, 24);
    }

    /* Crack generation with severity */
    function generateCracks(f, W, H, sc) {
        const cracks = [];
        const rng = mulberry32(f * 7 + 31);
        const count = Math.floor(rng() * 4) + 1;
        const sevs = ['Low', 'Medium', 'High'];
        const cols = { Low: '#10b981', Medium: '#f59e0b', High: '#ef4444' };
        for (let i = 0; i < count; i++) {
            const x = 40 + rng() * (W - 140);
            const y = 30 + rng() * (H - 100);
            const w = (50 + rng() * 100) * sc.crackScale;
            const h = (25 + rng() * 55) * sc.crackScale;
            const sev = sevs[Math.floor(rng() * 3)];
            const conf = 0.83 + rng() * 0.16;
            cracks.push({ x, y, w, h, severity: sev, confidence: conf, color: cols[sev] });
        }
        return cracks;
    }

    /* Spike field (clustered near cracks) */
    function drawSpikeField(ctx, W, H, f, cracks) {
        const rng = mulberry32(f * 13 + 97);
        ctx.save();
        ctx.globalAlpha = 0.35;
        for (let i = 0; i < 60; i++) {
            ctx.fillStyle = '#22d3ee';
            ctx.beginPath();
            ctx.arc(rng() * W, rng() * H, 0.8 + rng() * 1.2, 0, Math.PI * 2);
            ctx.fill();
        }
        ctx.globalAlpha = 0.65;
        cracks.forEach(c => {
            const cx = c.x + c.w / 2;
            const cy = c.y + c.h / 2;
            for (let i = 0; i < 25 + rng() * 35; i++) {
                const px = cx + (rng() - 0.5) * c.w * 2;
                const py = cy + (rng() - 0.5) * c.h * 2;
                ctx.fillStyle = '#ffff00';
                ctx.beginPath();
                ctx.arc(px, py, 1.2 + rng() * 1.8, 0, Math.PI * 2);
                ctx.fill();
            }
        });
        ctx.restore();
    }

    /* Detection boxes with glow */
    function drawCrackBoxes(ctx, cracks) {
        cracks.forEach(c => {
            ctx.shadowColor = c.color;
            ctx.shadowBlur = 12;
            ctx.strokeStyle = c.color;
            ctx.lineWidth = 2;
            ctx.setLineDash([6, 3]);
            ctx.strokeRect(c.x, c.y, c.w, c.h);
            ctx.setLineDash([]);

            const label = `${c.severity} ${(c.confidence * 100).toFixed(0)}%`;
            ctx.font = '600 10px Inter, sans-serif';
            const tw = ctx.measureText(label).width + 10;
            ctx.fillStyle = c.color;
            ctx.beginPath();
            ctx.roundRect(c.x, c.y - 18, tw, 16, [3]);
            ctx.fill();
            ctx.shadowBlur = 0;
            ctx.fillStyle = '#fff';
            ctx.fillText(label, c.x + 5, c.y - 5);
        });
        ctx.shadowBlur = 0;
    }

    /* Heads-up display overlay */
    function drawHUD(ctx, W, H, f, cracks) {
        ctx.font = '500 10px "JetBrains Mono", monospace';
        ctx.fillStyle = 'rgba(255,255,255,0.5)';
        ctx.textAlign = 'left';
        ctx.fillText(`f ${f}/${totalFrames}`, 8, H - 8);
        ctx.textAlign = 'right';
        ctx.fillStyle = 'rgba(255,255,255,0.45)';
        ctx.fillText(`${cracks.length} detected`, W - 8, H - 8);
        ctx.textAlign = 'left';

        // Waveform top-right
        ctx.save();
        ctx.translate(W - 100, 12);
        ctx.strokeStyle = 'rgba(34,211,238,0.45)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        for (let i = 0; i < 80; i++) {
            const y = Math.sin((i + f * 3) * 0.09) * 6 + Math.sin((i + f) * 0.16) * 3;
            i === 0 ? ctx.moveTo(i, y) : ctx.lineTo(i, y);
        }
        ctx.stroke();
        ctx.restore();
    }

    function updateDemoMetrics(f, cracks) {
        const rng = mulberry32(f);
        document.getElementById('dmCracks').textContent = cracks.length;
        document.getElementById('dmLatency').textContent = (0.3 + rng() * 0.4).toFixed(2) + ' ms';
        document.getElementById('dmSpikes').textContent = Math.floor(40 + rng() * 120);
        document.getElementById('dmBandwidth').textContent = (93 + rng() * 2).toFixed(1) + '%';
        document.getElementById('dmFPS').textContent = Math.floor(110 + rng() * 30);
    }

    // Auto-size canvases
    function resizeCanvases() {
        [rawCanvas, detCanvas].forEach(cvs => {
            const rect = cvs.parentElement.getBoundingClientRect();
            cvs.width = Math.round(rect.width);
            cvs.height = Math.round(rect.height || 300);
        });
    }
    window.addEventListener('resize', () => { if (!running) resizeCanvases(); });
    resizeCanvases();
    resetDemo();
}

// Seeded PRNG (mulberry32)
function mulberry32(a) {
    return function () {
        let t = a += 0x6D2B79F5;
        t = Math.imul(t ^ t >>> 15, t | 1);
        t ^= t + Math.imul(t ^ t >>> 7, t | 61);
        return ((t ^ t >>> 14) >>> 0) / 4294967296;
    };
}

/* ═══════════════════════════════════════════════════════════════════════════
   ROI CALCULATOR
   ═══════════════════════════════════════════════════════════════════════════ */
function initROI() {
    const drones    = document.getElementById('roiDrones');
    const hours     = document.getElementById('roiHours');
    const cellRate  = document.getElementById('roiCellRate');
    const resolution = document.getElementById('roiResolution');

    function calculate() {
        const n = parseInt(drones.value);
        const h = parseInt(hours.value);
        const rate = parseFloat(cellRate.value);
        const bitrate = parseFloat(resolution.value);

        // Monthly data per drone (GB)
        // bitrate (Mbps) * 3600 sec * hours / 8 / 1024 = GB
        const gbPerDrone = bitrate * 3600 * h / 8 / 1024;
        const totalGB = gbPerDrone * n;
        const monthlyCostBefore = totalGB * rate;

        // SFSVC reduces by 93.8%
        const reduction = 0.938;
        const sfsvcBitrate = bitrate * (1 - reduction);
        const sfsvcGB = totalGB * (1 - reduction);
        const monthlyCostAfter = sfsvcGB * rate;

        const monthlySavings = monthlyCostBefore - monthlyCostAfter;
        const annualSavings = monthlySavings * 12;

        // SFSVC license cost estimate: $500/drone/month for small, scaling down
        const sfsvcCost = n <= 10 ? n * 500 : n <= 50 ? n * 400 : n * 300;
        const sfsvcAnnual = sfsvcCost * 12;
        const netAnnualSavings = annualSavings - sfsvcAnnual;
        const paybackMonths = sfsvcAnnual > 0 ? Math.ceil(sfsvcAnnual / monthlySavings) : 0;
        const roi = sfsvcAnnual > 0 ? Math.round((netAnnualSavings / sfsvcAnnual) * 100) : 0;

        // Update UI
        document.getElementById('roiDronesVal').textContent = n + ' drones';
        document.getElementById('roiHoursVal').textContent = h + ' hrs';
        document.getElementById('roiCellRateVal').textContent = '$' + rate.toFixed(rate % 1 === 0 ? 0 : 1) + '/GB';

        document.getElementById('roiBefore').textContent = '$' + Math.round(monthlyCostBefore).toLocaleString();
        document.getElementById('roiBeforeData').textContent = 
            Math.round(totalGB).toLocaleString() + ' GB/mo · ' + bitrate.toFixed(1) + ' Mbps';

        document.getElementById('roiAfter').textContent = '$' + Math.round(monthlyCostAfter).toLocaleString();
        document.getElementById('roiAfterData').textContent = 
            Math.round(sfsvcGB).toLocaleString() + ' GB/mo · ' + sfsvcBitrate.toFixed(2) + ' Mbps';

        document.getElementById('roiMonthlySavings').textContent = '$' + Math.round(monthlySavings).toLocaleString();
        document.getElementById('roiAnnualSavings').textContent = '$' + Math.round(annualSavings).toLocaleString();
        document.getElementById('roiPayback').textContent = paybackMonths > 0 ? paybackMonths + ' months' : '—';
        document.getElementById('roiReturn').textContent = roi > 0 ? roi + '%' : '—';
    }

    [drones, hours, cellRate, resolution].forEach(el => {
        el.addEventListener('input', calculate);
    });

    calculate();

    // Download report
    document.getElementById('roiDownload').addEventListener('click', () => {
        const n = drones.value;
        const before = document.getElementById('roiBefore').textContent;
        const after = document.getElementById('roiAfter').textContent;
        const savings = document.getElementById('roiMonthlySavings').textContent;
        const annual = document.getElementById('roiAnnualSavings').textContent;
        const payback = document.getElementById('roiPayback').textContent;
        const roiVal = document.getElementById('roiReturn').textContent;

        const text = [
            '═══════════════════════════════════════════',
            '  AuraSense SFSVC — ROI Report',
            '  Generated: ' + new Date().toLocaleDateString(),
            '═══════════════════════════════════════════',
            '',
            `  Fleet Size:        ${n} drones`,
            `  Current Cost:      ${before}/month`,
            `  With SFSVC:        ${after}/month`,
            `  Monthly Savings:   ${savings}`,
            `  Annual Savings:    ${annual}`,
            `  Payback Period:    ${payback}`,
            `  Year 1 ROI:        ${roiVal}`,
            '',
            '═══════════════════════════════════════════',
            '  Contact: support@aurasensehk.com',
            '  www.aurasensehk.com',
            '═══════════════════════════════════════════',
        ].join('\n');

        const blob = new Blob([text], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `AuraSense_ROI_Report_${n}_drones.txt`;
        a.click();
        URL.revokeObjectURL(url);
    });
}

/* ═══════════════════════════════════════════════════════════════════════════
   CONTACT FORM
   ═══════════════════════════════════════════════════════════════════════════ */
function initContactForm() {
    const form = document.getElementById('contactForm');
    const success = document.getElementById('formSuccess');

    form.addEventListener('submit', (e) => {
        e.preventDefault();

        // Collect form data
        const data = new FormData(form);
        const payload = {};
        data.forEach((v, k) => payload[k] = v);

        // ─── FORM SUBMISSION OPTIONS ───────────────────────────
        // OPTION 1 (recommended): Formspree — free, no backend needed
        //   1. Sign up at https://formspree.io
        //   2. Create a form, get your ID (e.g., "xyzabcde")
        //   3. Uncomment below and replace YOUR_FORM_ID:
        //
        // fetch('https://formspree.io/f/YOUR_FORM_ID', {
        //     method: 'POST',
        //     body: data,
        //     headers: { 'Accept': 'application/json' }
        // }).then(r => {
        //     if (r.ok) showSuccess();
        //     else alert('Error — please email DicksonChau@aurasensehk.com directly.');
        // }).catch(() => fallbackMailto(payload));
        // return;
        //
        // OPTION 2: EmailJS — also free tier available
        // OPTION 3: GoDaddy email forwarding with PHP (if hosting supports it)
        // ────────────────────────────────────────────────────────

        // Current fallback: opens user's email client
        fallbackMailto(payload);
        showSuccess();
    });

    function fallbackMailto(payload) {
        const mailto = `mailto:support@aurasensehk.com?subject=${
            encodeURIComponent('AuraSense Demo Request — ' + (payload.company || ''))
        }&body=${
            encodeURIComponent(
                `Name: ${payload.name}\n` +
                `Email: ${payload.email}\n` +
                `Company: ${payload.company}\n` +
                `Fleet: ${payload.fleet}\n` +
                `Interest: ${payload.interest}\n` +
                `Message: ${payload.message}`
            )
        }`;
        window.open(mailto, '_blank');
    }

    function showSuccess() {
        form.style.display = 'none';
        success.style.display = 'block';
        setTimeout(() => {
            form.style.display = '';
            success.style.display = 'none';
            form.reset();
        }, 5000);
    }
}
