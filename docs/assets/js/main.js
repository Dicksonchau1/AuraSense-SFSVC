/* ═══════════════════════════════════════════════════════════════════════════
   AuraSense Corporate — Main JavaScript
   ═══════════════════════════════════════════════════════════════════════════ */

(function () {
    'use strict';

    // ── Navbar: scroll effect ──────────────────────────────────────────────
    const navbar = document.getElementById('navbar');
    if (navbar) {
        const onScroll = () => {
            navbar.classList.toggle('scrolled', window.scrollY > 20);
        };
        window.addEventListener('scroll', onScroll, { passive: true });
        onScroll();
    }

    // ── Mobile nav toggle ──────────────────────────────────────────────────
    const navToggle = document.getElementById('navToggle');
    const navLinks = document.getElementById('navLinks');

    if (navToggle && navLinks) {
        navToggle.addEventListener('click', () => {
            navToggle.classList.toggle('active');
            navLinks.classList.toggle('open');
            document.body.style.overflow = navLinks.classList.contains('open') ? 'hidden' : '';
        });

        // Close on link click (mobile)
        navLinks.querySelectorAll('a:not(.nav-dropdown-trigger)').forEach(link => {
            link.addEventListener('click', () => {
                if (navLinks.classList.contains('open')) {
                    navToggle.classList.remove('active');
                    navLinks.classList.remove('open');
                    document.body.style.overflow = '';
                }
            });
        });
    }

    // ── Mobile dropdown toggle ─────────────────────────────────────────────
    const dropdownTriggers = document.querySelectorAll('.nav-dropdown-trigger');
    dropdownTriggers.forEach(trigger => {
        trigger.addEventListener('click', (e) => {
            if (window.innerWidth <= 768) {
                e.preventDefault();
                trigger.closest('.nav-dropdown').classList.toggle('open');
            }
        });
    });

    // ── Scroll animations (data-aos) ───────────────────────────────────────
    const animatedEls = document.querySelectorAll('[data-aos]');
    if (animatedEls.length > 0) {
        const observer = new IntersectionObserver(
            (entries) => {
                entries.forEach(entry => {
                    if (entry.isIntersecting) {
                        entry.target.classList.add('visible');
                        observer.unobserve(entry.target);
                    }
                });
            },
            { threshold: 0.1, rootMargin: '0px 0px -40px 0px' }
        );
        animatedEls.forEach(el => observer.observe(el));
    }

    // ── Demo video tabs (SFSVC page) ────────────────────────────────────────
    const demoTabs = document.querySelectorAll('.demo-tab');
    const demoVideo = document.getElementById('demoVideo');

    if (demoTabs.length > 0 && demoVideo) {
        demoTabs.forEach(tab => {
            tab.addEventListener('click', () => {
                demoTabs.forEach(t => t.classList.remove('active'));
                tab.classList.add('active');

                const src = tab.getAttribute('data-video');
                if (src) {
                    demoVideo.pause();
                    demoVideo.querySelector('source').src = src;
                    demoVideo.load();
                    demoVideo.play().catch(() => { /* autoplay blocked */ });
                }
            });
        });
    }

    // ── Smooth scroll for anchor links ──────────────────────────────────────
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                e.preventDefault();
                target.scrollIntoView({ behavior: 'smooth', block: 'start' });
            }
        });
    });

})();
