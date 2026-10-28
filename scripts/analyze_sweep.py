"""Bước 5d: phân tích results/sweep.csv.

Output:
  results/sweep_summary.csv   mean ± CI 95% theo (grid, routing, traffic, pir)        (không commit)
  results/saturation.csv      điểm bão hòa theo 3 định nghĩa, tính trên lưới tương đối
  results/paired_diffs.csv    hiệu theo cặp (cùng RngRun, CRN) trên lưới tuyệt đối
  report/figures/sweep_*.png, old_reports_nf.png

Điểm bão hòa (pir_ratio, nội suy tuyến tính giữa hai điểm lưới; NaN nếu không vượt trong 0.1..1.3):
  (i)   throughput/offered < 0.95  — dễ trễ, vì chỉ các luồng đi qua kênh cổ chai bị chặn
  (ii)  lat_avg ≥ 2·16·hops_avg
  (iii) u_max ≥ 0.98
knee_pred: điểm gãy (ii) theo model Kleinrock + M/D/1 (mdl_predict.py).
"""

from __future__ import annotations

import argparse
import itertools
import math
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import mdl_predict
import nocrun

# Số liệu của báo cáo cũ (docs/, mục 4.2–4.3), chỉ có hai đầu PIR 0.010 và 0.014.
OLD_REPORT = {
    ("transpose1", "XY"): (1586, 2857),
    ("transpose1", "NF"): (162, 797),
    ("transpose1", "OE"): (183, 695),
    ("transpose2", "XY"): (1089, 2026),
    ("transpose2", "NF"): (1642, 2927),
    ("transpose2", "OE"): (185, 666),
}


def load(path) -> pd.DataFrame:
    # pir_arg là khóa dạng chuỗi (đúng chuỗi đã truyền vào --pir), không để pandas đổi thành số.
    df = pd.read_csv(path, dtype={"pir_arg": str, "ratio_target": str})
    df["thr_ratio"] = df["throughput"] / df["offered"]
    df["lat_norm"] = df["lat_avg"] / (16.0 * df["hops_avg"])
    return df


def summarize(df: pd.DataFrame) -> pd.DataFrame:
    keys = ["grid", "routing", "traffic", "pir_arg", "ratio_target"]
    metrics = ["lat_avg", "lat_avg_lb", "throughput", "offered", "thr_ratio", "lat_norm", "u_max",
               "pir", "pir_ratio", "pir_max_hop", "phi_hop", "saturated"]
    g = df.groupby(keys, dropna=False)
    out = g[metrics].mean()
    n = g["run"].nunique()
    out["n"] = n
    for m in ("lat_avg", "throughput", "thr_ratio", "u_max"):
        sd = g[m].std(ddof=1)
        out[f"{m}_ci"] = [nocrun.t975(k - 1) * s / math.sqrt(k) if k > 1 else math.nan
                          for k, s in zip(n, sd)]
    return out.reset_index()


def crossing(x: np.ndarray, y: np.ndarray, level: float, above: bool) -> float:
    """x nhỏ nhất mà y vượt level (above: y ≥ level; ngược lại y < level), nội suy tuyến tính."""
    hit = y >= level if above else y < level
    idx = np.flatnonzero(hit)
    if len(idx) == 0:
        return math.nan
    i = idx[0]
    if i == 0:
        return float(x[0])
    x0, x1, y0, y1 = x[i - 1], x[i], y[i - 1], y[i]
    return float(x0 + (level - y0) * (x1 - x0) / (y1 - y0)) if y1 != y0 else float(x1)


