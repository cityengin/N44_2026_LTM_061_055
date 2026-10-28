"""Bước 6b: model latency giải tích, so với mô phỏng.

Xấp xỉ Kleinrock independence + M/D/1: mỗi kênh là một hàng đợi M/D/1 độc lập, thời gian phục vụ
16 chu kỳ, luồng đến Poisson với tốc độ λ_ch = ρ_hop,ch·PIR (uniform: ρ chia N−1):
  U_ch = 16·λ_ch,  W_ch = U_ch·16 / (2(1 − U_ch))          (thời gian chờ M/D/1)
  Λ = số nguồn phát·PIR
  lat_pred = 16·hops_avg + Σ_ch λ_ch·W_ch / Λ             (định luật Little cho phần chờ)
hops_avg lý thuyết: transpose = 224/42; uniform = khoảng cách Manhattan trung bình giữa các cặp khác nhau.

Điểm gãy dự đoán knee_pred: nghiệm theo pir_ratio của lat_pred = 2·16·hops_avg (bisection trên (0, 1)).
analyze_sweep.py dùng knee_pred để thêm cột vào results/saturation.csv.

Output của main:
  results/mdl_predict.csv          lat_sim / lat_pred / sai số theo cấu hình × pir_ratio ≤ 0.95
  report/figures/mdl_predict.png   lat theo pir_ratio, mỗi traffic một ô (liền = sim, đứt = model)
"""

from __future__ import annotations

import argparse
import math
import sys

import numpy as np

import nocrun

SERVICE = 16.0  # chu kỳ một gói chiếm kênh
W_MESH = H_MESH = 7


def flows(traffic: str, w: int = W_MESH, h: int = H_MESH) -> list[tuple[int, int]]:
    """Các cặp (src, dst) giống MakeFlows trong noc/traffic.cc; node tự map vào chính nó không phát."""
    n = w * h
    if traffic == "uniform":
        return [(s, d) for s in range(n) for d in range(n) if s != d]
    out = []
    for s in range(n):
        x, y = s % w, s // w
        d = (h - 1 - x) * w + (w - 1 - y) if traffic == "transpose1" else x * w + y
        if d != s:
            out.append((s, d))
    return out


