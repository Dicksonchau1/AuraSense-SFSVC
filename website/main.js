/* ═══════════════════════════════════════════════════════════════════════════
   AuraSense SFSVC — Website JavaScript
   Handles: navigation, animations, demo canvas, ROI calculator, form
   ═══════════════════════════════════════════════════════════════════════════ */

document.addEventListener('DOMContentLoaded', () => {
    lucide.createIcons();
    initNavbar();
    initScrollAnimations();
    initCounters();
    initDemo();
    initROI();
    initContactForm();
    initParticles();
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
                }, i >= 0 ? i * 100 : 0);
                observer.unobserve(entry.target);
            }
        });
    }, { threshold: 0.1, rootMargin: '0px 0px -40px 0px' });

    document.querySelectorAll('[data-aos]').forEach(el => observer.observe(el));
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

/* ── Hero Particles ────────────────────────────────────────────────────────── */
function initParticles() {
    const container = document.getElementById('heroParticles');
    if (!container) return;

    const count = 50;
    for (let i = 0; i < count; i++) {
        const p = document.createElement('div');
        p.style.cssText = `
            position: absolute;
            width: ${1 + Math.random() * 3}px;
            height: ${1 + Math.random() * 3}px;
            background: rgba(167, 139, 250, ${0.2 + Math.random() * 0.4});
            border-radius: 50%;
            left: ${Math.random() * 100}%;
            top: ${Math.random() * 100}%;
            animation: particle-float ${5 + Math.random() * 10}s ${Math.random() * 5}s infinite ease-in-out;
        `;
        container.appendChild(p);
    }

    // Add keyframes dynamically
    const style = document.createElement('style');
    style.textContent = `
        @keyframes particle-float {
            0%, 100% { transform: translate(0, 0) scale(1); opacity: 0.3; }
            25% { transform: translate(${rr(-30, 30)}px, ${rr(-30, 30)}px) scale(1.2); opacity: 0.6; }
            50% { transform: translate(${rr(-20, 20)}px, ${rr(-40, 10)}px) scale(0.8); opacity: 0.4; }
            75% { transform: translate(${rr(-30, 30)}px, ${rr(-20, 20)}px) scale(1.1); opacity: 0.5; }
        }
    `;
    document.head.appendChild(style);
}

function rr(min, max) {
    return Math.floor(Math.random() * (max - min) + min);
}

/* ═══════════════════════════════════════════════════════════════════════════
   INTERACTIVE DEMO — Canvas-based crack detection visualization
   ═══════════════════════════════════════════════════════════════════════════ */
