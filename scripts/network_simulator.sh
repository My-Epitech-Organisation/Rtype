#!/bin/bash
#
# R-Type Network Condition Simulator
# Uses Linux tc (Traffic Control) to simulate network conditions
#
# Usage:
#   sudo ./scripts/network_simulator.sh <profile>
#
# Profiles:
#   - good       : Normal conditions
#   - mobile3g   : 3G mobile (384 Kbps, 200ms latency, 2% loss)
#   - mobile4g   : 4G mobile (10 Mbps, 50ms latency, 0.5% loss)
#   - wifi_bad   : Poor WiFi (1 Mbps, 30ms latency, 5% loss)
#   - stress     : Stress test (100 Kbps, 300ms latency, 10% loss)
#   - custom     : Custom settings via env vars
#   - reset      : Remove all traffic shaping
#
# Environment variables for custom:
#   BANDWIDTH=500kbit   (limit bandwidth)
#   LATENCY=100ms       (add latency)
#   LOSS=2%             (packet loss percentage)
#   DUPLICATE=1%        (packet duplication)
#   REORDER=5%          (packet reordering)
#

set -e

INTERFACE="${INTERFACE:-lo}"  # Default to loopback for local testing
PORT="${PORT:-4242}"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_root() {
    if [ "$EUID" -ne 0 ]; then
        log_error "This script requires root privileges"
        echo "Usage: sudo $0 <profile>"
        exit 1
    fi
}

show_help() {
    cat << EOF
🎮 R-Type Network Condition Simulator

Usage: sudo $0 <profile>

Profiles:
  good       - Ideal conditions (removes all shaping)
  mobile3g   - 3G mobile simulation (384 Kbps, 200ms, 2% loss)
  mobile4g   - 4G mobile simulation (10 Mbps, 50ms, 0.5% loss)
  wifi_bad   - Poor WiFi (1 Mbps, 30ms, 5% loss)
  wifi_good  - Good WiFi (50 Mbps, 5ms, 0.1% loss)
  dsl        - DSL connection (5 Mbps, 20ms, 0.5% loss)
  stress     - Stress test (100 Kbps, 300ms, 10% loss)
  minimal    - Minimal viable (50 Kbps, 500ms, 15% loss)
  custom     - Use environment variables (see below)
  reset      - Remove all traffic shaping
  status     - Show current network conditions

Environment Variables (for 'custom' profile):
  INTERFACE=lo        Network interface (default: lo)
  PORT=4242           UDP port to shape
  BANDWIDTH=500kbit   Bandwidth limit
  LATENCY=100ms       Added latency
  JITTER=10ms         Latency variation
  LOSS=2%             Packet loss
  DUPLICATE=1%        Packet duplication
  REORDER=5%          Packet reordering
  CORRUPT=0.1%        Packet corruption

Examples:
  sudo $0 mobile3g
  sudo BANDWIDTH=200kbit LATENCY=150ms $0 custom
  sudo INTERFACE=eth0 $0 wifi_bad

EOF
}

reset_tc() {
    log_info "Resetting traffic control on $INTERFACE..."
    tc qdisc del dev "$INTERFACE" root 2>/dev/null || true
    tc qdisc del dev "$INTERFACE" ingress 2>/dev/null || true
    log_success "Traffic shaping removed"
}

