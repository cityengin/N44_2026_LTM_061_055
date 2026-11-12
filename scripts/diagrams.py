"""Bước 8: hình minh họa cho báo cáo (results/figures/diag_*.png).

  diag_mesh.png      lưới 4×4: id = y·W + x, hướng, kênh có hướng u→v
  diag_turns.png     8 hướng rẽ và các rẽ bị cấm của XY, YX, NF, NF (bài báo), OE cột chẵn/lẻ
  diag_path_hop.png  3×3, NF/transpose1: ρ từng kênh theo path và theo hop (đọc results/example_3x3.txt)
  diag_arch.png      kiến trúc "một hàm routing"
  diag_ns3.png       vòng đời một gói trong mô hình ns-3 và trục thời gian warmup/measure/drain

Chỉ diag_path_hop.png dùng số liệu; results/example_3x3.txt là output của
`noc --mode=phi --W=3 --H=3 --routing=NF --traffic=transpose1 --channels=1` (reproduce.sh tạo lại).
Các rẽ bị cấm lấy từ quy ước của đề tài (mục 2.2.3 của báo cáo), được selftest kiểm (turn_rules).
"""

from __future__ import annotations

import sys
from fractions import Fraction

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize
from matplotlib.patches import FancyArrowPatch, FancyBboxPatch

import nocrun

INK = "#1f1f1f"
MUTED = "#6b6b6b"
GRID = "#b8b8b8"
ACCENT = "#1f5fa8"
FORBID = "#c0392b"
NODE_FILL = "#f2f2f2"
DPI = 150
VEC = {"E": (1, 0), "W": (-1, 0), "N": (0, 1), "S": (0, -1)}  # tọa độ vẽ: Bắc hướng lên


def save(fig, name: str) -> None:
    fig.savefig(nocrun.FIGURES / name, dpi=DPI, bbox_inches="tight")
    plt.close(fig)
    print(f"# {nocrun.FIGURES / name}")


