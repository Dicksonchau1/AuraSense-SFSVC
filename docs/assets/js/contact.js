/* ═══════════════════════════════════════════════════════════════════════════
   AuraSense Corporate — Contact Form Handler
   ═══════════════════════════════════════════════════════════════════════════ */

(function () {
    'use strict';

    const form = document.getElementById('contactForm');
    const successMsg = document.getElementById('formSuccess');
    const submitBtn = document.getElementById('submitBtn');

    if (!form) return;

    // ── Pre-fill topic from URL param (?topic=sfsvc) ───────────────────────
    const params = new URLSearchParams(window.location.search);
    const topicParam = params.get('topic');
    if (topicParam) {
        const topicSelect = document.getElementById('topic');
        if (topicSelect) {
            const opt = topicSelect.querySelector(`option[value="${topicParam}"]`);
            if (opt) {
                opt.selected = true;
            }
        }
    }

    // ── Form submission ────────────────────────────────────────────────────
    form.addEventListener('submit', async function (e) {
        e.preventDefault();

        // Basic validation
        const required = form.querySelectorAll('[required]');
        let valid = true;
        required.forEach(field => {
            if (!field.value.trim()) {
                field.style.borderColor = '#EF4444';
                valid = false;
            } else {
                field.style.borderColor = '';
            }
        });

        if (!valid) return;

        // Submit
        submitBtn.disabled = true;
        submitBtn.innerHTML = 'Sending...';

        try {
            const data = new FormData(form);
            const response = await fetch('/', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: new URLSearchParams(data).toString()
            });

            if (response.ok) {
                form.style.display = 'none';
                if (successMsg) successMsg.style.display = 'block';
            } else {
                throw new Error('Submit failed');
            }
        } catch (err) {
            // Fallback: open mailto
            const name = form.querySelector('#firstName').value + ' ' + form.querySelector('#lastName').value;
            const email = form.querySelector('#email').value;
            const topic = form.querySelector('#topic').value;
            const message = form.querySelector('#message').value;
            const company = form.querySelector('#company').value;

            const subject = encodeURIComponent(`[${topic}] Inquiry from ${name}`);
            const body = encodeURIComponent(
                `Name: ${name}\nEmail: ${email}\nCompany: ${company}\nTopic: ${topic}\n\n${message}`
            );

            window.location.href = `mailto:support@aurasensehk.com?subject=${subject}&body=${body}`;

            submitBtn.disabled = false;
            submitBtn.innerHTML = 'Send Message <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="22" y1="2" x2="11" y2="13"/><polygon points="22 2 15 22 11 13 2 9 22 2"/></svg>';
        }
    });

    // ── Clear error styling on input ───────────────────────────────────────
    form.querySelectorAll('input, select, textarea').forEach(field => {
        field.addEventListener('input', () => {
            field.style.borderColor = '';
        });
    });

})();
