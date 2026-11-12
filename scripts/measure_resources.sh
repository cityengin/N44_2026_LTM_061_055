#!/usr/bin/env bash
# Đo thời gian chạy và bộ nhớ đỉnh của các tác vụ chính (cho mục "Thời gian chạy và tài nguyên" của báo cáo).
# Ghi cấu hình máy và một dòng RESULT cho mỗi tác vụ vào $NOC_RESULTS/resources.txt (mặc định results/).
#
# KHÔNG thuộc reproduce.sh: thời gian chạy thay đổi giữa các lần và các máy, nên file này không so từng byte.
# Cần GNU time (/usr/bin/time, gói "time" trên Ubuntu) và binary noc profile release.
#
# Dùng: scripts/measure_resources.sh            (binary mặc định ~/ns-3-dev/build/scratch/noc/ns3.48-noc)
#       NOC_BIN=/đường/dẫn/noc scripts/measure_resources.sh

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${NOC_BIN:-$HOME/ns-3-dev/build/scratch/noc/ns3.48-noc}"
OUT_DIR="${NOC_RESULTS:-$REPO/results}"
OUT="$OUT_DIR/resources.txt"
REPEAT="${REPEAT:-3}"   # mỗi tác vụ chạy REPEAT lần, lấy trung vị thời gian và bộ nhớ lớn nhất

[[ -x "$BIN" ]] || { echo "không tìm thấy binary noc: $BIN (đặt NOC_BIN)" >&2; exit 1; }
[[ -x /usr/bin/time ]] || { echo "cần GNU time: sudo apt install time" >&2; exit 1; }
mkdir -p "$OUT_DIR"

{
    echo "# measure_resources.sh"
    echo "MACHINE,cpu=$(lscpu | sed -n 's/^Model name:[[:space:]]*//p' | head -1)"
    echo "MACHINE,threads=$(nproc)"
    echo "MACHINE,mem_gb=$(awk '/MemTotal/ {printf "%.1f", $2/1048576}' /proc/meminfo)"
    echo "MACHINE,os=$(. /etc/os-release && echo "$PRETTY_NAME"), kernel $(uname -r)"
    echo "MACHINE,compiler=$(c++ --version | head -1)"
    echo "MACHINE,binary=$(basename "$BIN")"
} > "$OUT"

# measure <tên> <tham số noc…>: chạy REPEAT lần, ghi RESULT,task=…,wall_s=…,maxrss_mb=…
measure() {
    local task="$1"; shift
    local times=() rss=0 t m
    for _ in $(seq "$REPEAT"); do
        /usr/bin/time -f "%e %M" -o "$OUT_DIR/.time" "$BIN" "$@" > /dev/null
        read -r t m < "$OUT_DIR/.time"
        times+=("$t")
        (( m > rss )) && rss=$m
    done
    local med
    med=$(printf '%s\n' "${times[@]}" | sort -g | awk '{a[NR]=$1} END {print a[int((NR+1)/2)]}')
    printf 'RESULT,task=%s,wall_s=%s,maxrss_mb=%s,repeat=%s\n' "$task" "$med" \
        "$(awk -v k="$rss" 'BEGIN {printf "%.1f", k/1024}')" "$REPEAT" | tee -a "$OUT"
}

measure phi_all      --mode=phi --routing=all --traffic=all
measure selftest     --mode=selftest
measure simcheck     --mode=simcheck
# Một lần chạy của sweep dưới ngưỡng (XY, transpose1, pir_ratio 0,9) và một lần quá tải (XY, uniform, pir_ratio 1,3).
measure sim_xy_t1_r0.9 --mode=sim --routing=XY --traffic=transpose1 --pir=0.0093753 \
    --warmup=10000 --measure=20000 --drain=20000 --RngRun=1
measure sim_xy_uni_r1.3 --mode=sim --routing=XY --traffic=uniform --pir=0.0464282 \
    --warmup=10000 --measure=20000 --drain=20000 --RngRun=1
rm -f "$OUT_DIR/.time"
echo "# đã ghi $OUT"
