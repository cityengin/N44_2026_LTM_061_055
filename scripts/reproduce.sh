#!/usr/bin/env bash
# Tái lập toàn bộ kết quả từ đầu vào một thư mục tạm, rồi so từng byte với các file đã commit.
# KHÔNG BAO GIỜ ghi vào results/ hay report/ của repo.
#
# Thứ tự: build → selftest → simcheck → phi (--channels, --turns) → link_validate → sweep
#         → analyze_sweep → mdl_predict → heatmap → RESULTS.md → so sánh.
# Mỗi bước in thời gian chạy; dừng ngay ở bước đầu tiên fail.
# So sánh BẮT BUỘC (quyết định exit code): mọi file trong results/ được git theo dõi và report/RESULTS.md.
# So sánh THÔNG TIN: các hình report/figures/*.png.
#
# Dùng: scripts/reproduce.sh [thư_mục_tạm]
# Exit: 0 = mọi file bắt buộc giống từng byte; 1 = một bước fail; 3 = có file khác.

set -uo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NS3_DIR="${NS3_DIR:-$HOME/ns-3-dev}"
PY="$REPO/.venv/bin/python"
BIN="$NS3_DIR/build/scratch/noc/ns3.48-noc"
OUT="${1:-$(mktemp -d "${TMPDIR:-/tmp}/noc-reproduce.XXXXXX")}"

mkdir -p "$OUT/results" "$OUT/figures" "$OUT/report" "$OUT/logs"
export NOC_RESULTS="$OUT/results" NOC_FIGURES="$OUT/figures"

now() { date +%s.%N; }
elapsed() { awk -v a="$1" -v b="$(now)" 'BEGIN { printf "%.1f", b - a }'; }

T_ALL=$(now)
echo "# thư mục tạm: $OUT"

# step <tên> <lệnh...>: chạy lệnh, ghi log vào $OUT/logs/<tên>.log, in thời gian; fail thì dừng.
step() {
    local name="$1"
    shift
    local t0
    t0=$(now)
    if "$@" > "$OUT/logs/$name.log" 2>&1; then
        printf '%-14s OK   %7ss\n' "$name" "$(elapsed "$t0")"
    else
        printf '%-14s FAIL %7ss  (log: %s)\n' "$name" "$(elapsed "$t0")" "$OUT/logs/$name.log"
        tail -5 "$OUT/logs/$name.log"
        exit 1
    fi
}

check_profile() {
    # Kết quả đã commit được tạo bằng build release (-O3, không -march=native).
    grep -q '^build_profile_suffix:INTERNAL=$' "$NS3_DIR/cmake-cache/CMakeCache.txt" || {
        echo "ns-3 không ở profile release; chạy: cd $NS3_DIR && ./ns3 configure --build-profile=release --enable-examples"
        return 1
    }
}
build() { (cd "$NS3_DIR" && ./ns3 build noc); }
selftest() { "$BIN" --mode=selftest > "$NOC_RESULTS/selftest.txt"; }
simcheck() { "$BIN" --mode=simcheck > "$NOC_RESULTS/simcheck.txt"; }
phi() { "$BIN" --mode=phi --channels=1 --turns=1 > "$NOC_RESULTS/phi_all.txt"; }
py() { (cd "$REPO/scripts" && "$PY" "$@"); }

step profile check_profile
step build build
step selftest selftest
step simcheck simcheck
step phi phi
step link_validate py link_validate.py
step sweep py sweep.py
step analyze_sweep py analyze_sweep.py
step mdl_predict py mdl_predict.py
step heatmap py heatmap.py
step results_md py make_results_md.py --results "$NOC_RESULTS" --out "$OUT/report/RESULTS.md"
echo "tổng thời gian:      $(elapsed "$T_ALL")s"

# So sánh từng byte. In file và dòng đầu tiên khác; không ghi đè gì.
status=0
compare() {
    local committed="$1" fresh="$2" required="$3"
    if [[ ! -f "$fresh" ]]; then
        echo "THIẾU  $committed (không được tạo lại)"
        [[ "$required" == 1 ]] && status=3
    elif cmp -s "$REPO/$committed" "$fresh"; then
        echo "GIỐNG  $committed"
    else
        local line
        line=$(cmp "$REPO/$committed" "$fresh" 2>/dev/null | sed -n 's/.* line \([0-9]*\).*/\1/p')
        echo "KHÁC   $committed (dòng đầu tiên khác: ${line:-?})"
        if [[ "$committed" != *.png && -n "$line" ]]; then
            echo "  đã commit: $(sed -n "${line}p" "$REPO/$committed" | cut -c1-200)"
            echo "  tạo lại:   $(sed -n "${line}p" "$fresh" | cut -c1-200)"
        fi
        [[ "$required" == 1 ]] && status=3
    fi
}

echo "# so sánh bắt buộc"
while IFS= read -r f; do
    compare "$f" "$NOC_RESULTS/$(basename "$f")" 1
done < <(git -C "$REPO" ls-files results/)
compare report/RESULTS.md "$OUT/report/RESULTS.md" 1

echo "# so sánh thông tin (hình)"
while IFS= read -r f; do
    compare "$f" "$NOC_FIGURES/$(basename "$f")" 0
done < <(git -C "$REPO" ls-files 'report/figures/*.png')

if [[ $status == 0 ]]; then
    echo "KẾT QUẢ: mọi file bắt buộc giống từng byte"
else
    echo "KẾT QUẢ: có file khác; results/ và report/ KHÔNG bị ghi đè (bản tạo lại ở $OUT)"
fi
exit $status
