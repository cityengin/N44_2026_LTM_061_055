"""Thư viện dùng chung cho các script: tìm binary noc, chạy, parse dòng RESULT/SIMCHAN."""

from __future__ import annotations

import math
import os
import re
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import Callable, Iterable

NS3_DIR = Path(os.environ.get("NS3_DIR", Path.home() / "ns-3-dev"))
REPO = Path(__file__).resolve().parent.parent
RESULTS = REPO / "results"
FIGURES = REPO / "report" / "figures"

ALGOS = ["XY", "YX", "NF", "NF_WS", "OE"]
TRAFFICS = ["uniform", "transpose1", "transpose2"]

# Nhãn khi so với bài báo (mục 4.1 của báo cáo): NF_WS là "NF (bài báo)".
LABELS = {
    "XY": "XY",
    "YX": "YX",
    "NF": "NF (quy ước chính)",
    "NF_WS": "NF (bài báo)",
    "OE": "OE",
    "FULL": "FULL",
}

# t_{0.975, df}, df = 1..30 (không có scipy trong .venv).
_T975 = [12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
         2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
         2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042]


def t975(df: int) -> float:
    return _T975[df - 1] if 1 <= df <= len(_T975) else 1.96


def current_profile_suffix() -> str:
    """Hậu tố binary của build profile đang cấu hình ("" với release, "-default", ...)."""
    cache = NS3_DIR / "cmake-cache" / "CMakeCache.txt"
    m = re.search(r"^build_profile_suffix:INTERNAL=(.*)$", cache.read_text(), re.MULTILINE)
    if m is None:
        raise SystemExit(f"không đọc được build_profile_suffix trong {cache}")
    return m.group(1).strip()


def find_binary(allow_default: bool = False) -> Path:
    """Binary noc của build profile HIỆN TẠI (binary của profile cũ vẫn nằm lại trong build/).

    Nhận release (không hậu tố hoặc "-release") và optimized; từ chối "-default" trừ khi
    allow_default. Có thể ghi đè bằng biến môi trường NOC_BIN.
    """
    env = os.environ.get("NOC_BIN")
    if env:
        return Path(env)
    suffix = current_profile_suffix()
    if suffix == "-default" and not allow_default:
        raise SystemExit("ns-3 đang ở profile default; configure --build-profile=release "
                         "trước khi chạy, hoặc dùng --allow-default")
    d = NS3_DIR / "build" / "scratch" / "noc"
    for name in (f"ns3.48-noc{suffix}", "ns3.48-noc-release"):
        if (d / name).exists():
            return d / name
    raise SystemExit(f"không tìm thấy binary noc{suffix} trong {d}; chạy ./ns3 build noc")


def run_noc(binary: Path, args: list[str], timeout: float = 3600) -> list[str]:
    proc = subprocess.run([str(binary), *args], capture_output=True, text=True, timeout=timeout)
    if proc.returncode != 0:
        tail = (proc.stderr or proc.stdout).strip().splitlines()[-3:]
        raise RuntimeError(f"noc {' '.join(args)} → exit {proc.returncode}: {' | '.join(tail)}")
    return proc.stdout.splitlines()


def parse_line(line: str) -> tuple[str, dict[str, str]]:
    """'RESULT,a=1,b=x' → ('RESULT', {'a': '1', 'b': 'x'})."""
    kind, *fields = line.strip().split(",")
    return kind, dict(f.split("=", 1) for f in fields if "=" in f)


def lines_of(lines: Iterable[str], kind: str) -> list[dict[str, str]]:
    return [kv for k, kv in map(parse_line, lines) if k == kind]


def fmt_pir(x: float) -> str:
    """Chuỗi truyền vào --pir; cũng là khóa để chạy tiếp (không so số thực)."""
    return f"{x:.6g}"


def phi_table(binary: Path, w: int = 7, h: int = 7) -> dict[tuple[str, str], dict[str, float]]:
    """φ và PIR_max của mọi (routing, traffic) theo cả hai phương pháp (φ đã chuẩn hóa)."""
    out = run_noc(binary, ["--mode=phi", f"--W={w}", f"--H={h}"])
    table: dict[tuple[str, str], dict[str, float]] = {}
    for kv in lines_of(out, "RESULT"):
        entry = table.setdefault((kv["routing"], kv["traffic"]), {})
        entry[f"phi_{kv['method']}"] = float(kv["phi_norm"])
        entry[f"pir_max_{kv['method']}"] = float(kv["pir_max"])
    return table


def channel_pressure(binary: Path, method: str = "hop", w: int = 7,
                     h: int = 7) -> dict[tuple[str, str], dict[str, float]]:
    """ρ từng kênh (chưa chuẩn hóa) của mọi (routing, traffic): {(routing, traffic): {"u-v": ρ}}."""
    out = run_noc(binary, ["--mode=phi", f"--method={method}", "--channels=1", f"--W={w}", f"--H={h}"])
    table: dict[tuple[str, str], dict[str, float]] = {}
    for kv in lines_of(out, "CHAN"):
        table.setdefault((kv["routing"], kv["traffic"]), {})[kv["ch"]] = float(kv["rho"])
    return table


def max_channels(rho: dict[str, float], frac: float = 1.0) -> list[str]:
    """Các kênh có ρ ≥ frac·max ρ (frac = 1: các kênh đạt φ, sai số 1e-6 tương đối)."""
    phi = max(rho.values())
    return [ch for ch, r in rho.items() if r >= frac * phi - 1e-6 * phi]


def run_parallel(jobs: list, worker: Callable, jobs_n: int, on_done: Callable) -> int:
    """Chạy worker(job) song song; on_done(job, result_or_exception) gọi ở luồng chính.
    Trả về số job lỗi."""
    errors = 0
    with ThreadPoolExecutor(max_workers=jobs_n) as pool:
        futures = {pool.submit(worker, job): job for job in jobs}
        for fut in as_completed(futures):
            job = futures[fut]
            try:
                on_done(job, fut.result())
            except Exception as exc:  # noqa: BLE001 — ghi lại mọi lỗi của job
                errors += 1
                on_done(job, exc)
    return errors


def default_jobs() -> int:
    return max(1, (os.cpu_count() or 2) - 1)


def t_tail_prob(t: float, df: int, steps: int = 200000) -> float:
    """P(|T_df| > t) bằng tích phân số (không có scipy)."""
    import numpy as np

    c = math.gamma((df + 1) / 2) / (math.sqrt(df * math.pi) * math.gamma(df / 2))
    x = np.linspace(t, t + 2000.0, steps)
    y = c * (1 + x * x / df) ** (-(df + 1) / 2)
    return float(2 * np.trapezoid(y, x))