def theory(traffic: str, w: int = W_MESH, h: int = H_MESH) -> tuple[int, float]:
    """(số nguồn phát, hops_avg lý thuyết). Mọi nguồn phát cùng PIR; uniform chia đều cho N−1 đích."""
    f = flows(traffic, w, h)
    dist = [abs(s % w - d % w) + abs(s // w - d // w) for s, d in f]
    return len({s for s, _ in f}), float(np.mean(dist))


def normalized_rho(rho_raw: dict[str, float], traffic: str, w: int = W_MESH, h: int = H_MESH) -> np.ndarray:
    """ρ_hop từng kênh theo đơn vị gói/chu kỳ trên mỗi đơn vị PIR (uniform: chia N−1)."""
    norm = 1.0 / (w * h - 1) if traffic == "uniform" else 1.0
    return np.array(list(rho_raw.values())) * norm


def queue_pred(rho: np.ndarray, pir: float, n_src: int) -> float:
    """Σ_ch λ_ch·W_ch / Λ; vô cùng nếu có kênh U ≥ 1."""
    lam = rho * pir
    u = SERVICE * lam
    if u.max() >= 1.0:
        return math.inf
    wait = u * SERVICE / (2.0 * (1.0 - u))
    return float((lam * wait).sum() / (n_src * pir))


def lat_pred(rho: np.ndarray, pir: float, n_src: int, hops: float) -> float:
    return SERVICE * hops + queue_pred(rho, pir, n_src)


def knee_pred(rho: np.ndarray, pir_max: float, n_src: int, hops: float, tol: float = 1e-7) -> float:
    """pir_ratio mà lat_pred = 2·16·hops. lat_pred tăng đơn điệu theo ratio và → ∞ khi ratio → 1
    (kênh đạt φ có U = ratio), nên nghiệm luôn nằm trong (0, 1)."""
    target = 2.0 * SERVICE * hops
    lo, hi = 0.0, 1.0
    while hi - lo > tol:
        mid = 0.5 * (lo + hi)
        if lat_pred(rho, mid * pir_max, n_src, hops) >= target:
            hi = mid
        else:
            lo = mid
    return 0.5 * (lo + hi)


def knee_for(rho_raw: dict[str, float], traffic: str) -> float:
    """knee_pred của một cấu hình; PIR_max = 1/(16·max ρ) tính từ chính ρ (không làm tròn)."""
    n_src, hops = theory(traffic)
    rho = normalized_rho(rho_raw, traffic)
    return knee_pred(rho, 1.0 / (SERVICE * rho.max()), n_src, hops)


def main() -> int:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import pandas as pd

    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sweep", default=str(nocrun.RESULTS / "sweep.csv"))
    ap.add_argument("--saturation", default=str(nocrun.RESULTS / "saturation.csv"))
    ap.add_argument("--max-ratio", type=float, default=0.95)
    ap.add_argument("--allow-default", action="store_true")
    args = ap.parse_args()

    rho_raw = nocrun.channel_pressure(nocrun.find_binary(args.allow_default), "hop")
    df = pd.read_csv(args.sweep, dtype={"pir_arg": str, "ratio_target": str})
    rel = df[df["grid"] == "rel"].copy()
    rel["ratio"] = rel["ratio_target"].astype(float)
    rel = rel[rel["ratio"] <= args.max_ratio + 1e-9]
    g = rel.groupby(["routing", "traffic", "ratio"])
    sim = g.agg(pir=("pir", "first"), lat_sim=("lat_avg", "mean"), hops_sim=("hops_avg", "mean"),
                n=("run", "nunique")).reset_index()

    rows = []
    for _, r in sim.iterrows():
        n_src, hops = theory(r["traffic"])
        rho = normalized_rho(rho_raw[(r["routing"], r["traffic"])], r["traffic"])
        q_pred = queue_pred(rho, r["pir"], n_src)
        q_sim = r["lat_sim"] - SERVICE * r["hops_sim"]
        lp = SERVICE * hops + q_pred
        rows.append({"routing": r["routing"], "traffic": r["traffic"], "ratio": r["ratio"],
                     "pir": r["pir"], "hops_theory": hops, "hops_sim": r["hops_sim"],
                     "lat_sim": r["lat_sim"], "lat_pred": lp,
                     "lat_rel_err": (lp - r["lat_sim"]) / r["lat_sim"],
                     "queue_sim": q_sim, "queue_pred": q_pred,
                     "queue_ratio": q_pred / q_sim if q_sim > 0 else math.nan})
    out = pd.DataFrame(rows)
    out.to_csv(nocrun.RESULTS / "mdl_predict.csv", index=False, float_format="%.6g")

    # Hình: lat theo pir_ratio, liền = sim, đứt = model.
    nocrun.FIGURES.mkdir(parents=True, exist_ok=True)
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.8), sharey=False)
    for ax, traffic in zip(axes, nocrun.TRAFFICS):
        for i, algo in enumerate(nocrun.ALGOS):
            part = out[(out["traffic"] == traffic) & (out["routing"] == algo)].sort_values("ratio")
            ax.plot(part["ratio"], part["lat_sim"], "-o", ms=3, color=f"C{i}", label=f"{nocrun.LABELS[algo]} (sim)")
            ax.plot(part["ratio"], part["lat_pred"], "--", color=f"C{i}", label=f"{nocrun.LABELS[algo]} (model)")
        ax.set_title(traffic)
        ax.set_xlabel("pir_ratio = PIR / PIR_max_hop")
        ax.grid(alpha=0.3)
    axes[0].set_ylabel("lat_avg [chu kỳ]")
    axes[-1].legend(fontsize=7, ncol=2)
    fig.suptitle("Latency: mô phỏng (nét liền) và model Kleinrock + M/D/1 (nét đứt)\n"
                 "Model chỉ phụ thuộc tập giá trị ρ nên nhiều đường trùng nhau: XY ≡ YX; "
                 "t1: XY ≡ NF (bài báo); t2: XY ≡ NF (quy ước chính); uniform: NF ≡ NF (bài báo)",
                 fontsize=10)
    fig.tight_layout()
    fig.savefig(nocrun.FIGURES / "mdl_predict.png", dpi=150)
    plt.close(fig)

    pd.set_option("display.width", 220)
    print("## Sai số tương đối lat_pred so với lat_sim [%], theo cấu hình × pir_ratio")
    piv = out.pivot_table(index=["routing", "traffic"], columns="ratio", values="lat_rel_err") * 100
    print(piv.to_string(float_format=lambda v: f"{v:+.1f}"))
    print("\n## Tỉ số queue_pred / queue_sim (phần chờ, bỏ 16·hops)")
    piv_q = out.pivot_table(index=["routing", "traffic"], columns="ratio", values="queue_ratio")
    print(piv_q.to_string(float_format=lambda v: f"{v:.2f}"))

    print("\n## Kiểm ở tải thấp: queue_delay tại PIR = 0.001 (simcheck đo được 0.48–0.68)")
    for traffic in ("transpose1", "transpose2"):
        n_src, _ = theory(traffic)
        rho = normalized_rho(rho_raw[("XY", traffic)], traffic)
        print(f"  XY/{traffic}: queue_pred = {queue_pred(rho, 0.001, n_src):.3f} chu kỳ")

    sat = pd.read_csv(args.saturation)
    if "knee_pred" not in sat.columns:
        print(f"\n# {args.saturation} chưa có cột knee_pred; chạy analyze_sweep.py trước", file=sys.stderr)
        return 1
    print("\n## Điểm gãy: knee_pred (model) so với (ii) (mô phỏng)")
    sat["diff"] = sat["knee_pred"] - sat["sat_ratio_lat"]
    print(sat[["label", "traffic", "n_near_max", "sat_ratio_lat", "knee_pred", "diff"]]
          .sort_values(["n_near_max", "sat_ratio_lat"]).to_string(index=False, float_format=lambda v: f"{v:.3f}"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
