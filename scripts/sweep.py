"""Bước 5c: quét PIR cho {XY, YX, NF, NF_WS, OE} × {uniform, transpose1, transpose2}.

Lưới tương đối: pir = ratio · PIR_max_hop của từng cấu hình, ratio 0.1..1.3 bước 0.1.
Lưới tuyệt đối: PIR 0.009..0.015 bước 0.001 (so với Fig. 12/13 của bài báo; dùng cho hiệu theo cặp).
Mỗi điểm chạy RngRun 1..5, song song. Ghi results/sweep.csv; chạy lại thì bỏ qua các dòng đã có
(khóa: grid, routing, traffic, mesh, pir_arg, run, warmup, measure, drain). Job lỗi được ghi vào
results/sweep_errors.log và sẽ được thử lại ở lần chạy sau.
"""

from __future__ import annotations

import argparse
import csv
import sys
import time
from pathlib import Path

import nocrun

# Không ghi thời gian chạy vào CSV: mọi cột đều tất định nên file tái lập được từng byte.
META = ["grid", "ratio_target", "pir_arg"]
# Thứ tự trường của dòng RESULT,mode=sim (noc-sim.cc, FormatResult).
RESULT_FIELDS = ["mode", "routing", "variant", "traffic", "mesh", "pir", "seed", "run", "warmup",
                 "measure", "drain", "generated", "delivered", "undelivered", "saturated", "drops",
                 "hop_mismatch", "lat_avg", "lat_avg_lb", "lat_p95", "hops_avg", "throughput",
                 "offered", "phi_hop", "pir_max_hop", "pir_ratio", "u_max", "u_max_ch", "stop"]
KEY = ["grid", "routing", "traffic", "mesh", "pir_arg", "run", "warmup", "measure", "drain"]

RATIOS = [round(0.1 * i, 1) for i in range(1, 14)]
ABS_PIRS = [round(0.009 + 0.001 * i, 3) for i in range(7)]


def build_jobs(phi, args) -> list[dict]:
    jobs = []
    for algo in nocrun.ALGOS:
        for traffic in nocrun.TRAFFICS:
            pir_max = phi[(algo, traffic)]["pir_max_hop"]
            points = [("rel", r, r * pir_max) for r in RATIOS] + [("abs", "", p) for p in ABS_PIRS]
            for grid, ratio, pir in points:
                for run in range(1, args.runs + 1):
                    jobs.append({"grid": grid, "ratio_target": str(ratio), "routing": algo,
                                 "traffic": traffic, "mesh": "7x7", "pir_arg": nocrun.fmt_pir(pir),
                                 "run": str(run), "warmup": str(args.warmup),
                                 "measure": str(args.measure), "drain": str(args.drain)})
    # Job nặng (PIR cao) chạy trước để cân tải giữa các luồng.
    jobs.sort(key=lambda j: -float(j["pir_arg"]))
    return jobs


def sort_key(row: dict) -> tuple:
    return (row["grid"], row["routing"], row["traffic"], float(row["pir_arg"]), int(row["run"]),
            int(row["warmup"]), int(row["measure"]), int(row["drain"]))


def rewrite_sorted(path) -> None:
    """Sắp xếp các dòng theo khóa: thứ tự ghi phụ thuộc job nào xong trước, thứ tự cuối thì cố định."""
    with path.open(newline="") as f:
        rows = list(csv.DictReader(f))
    rows.sort(key=sort_key)
    tmp = path.with_name(path.name + ".tmp")
    with tmp.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=META + RESULT_FIELDS, extrasaction="raise")
        writer.writeheader()
        writer.writerows(rows)
    tmp.replace(path)


def done_keys(path) -> set[tuple]:
    if not path.exists():
        return set()
    with path.open(newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames != META + RESULT_FIELDS:
            raise SystemExit(f"{path}: header khác định dạng hiện tại; đổi --out hoặc xóa file")
        return {tuple(row[k] for k in KEY) for row in reader}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--runs", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=10000)
    ap.add_argument("--measure", type=int, default=20000)
    ap.add_argument("--drain", type=int, default=20000)
    ap.add_argument("--jobs", type=int, default=nocrun.default_jobs())
    ap.add_argument("--out", default=str(nocrun.RESULTS / "sweep.csv"))
    ap.add_argument("--limit", type=int, default=0, help="chỉ chạy N job còn thiếu đầu tiên")
    ap.add_argument("--dry-run", action="store_true", help="chỉ in số job tổng / đã có / sẽ chạy")
    ap.add_argument("--allow-default", action="store_true")
    args = ap.parse_args()

    out = Path(args.out)
    binary = nocrun.find_binary(args.allow_default)
    jobs = build_jobs(nocrun.phi_table(binary), args)
    done = done_keys(out)
    missing = [j for j in jobs if tuple(j[k] for k in KEY) not in done]
    todo = missing[: args.limit] if args.limit else missing
    print(f"# binary={binary} tổng={len(jobs)} đã có={len(jobs) - len(missing)} "
          f"sẽ chạy={len(todo)} workers={args.jobs}", flush=True)
    if args.dry_run:
        return 0
    if not todo:
        if out.exists():
            rewrite_sorted(out)
        return 0

    out.parent.mkdir(parents=True, exist_ok=True)
    new_file = not out.exists()
    f = out.open("a", newline="")
    writer = csv.DictWriter(f, fieldnames=META + RESULT_FIELDS, restval="", extrasaction="raise")
    if new_file:
        writer.writeheader()
        f.flush()
    errlog = out.with_name(out.stem + "_errors.log").open("a")

    def worker(job):
        lines = nocrun.run_noc(binary, [
            "--mode=sim", f"--routing={job['routing']}", f"--traffic={job['traffic']}",
            f"--pir={job['pir_arg']}", f"--RngRun={job['run']}", f"--warmup={job['warmup']}",
            f"--measure={job['measure']}", f"--drain={job['drain']}"])
        return nocrun.lines_of(lines, "RESULT")[0]

    count = 0
    t_start = time.monotonic()

    def on_done(job, result):
        nonlocal count
        count += 1
        if isinstance(result, Exception):
            errlog.write(f"{time.strftime('%F %T')} {job} {result}\n")
            errlog.flush()
            return
        kv = result
        if kv["mode"] != "sim" or kv["run"] != job["run"] or kv["routing"] != job["routing"]:
            raise RuntimeError(f"dòng RESULT không khớp job: {kv} / {job}")
        writer.writerow({"grid": job["grid"], "ratio_target": job["ratio_target"],
                         "pir_arg": job["pir_arg"], **kv})
        f.flush()
        if count % 50 == 0 or count == len(todo):
            print(f"# {count}/{len(todo)} ({time.monotonic() - t_start:.0f}s)", flush=True)

    errors = nocrun.run_parallel(todo, worker, args.jobs, on_done)
    f.close()
    errlog.close()
    rewrite_sorted(out)
    print(f"# xong: {len(todo) - errors} dòng mới, {errors} lỗi (xem {errlog.name})", flush=True)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