def node_xy(i: int, w: int) -> tuple[float, float]:
    """Tọa độ vẽ của node i: x về Đông, y tăng về Nam nên vẽ hướng xuống."""
    return float(i % w), float(-(i // w))


def channel_arrow(ax, p, q, offset, shrink, **kw):
    """Mũi tên p→q, lệch sang phải theo chiều đi để hai chiều của một link không chồng nhau."""
    dx, dy = q[0] - p[0], q[1] - p[1]
    ox, oy = dy * offset, -dx * offset
    a = (p[0] + dx * shrink + ox, p[1] + dy * shrink + oy)
    b = (q[0] - dx * shrink + ox, q[1] - dy * shrink + oy)
    ax.add_patch(FancyArrowPatch(a, b, arrowstyle="-|>", mutation_scale=kw.pop("ms", 10), **kw))
    return a, b


def draw_nodes(ax, w, h, label, size=0.36):
    for i in range(w * h):
        x, y = node_xy(i, w)
        ax.add_patch(FancyBboxPatch((x - size / 2, y - size / 2), size, size, boxstyle="round,pad=0.02",
                                    fc=NODE_FILL, ec=INK, lw=1.0, zorder=3))
        ax.text(x, y, label(i), ha="center", va="center", fontsize=8, color=INK, zorder=4)


def fig_mesh() -> None:
    w = h = 4
    fig, ax = plt.subplots(figsize=(7.2, 6.0))
    for u in range(w * h):
        for v in (u + 1 if (u % w) < w - 1 else None, u + w if u + w < w * h else None):
            if v is None:
                continue
            for a, b in ((u, v), (v, u)):
                hl = {(5, 6), (6, 5), (6, 10)}
                channel_arrow(ax, node_xy(a, w), node_xy(b, w), 0.07, 0.24,
                              color=ACCENT if (a, b) in hl else GRID, lw=2.0 if (a, b) in hl else 1.0,
                              zorder=2)
    draw_nodes(ax, w, h, lambda i: f"{i}\n({i % w},{i // w})")
    for (a, b), (dx, dy) in {(5, 6): (0, -0.17), (6, 5): (0, 0.17), (6, 10): (-0.3, 0)}.items():
        pa, pb = node_xy(a, w), node_xy(b, w)
        ax.text((pa[0] + pb[0]) / 2 + dx, (pa[1] + pb[1]) / 2 + dy, f"kênh {a}-{b}", ha="center",
                va="center", fontsize=8, color=ACCENT,
                bbox=dict(fc="white", ec="none", pad=0.5) if dx else None)
    # Trục và la bàn.
    ax.annotate("", xy=(3.3, 0.55), xytext=(-0.3, 0.55), arrowprops=dict(arrowstyle="-|>", color=INK, lw=1))
    ax.text(3.35, 0.55, "x", va="center", fontsize=10, color=INK)
    ax.annotate("", xy=(-0.55, -3.3), xytext=(-0.55, 0.3), arrowprops=dict(arrowstyle="-|>", color=INK, lw=1))
    ax.text(-0.55, -3.45, "y", ha="center", fontsize=10, color=INK)
    cx, cy = 4.35, -0.2
    for d, (vx, vy) in VEC.items():
        ax.annotate("", xy=(cx + 0.32 * vx, cy + 0.32 * vy), xytext=(cx, cy),
                    arrowprops=dict(arrowstyle="-|>", color=INK, lw=1))
        name = {"E": "Đông (E)", "W": "Tây (W)", "N": "Bắc (N)", "S": "Nam (S)"}[d]
        ax.text(cx + 0.78 * vx, cy + 0.48 * vy, name, ha="center", va="center", fontsize=8, color=INK)
    ax.text(4.35, -1.4, "id = y·W + x\nkênh u-v: link có hướng\ntừ node u sang node v", ha="center",
            va="center", fontsize=8.5, color=MUTED)
    ax.set_xlim(-0.9, 5.3)
    ax.set_ylim(-3.7, 0.85)
    ax.set_aspect("equal")
    ax.axis("off")
    save(fig, "diag_mesh.png")


# Thứ tự vẽ 8 hướng rẽ: hàng trên là chu trình theo chiều kim đồng hồ, hàng dưới ngược chiều.
TURN_ROWS = [["E→S", "S→W", "W→N", "N→E"], ["E→N", "N→W", "W→S", "S→E"]]
FORBIDDEN = {
    "XY": {"N→E", "N→W", "S→E", "S→W"},
    "YX": {"E→N", "E→S", "W→N", "W→S"},
    "NF (quy ước chính: âm = W, N)": {"E→N", "S→W"},
    "NF (bài báo: âm = W, S)": {"E→S", "N→W"},
    "OE, cột chẵn": {"E→N", "E→S"},
    "OE, cột lẻ": {"N→W", "S→W"},
}


def draw_turn(ax, cx, cy, turn, forbidden, L=0.32):
    a, b = turn.split("→")
    va, vb = VEC[a], VEC[b]
    # Đặt điểm rẽ sao cho hình chữ L nằm giữa ô.
    sx, sy = -va[0] * L, -va[1] * L
    ex, ey = vb[0] * L, vb[1] * L
    mx, my = (sx + ex) / 2, (sy + ey) / 2
    pts = [(cx + sx - mx, cy + sy - my), (cx - mx, cy - my), (cx + ex - mx, cy + ey - my)]
    color = FORBID if forbidden else INK
    style = (0, (3, 2)) if forbidden else "solid"
    ax.plot([pts[0][0], pts[1][0]], [pts[0][1], pts[1][1]], color=color, lw=1.6, ls=style,
            solid_capstyle="round")
    ax.add_patch(FancyArrowPatch(pts[1], pts[2], arrowstyle="-|>", mutation_scale=10, color=color, lw=1.6,
                                 linestyle=style, shrinkA=0, shrinkB=0))
    if forbidden:
        ax.plot(*pts[1], marker="x", ms=9, mew=2.2, color=FORBID)
    ax.text(cx, cy - 0.5, turn, ha="center", va="center", fontsize=8,
            color=FORBID if forbidden else MUTED, fontweight="bold" if forbidden else "normal")


def fig_turns() -> None:
    fig, axes = plt.subplots(3, 2, figsize=(9.0, 8.4))
    for ax, (name, forb) in zip(axes.flat, FORBIDDEN.items()):
        for r, row in enumerate(TURN_ROWS):
            for c, turn in enumerate(row):
                draw_turn(ax, c * 1.15, -r * 1.35, turn, turn in forb)
        ax.text(-0.75, 0.0, "thuận\nchiều\nkim đồng hồ", ha="center", va="center", fontsize=7, color=MUTED)
        ax.text(-0.75, -1.35, "ngược\nchiều\nkim đồng hồ", ha="center", va="center", fontsize=7, color=MUTED)
        ax.set_title(f"{name}: cấm {len(forb)} hướng rẽ", fontsize=9.5, color=INK)
        ax.set_xlim(-1.25, 3.95)
        ax.set_ylim(-2.15, 0.6)
        ax.set_aspect("equal")
        ax.axis("off")
    fig.text(0.5, 0.005, "Nét liền: rẽ được phép.  Nét đứt màu đỏ, dấu ×: rẽ bị cấm.  Bắc hướng lên.",
             ha="center", fontsize=8.5, color=MUTED)
    fig.tight_layout(rect=(0, 0.02, 1, 1))
    save(fig, "diag_turns.png")


def frac(x: float) -> str:
    f = Fraction(x).limit_denominator(24)
    return str(f.numerator) if f.denominator == 1 else f"{f.numerator}/{f.denominator}"


def fig_path_hop() -> None:
    lines = (nocrun.RESULTS / "example_3x3.txt").read_text().splitlines()
    rho: dict[str, dict[str, float]] = {"path": {}, "hop": {}}
    phi: dict[str, float] = {}
    for kv in nocrun.lines_of(lines, "CHAN"):
        rho[kv["method"]][kv["ch"]] = float(kv["rho"])
    for kv in nocrun.lines_of(lines, "RESULT"):
        phi[kv["method"]] = float(kv["phi"])
    w = 3
    cmap = plt.get_cmap("Blues")
    norm = Normalize(0.0, 1.0)
    fig, axes = plt.subplots(1, 2, figsize=(11.0, 5.4))
    titles = {"path": "(a) chia theo đường (path, k/p)", "hop": "(b) chia theo nhánh (hop)"}
    for ax, method in zip(axes, ("path", "hop")):
        vals = rho[method]
        top = nocrun.max_channels(vals)
        for ch, r in vals.items():
            u, v = map(int, ch.split("-"))
            is_max = ch in top
            a, b = channel_arrow(ax, node_xy(u, w), node_xy(v, w), 0.11, 0.2, color=cmap(norm(0.25 + 0.75 * r)),
                                 lw=3.2 if is_max else 2.0, zorder=2, ms=12)
            mx, my = (a[0] + b[0]) / 2, (a[1] + b[1]) / 2
            pu, pv = node_xy(u, w), node_xy(v, w)
            ox, oy = (pv[1] - pu[1]) * 0.13, -(pv[0] - pu[0]) * 0.13
            ax.text(mx + ox, my + oy, frac(r), ha="center", va="center", fontsize=8.5, color=INK,
                    fontweight="bold" if is_max else "normal", zorder=5)
        draw_nodes(ax, w, w, str, size=0.3)
        ax.set_title(f"{titles[method]}\nφ = {frac(phi[method])}, đạt tại {len(top)} kênh (số đậm, nét dày)",
                     fontsize=10, color=INK)
        ax.set_xlim(-0.45, 2.45)
        ax.set_ylim(-2.45, 0.45)
        ax.set_aspect("equal")
        ax.axis("off")
    fig.text(0.5, 0.01, "Lưới 3×3, NF (quy ước chính), transpose1: 6 luồng, mọi cặp đều thích ứng hoàn toàn. "
             "Số trên mỗi mũi tên là ρ của kênh; màu đậm hơn khi ρ lớn hơn.", ha="center", fontsize=8.5,
             color=MUTED)
    fig.tight_layout(rect=(0, 0.04, 1, 1))
    save(fig, "diag_path_hop.png")


def box(ax, x, y, w, h, text, fc=NODE_FILL, ec=INK, lw=1.0, fs=8.5, bold=False):
    ax.add_patch(FancyBboxPatch((x - w / 2, y - h / 2), w, h, boxstyle="round,pad=0.02,rounding_size=0.06",
                                fc=fc, ec=ec, lw=lw, zorder=2))
    ax.text(x, y, text, ha="center", va="center", fontsize=fs, color=INK, zorder=3,
            fontweight="bold" if bold else "normal")


def arrow(ax, a, b, color=INK, style="solid", text=None, rad=0.0):
    ax.add_patch(FancyArrowPatch(a, b, arrowstyle="-|>", mutation_scale=11, color=color, lw=1.2,
                                 linestyle=style, connectionstyle=f"arc3,rad={rad}", zorder=1))
    if text:
        ax.text((a[0] + b[0]) / 2 + 0.08, (a[1] + b[1]) / 2, text, fontsize=7.5, color=MUTED, va="center")


def fig_arch() -> None:
    fig, ax = plt.subplots(figsize=(10.0, 6.6))
    box(ax, 5.0, 6.0, 4.2, 0.9, "routing.cc: routeSet(algo, cur, src, dst)\n→ tập cổng hợp lệ ⊆ {E, W, N, S}",
        fc="#dbe7f5", ec=ACCENT, lw=1.8, bold=True)
    # Nhánh phân tích tĩnh.
    ax.text(2.0, 5.05, "phân tích tĩnh", ha="center", fontsize=8.5, color=MUTED, style="italic")
    box(ax, 2.0, 4.4, 3.6, 0.8, "searchtree.cc: ForEachPath\n(DFS mọi đường, xác suất theo nhánh)")
    box(ax, 1.1, 2.9, 2.5, 0.8, "phi.cc\nρ, φ, PIR_max (path, hop)")
    box(ax, 3.5, 2.9, 1.8, 0.8, "cdg.cc: CheckCdg\n(DFS 3 màu)")
    # Nhánh mô phỏng.
    ax.text(8.0, 5.05, "mô phỏng ns-3", ha="center", fontsize=8.5, color=MUTED, style="italic")
    box(ax, 8.0, 4.4, 3.0, 0.8, "noc-router.cc: NocRouter::Forward\n(gọi routeSet tại mỗi hop)")
    box(ax, 9.25, 2.9, 1.5, 0.8, "selection.cc\nSelectUniform")
    box(ax, 7.2, 2.9, 2.3, 0.8, "noc-sim.cc: RunSim\n(link PPP, queue, trace)")
    arrow(ax, (4.0, 5.65), (2.6, 4.82))
    arrow(ax, (6.0, 5.65), (7.4, 4.82))
    arrow(ax, (1.7, 4.0), (1.25, 3.32))
    arrow(ax, (2.4, 4.0), (3.3, 3.32))
    arrow(ax, (9.25, 3.32), (8.9, 4.0), text="chọn 1 cổng")
    arrow(ax, (7.6, 4.0), (7.3, 3.32))
    # Đầu ra.
    box(ax, 4.75, 1.55, 8.2, 0.75, "noc.cc: --mode = phi | selftest | sim | simcheck  →  dòng RESULT,key=value,…")
    for x in (1.1, 3.5, 7.2):
        arrow(ax, (x, 2.48), (x, 1.95))
    box(ax, 4.75, 0.35, 8.2, 0.75, "scripts/*.py  →  results/*.csv, results/figures/*.png")
    arrow(ax, (4.75, 1.15), (4.75, 0.75))
    box(ax, 9.65, 0.95, 1.1, 1.1, "selftest.cc\nkiểm cả hai\nnhánh", fc="white", ec=MUTED, fs=7.5)
    ax.set_xlim(-0.35, 10.3)
    ax.set_ylim(-0.2, 6.6)
    ax.axis("off")
    save(fig, "diag_arch.png")


def fig_ns3() -> None:
    fig, (ax, bx) = plt.subplots(2, 1, figsize=(11.0, 5.6), gridspec_kw={"height_ratios": [2.4, 1.0]})
    # (a) vòng đời một gói.
    y1, y2 = 2.0, 0.6
    steps1 = [
        (0.9, "Tạo gói (Bernoulli)\nNoCHeader 10 B\n+ CreationTimeTag"),
        (3.0, "NocRouter::Forward\nrouteSet +\nSelectUniform"),
        (5.1, "NetDevice::Send\n(p, broadcast,\n0x0800)"),
        (7.2, "DropTailQueue\n(chờ khi link bận)"),
        (9.3, "PPP: + header 2 B\nframe 32 B, 16 Gbps\n→ 16 ns trên link"),
    ]
    for x, t in steps1:
        box(ax, x, y1, 1.85, 0.8, t, fs=8)
    for (xa, _), (xb, _) in zip(steps1, steps1[1:]):
        arrow(ax, (xa + 0.95, y1), (xb - 0.95, y1))
    steps2 = [
        (9.3, "Kênh delay 0\n→ NetDevice\ncủa node kế"),
        (7.2, "ReceiveCallback\n→ NocRouter::Receive\n(RemoveHeader, hops + 1)"),
        (5.1, "dst = node này?"),
    ]
    for x, t in steps2:
        box(ax, x, y2, 1.85 if x != 5.1 else 1.5, 0.8, t, fs=8,
            fc="white" if x == 5.1 else NODE_FILL)
    arrow(ax, (9.3, y1 - 0.42), (9.3, y2 + 0.42))
    arrow(ax, (8.35, y2), (8.15, y2))
    arrow(ax, (6.25, y2), (5.9, y2))
    box(ax, 2.2, y2, 2.5, 0.75, "giao gói: latency = now − lúc tạo\n(chỉ tính gói tạo trong cửa sổ đo)", fs=8)
    arrow(ax, (4.35, y2), (3.5, y2), text=None)
    ax.text(3.9, y2 + 0.18, "có", fontsize=8, color=MUTED, ha="center")
    arrow(ax, (5.1, y2 + 0.42), (3.6, y1 - 0.42), style=(0, (3, 2)), rad=-0.15)
    ax.text(5.2, 1.3, "không: AddHeader, Forward tiếp", fontsize=8, color=MUTED, ha="left")
    ax.text(0.0, 2.8, "(a) Vòng đời của một gói: mỗi ô là một lời gọi hàm hoặc sự kiện của Simulator ns-3",
            fontsize=9.5, color=INK)
    ax.set_xlim(-0.2, 10.4)
    ax.set_ylim(-0.05, 3.0)
    ax.axis("off")
    # (b) trục thời gian.
    segs = [(0, 2.0, "warmup", "gói được tạo và truyền,\nkhông thống kê", "#ececec"),
            (2.0, 6.5, "measure", "thống kê mọi gói tạo trong [warmup, warmup + measure)", "#dbe7f5"),
            (6.5, 10.0, "drain", "vẫn tạo gói; dừng khi mọi gói của cửa sổ đã tới đích\n"
             "(tối đa drain chu kỳ; còn sót thì saturated = 1)", "#f5f5f5")]
    for x0, x1, name, desc, fc in segs:
        bx.add_patch(FancyBboxPatch((x0, 0.0), x1 - x0, 0.5, boxstyle="square,pad=0", fc=fc, ec=INK, lw=0.8))
        bx.text((x0 + x1) / 2, 0.25, name, ha="center", va="center", fontsize=9, color=INK, fontweight="bold")
        bx.text((x0 + x1) / 2, -0.35, desc, ha="center", va="center", fontsize=7.8, color=MUTED)
    bx.annotate("", xy=(10.3, 0.0), xytext=(0, 0.0), arrowprops=dict(arrowstyle="-|>", color=INK, lw=1))
    bx.text(10.35, 0.0, "t (ns = chu kỳ)", va="center", fontsize=8, color=INK)
    bx.text(0.0, 0.85, "(b) Trục thời gian của một lần chạy", fontsize=9.5, color=INK)
    bx.set_xlim(-0.2, 11.4)
    bx.set_ylim(-0.75, 1.05)
    bx.axis("off")
    fig.tight_layout()
    save(fig, "diag_ns3.png")


def main() -> int:
    nocrun.FIGURES.mkdir(parents=True, exist_ok=True)
    fig_mesh()
    fig_turns()
    fig_path_hop()
    fig_arch()
    fig_ns3()
    return 0


if __name__ == "__main__":
    sys.exit(main())