def saturation(summary: pd.DataFrame, rho: dict) -> pd.DataFrame:
    """rho: ρ_hop từng kênh theo (routing, traffic), để đếm số kênh đạt φ và gần φ."""
    rel = summary[summary["grid"] == "rel"].copy()
    rel["ratio"] = rel["ratio_target"].astype(float)
    rows = []
    for (algo, traffic), part in rel.groupby(["routing", "traffic"]):
        part = part.sort_values("ratio")
        x = part["ratio"].to_numpy()
        r_thr = crossing(x, part["thr_ratio"].to_numpy(), 0.95, above=False)
        r_lat = crossing(x, part["lat_norm"].to_numpy(), 2.0, above=True)
        r_umax = crossing(x, part["u_max"].to_numpy(), 0.98, above=True)
        pir_max = float(part["pir_max_hop"].iloc[0])
        rows.append({"routing": algo, "label": nocrun.LABELS[algo], "traffic": traffic,
                     "phi_hop": float(part["phi_hop"].iloc[0]), "pir_max_hop": pir_max,
                     "n_at_max": len(nocrun.max_channels(rho[(algo, traffic)])),
                     "n_near_max": len(nocrun.max_channels(rho[(algo, traffic)], 0.95)),
                     "knee_pred": mdl_predict.knee_for(rho[(algo, traffic)], traffic),
                     "sat_ratio_thr": r_thr, "sat_ratio_lat": r_lat, "sat_ratio_umax": r_umax,
                     "sat_pir_thr": r_thr * pir_max, "sat_pir_lat": r_lat * pir_max,
                     "sat_pir_umax": r_umax * pir_max})
    return pd.DataFrame(rows)


def paired(df: pd.DataFrame) -> pd.DataFrame:
    """Hiệu theo cặp trên lưới tuyệt đối: cùng traffic, pir, RngRun ⇒ cùng tập gói (CRN)."""
    ab = df[df["grid"] == "abs"]
    rows = []
    for (traffic, pir), part in ab.groupby(["traffic", "pir_arg"]):
        piv = {m: part.pivot(index="run", columns="routing", values=m)
               for m in ("lat_avg", "throughput", "u_max")}
        for a, b in itertools.combinations(nocrun.ALGOS, 2):
            for metric in ("lat_avg", "throughput"):
                p = piv[metric][[a, b]].dropna()
                n = len(p)
                d = p[a] - p[b]
                sd_d = d.std(ddof=1)
                hw = nocrun.t975(n - 1) * sd_d / math.sqrt(n) if n > 1 else math.nan
                sd_unpaired = math.sqrt(p[a].var(ddof=1) + p[b].var(ddof=1))
                rows.append({
                    "traffic": traffic, "pir": pir, "metric": metric, "a": a, "b": b,
                    "label_a": nocrun.LABELS[a], "label_b": nocrun.LABELS[b], "n": n,
                    "mean_a": p[a].mean(), "mean_b": p[b].mean(), "mean_diff": d.mean(),
                    "ci_half": hw, "significant": bool(abs(d.mean()) > hw) if n > 1 else False,
                    "sd_diff": sd_d, "var_reduction": sd_d / sd_unpaired if sd_unpaired > 0 else math.nan,
                    "u_max_a": piv["u_max"][a].mean(), "u_max_b": piv["u_max"][b].mean()})
    return pd.DataFrame(rows)


def plot_rel(summary: pd.DataFrame, metric: str, ylabel: str, fname: str, log: bool = False,
             identity: bool = False) -> None:
    rel = summary[summary["grid"] == "rel"].copy()
    rel["ratio"] = rel["ratio_target"].astype(float)
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.5), sharey=True)
    for ax, traffic in zip(axes, nocrun.TRAFFICS):
        for algo in nocrun.ALGOS:
            part = rel[(rel["traffic"] == traffic) & (rel["routing"] == algo)].sort_values("ratio")
            ax.plot(part["ratio"], part[metric], marker="o", ms=3, label=nocrun.LABELS[algo])
        if identity:
            ax.plot([0, 1.3], [0, 1.3], "k--", lw=1, label="y = x")
        ax.axvline(1.0, color="gray", ls=":", lw=1)
        ax.set_title(traffic)
        ax.set_xlabel("pir_ratio = PIR / PIR_max_hop")
        ax.grid(alpha=0.3)
        if log:
            ax.set_yscale("log")
    axes[0].set_ylabel(ylabel)
    axes[-1].legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(nocrun.FIGURES / fname, dpi=150)
    plt.close(fig)


