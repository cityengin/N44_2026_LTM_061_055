"""Bước 5b: kiểm chứng ρ trên từng kênh.

So U_đo (mô phỏng, số gói·16/measure) với U_hop = ρ_hop·PIR·16 và U_path = ρ_path·PIR·16 cho
{XY, YX, NF, NF_WS, OE} × {uniform, t1, t2} × pir_ratio {0.5, 0.9} × RngRun 1..5.

Output:
  results/link_validate_channels.csv   trung bình theo kênh (không commit)
  results/link_validate_summary.csv    R², slope, RMSE, max|err| cho hop và path
  results/link_validate_z.csv          z = (mean U_đo − U_pred)/(sd/√n) theo cấu hình và gộp
  report/figures/link_validate.png     scatter U_đo theo U_hop và U_path
"""

from __future__ import annotations

import argparse
import math
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import nocrun


def build_jobs(phi, args):
    jobs = []
    for algo in nocrun.ALGOS:
        for traffic in nocrun.TRAFFICS:
            pir_max = phi[(algo, traffic)]["pir_max_hop"]
            for ratio in args.ratios:
                for run in range(args.first_run, args.first_run + args.runs):
                    jobs.append({"routing": algo, "traffic": traffic, "ratio": ratio, "run": run,
                                 "pir": nocrun.fmt_pir(ratio * pir_max)})
    return jobs[: args.limit] if args.limit else jobs


def metrics(df: pd.DataFrame, pred: str) -> dict:
    y = df["u_meas"].to_numpy()
    x = df[pred].to_numpy()
    err = y - x
    ss_tot = float(((y - y.mean()) ** 2).sum())
    slope, intercept = np.polyfit(x, y, 1) if len(x) > 1 and x.std() > 0 else (math.nan, math.nan)
    i = int(np.abs(err).argmax())
    at = df.iloc[i]
    return {
        "n": len(df),
        "r2": 1 - float((err ** 2).sum()) / ss_tot if ss_tot > 0 else math.nan,
        "slope": float(slope),
        "intercept": float(intercept),
        "rmse": float(np.sqrt((err ** 2).mean())),
        "max_abs_err": float(abs(err[i])),
        "max_at": f"{at['routing']}/{at['traffic']}/r{at['ratio']}/{at['ch']}",
    }


