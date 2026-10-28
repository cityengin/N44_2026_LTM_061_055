"""Bước 6a: heatmap độ sử dụng kênh trên lưới 7×7.

Mỗi kênh có hướng là một mũi tên; hai chiều của một link lệch sang phải theo chiều đi.
Lưới ô: hàng {XY, OE, NF (bài báo)}, cột {transpose1, transpose2}.
  heatmap_pred.png  U_hop = ρ_hop·PIR·16 tại pir_ratio 0.9 (dự đoán; kênh đạt φ có U = 0.9)
  heatmap_meas.png  U_đo trung bình 5 RngRun tại pir_ratio 0.9 (mô phỏng)
Hai bản dùng chung thang màu 0..1; viền đậm các kênh đạt φ (ρ_hop = max). Kênh không dùng (U = 0)
vẽ mảnh màu xám.
Dữ liệu: results/link_validate_channels.csv (chạy link_validate.py trước).
"""

from __future__ import annotations

import argparse
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patheffects as pe
from matplotlib.cm import ScalarMappable
from matplotlib.colors import Normalize
from matplotlib.patches import FancyArrowPatch
import pandas as pd

import nocrun

ROWS = ["XY", "OE", "NF_WS"]
COLS = ["transpose1", "transpose2"]
RATIO = 0.9
W = H = 7
OFFSET = 0.09  # lệch sang phải theo chiều đi, để hai chiều của một link không chồng nhau
SHRINK = 0.17  # chừa khoảng quanh node
CMAP = plt.get_cmap("YlOrRd")
NORM = Normalize(0.0, 1.0)


def node_xy(i: int) -> tuple[float, float]:
    """Tọa độ vẽ: x về Đông, y tăng về Nam nên vẽ hướng xuống."""
    return float(i % W), -float(i // W)


def draw(ax, part: pd.DataFrame, col: str, phi_set: set[str], title: str) -> None:
    for i in range(W * H):
        x, y = node_xy(i)
        ax.plot(x, y, "o", color="0.55", ms=3, zorder=1)
        ax.text(x - 0.12, y + 0.12, str(i), fontsize=5, color="0.45", ha="right", va="bottom")
    for _, r in part.iterrows():
        u, v = (int(t) for t in r["ch"].split("-"))
        (x0, y0), (x1, y1) = node_xy(u), node_xy(v)
        dx, dy = x1 - x0, y1 - y0
        ox, oy = dy * OFFSET, -dx * OFFSET
        start = (x0 + dx * SHRINK + ox, y0 + dy * SHRINK + oy)
        end = (x1 - dx * SHRINK + ox, y1 - dy * SHRINK + oy)
        val = r[col]
        used = val > 1e-12
        bold = r["ch"] in phi_set
        lw = 2.4 if used else 0.6
        arrow = FancyArrowPatch(
            start, end, arrowstyle="-|>", mutation_scale=7 if used else 4, lw=lw,
            color=CMAP(NORM(val)) if used else "0.82", zorder=3 if bold else 2,
            path_effects=[pe.withStroke(linewidth=lw + 2.6, foreground="black")] if bold else None)
        ax.add_patch(arrow)
    ax.set_xlim(-0.6, W - 0.4)
    ax.set_ylim(-(H - 0.4), 0.6)
    ax.set_aspect("equal")
    ax.axis("off")
    ax.set_title(title, fontsize=9)


def render(df: pd.DataFrame, phi: dict, col: str, fname: str, suptitle: str) -> None:
    fig, axes = plt.subplots(len(ROWS), len(COLS), figsize=(11, 15.5),
                             gridspec_kw={"wspace": 0.08, "hspace": 0.12})
    for i, algo in enumerate(ROWS):
        for j, traffic in enumerate(COLS):
            part = df[(df["routing"] == algo) & (df["traffic"] == traffic)]
            # Kênh đạt φ theo dự đoán (U_hop lớn nhất); dùng cùng tập cho cả hai bản.
            phi_set = set(nocrun.max_channels(dict(zip(part["ch"], part["u_hop"]))))
            top = part.loc[part[col].idxmax()]
            title = (f"{nocrun.LABELS[algo]} — {traffic}\n"
                     f"φ_hop = {phi[(algo, traffic)]['phi_hop']:.4g}, {len(phi_set)} kênh đạt φ; "
                     f"max U = {top[col]:.3f} ({top['ch']})")
            draw(axes[i, j], part, col, phi_set, title)
    fig.colorbar(ScalarMappable(norm=NORM, cmap=CMAP), ax=axes, fraction=0.025, pad=0.02,
                 label="U (độ sử dụng kênh)")
    fig.suptitle(f"{suptitle}\nviền đen: các kênh đạt φ theo dự đoán; xám mảnh: kênh không dùng",
                 fontsize=11, y=0.93)
    fig.savefig(nocrun.FIGURES / fname, dpi=150, bbox_inches="tight")
    plt.close(fig)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--channels", default=str(nocrun.RESULTS / "link_validate_channels.csv"))
    ap.add_argument("--allow-default", action="store_true")
    args = ap.parse_args()

    df = pd.read_csv(args.channels)
    df = df[(df["ratio"] == RATIO) & df["routing"].isin(ROWS) & df["traffic"].isin(COLS)]
    if df.empty:
        print(f"không có dữ liệu ratio {RATIO} trong {args.channels}; chạy link_validate.py trước",
              file=sys.stderr)
        return 1
    phi = nocrun.phi_table(nocrun.find_binary(args.allow_default))
    nocrun.FIGURES.mkdir(parents=True, exist_ok=True)
    render(df, phi, "u_hop", "heatmap_pred.png",
           f"Dự đoán: U_hop = ρ_hop·PIR·16 tại pir_ratio {RATIO} (7×7)")
    render(df, phi, "u_meas", "heatmap_meas.png",
           f"Mô phỏng: U_đo trung bình 5 RngRun tại pir_ratio {RATIO} (7×7, measure 100000)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
