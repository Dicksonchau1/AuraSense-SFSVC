# ✅ AuraSense SFSVC Fleet Deployment Checklist

**Pre-Deployment Phase:** Week 1
**Pilot Phase:** Weeks 1-2
**Rollout Phase:** Weeks 3-8

---

## 📋 Pre-Deployment (Week 1)

### Sales & Validation
- [ ] Customer meets hardware requirements (pilot_benchmark.py)
- [ ] ROI analysis shared (customer_demo.py output)
- [ ] Pilot contract signed
- [ ] NDA executed if required
- [ ] Support contact assigned (DicksonChau@aurasensehk.com)
- [ ] Slack channel created for pilot

### Logistics
- [ ] Test server/hardware provisioned
- [ ] Network bandwidth verified (minimum 10Mbps)
- [ ] System access credentials provided securely
- [ ] VPN configured if remote
- [ ] Firewall rules validated (port 9090 for metrics)

### Documentation
- [ ] Custom integration guide written (if ROS/special)
- [ ] Deployment runbook created
- [ ] Emergency contact list shared
- [ ] Knowledge base article created

---

## 🧪 Pilot Phase (Weeks 1-2)

### Day 1: Installation & Setup
- [ ] SFSVC library installed successfully
- [ ] Unit tests pass (test_delta_spike_avx2)
- [ ] Configuration file customized
- [ ] Monitoring dashboard accessible
- [ ] Logs flowing to central store

### Days 2-5: Integration & Testing
- [ ] SFSVC connected to live camera feed
- [ ] Frame processing verified (no drops)
- [ ] Crack detection accuracy validated
- [ ] Performance baseline established
- [ ] Alerts configured for anomalies

### Days 6-10: Sustained Operation
- [ ] 48-72hr continuous run completed
- [ ] Reliability metrics documented
- [ ] Performance stable (<5% variance)
- [ ] Customer team trained on operations
- [ ] Success criteria verified

### Day 11-14: Handoff Preparation
- [ ] Final performance report generated
- [ ] Operational runbook reviewed and signed off
- [ ] Access transferred to customer ops team
- [ ] Support escalation process documented
- [ ] Green light for production rollout

### Validation Success Criteria
- [ ] ✅ 100% frame processing (zero drops)
- [ ] ✅ >95% crack detection accuracy
- [ ] ✅ P95 latency stable (within ±5%)
- [ ] ✅ Bandwidth reduction >90%
- [ ] ✅ 99%+ system uptime

---

## 🚀 Fleet Rollout Phase (Weeks 3-8)

### Phase 1: 5% Fleet (2 drones)
- [ ] SFSVC deployed to initial 2 drones
- [ ] Real-world performance validated
- [ ] Customer runs baseline measurement
- [ ] Cell coverage verified in all areas
- [ ] Network latency acceptable (<100ms)
- [ ] Monitoring dashboards live
- [ ] Daily health checks pass
- [ ] No customer escalations

**Go/No-Go Decision:** ✅ Pass

### Phase 2: 20% Fleet (6 drones)
- [ ] Roll out to next 4-6 drones
- [ ] Performance metrics consistent with pilot
- [ ] Bandwidth baseline updated
- [ ] ROI calculations validated
- [ ] Team comfort with operations high
- [ ] Zero unexpected issues

**Go/No-Go Decision:** ✅ Pass

### Phase 3: 100% Fleet (30 drones)
- [ ] Full fleet deployment complete
- [ ] All drones reporting metrics
- [ ] Backup systems operational
- [ ] Disaster recovery tested
- [ ] Cost savings calculation final
- [ ] Operations team independent

**Go/No-Go Decision:** ✅ Pass → Proceed to ops support

---

## 📊 Performance Milestones

### Baseline Targets
```
Latency:
  ✓ P50: < 0.20ms (bare-metal) / < 0.50ms (VM)
  ✓ P95: < 0.35ms (bare-metal) / < 0.60ms (VM)
  ✓ P99: < 0.80ms (bare-metal) / < 1.00ms (VM)

Throughput:
  ✓ Sustained: > 100 fps
  ✓ Burst: > 120 fps

Reliability:
  ✓ Frame drop rate: < 0.1%
  ✓ Uptime: > 99.5%
  ✓ False positive rate: < 0.3%
```

### Monitoring Dashboard
- [ ] Real-time latency distribution
- [ ] Per-drone throughput
- [ ] System health status
- [ ] Alert history
- [ ] Trend analysis (weekly)

