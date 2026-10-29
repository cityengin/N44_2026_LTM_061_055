#!/usr/bin/env bash
# Kiểm tra report/bao-cao.md (check_report.py) rồi xuất report/bao-cao.docx bằng pandoc.
# Dùng report/template.docx làm reference doc nếu có (tạo bằng scripts/make_template.py).
# Có lỗi kiểm tra thì dừng, KHÔNG tạo docx.
#
# Dùng: scripts/build_report.sh [--final]
#   --final: bản cuối; tài liệu chưa trích, hình chưa dùng, nhãn chưa tồn tại thành lỗi.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PY="$REPO/.venv/bin/python"
SRC="$REPO/report/bao-cao.md"
OUT="$REPO/report/bao-cao.docx"
TEMPLATE="$REPO/report/template.docx"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/noc-report.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

final=()
[[ "${1:-}" == "--final" ]] && final=(--final)

"$PY" "$REPO/scripts/check_report.py" "$SRC" "${final[@]}" --expand "$TMP/bao-cao.md"

args=(-f markdown -t docx --number-sections -M lang=vi-VN --resource-path="$REPO/report" -o "$OUT")
if [[ -f "$TEMPLATE" ]]; then
    args+=(--reference-doc="$TEMPLATE")
else
    echo "# không có $TEMPLATE: dùng style mặc định của pandoc"
fi
pandoc "$TMP/bao-cao.md" "${args[@]}"
echo "# đã ghi $OUT ($(du -h "$OUT" | cut -f1))"

echo "# Nhắc việc trước khi nộp:"
echo "  - Mở bằng Word, chọn mục lục rồi bấm Update Field (F9) để tạo số trang."
echo "  - Làm lại trang bìa trong Word: chèn logo trường, căn chỉnh theo mẫu của khoa."
echo "  - Điền các chỗ [ĐIỀN: …] còn lại:"
grep -n -o '\[ĐIỀN[^]]*\]' "$SRC" | sed 's/^/      dòng /' || echo "      (không còn)"