def plot_break_vs_near_max(sat: pd.DataFrame, fname: str) -> None:
    """Điểm gãy (ii) lat_avg ≥ 2·16·hops_avg theo số kênh có ρ_hop ≥ 0.95·φ."""
    markers = {"uniform": "o", "transpose1": "^", "transpose2": "s"}
    colors = {algo: f"C{i}" for i, algo in enumerate(nocrun.ALGOS)}
    fig, ax = plt.subplots(figsize=(9, 5.5))
    for n, grp in sat.groupby("n_near_max"):
        grp = grp.sort_values("sat_ratio_lat", ascending=False).reset_index(drop=True)
        # Tách nhẹ các điểm cùng x (thang log) để không đè lên nhau.
        for k, r in grp.iterrows():
            x = n * (1 + 0.035 * (k - (len(grp) - 1) / 2))
            ax.scatter(x, r["sat_ratio_lat"], marker=markers[r["traffic"]],
                       color=colors[r["routing"]], s=55, edgecolor="black", linewidth=0.5)
        # Gom các điểm có điểm gãy gần nhau (< 0.02) thành một khối nhãn.
        clusters: list[list] = []
        for _, r in grp.iterrows():
            if clusters and abs(clusters[-1][-1]["sat_ratio_lat"] - r["sat_ratio_lat"]) < 0.02:
                clusters[-1].append(r)
            else:
                clusters.append([r])
        for cl in clusters:
            text = "\n".join(f"{r['routing']}/{r['traffic'].replace('transpose', 't')}"
                             f" {r['sat_ratio_lat']:.3f}" for r in cl)
            y = float(np.mean([r["sat_ratio_lat"] for r in cl]))
            ax.annotate(text, (n * (1 + 0.035 * len(grp) / 2), y), textcoords="offset points",
                        xytext=(8, 0), va="center", fontsize=6.5)
    for algo in nocrun.ALGOS:
        ax.scatter([], [], color=colors[algo], label=nocrun.LABELS[algo])
    for traffic, m in markers.items():
        ax.scatter([], [], marker=m, color="gray", label=traffic)
    ax.axhline(1.0, color="gray", ls=":", lw=1)
    ax.set_xscale("log")
    ax.set_xlabel("n_near_max = số kênh có ρ_hop ≥ 0.95·φ_hop (thang log)")
    ax.set_ylabel("điểm gãy (ii): pir_ratio khi lat_avg ≥ 2·16·hops_avg")
    ax.grid(alpha=0.3, which="both")
    ax.legend(fontsize=7, ncol=2)
    fig.tight_layout()
    fig.savefig(nocrun.FIGURES / fname, dpi=150)
    plt.close(fig)