---

## 💻 Technical Handoff

### Customer Operations Team Receives
- [ ] SFSVC source code & build instructions
- [ ] Docker images pre-built
- [ ] Configuration templates
- [ ] Monitoring setup (Prometheus/Grafana)
- [ ] Log parsing scripts
- [ ] Troubleshooting guide
- [ ] Emergency procedures

### Training Completed
- [ ] Installation & upgrades
- [ ] Configuration tuning
- [ ] Monitoring & alerting
- [ ] Troubleshooting common issues
- [ ] Performance optimization
- [ ] Disaster recovery

---

## 🔐 Security & Compliance

- [ ] SSL/TLS configured (if remote)
- [ ] Access logs retained (90 days)
- [ ] Secrets management (not in code/configs)
- [ ] Data retention policy defined
- [ ] Backup strategy implemented
- [ ] Compliance audit passed (if required)

---

## 📞 Support Transition

### Week 1-2: AuraSense Support
- [ ] 24/5 response time (business hours)
- [ ] Slack channel monitored
- [ ] Escalation path to engineering
- [ ] On-call available for critical issues

### Week 3+: Customer Self-Sufficient
- [ ] Customer operations team primary contact
- [ ] AuraSense on standby (advisory only)
- [ ] Email support for non-critical items
- [ ] Quarterly check-ins scheduled

---

## 💰 Financial Close-Out

### Pilot Phase
- [ ] Pilot fees invoiced (if applicable)
- [ ] Payment terms agreed (Net 30)
- [ ] ROI calculation signed off

### First Month
- [ ] Monthly operational fee invoiced
- [ ] Bandwidth savings validated
- [ ] Cost-benefit analysis shared with customer

### Recurring
- [ ] Monthly support invoiced
- [ ] Annual renewal option offered
- [ ] Volume discounts applied for fleet growth

---

## 🎯 Success Metrics Summary

**Pilot Success (Week 2)**
- [ ] ✅ Functional tests pass
- [ ] ✅ Performance within spec
- [ ] ✅ Accuracy validated
- [ ] ✅ Customer satisfied

**Phase 1 Success (Week 3-4)**
- [ ] ✅ 2 drones stable
- [ ] ✅ Performance consistent
- [ ] ✅ Zero critical issues
- [ ] ✅ Ready for phase 2

**Phase 2 Success (Week 5-6)**
- [ ] ✅ 8 drones stable
- [ ] ✅ Team comfortable with ops
- [ ] ✅ Scaling proven
- [ ] ✅ Ready for phase 3

**Phase 3 Success (Week 7-8)**
- [ ] ✅ 30 drones deployed
- [ ] ✅ Monthly ROI realized
- [ ] ✅ Customer independent
- [ ] ✅ Renewal discussion started

---

## 🚨 Rollback Plan (If Needed)

**Quick Rollback (< 1 hour):**
```bash
# Disable SFSVC, resume H.265-only
for drone in {1..30}; do
    ssh drone-$drone "systemctl disable aurasense"
    ssh drone-$drone "systemctl restart media-ingestion"
done
```

**Conditions for Rollback:**
- [ ] Accuracy drops below 95%
- [ ] P95 latency exceeds 2ms
- [ ] Frame drop rate > 1%
- [ ] Customer requests pause
- [ ] Critical security issue found

---

## 📅 Timeline Template

```
Week 1 (Days 1-5):    Pre-deployment setup
Day 1:               Installation ✓
Days 2-5:            Integration testing
Days 6-10:           Sustained pilot
Days 11-14:          Validation & sign-off

Week 3-8:            Progressive fleet rollout
Week 3-4 (Phase 1):  5% fleet (2 drones) → Go/No-Go
Week 5-6 (Phase 2):  20% fleet (8 drones) → Go/No-Go
Week 7-8 (Phase 3):  100% fleet (30 drones) → Production
```

---

## 📋 Sign-Off

**Customer:**
- [ ] Deployment manager signature: _____________
- [ ] Operations lead signature: _____________
- [ ] Date: _____________

**AuraSense:**
- [ ] Deployment lead signature: _____________
- [ ] Director of Customer Success: _____________
- [ ] Date: _____________

---

**Document Version:** 1.0
**Last Updated:** 2026-02-24
**Contact:** DicksonChau@aurasensehk.com