apply_netem() {
    local bandwidth="$1"
    local latency="$2"
    local jitter="${3:-0ms}"
    local loss="${4:-0%}"
    local duplicate="${5:-0%}"
    local reorder="${6:-0%}"
    local corrupt="${7:-0%}"

    # Reset first
    reset_tc 2>/dev/null || true

    log_info "Applying network conditions to $INTERFACE..."
    echo "  📊 Bandwidth:  $bandwidth"
    echo "  ⏱️  Latency:    $latency (±$jitter)"
    echo "  ❌ Loss:       $loss"
    echo "  📋 Duplicate:  $duplicate"
    echo "  🔀 Reorder:    $reorder"
    echo "  💥 Corrupt:    $corrupt"

    # Build netem command
    local netem_opts="delay $latency"
    [ "$jitter" != "0ms" ] && netem_opts="$netem_opts $jitter"
    [ "$loss" != "0%" ] && netem_opts="$netem_opts loss $loss"
    [ "$duplicate" != "0%" ] && netem_opts="$netem_opts duplicate $duplicate"
    [ "$reorder" != "0%" ] && netem_opts="$netem_opts reorder $reorder"
    [ "$corrupt" != "0%" ] && netem_opts="$netem_opts corrupt $corrupt"

    # Add root qdisc with bandwidth limit
    tc qdisc add dev "$INTERFACE" root handle 1: htb default 10
    tc class add dev "$INTERFACE" parent 1: classid 1:10 htb rate "$bandwidth" burst 15k

    # Add netem for latency/loss/etc
    tc qdisc add dev "$INTERFACE" parent 1:10 handle 10: netem $netem_opts

    # Filter for our port only (optional - comment out to affect all traffic)
    # tc filter add dev "$INTERFACE" protocol ip parent 1:0 prio 1 u32 \
    #     match ip dport "$PORT" 0xffff flowid 1:10

    log_success "Network conditions applied!"
}

show_status() {
    log_info "Current traffic control status on $INTERFACE:"
    echo ""
    echo "=== Qdiscs ==="
    tc qdisc show dev "$INTERFACE" 2>/dev/null || echo "  (none)"
    echo ""
    echo "=== Classes ==="
    tc class show dev "$INTERFACE" 2>/dev/null || echo "  (none)"
    echo ""
    echo "=== Filters ==="
    tc filter show dev "$INTERFACE" 2>/dev/null || echo "  (none)"
}

# Profile definitions
profile_good() {
    reset_tc
}

profile_mobile3g() {
    apply_netem "384kbit" "200ms" "50ms" "2%" "0.5%" "0%" "0%"
}

profile_mobile4g() {
    apply_netem "10mbit" "50ms" "10ms" "0.5%" "0.1%" "0%" "0%"
}

profile_wifi_bad() {
    apply_netem "1mbit" "30ms" "20ms" "5%" "1%" "2%" "0.1%"
}

profile_wifi_good() {
    apply_netem "50mbit" "5ms" "2ms" "0.1%" "0%" "0%" "0%"
}

profile_dsl() {
    apply_netem "5mbit" "20ms" "5ms" "0.5%" "0.1%" "0%" "0%"
}

profile_stress() {
    apply_netem "100kbit" "300ms" "100ms" "10%" "2%" "5%" "0.5%"
}

profile_minimal() {
    apply_netem "50kbit" "500ms" "200ms" "15%" "5%" "10%" "1%"
}

profile_custom() {
    local bw="${BANDWIDTH:-1mbit}"
    local lat="${LATENCY:-50ms}"
    local jit="${JITTER:-10ms}"
    local loss="${LOSS:-0%}"
    local dup="${DUPLICATE:-0%}"
    local reord="${REORDER:-0%}"
    local corr="${CORRUPT:-0%}"

    apply_netem "$bw" "$lat" "$jit" "$loss" "$dup" "$reord" "$corr"
}

# Main
main() {
    local profile="${1:-help}"

    case "$profile" in
        help|--help|-h)
            show_help
            exit 0
            ;;
        status)
            show_status
            exit 0
            ;;
        *)
            check_root
            ;;
    esac

    echo ""
    echo "🎮 R-Type Network Condition Simulator"
    echo "   Interface: $INTERFACE"
    echo "   Port: $PORT"
    echo ""

    case "$profile" in
        good|reset)
            profile_good
            ;;
        mobile3g|3g)
            profile_mobile3g
            ;;
        mobile4g|4g)
            profile_mobile4g
            ;;
        wifi_bad|wifibad)
            profile_wifi_bad
            ;;
        wifi_good|wifigood)
            profile_wifi_good
            ;;
        dsl)
            profile_dsl
            ;;
        stress)
            profile_stress
            ;;
        minimal)
            profile_minimal
            ;;
        custom)
            profile_custom
            ;;
        *)
            log_error "Unknown profile: $profile"
            echo "Run '$0 help' for available profiles"
            exit 1
            ;;
    esac

    echo ""
    log_info "To test, run the game and monitor with:"
    echo "   python3 scripts/bandwidth_monitor.py --duration 30"
    echo ""
    log_info "To reset network conditions:"
    echo "   sudo $0 reset"
}

main "$@"