function initDemo() {
    const canvas   = document.getElementById('demoCanvas');
    const ctx      = canvas.getContext('2d');
    const overlay  = document.getElementById('demoOverlay');
    const playBtn  = document.getElementById('demoPlayBtn');
    const status   = document.getElementById('demoStatus');

    let running = false;
    let frame = 0;
    let animId = null;

    playBtn.addEventListener('click', () => {
        if (!running) startDemo();
    });

    function startDemo() {
        running = true;
        overlay.classList.add('hidden');
        status.textContent = '● RUNNING';
        status.style.color = '#10b981';
        frame = 0;
        requestAnimationFrame(render);
    }

    function render() {
        if (!running) return;

        const W = canvas.width;
        const H = canvas.height;
        frame++;

        // Dark background simulating concrete surface
        ctx.fillStyle = '#1a1640';
        ctx.fillRect(0, 0, W, H);

        // Simulated concrete texture
        drawConcreteTexture(ctx, W, H, frame);

        // Simulate cracks
        const cracks = generateCracks(frame, W, H);

        // Draw spike events (yellow dots)
        drawSpikeEvents(ctx, W, H, frame, cracks);

        // Draw crack detection boxes
        drawCrackBoxes(ctx, cracks);

        // Draw HUD
        drawHUD(ctx, W, H, frame, cracks);

        // Update metrics
        updateDemoMetrics(frame, cracks);

        if (frame < 600) {
            animId = requestAnimationFrame(render);
        } else {
            running = false;
            status.textContent = '● COMPLETE';
            status.style.color = '#f59e0b';
        }
    }

    function drawConcreteTexture(ctx, W, H, frame) {
        // Static grain (cached via seed)
        const seed = 42;
        const rng = mulberry32(seed);

        ctx.save();
        ctx.globalAlpha = 0.15;
        for (let i = 0; i < 300; i++) {
            const x = rng() * W;
            const y = rng() * H;
            const s = 0.5 + rng() * 2;
            ctx.fillStyle = rng() > 0.5 ? '#3a366e' : '#2a2656';
            ctx.fillRect(x, y, s, s);
        }
        ctx.globalAlpha = 1;

        // Moving scan line
        const scanY = (frame * 2) % H;
        const gradient = ctx.createLinearGradient(0, scanY - 10, 0, scanY + 10);
        gradient.addColorStop(0, 'rgba(102, 126, 234, 0)');
        gradient.addColorStop(0.5, 'rgba(102, 126, 234, 0.3)');
        gradient.addColorStop(1, 'rgba(102, 126, 234, 0)');
        ctx.fillStyle = gradient;
        ctx.fillRect(0, scanY - 10, W, 20);

        // Grid overlay
        ctx.strokeStyle = 'rgba(102, 126, 234, 0.05)';
        ctx.lineWidth = 1;
        for (let x = 0; x < W; x += 40) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, H);
            ctx.stroke();
        }
        for (let y = 0; y < H; y += 40) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(W, y);
            ctx.stroke();
        }
        ctx.restore();
    }

    function generateCracks(frame, W, H) {
        const cracks = [];
        const rng = mulberry32(frame * 7 + 31);

        const count = Math.floor(rng() * 4) + 1;
        const severities = ['Low', 'Medium', 'High'];
        const colors = { Low: '#10b981', Medium: '#f59e0b', High: '#ef4444' };

        for (let i = 0; i < count; i++) {
            const x = 60 + rng() * (W - 200);
            const y = 40 + rng() * (H - 140);
            const w = 60 + rng() * 120;
            const h = 30 + rng() * 60;
            const sev = severities[Math.floor(rng() * 3)];
            const conf = 0.82 + rng() * 0.17;

            cracks.push({ x, y, w, h, severity: sev, confidence: conf, color: colors[sev] });
        }
        return cracks;
    }

    function drawSpikeEvents(ctx, W, H, frame, cracks) {
        const rng = mulberry32(frame * 13 + 97);

        // Background spikes
        ctx.globalAlpha = 0.4;
        for (let i = 0; i < 80; i++) {
            const x = rng() * W;
            const y = rng() * H;
            ctx.fillStyle = '#00ffe0';
            ctx.beginPath();
            ctx.arc(x, y, 1 + rng() * 1.5, 0, Math.PI * 2);
            ctx.fill();
        }

        // Clustered spikes near cracks
        ctx.globalAlpha = 0.7;
        cracks.forEach(c => {
            const cx = c.x + c.w / 2;
            const cy = c.y + c.h / 2;
            const pts = [];

            for (let i = 0; i < 30 + rng() * 40; i++) {
                const px = cx + (rng() - 0.5) * c.w * 2;
                const py = cy + (rng() - 0.5) * c.h * 2;
                ctx.fillStyle = '#ffff00';
                ctx.beginPath();
                ctx.arc(px, py, 1.5 + rng() * 2, 0, Math.PI * 2);
                ctx.fill();
                pts.push([px, py]);
            }

            // Connect nearby spikes
            ctx.strokeStyle = 'rgba(255, 255, 0, 0.2)';
            ctx.lineWidth = 0.5;
            for (let i = 0; i < pts.length - 1; i++) {
                if (Math.abs(pts[i][0] - pts[i + 1][0]) < 40) {
                    ctx.beginPath();
                    ctx.moveTo(pts[i][0], pts[i][1]);
                    ctx.lineTo(pts[i + 1][0], pts[i + 1][1]);
                    ctx.stroke();
                }
            }
        });
        ctx.globalAlpha = 1;
    }

    function drawCrackBoxes(ctx, cracks) {
        cracks.forEach(c => {
            // Glow effect
            ctx.shadowColor = c.color;
            ctx.shadowBlur = 10;

            // Box
            ctx.strokeStyle = c.color;
            ctx.lineWidth = 2;
            ctx.setLineDash([6, 3]);
            ctx.strokeRect(c.x, c.y, c.w, c.h);
            ctx.setLineDash([]);

            // Label background
            const label = `${c.severity} ${(c.confidence * 100).toFixed(0)}%`;
            ctx.font = '600 11px Inter, sans-serif';
            const tw = ctx.measureText(label).width + 12;
            ctx.fillStyle = c.color;
            const radius = 4;
            const lx = c.x;
            const ly = c.y - 20;
            ctx.beginPath();
            ctx.roundRect(lx, ly, tw, 18, [radius]);
            ctx.fill();

            // Label text
            ctx.shadowBlur = 0;
            ctx.fillStyle = '#fff';
            ctx.fillText(label, lx + 6, ly + 13);
        });
        ctx.shadowBlur = 0;
    }

    function drawHUD(ctx, W, H, frame, cracks) {
        // Frame counter
        ctx.font = '500 12px "JetBrains Mono", monospace';
        ctx.fillStyle = 'rgba(255, 255, 255, 0.6)';
        ctx.fillText(`Frame ${frame}/600`, 10, H - 10);

        // Status bar
        const progress = frame / 600;
        ctx.fillStyle = 'rgba(102, 126, 234, 0.3)';
        ctx.fillRect(0, H - 4, W, 4);
        ctx.fillStyle = '#667eea';
        ctx.fillRect(0, H - 4, W * progress, 4);

        // System info
        ctx.textAlign = 'right';
        ctx.fillStyle = 'rgba(255, 255, 255, 0.5)';
        ctx.fillText(`SFSVC v1.0 | ${cracks.length} detected | spikes: ON`, W - 10, H - 10);
        ctx.textAlign = 'left';

        // Waveform in top-right
        ctx.save();
        ctx.translate(W - 140, 15);
        ctx.strokeStyle = 'rgba(0, 255, 224, 0.5)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        for (let i = 0; i < 120; i++) {
            const y = Math.sin((i + frame * 3) * 0.08) * 8 + Math.sin((i + frame) * 0.15) * 4;
            if (i === 0) ctx.moveTo(i, y);
            else ctx.lineTo(i, y);
        }
        ctx.stroke();
        ctx.restore();
    }

    function updateDemoMetrics(frame, cracks) {
        const rng = mulberry32(frame);
        document.getElementById('dmCracks').textContent = cracks.length;
        document.getElementById('dmLatency').textContent = (0.3 + rng() * 0.4).toFixed(2) + ' ms';
        document.getElementById('dmSpikes').textContent = Math.floor(40 + rng() * 120);
        document.getElementById('dmBandwidth').textContent = (0.28 + rng() * 0.08).toFixed(2) + ' Mbps';
        document.getElementById('dmFPS').textContent = Math.floor(110 + rng() * 30);
    }
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
            '  Contact: DicksonChau@aurasensehk.com',
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
        const mailto = `mailto:DicksonChau@aurasensehk.com?subject=${
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
