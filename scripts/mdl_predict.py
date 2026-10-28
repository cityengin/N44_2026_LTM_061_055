"""Bước 6b–6c: model latency giải tích, so với mô phỏng. Phần mô hình đã khóa sau 6c.

"kleinrock" — Kleinrock independence + M/D/1: mỗi kênh là một hàng đợi M/D/1 độc lập, phục vụ 16 chu kỳ,
luồng đến Poisson với tốc độ λ_ch = ρ_hop,ch·PIR (uniform: ρ chia N−1):
  U_ch = 16·λ_ch,  W_ch = U_ch·16 / (2(1 − U_ch))          (thời gian chờ M/D/1)
  Λ = số nguồn phát·PIR
  lat_pred = 16·hops_avg + Σ_ch λ_ch·W_ch / Λ             (định luật Little cho phần chờ)
"kleinrock_sc" — bỏ self-contention (heuristic, 6c): gói từ cùng một cổng vào kênh đã cách nhau ≥ 16 chu
kỳ nên không tự xung đột; nhân W_ch với (1 − Σ_i p_i²), p_i = rate(i→ch)/ρ(ch), i chạy trên các kênh vào
từ node láng giềng (luồng local KHÔNG nằm trong tổng). rate lấy từ `noc --mode=phi --turns=1`.
hops_avg lý thuyết: transpose = 224/42; uniform = khoảng cách Manhattan trung bình giữa các cặp khác nhau.

Điểm gãy dự đoán: nghiệm theo pir_ratio của lat_pred = 2·16·hops_avg (bisection trên (0, 1)); NaN nếu
không có nghiệm trước pir_ratio 1. analyze_sweep.py dùng knee_for để thêm cột knee_pred, knee_pred_sc.

Output của main:
  results/mdl_predict.csv          lat_sim / lat_pred / phần chờ theo cấu hình × pir_ratio ≤ 0.95, cả hai model
  report/figures/mdl_predict.png   lat theo pir_ratio; hàng 1: kleinrock, hàng 2: kleinrock_sc
"""

from __future__ import annotations

import argparse
import math
import sys
from collections import defaultdict

import numpy as np

import nocrun

SERVICE = 16.0  # chu kỳ một gói chiếm kênh
W_MESH = H_MESH = 7
MODELS = ["kleinrock", "kleinrock_sc"]


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
    """ρ_hop từng kênh (thứ tự theo rho_raw) theo đơn vị gói/chu kỳ trên mỗi đơn vị PIR (uniform: chia N−1)."""
    norm = 1.0 / (w * h - 1) if traffic == "uniform" else 1.0
    return np.array(list(rho_raw.values())) * norm


def sc_factor(channels: list[str], turns: dict[tuple[str, str], float]) -> np.ndarray:
    """1 − Σ_i p_i² theo từng kênh (thứ tự channels); p_i = rate(i→ch) / Σ_in rate(in→ch), i là kênh vào
    từ node láng giềng (bỏ "local"). Σ_in rate(in→ch) = ρ_hop(ch) (test HARD turn_in_sum_eq_rho)."""
    inflow: dict[str, float] = defaultdict(float)
    for (_, out), r in turns.items():
        inflow[out] += r
    sq: dict[str, float] = defaultdict(float)
    for (i, out), r in turns.items():
        if i != "local" and inflow[out] > 0:
            sq[out] += (r / inflow[out]) ** 2
    return np.array([1.0 - sq.get(ch, 0.0) for ch in channels])


def queue_pred(rho: np.ndarray, pir: float, n_src: int, factor: np.ndarray | None = None) -> float:
    """Σ_ch λ_ch·W_ch / Λ. factor = None: kleinrock; ngược lại W nhân factor (kleinrock_sc).
    Vô cùng nếu có kênh U > 1, hoặc U ≥ 1 ở kênh có factor > 0."""
    lam = rho * pir
    u = SERVICE * lam
    f = np.ones_like(u) if factor is None else factor
    active = (lam > 0) & (f > 1e-12)
    if u.max() > 1.0 or (u[active] >= 1.0).any():
        return math.inf
    wait = np.zeros_like(u)
    wait[active] = u[active] * SERVICE / (2.0 * (1.0 - u[active])) * f[active]
    return float((lam * wait).sum() / (n_src * pir))