def plot_abs(summary: pd.DataFrame, algos: list[str], traffics: list[str], fname: str,
             title: str, old: bool = False) -> None:
    ab = summary[summary["grid"] == "abs"]
    fig, axes = plt.subplots(1, len(traffics), figsize=(6 * len(traffics), 4.5), sharey=True)
    for ax, traffic in zip(np.atleast_1d(axes), traffics):
        for algo in algos:
            part = ab[(ab["traffic"] == traffic) & (ab["routing"] == algo)].sort_values("pir")
            line, = ax.plot(part["pir"], part["lat_avg"], marker="o", ms=3, label=nocrun.LABELS[algo])
            ax.axvline(part["pir_max_hop"].iloc[0], color=line.get_color(), ls=":", lw=1)
            if old and (traffic, algo) in OLD_REPORT:
                ax.scatter([0.010, 0.014], OLD_REPORT[(traffic, algo)], marker="x", s=50,
                           color=line.get_color(), label=f"{algo} (báo cáo cũ)")
        ax.set_yscale("log")
        ax.set_title(traffic)
        ax.set_xlabel("PIR [gói/node/chu kỳ]  (đường chấm: PIR_max_hop)")
        ax.grid(alpha=0.3)
    np.atleast_1d(axes)[0].set_ylabel("lat_avg [chu kỳ]")
    np.atleast_1d(axes)[-1].legend(fontsize=8)
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(nocrun.FIGURES / fname, dpi=150)
    plt.close(fig)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sweep", default=str(nocrun.RESULTS / "sweep.csv"))
    ap.add_argument("--allow-default", action="store_true")
    args = ap.parse_args()

    df = load(args.sweep)
    summary = summarize(df)
    summary.to_csv(nocrun.RESULTS / "sweep_summary.csv", index=False, float_format="%.6g")
    rho = nocrun.channel_pressure(nocrun.find_binary(args.allow_default), "hop")
    sat = saturation(summary, rho)
    sat.to_csv(nocrun.RESULTS / "saturation.csv", index=False, float_format="%.4f")
    pdiff = paired(df)
    pdiff.to_csv(nocrun.RESULTS / "paired_diffs.csv", index=False, float_format="%.6g")

    nocrun.FIGURES.mkdir(parents=True, exist_ok=True)
    plot_rel(summary, "lat_avg", "lat_avg [chu kỳ]", "sweep_latency_rel.png", log=True)
    plot_rel(summary, "thr_ratio", "throughput / offered", "sweep_throughput_rel.png")
    plot_rel(summary, "u_max", "u_max (kênh bận nhất)", "sweep_umax_rel.png", identity=True)
    plot_break_vs_near_max(sat, "saturation_vs_nearmax.png")
    plot_abs(summary, nocrun.ALGOS, ["transpose1", "transpose2"], "sweep_latency_abs.png",
             "Latency theo PIR tuyệt đối (so với Fig. 12/13 của bài báo)")
    plot_abs(summary, ["XY", "NF", "NF_WS"], ["transpose1", "transpose2"], "old_reports_nf.png",
             "NF (quy ước chính) và NF (bài báo) so với XY; dấu x: báo cáo cũ", old=True)

    pd.set_option("display.width", 220)
    fmt = lambda v: f"{v:.4f}"  # noqa: E731
    print("## Điểm bão hòa (pir_ratio; 1.0 = PIR_max_hop)")
    print(sat[["label", "traffic", "phi_hop", "pir_max_hop", "n_at_max", "n_near_max",
               "sat_ratio_thr", "sat_ratio_lat", "sat_ratio_umax", "knee_pred"]].to_string(index=False, float_format=fmt))

    print("\n## Hiệu theo cặp lat_avg (lưới tuyệt đối, CRN): A − B ± CI 95% (* = có ý nghĩa)")
    lat = pdiff[pdiff["metric"] == "lat_avg"].set_index(["traffic", "pir", "a", "b"])
    wanted = [("OE", "XY"), ("NF_WS", "XY"), ("OE", "NF_WS"), ("NF", "XY")]
    table = []
    for traffic in ("transpose1", "transpose2"):
        for pir in sorted(pdiff["pir"].unique(), key=float):
            row = {"traffic": traffic, "pir": pir}
            for a, b in wanted:
                # paired_diffs.csv lưu theo thứ tự tổ hợp (a trước b trong ALGOS); đổi dấu nếu cần.
                sign, key = (1, (a, b)) if (traffic, pir, a, b) in lat.index else (-1, (b, a))
                r = lat.loc[(traffic, pir, *key)]
                row[f"{nocrun.LABELS[a]} − {nocrun.LABELS[b]}"] = (
                    f"{sign * r['mean_diff']:.1f} ± {r['ci_half']:.1f}{'*' if r['significant'] else ''}")
            table.append(row)
    print(pd.DataFrame(table).to_string(index=False))

    print("\n## Giả thuyết về báo cáo cũ: lat_avg ở PIR 0.010 và 0.014")
    ab = summary[summary["grid"] == "abs"]
    rows = []
    for traffic in ("transpose1", "transpose2"):
        for algo in ("XY", "NF", "NF_WS", "OE"):
            part = ab[(ab["traffic"] == traffic) & (ab["routing"] == algo)].set_index("pir_arg")
            old = OLD_REPORT.get((traffic, algo), (math.nan, math.nan)) if algo != "NF_WS" else (math.nan, math.nan)
            rows.append({"traffic": traffic, "routing": nocrun.LABELS[algo],
                         "lat@0.010": part.loc["0.01", "lat_avg"], "lat@0.014": part.loc["0.014", "lat_avg"],
                         "old@0.010": old[0], "old@0.014": old[1]})
    print(pd.DataFrame(rows).to_string(index=False, float_format=lambda v: f"{v:.1f}"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