def z_stats(df: pd.DataFrame, pred: str, runs: int) -> dict:
    se = df["u_meas_sd"] / math.sqrt(runs)
    ok = se > 0
    z = (df.loc[ok, "u_meas"] - df.loc[ok, pred]) / se[ok]
    return {
        "n": int(ok.sum()),
        "n_sd0": int((~ok).sum()),
        "mean_z": float(z.mean()),
        "se_mean_z": float(z.std(ddof=1) / math.sqrt(len(z))) if len(z) > 1 else math.nan,
        "sd_z": float(z.std(ddof=1)) if len(z) > 1 else math.nan,
        "frac_abs_gt3": float((z.abs() > 3).mean()),
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ratios", type=float, nargs="+", default=[0.5, 0.9])
    ap.add_argument("--runs", type=int, default=5)
    ap.add_argument("--first-run", type=int, default=1, help="RngRun đầu tiên (chạy first-run .. first-run+runs-1)")
    ap.add_argument("--tag", default="", help="hậu tố tên file output, vd _r6-15 (để kiểm tra phụ)")
    ap.add_argument("--warmup", type=int, default=10000)
    ap.add_argument("--measure", type=int, default=100000)
    ap.add_argument("--drain", type=int, default=20000)
    ap.add_argument("--jobs", type=int, default=nocrun.default_jobs())
    ap.add_argument("--limit", type=int, default=0, help="chỉ chạy N job đầu (để thử)")
    ap.add_argument("--allow-default", action="store_true")
    args = ap.parse_args()

    binary = nocrun.find_binary(args.allow_default)
    phi = nocrun.phi_table(binary)
    jobs = build_jobs(phi, args)
    print(f"# binary={binary} jobs={len(jobs)} workers={args.jobs}", flush=True)

    def worker(job):
        return nocrun.run_noc(binary, [
            "--mode=sim", f"--routing={job['routing']}", f"--traffic={job['traffic']}",
            f"--pir={job['pir']}", f"--RngRun={job['run']}", f"--warmup={args.warmup}",
            f"--measure={args.measure}", f"--drain={args.drain}", "--channels=1"])

    chan_rows, bad_runs = [], []

    def on_done(job, out):
        if isinstance(out, Exception):
            print(f"# LỖI {job}: {out}", file=sys.stderr, flush=True)
            return
        res = nocrun.lines_of(out, "RESULT")[0]
        if res["drops"] != "0" or res["hop_mismatch"] != "0" or res["saturated"] != "0":
            bad_runs.append({**job, "drops": res["drops"], "hop_mismatch": res["hop_mismatch"],
                             "saturated": res["saturated"]})
        for kv in nocrun.lines_of(out, "SIMCHAN"):
            chan_rows.append({"routing": job["routing"], "traffic": job["traffic"],
                              "ratio": job["ratio"], "run": job["run"], "ch": kv["ch"],
                              "u_meas": float(kv["u_meas"]), "u_hop": float(kv["u_hop"]),
                              "u_path": float(kv["u_path"])})

    errors = nocrun.run_parallel(jobs, worker, args.jobs, on_done)
    if errors:
        print(f"# {errors} job lỗi; dừng", file=sys.stderr)
        return 1

    # Job xong theo thứ tự bất kỳ; sắp xếp để tổng (trung bình) tất định từng bit.
    raw = pd.DataFrame(chan_rows).sort_values(["routing", "traffic", "ratio", "ch", "run"],
                                              kind="mergesort").reset_index(drop=True)
    keys = ["routing", "traffic", "ratio", "ch"]
    agg = raw.groupby(keys).agg(u_meas=("u_meas", "mean"), u_meas_sd=("u_meas", "std"),
                                u_hop=("u_hop", "first"), u_path=("u_path", "first"),
                                n_runs=("run", "nunique")).reset_index()
    nocrun.RESULTS.mkdir(exist_ok=True)
    agg.to_csv(nocrun.RESULTS / f"link_validate{args.tag}_channels.csv", index=False)

    det = agg[agg["routing"].isin(["XY", "YX"])]
    assert np.allclose(det["u_hop"], det["u_path"]), "XY/YX phải có U_hop == U_path"

    active = agg[(agg["u_hop"] > 0) | (agg["u_path"] > 0) | (agg["u_meas"] > 0)]
    noise_floor = float((active["u_meas_sd"] / math.sqrt(args.runs)).mean())

    # Bảng chỉ số dự đoán.
    summary = []
    for pred in ("u_hop", "u_path"):
        for ratio in [*args.ratios, "all"]:
            for subset, df in (("all", agg), ("active", active)):
                part = df if ratio == "all" else df[df["ratio"] == ratio]
                summary.append({"predictor": pred[2:], "ratio": ratio, "channels": subset,
                                **metrics(part, pred)})
    summary_df = pd.DataFrame(summary)
    summary_df.to_csv(nocrun.RESULTS / f"link_validate{args.tag}_summary.csv", index=False, float_format="%.6g")

    # z theo cấu hình và gộp (chỉ kênh hoạt động).
    zrows = []
    for pred in ("u_hop", "u_path"):
        zrows.append({"predictor": pred[2:], "scope": "all", **z_stats(active, pred, args.runs)})
        for (algo, traffic, ratio), part in active.groupby(["routing", "traffic", "ratio"]):
            zrows.append({"predictor": pred[2:], "scope": f"{algo}/{traffic}/r{ratio}",
                          **z_stats(part, pred, args.runs)})
    z_df = pd.DataFrame(zrows)
    z_df.to_csv(nocrun.RESULTS / f"link_validate{args.tag}_z.csv", index=False, float_format="%.6g")

    # Hình.
    nocrun.FIGURES.mkdir(parents=True, exist_ok=True)
    fig, axes = plt.subplots(1, 2, figsize=(11, 5), sharey=True)
    markers = {args.ratios[0]: "o", args.ratios[-1]: "^"}
    for ax, pred, title in ((axes[0], "u_hop", "U_hop (ρ_hop·PIR·16)"),
                            (axes[1], "u_path", "U_path (ρ_path·PIR·16)")):
        for algo in nocrun.ALGOS:
            for ratio in args.ratios:
                part = active[(active["routing"] == algo) & (active["ratio"] == ratio)]
                ax.scatter(part[pred], part["u_meas"], s=8, alpha=0.5,
                           marker=markers.get(ratio, "o"),
                           label=f"{nocrun.LABELS[algo]}, ratio {ratio}")
        ax.plot([0, 1], [0, 1], "k--", lw=1)
        ax.set_xlabel(title)
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
        ax.set_aspect("equal")
        ax.grid(alpha=0.3)
    axes[0].set_ylabel("U_đo (mô phỏng, trung bình 5 RngRun)")
    axes[1].legend(fontsize=7, loc="upper left")
    fig.suptitle("Độ sử dụng kênh: mô phỏng so với dự đoán (7×7, kênh hoạt động)")
    fig.tight_layout()
    fig.savefig(nocrun.FIGURES / f"link_validate{args.tag}.png", dpi=150)

    # In bảng.
    pd.set_option("display.width", 200)
    print(f"# kênh hoạt động={len(active)}, sàn nhiễu mean(sd/√{args.runs})={noise_floor:.5f}")
    print(f"# run có drops/hop_mismatch/saturated ≠ 0: {len(bad_runs)}")
    for b in bad_runs:
        print(f"#   {b}")
    print(summary_df.to_string(index=False, float_format=lambda v: f"{v:.5f}"))
    df_t = args.runs - 1
    p_t = nocrun.t_tail_prob(3.0, df_t)
    sd_t = f"{math.sqrt(df_t / (df_t - 2)):.3f}" if df_t > 2 else "∞"
    print(f"# z: kỳ vọng mean≈0; nếu sd ước lượng từ {args.runs} mẫu thì z ~ t_{df_t}: "
          f"sd(z)≈{sd_t}, P(|z|>3)≈{p_t:.4f} (chuẩn: sd 1, P≈0.0027)")
    print(z_df.to_string(index=False, float_format=lambda v: f"{v:.4f}"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