def lat_pred(rho: np.ndarray, pir: float, n_src: int, hops: float, factor: np.ndarray | None = None) -> float:
    return SERVICE * hops + queue_pred(rho, pir, n_src, factor)


def knee_pred(rho: np.ndarray, pir_max: float, n_src: int, hops: float,
              factor: np.ndarray | None = None, tol: float = 1e-7) -> float:
    """pir_ratio mà lat_pred = 2·16·hops (lat_pred tăng đơn điệu theo ratio). NaN nếu lat_pred vẫn
    dưới ngưỡng khi ratio → 1 (vd kênh đạt φ chỉ nhận gói từ một cổng nên không phải chờ)."""
    target = 2.0 * SERVICE * hops
    if lat_pred(rho, (1.0 - 1e-9) * pir_max, n_src, hops, factor) < target:
        return math.nan
    lo, hi = 0.0, 1.0
    while hi - lo > tol:
        mid = 0.5 * (lo + hi)
        if lat_pred(rho, mid * pir_max, n_src, hops, factor) >= target:
            hi = mid
        else:
            lo = mid
    return 0.5 * (lo + hi)


def knee_for(rho_raw: dict[str, float], traffic: str,
             turns: dict[tuple[str, str], float] | None = None) -> float:
    """knee_pred của một cấu hình; turns = None: kleinrock, ngược lại kleinrock_sc.
    PIR_max = 1/(16·max ρ) tính từ chính ρ (không làm tròn)."""
    n_src, hops = theory(traffic)
    rho = normalized_rho(rho_raw, traffic)
    factor = None if turns is None else sc_factor(list(rho_raw), turns)
    return knee_pred(rho, 1.0 / (SERVICE * rho.max()), n_src, hops, factor)


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

    binary = nocrun.find_binary(args.allow_default)
    rho_raw = nocrun.channel_pressure(binary, "hop")
    turns = nocrun.turn_flows(binary)
    df = pd.read_csv(args.sweep, dtype={"pir_arg": str, "ratio_target": str})
    rel = df[df["grid"] == "rel"].copy()
    rel["ratio"] = rel["ratio_target"].astype(float)
    rel = rel[rel["ratio"] <= args.max_ratio + 1e-9]
    rel = rel.sort_values(["routing", "traffic", "ratio", "run"], kind="mergesort")
    g = rel.groupby(["routing", "traffic", "ratio"])
    sim = g.agg(pir=("pir", "first"), lat_sim=("lat_avg", "mean"), hops_sim=("hops_avg", "mean"),
                n=("run", "nunique")).reset_index()

    rows = []
    for _, r in sim.iterrows():
        key = (r["routing"], r["traffic"])
        n_src, hops = theory(r["traffic"])
        rho = normalized_rho(rho_raw[key], r["traffic"])
        factor = sc_factor(list(rho_raw[key]), turns[key])
        q_sim = r["lat_sim"] - SERVICE * r["hops_sim"]
        row = {"routing": r["routing"], "traffic": r["traffic"], "ratio": r["ratio"], "pir": r["pir"],
               "hops_theory": hops, "hops_sim": r["hops_sim"], "lat_sim": r["lat_sim"], "queue_sim": q_sim}
        for model, fac in (("kleinrock", None), ("kleinrock_sc", factor)):
            sfx = "" if model == "kleinrock" else "_sc"
            q = queue_pred(rho, r["pir"], n_src, fac)
            lp = SERVICE * hops + q
            row.update({f"lat_pred{sfx}": lp, f"lat_rel_err{sfx}": (lp - r["lat_sim"]) / r["lat_sim"],
                        f"queue_pred{sfx}": q,
                        f"queue_ratio{sfx}": q / q_sim if q_sim > 0 else math.nan})
        rows.append(row)
    out = pd.DataFrame(rows)
    out.to_csv(nocrun.RESULTS / "mdl_predict.csv", index=False, float_format="%.6g")

    # Hình: lat theo pir_ratio; hàng 1 kleinrock, hàng 2 kleinrock_sc; liền = sim, đứt = model.
    nocrun.FIGURES.mkdir(parents=True, exist_ok=True)
    fig, axes = plt.subplots(2, 3, figsize=(15, 9), sharex=True)
    for row_i, (model, col) in enumerate((("kleinrock", "lat_pred"), ("kleinrock_sc", "lat_pred_sc"))):
        for ax, traffic in zip(axes[row_i], nocrun.TRAFFICS):
            for i, algo in enumerate(nocrun.ALGOS):
                part = out[(out["traffic"] == traffic) & (out["routing"] == algo)].sort_values("ratio")
                ax.plot(part["ratio"], part["lat_sim"], "-o", ms=3, color=f"C{i}",
                        label=f"{nocrun.LABELS[algo]} (sim)")
                ax.plot(part["ratio"], part[col], "--", color=f"C{i}", label=f"{nocrun.LABELS[algo]} ({model})")
            ax.set_title(f"{traffic} — {model}")
            ax.grid(alpha=0.3)
            if row_i == 1:
                ax.set_xlabel("pir_ratio = PIR / PIR_max_hop")
        axes[row_i][0].set_ylabel("lat_avg [chu kỳ]")
        axes[row_i][-1].legend(fontsize=7, ncol=2)
    fig.suptitle("Latency: mô phỏng (nét liền) và model (nét đứt). Hàng trên: Kleinrock + M/D/1; "
                 "hàng dưới: bỏ self-contention (heuristic)\nNhiều đường trùng nhau vì cùng tập giá trị ρ "
                 "(vd XY ≡ YX; t1: XY ≡ NF (bài báo); t2: XY ≡ NF (quy ước chính))", fontsize=10)
    fig.tight_layout()
    fig.savefig(nocrun.FIGURES / "mdl_predict.png", dpi=150)
    plt.close(fig)

    pd.set_option("display.width", 220)
    print("## Tỉ số queue_pred / queue_sim (phần chờ, bỏ 16·hops), cả hai model")
    sel = out[out["ratio"].isin([0.1, 0.5, 0.9])]
    piv = sel.pivot_table(index=["routing", "traffic"], columns="ratio",
                          values=["queue_ratio", "queue_ratio_sc"])
    piv.columns = [f"{'kleinrock' if m == 'queue_ratio' else 'sc'}@{r}" for m, r in piv.columns]
    piv = piv[[f"{m}@{r}" for r in (0.1, 0.5, 0.9) for m in ("kleinrock", "sc")]]
    print(piv.to_string(float_format=lambda v: f"{v:.2f}"))

    print("\n## Sai số tương đối latency tổng [%] tại pir_ratio 0.9")
    p9 = out[out["ratio"] == 0.9].set_index(["routing", "traffic"])[["lat_rel_err", "lat_rel_err_sc"]] * 100
    print(p9.to_string(float_format=lambda v: f"{v:+.1f}"))

    print("\n## Kiểm ở tải thấp: queue_delay tại PIR = 0.001 (simcheck đo được 0.48–0.68)")
    for traffic in ("transpose1", "transpose2"):
        n_src, _ = theory(traffic)
        key = ("XY", traffic)
        rho = normalized_rho(rho_raw[key], traffic)
        fac = sc_factor(list(rho_raw[key]), turns[key])
        print(f"  XY/{traffic}: kleinrock = {queue_pred(rho, 0.001, n_src):.3f}, "
              f"kleinrock_sc = {queue_pred(rho, 0.001, n_src, fac):.3f} chu kỳ")

    sat = pd.read_csv(args.saturation)
    if "knee_pred_sc" not in sat.columns:
        print(f"\n# {args.saturation} chưa có cột knee_pred_sc; chạy analyze_sweep.py trước", file=sys.stderr)
        return 1
    print("\n## Điểm gãy: (ii) mô phỏng, knee_pred (kleinrock), knee_pred_sc (kleinrock_sc); NaN = không gãy trước 1")
    sat["diff"] = sat["knee_pred"] - sat["sat_ratio_lat"]
    sat["diff_sc"] = sat["knee_pred_sc"] - sat["sat_ratio_lat"]
    print(sat[["label", "traffic", "n_near_max", "sat_ratio_lat", "knee_pred", "diff", "knee_pred_sc", "diff_sc"]]
          .sort_values(["n_near_max", "sat_ratio_lat"]).to_string(index=False, float_format=lambda v: f"{v:.3f}"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
