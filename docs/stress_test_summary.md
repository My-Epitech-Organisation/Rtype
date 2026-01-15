**R‑Type Server Stress Test — Summary**

Overview
--------
- Purpose: realistic, reproducible stress testing for the R‑Type server using RTGP (UDP-like protocol with a small reliable layer).
- Tool: `scripts/stress_test.py` — asyncio-based UDP client orchestrator that can spawn many lightweight clients, simulate inputs, and stress reliable/unreliable channels.
- Protocol: See [RFC_RTGP_v1.4.3](RFC/RFC_RTGP_v1.4.3.md) for protocol details.

Key changes in the stress tester
--------------------------------
- Fixed RTGP packet header/packing (16‑byte header, magic 0xA1, struct '!BBHIHHBxxx').
- Removed dangerous keepalive that used `C_INPUT` (it triggered the server's start/stop loop).
- Added client ACKs for reliable packets and a PING keepalive to avoid idle timeouts.
- Implemented aggressive features: `--aggressive`, `--burst-size`, `--burst-interval`, `--churn-percent`, `--churn-interval`, and `--force-game-loop`.
- Implemented `_reliable_burst()` to send bursts of reliable `C_GET_USERS` requests and a churn loop to randomize disconnects/reconnects.

Test runs & findings (high level)
---------------------------------
- Environment: server run without Valgrind (real performance).
- Large aggressive run: 200 clients, 60s, `--aggressive --burst-size 200 --burst-interval 0.5`:
  - Packets sent: ~3.0M; received: ~361k; overall loss ~88%.
  - Reliable loss: ~92% (critical). Avg RTT ~1.7s — server could not keep up with reliable bursts.
- Parameter sweep (200 clients, 30s each):
  - Burst 25: reliable OK (server replies > client sends), avg RTT ~2.7s — GOOD
  - Burst 50: reliable OK, avg RTT ~3.9s — GOOD (higher latency)
  - Burst 100: reliable ~1.6% loss, avg RTT ~5.1s — acceptable but high latency
  - Burst 200: reliable ~9.3% loss, avg RTT ~6.2s — server struggled

What "aggressive mode" and "burst" mean
---------------------------------------
- Aggressive mode: a test configuration that ramps up frequency and scale of traffic to exercise server worst-case paths (extra pings, large bursts of reliable packets, optional churn and forced game-loop inputs). It is intentionally destructive and should be used with care.
- Burst: a short series of back‑to‑back reliable requests (the `--burst-size` number) sent by each client at the interval defined with `--burst-interval`. Bursts stress the server's reliable delivery queue and processing pipeline.

Interpretation & recommendations
--------------------------------
- The server's reliable-processing path is the bottleneck: latency and retry warnings indicate the server couldn't drain pending reliable sends.
- Operational suggestions:
  - Avoid per‑client bursts of 200 at 0.5s intervals for production; prefer burst ≤ 100 or increase interval.
  - Add client-side throttling/backoff for reliable bursts. Example: cap per-client bursts or exponential backoff when ACK RTT > 1s.
  - Instrument server reliable queue (enqueue/dequeue counters, queue depth, per-client pending counts) to find hotspots.
  - If necessary, coalesce or batch non-critical reliable requests server-side.

Quick reproduction commands
-------------------------
```bash
# Small validation run
python3 scripts/stress_test.py --clients 50 --duration 30 --ramp-up 0

# Sweep example
python3 scripts/stress_test.py --clients 200 --aggressive --burst-size 25 --burst-interval 0.5 --duration 30 --ramp-up 0
python3 scripts/stress_test.py --clients 200 --aggressive --burst-size 50 --burst-interval 0.5 --duration 30 --ramp-up 0
python3 scripts/stress_test.py --clients 200 --aggressive --burst-size 100 --burst-interval 0.5 --duration 30 --ramp-up 0
python3 scripts/stress_test.py --clients 200 --aggressive --burst-size 200 --burst-interval 0.5 --duration 30 --ramp-up 0
```

Next steps
----------
- Add optional client throttling/backoff in `scripts/stress_test.py` and re-run failing scenarios.
- Add server instrumentation around reliable queue processing.
- Run targeted sweeps for different client counts (100/200/400) and intervals to produce a capacity matrix.

If you'd like, I can: add the throttling implementation and re-run the 200‑client breaking test, or create a small README section describing safe defaults for CI use. Tell me which one to do next.

Generated: `docs/stress_test_summary.md`
