"""Hậu xử lý report/bao-cao.docx sau pandoc (build_report.sh gọi).

Pandoc dùng cùng style "Compact" cho ô bảng và cho mục của danh sách gọn (gạch đầu dòng, đánh số, mã giả). Ô bảng
cần chữ nhỏ và giãn dòng đơn, còn danh sách phải giống thân bài (Times New Roman 13, giãn dòng 1,5, căn đều). Script
đổi style của các đoạn Compact có đánh số (w:numPr) sang "ListCompact", định nghĩa trong report/template.docx
(scripts/make_template.py). Ô bảng giữ nguyên Compact.

Dùng: python scripts/fix_docx.py report/bao-cao.docx
"""

from __future__ import annotations

import re
import sys
import zipfile
from pathlib import Path

LIST_PARA = re.compile(r'<w:pPr>(<w:numPr>(?:(?!</w:numPr>).)*</w:numPr>)<w:pStyle w:val="Compact" ?/>', re.S)


def fix_document(xml: str) -> tuple[str, int]:
    return LIST_PARA.subn(r'<w:pPr><w:pStyle w:val="ListCompact" />\1', xml)


def main() -> int:
    path = Path(sys.argv[1])
    tmp = path.with_suffix(".fix.docx")
    n = 0
    with zipfile.ZipFile(path) as zin, zipfile.ZipFile(tmp, "w", zipfile.ZIP_DEFLATED) as zout:
        styles = zin.read("word/styles.xml").decode("utf-8")
        if 'w:styleId="ListCompact"' not in styles:
            print("fix_docx: template không có style ListCompact; chạy scripts/make_template.py", file=sys.stderr)
            return 1
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == "word/document.xml":
                xml, n = fix_document(data.decode("utf-8"))
                data = xml.encode("utf-8")
            zout.writestr(item, data, compress_type=zipfile.ZIP_DEFLATED)
    tmp.replace(path)
    print(f"# fix_docx: {n} đoạn danh sách đổi sang style ListCompact")
    return 0


if __name__ == "__main__":
    sys.exit(main())
