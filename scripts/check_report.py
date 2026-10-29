"""Kiểm tra report/bao-cao.md trước khi xuất docx (bước 8). Exit 1 nếu có lỗi.

Lỗi:
  - hình: đường dẫn không tồn tại, hoặc file hình không có trong comment nguồn;
  - đánh số "Hình c.k:", "Bảng c.k:", "Đoạn mã c.k:": c khác số chương chứa nó, k không liên tục từ 1,
    trùng nhãn; hình/bảng trong chương đánh số mà không có nhãn; heading tự gõ số (pandoc đã đánh số);
  - nhãn không được nhắc trong văn bản (ngoài caption);
  - thiếu <!-- nguồn: f1, f2, <commit> --> ngay trước hình/bảng/đoạn mã; file hoặc commit không tồn tại;
    file hiện tại khác bản ở commit đó (git diff <commit> -- f);
  - số thập phân (dấu phẩy; hàng nghìn tách bằng U+00A0):
      trong BẢNG: phải khớp một số trong report/RESULTS.md hoặc results/* (sau khi làm tròn tới đúng
      số chữ số thập phân đã viết);
      trong VĂN BẢN: khớp như trên, HOẶC xuất hiện nguyên văn trong <!-- suy-ra: ... --> đặt ngay trước đoạn;
  - số thập phân viết bằng dấu chấm; số ≥ 5 chữ số không tách hàng nghìn bằng U+00A0;
  - trích dẫn [n] không có trong "Tài liệu tham khảo", hoặc danh mục không đánh số liên tục từ 1;
  - dòng của đoạn mã không có trong file nguồn ở commit được ghi (dòng "..." được bỏ qua).
Cảnh báo (thành lỗi khi --final): mục tài liệu không được trích, hình trong report/figures/ chưa dùng,
nhắc tới nhãn chưa tồn tại. Cảnh báo luôn: số trang ước lượng ngoài 25–35.

Phần không kiểm số: công thức ($…$, $$…$$), mã (`…`, khối ```), comment HTML, URL, mục "Tài liệu tham khảo".

--expand OUT: ghi bản md cho pandoc, thay <!-- MUC-LUC --> bằng field mục lục của Word và
<!-- DANH-MUC-HINH -->, <!-- DANH-MUC-BANG -->, <!-- DANH-MUC-DOAN-MA --> bằng danh mục lấy từ caption.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass, field
from decimal import ROUND_HALF_EVEN, ROUND_HALF_UP, Decimal
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
NBSP = "\u00a0"
MINUS = "\u2212"

# Hằng số của mô hình, không phải kết quả đo hay tính (giữ danh sách này ngắn).
EXEMPT = {
    "0,0625": "PIR_max = 0,0625/φ: frate/len = 0,5/8 (quy ước của bài báo [1])",
    "0,5": "frate = 0,5 flit/chu kỳ (quy ước của bài báo [1])",
}

REF_HEADING = "Tài liệu tham khảo"
# Số trong VĂN BẢN được đối chiếu với các file tổng hợp này, cộng các file ghi trong <!-- nguồn: … --> đặt ngay
# trước đoạn. Số trong BẢNG chỉ đối chiếu với RESULTS.md và các file trong comment nguồn của bảng. File dữ liệu lớn
# (sweep.csv, link_validate_channels.csv, phi_all.txt…) chứa hàng nghìn số, khớp ngẫu nhiên quá dễ, nên phải được
# ghi rõ trong comment nguồn mới được dùng.
PROSE_SOURCES = ["report/RESULTS.md", "results/link_validate_summary.csv", "results/link_validate_z.csv",
                 "results/saturation.csv", "results/selftest.txt", "results/simcheck.txt"]
# Nhãn, số mục và số phiên bản: không phải số thập phân, bỏ trước khi kiểm số.
NONDECIMAL_SPANS = [
    re.compile(r"(?:Hình|Bảng|Đoạn mã|[Mm]ục|[Cc]hương|Fig\.|Figs\.|Table|Definition|§)[ \u00a0]*"
               r"\d+(?:\.\d+)*(?:[ \u00a0]*(?:–|-|,|và|đến|hoặc)[ \u00a0]*\d+(?:\.\d+)*)*"),
    re.compile(r"ns-3\.\d+"),
    re.compile(r"(?<![\w.])\d+(?:\.\d+){2,}(?![\w.])"),  # x.y.z
]
CODE_SPAN = re.compile(r"`[^`]*`")
MATH_SPAN = re.compile(r"\$[^$]+\$")
URL = re.compile(r"https?://\S+")
COMMENT_SPAN = re.compile(r"<!--.*?-->")
DECIMAL = re.compile(r"(?<![\d,.\u00a0])(\d{1,3}(?:\u00a0\d{3})+|\d+),(\d+)(?![\d])")
DOT_DECIMAL = re.compile(r"(?<![\w.])\d+\.\d+(?![\w.])")
UNGROUPED = re.compile(r"(?<![\w.,\u00a0])\d{5,}(?![\w])")
CITE = re.compile(r"\[(\d+)\]")
MENTION = re.compile(r"(Hình|Bảng|Đoạn mã)[ \u00a0](\d+\.\d+)")
NGUON = re.compile(r"<!--\s*nguồn:\s*(.+?)\s*-->", re.S)
SUYRA = re.compile(r"<!--\s*suy-ra:\s*(.+?)\s*-->", re.S)
FIG = re.compile(r"^!\[(Hình (\d+)\.(\d+)): (.+)\]\(([^)\s]+)\)(\{[^}]*\})?\s*$")
TABCAP = re.compile(r"^Table: (Bảng (\d+)\.(\d+)): (.+)$")
CODECAP = re.compile(r"^(Đoạn mã (\d+)\.(\d+)): (.+)$")
COMMIT = re.compile(r"^[0-9a-f]{7,40}$")


@dataclass
class Block:
    kind: str  # heading, text, table, tabcap, figure, code, comment, math, div
    line: int  # dòng đầu (đánh số từ 1)
    lines: list[str]
    chapter: int = 0  # 0 = phần không đánh số
    section: str = ""  # tiêu đề heading cấp 1 đang chứa block
    comments: list[str] = field(default_factory=list)  # comment liền trước (bỏ qua div)

    @property
    def text(self) -> str:
        return "\n".join(self.lines)


@dataclass
class Label:
    name: str  # "Hình 4.1"
    kind: str
    chapter: int
    num: int
    caption: str
    line: int


class Report:
    def __init__(self) -> None:
        self.errors: list[str] = []
        self.warnings: list[str] = []

    def err(self, line: int, msg: str) -> None:
        self.errors.append(f"dòng {line}: {msg}")

    def warn(self, line: int, msg: str) -> None:
        self.warnings.append(f"dòng {line}: {msg}" if line else msg)


def git(*args: str) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "-C", str(REPO), *args], capture_output=True, text=True)


def parse_blocks(lines: list[str]) -> list[Block]:
    blocks: list[Block] = []
    i, n = 0, len(lines)
    if lines and lines[0].strip() == "---":  # YAML
        i = 1
        while i < n and lines[i].strip() not in ("---", "..."):
            i += 1
        i += 1
    while i < n:
        line = lines[i]
        s = line.strip()
        if not s:
            i += 1
            continue
        start = i
        m = re.match(r"^(`{3,}|~{3,})", s)
        if m:
            fence = m.group(1)
            i += 1
            while i < n and not lines[i].strip().startswith(fence):
                i += 1
            blocks.append(Block("code", start + 1, lines[start:i + 1]))
            i += 1
        elif s.startswith("<!--"):
            while i < n and "-->" not in lines[i]:
                i += 1
            blocks.append(Block("comment", start + 1, lines[start:i + 1]))
            i += 1
        elif s.startswith("$$"):
            if s.count("$$") >= 2 and len(s) > 2:
                i += 1
            else:
                i += 1
                while i < n and "$$" not in lines[i]:
                    i += 1
                i += 1
            blocks.append(Block("math", start + 1, lines[start:i]))
        elif s.startswith(":::"):
            blocks.append(Block("div", start + 1, [line]))
            i += 1
        elif s.startswith("#"):
            blocks.append(Block("heading", start + 1, [line]))
            i += 1
        else:
            while i < n and lines[i].strip() and not re.match(r"^(`{3,}|~{3,}|<!--|:::|#)", lines[i].strip()):
                i += 1
            chunk = lines[start:i]
            first = chunk[0].lstrip()
            if first.startswith("|"):
                kind = "table"
            elif first.startswith("Table:"):
                kind = "tabcap"
            elif first.startswith("!["):
                kind = "figure"
            else:
                kind = "text"
            blocks.append(Block(kind, start + 1, chunk))
    # Gắn chương và comment liền trước.
    chapter, section = 0, ""
    pending: list[str] = []
    for b in blocks:
        if b.kind == "heading" and b.lines[0].startswith("# "):
            unnumbered = re.search(r"\{[^}]*(-|\.unnumbered)[^}]*\}\s*$", b.lines[0]) is not None
            section = re.sub(r"\s*\{.*\}\s*$", "", b.lines[0][2:]).strip()
            chapter = 0 if unnumbered else chapter_counter(blocks, b)
        b.chapter, b.section = chapter, section
        if b.kind == "comment":
            pending.append(b.text)
            continue
        if b.kind == "div":
            continue
        b.comments = pending
        pending = []
    return blocks


def chapter_counter(blocks: list[Block], upto: Block) -> int:
    """Số thứ tự của heading cấp 1 có đánh số `upto` (pandoc --number-sections)."""
    k = 0
    for b in blocks:
        if b.kind == "heading" and b.lines[0].startswith("# ") and \
                re.search(r"\{[^}]*(-|\.unnumbered)[^}]*\}\s*$", b.lines[0]) is None:
            k += 1
        if b is upto:
            return k
    raise AssertionError


def strip_nonnumeric(text: str) -> str:
    """Bỏ mã, công thức, URL, comment và nhãn/số mục/phiên bản (không phải số thập phân)."""
    for pat in (COMMENT_SPAN, CODE_SPAN, MATH_SPAN, URL):
        text = pat.sub(" ", text)
    for pat in NONDECIMAL_SPANS:
        text = pat.sub(" ", text)
    return text


def decimals_in(text: str) -> list[tuple[str, str]]:
    """[(chuỗi như đã viết, giá trị chuẩn hóa "−1586.25" → "-1586.25")]."""
    out = []
    for m in DECIMAL.finditer(text):
        intpart = m.group(1).replace(NBSP, "")
        written = m.group(0)
        sign = ""
        pre = text[:m.start()]
        if pre.endswith(MINUS):
            sign, written = "-", MINUS + written
        elif pre.endswith("-") and (len(pre) == 1 or pre[-2] in " (|[\u00a0"):
            sign, written = "-", "-" + written
        value = f"{sign}{intpart}.{m.group(2)}"
        if Decimal(value) == 0:
            value = value.lstrip("-")
        out.append((written, value))
    return out


def source_numbers(f: Path) -> set[str]:
    """Mọi số trong file, làm tròn tới 0..8 chữ số thập phân (half-up và half-even)."""
    vals: set[str] = set()
    num = re.compile(r"[-+]?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?")
    for tok in set(num.findall(f.read_text(encoding="utf-8", errors="replace"))):
        try:
            d = Decimal(tok)
        except Exception:  # noqa: BLE001
            continue
        if not d.is_finite():
            continue
        for k in range(0, 9):
            q = Decimal(1).scaleb(-k)
            for mode in (ROUND_HALF_UP, ROUND_HALF_EVEN):
                r = f"{d.quantize(q, rounding=mode):f}"
                vals.add(r.lstrip("-") if Decimal(r) == 0 else r)
    return vals


def check(md: Path, final: bool, audit: list | None = None) -> tuple[Report, list[Block], list[Label]]:
    rep = Report()
    lines = md.read_text(encoding="utf-8").splitlines()
    blocks = parse_blocks(lines)

    vals_cache: dict[str, set[str]] = {}

    def vals_of(f: str) -> set[str]:
        if f not in vals_cache:
            path = REPO / f
            ok = path.is_file() and (f.startswith("results/") or f == "report/RESULTS.md")
            vals_cache[f] = source_numbers(path) if ok else set()
        return vals_cache[f]

    labels: list[Label] = []
    counters: dict[tuple[str, int], int] = {}
    used_figs: set[str] = set()
    git_ok: dict[tuple[str, str], bool] = {}

    def check_nguon(b: Block, what: str, must_include: str | None = None,
                    required: bool = True) -> list[tuple[str, str]]:
        ng = [m for c in b.comments for m in NGUON.findall(c)]
        if not ng:
            if required:
                rep.err(b.line, f"{what}: thiếu <!-- nguồn: file, commit --> ngay trước")
            return []
        parts = [p.strip() for p in ng[-1].split(",")]
        commit, files = parts[-1], parts[:-1]
        if not COMMIT.match(commit) or not files:
            rep.err(b.line, f"{what}: comment nguồn phải có dạng 'file[, file…], <commit>'")
            return []
        if git("cat-file", "-e", f"{commit}^{{commit}}").returncode != 0:
            rep.err(b.line, f"{what}: commit {commit} không tồn tại")
            return []
        for f in files:
            key = (commit, f)
            if key not in git_ok:
                git_ok[key] = (git("cat-file", "-e", f"{commit}:{f}").returncode == 0
                               and git("diff", "--quiet", commit, "--", f).returncode == 0)
            if not (REPO / f).exists():
                rep.err(b.line, f"{what}: file nguồn {f} không tồn tại")
            elif not git_ok[key]:
                rep.err(b.line, f"{what}: {f} ở commit {commit} khác bản hiện tại (hoặc chưa có ở commit đó)")
        if must_include and must_include not in files:
            rep.err(b.line, f"{what}: comment nguồn phải liệt kê {must_include}")
        return [(commit, f) for f in files]

    def add_label(b: Block, kind: str, name: str, ch: int, k: int, caption: str) -> None:
        if b.chapter == 0:
            rep.err(b.line, f"{name}: nằm trong phần không đánh số")
        elif ch != b.chapter:
            rep.err(b.line, f"{name}: số chương {ch} khác chương hiện tại {b.chapter}")
        expect = counters.get((kind, b.chapter), 0) + 1
        if k != expect:
            rep.err(b.line, f"{name}: số thứ tự phải là {b.chapter}.{expect}")
        counters[(kind, b.chapter)] = max(k, counters.get((kind, b.chapter), 0))
        if any(lb.name == name for lb in labels):
            rep.err(b.line, f"{name}: trùng nhãn")
        labels.append(Label(name, kind, ch, k, caption, b.line))

    def check_numbers(b: Block, text: str, strict: bool, files: list[str] = ()) -> None:
        clean = strip_nonnumeric(text)
        for m in DOT_DECIMAL.finditer(clean):
            rep.err(b.line, f"'{m.group(0)}': số thập phân phải dùng dấu phẩy")
        for m in UNGROUPED.finditer(clean):
            rep.err(b.line, f"'{m.group(0)}': số ≥ 5 chữ số phải tách hàng nghìn bằng U+00A0")
        suy = " ".join(m for c in b.comments for m in SUYRA.findall(c))
        cands = ["report/RESULTS.md", *files] + ([] if strict else PROSE_SOURCES)
        for written, value in decimals_in(clean):
            hit = next((f for f in cands if value in vals_of(f)), None)
            if hit is None and written.lstrip("-" + MINUS) in EXEMPT:
                hit = "EXEMPT"
            if hit is None and not strict and written.lstrip("-" + MINUS) in suy:
                hit = "suy-ra"
            if audit is not None:
                audit.append((b.line, written, hit or "-"))
            if hit is None:
                where = "bảng" if strict else "văn bản"
                hint = "" if strict else " và không có trong <!-- suy-ra: … --> ngay trước đoạn"
                rep.err(b.line, f"{where}: số {written} không khớp {', '.join(dict.fromkeys(cands))}{hint}")

    in_refs = False
    ref_entries: dict[int, int] = {}
    cites: dict[int, int] = {}
    mentions: dict[str, int] = {}
    prev_codecap: tuple[Block, list[tuple[str, str]]] | None = None
    table_files: list[str] = []
    figures_dir = REPO / "report"

    for idx, b in enumerate(blocks):
        if b.kind == "heading":
            h = b.lines[0]
            if h.startswith("# "):
                in_refs = b.section == REF_HEADING
            if re.match(r"^#+\s+\d", h):
                rep.err(b.line, "heading không được tự gõ số (pandoc --number-sections đánh số)")
            if not in_refs:
                check_numbers(b, h, strict=False)
            continue
        if b.kind in ("comment", "div", "math"):
            continue
        if b.kind == "code":
            if prev_codecap is not None:
                cap, srcs = prev_codecap
                body = [ln.strip() for ln in b.lines[1:-1]]
                texts = []
                for commit, f in srcs:
                    r = git("show", f"{commit}:{f}")
                    texts.append({ln.strip() for ln in r.stdout.splitlines()})
                for off, ln in enumerate(body):
                    if not ln or ln in ("...", "…", "// ...", "// …"):
                        continue
                    if not any(ln in t for t in texts):
                        rep.err(b.line + 1 + off, f"đoạn mã: dòng không có trong nguồn: {ln[:80]}")
                prev_codecap = None
            continue
        text = b.text
        if b.kind == "figure":
            m = FIG.match(text.strip())
            if not m:
                rep.err(b.line, "hình phải có dạng ![Hình c.k: chú thích](figures/x.png){width=…}")
                continue
            name, ch, k, cap, path = m.group(1), int(m.group(2)), int(m.group(3)), m.group(4), m.group(5)
            add_label(b, "Hình", name, ch, k, cap)
            if not (figures_dir / path).exists():
                rep.err(b.line, f"{name}: không có file {path}")
            used_figs.add(Path(path).name)
            srcs = check_nguon(b, name, must_include=f"report/{path}")
            check_numbers(b, cap, strict=False, files=[f for _, f in srcs])
            continue
        if b.kind == "tabcap":
            m = TABCAP.match(b.lines[0].strip())
            if not m:
                rep.err(b.line, "caption bảng phải có dạng 'Table: Bảng c.k: …'")
                continue
            name = m.group(1)
            add_label(b, "Bảng", name, int(m.group(2)), int(m.group(3)), m.group(4))
            table_files = [f for _, f in check_nguon(b, name)]
            check_numbers(b, m.group(4), strict=False, files=table_files)
            nxt = blocks[idx + 1] if idx + 1 < len(blocks) else None
            if nxt is None or nxt.kind != "table":
                rep.err(b.line, f"{name}: ngay sau caption phải là bảng")
            continue
        if b.kind == "table":
            prev = blocks[idx - 1] if idx else None
            if b.chapter and (prev is None or prev.kind != "tabcap"):
                rep.err(b.line, "bảng trong chương đánh số phải có caption 'Table: Bảng c.k: …' ngay trước")
            for ln in b.lines:
                for c in CITE.findall(ln):
                    cites.setdefault(int(c), b.line)
                for mm in MENTION.finditer(ln):
                    mentions.setdefault(f"{mm.group(1)} {mm.group(2)}", b.line)
            check_numbers(b, text, strict=True, files=table_files if prev and prev.kind == "tabcap" else [])
            continue
        # text
        m = CODECAP.match(text.strip())
        if m:
            name = m.group(1)
            add_label(b, "Đoạn mã", name, int(m.group(2)), int(m.group(3)), m.group(4))
            srcs = check_nguon(b, name)
            prev_codecap = (b, srcs)
            nxt = next((x for x in blocks[idx + 1:] if x.kind != "div"), None)
            if nxt is None or nxt.kind != "code":
                rep.err(b.line, f"{name}: ngay sau caption phải là khối mã")
            continue
        if in_refs:
            mm = re.match(r"^\[(\d+)\]\s", text)
            if mm:
                ref_entries[int(mm.group(1))] = b.line
            continue
        plain = COMMENT_SPAN.sub(" ", text)
        for c in CITE.findall(CODE_SPAN.sub(" ", plain)):
            cites.setdefault(int(c), b.line)
        for mm in MENTION.finditer(plain):
            mentions.setdefault(f"{mm.group(1)} {mm.group(2)}", b.line)
        srcs = check_nguon(b, "đoạn văn", required=False)
        check_numbers(b, text, strict=False, files=[f for _, f in srcs])

    # Nhãn được nhắc?
    names = {lb.name for lb in labels}
    for lb in labels:
        if lb.name not in mentions:
            rep.err(lb.line, f"{lb.name} không được nhắc trong văn bản")
    for name, line in sorted(mentions.items(), key=lambda x: x[1]):
        if name not in names:
            (rep.err if final else rep.warn)(line, f"nhắc tới {name} nhưng chưa có nhãn này")

    # Tài liệu tham khảo.
    if ref_entries:
        if sorted(ref_entries) != list(range(1, len(ref_entries) + 1)):
            rep.err(min(ref_entries.values()), "tài liệu tham khảo phải đánh số liên tục từ [1]")
    for c, line in sorted(cites.items()):
        if c not in ref_entries:
            rep.err(line, f"trích dẫn [{c}] không có trong '{REF_HEADING}'")
    for r, line in sorted(ref_entries.items()):
        if r not in cites:
            (rep.err if final else rep.warn)(line, f"tài liệu [{r}] chưa được trích trong văn bản")

    # Hình chưa dùng.
    for p in sorted((REPO / "report" / "figures").glob("*.png")):
        if p.name not in used_figs:
            (rep.err if final else rep.warn)(0, f"report/figures/{p.name} chưa được dùng")

    return rep, blocks, labels


def estimate_pages(blocks: list[Block], n_labels: int = 0) -> tuple[int, float]:
    """Ước lượng thô số trang Word (TNR 13, giãn dòng 1,5, A4, lề 3/2/2/2 cm). Chỉ để định hướng độ dài."""
    words = sum(len(strip_nonnumeric(b.text).split()) for b in blocks if b.kind in ("text", "heading"))
    words += sum(len(MATH_SPAN.sub(" x ", b.text).split()) for b in blocks if b.kind == "math")
    pages = words / 480  # khổ chữ 16 cm: ~30 dòng × ~16 từ
    pages += 0.5 * sum(b.kind == "figure" for b in blocks)
    pages += sum(0.045 * len(b.lines) + 0.15 for b in blocks if b.kind == "table")
    pages += sum(0.018 * len(b.lines) + 0.1 for b in blocks if b.kind == "code")  # cỡ 9,5, cách dòng đơn
    pages += sum(0.08 for b in blocks if b.kind == "heading")
    # Mỗi heading cấp 1 sang trang mới: trung bình bỏ trống nửa trang.
    pages += 0.5 * sum(b.kind == "heading" and b.lines[0].startswith("# ") for b in blocks)
    pages += 1.0 + 2.0 + 0.03 * n_labels  # bìa, mục lục (field), danh mục hình/bảng/đoạn mã
    return words, pages


TOC_XML = """```{=openxml}
<w:p><w:pPr><w:pStyle w:val="TOCHeading"/></w:pPr><w:r><w:t>Mục lục</w:t></w:r></w:p>
<w:p><w:r><w:fldChar w:fldCharType="begin" w:dirty="true"/></w:r>\
<w:r><w:instrText xml:space="preserve">TOC \\o "1-3" \\h \\z \\u</w:instrText></w:r>\
<w:r><w:fldChar w:fldCharType="separate"/></w:r>\
<w:r><w:t>Mở bằng Word và chọn Update Field (F9) để tạo mục lục.</w:t></w:r>\
<w:r><w:fldChar w:fldCharType="end"/></w:r></w:p>
```"""


def expand(md: Path, labels: list[Label], out: Path) -> None:
    def listing(kind: str) -> str:
        items = [lb for lb in labels if lb.kind == kind]
        body = "\n\n".join(f"{lb.name}: {lb.caption}" for lb in items)
        return f'::: {{custom-style="List Entry"}}\n{body}\n:::'

    text = md.read_text(encoding="utf-8")
    text = text.replace("<!-- MUC-LUC -->", TOC_XML)
    text = text.replace("<!-- DANH-MUC-HINH -->", listing("Hình"))
    text = text.replace("<!-- DANH-MUC-BANG -->", listing("Bảng"))
    text = text.replace("<!-- DANH-MUC-DOAN-MA -->", listing("Đoạn mã"))
    out.write_text(text, encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("md", nargs="?", default=str(REPO / "report" / "bao-cao.md"))
    ap.add_argument("--final", action="store_true", help="bản cuối: cảnh báo về độ đầy đủ thành lỗi")
    ap.add_argument("--expand", metavar="OUT", help="ghi bản md đã chèn mục lục và danh mục")
    ap.add_argument("--audit", action="store_true", help="in từng số thập phân và file nguồn khớp với nó")
    args = ap.parse_args()
    md = Path(args.md)
    audit: list | None = [] if args.audit else None
    rep, blocks, labels = check(md, args.final, audit)
    for line, written, hit in audit or []:
        print(f"SỐ dòng {line}: {written} ← {hit}")
    words, pages = estimate_pages(blocks, len(labels))
    for w in rep.warnings:
        print(f"CẢNH BÁO {w}")
    for e in rep.errors:
        print(f"LỖI {e}")
    kinds = {k: sum(lb.kind == k for lb in labels) for k in ("Hình", "Bảng", "Đoạn mã")}
    print(f"# {md.name}: {words} từ, {kinds['Hình']} hình, {kinds['Bảng']} bảng, {kinds['Đoạn mã']} đoạn mã, "
          f"ước lượng ≈ {pages:.0f} trang; {len(rep.errors)} lỗi, {len(rep.warnings)} cảnh báo")
    if not 25 <= pages <= 35:
        print(f"CẢNH BÁO số trang ước lượng {pages:.0f} nằm ngoài khoảng 25–35")
    if rep.errors:
        return 1
    if args.expand:
        expand(md, labels, Path(args.expand))
    return 0


if __name__ == "__main__":
    sys.exit(main())
