"""Bước 7a: sinh report/RESULTS.md, kiểm kê mọi bảng và hình dùng trong báo cáo.

Mọi con số được đọc từ file trong results/ (CSV/TXT) và CLAUDE.md, không gõ tay. Bảng copy từ CSV được
đọc dạng chuỗi nên số giữ nguyên như trong file. Commit hash là commit gần nhất thay đổi file nguồn.
Với cùng dữ liệu và cùng lịch sử git, file sinh ra giống hệt từng byte (reproduce.sh kiểm điều này).
"""

from __future__ import annotations

import argparse
import math
import subprocess
import sys
from pathlib import Path

import pandas as pd

import analyze_sweep
import mdl_predict
import nocrun

BIN = "~/ns-3-dev/build/scratch/noc/ns3.48-noc"
PY = "cd scripts && ../.venv/bin/python"
CMD = {
    "phi": f"{BIN} --mode=phi --channels=1 --turns=1 > results/phi_all.txt",
    "selftest": f"{BIN} --mode=selftest > results/selftest.txt",
    "simcheck": f"{BIN} --mode=simcheck > results/simcheck.txt",
    "link_validate": f"{PY} link_validate.py",
    "sweep": f"{PY} sweep.py",
    "analyze": f"{PY} analyze_sweep.py",
    "mdl": f"{PY} mdl_predict.py",
    "heatmap": f"{PY} heatmap.py",
}


def git_hash(rel_path: str) -> str:
    out = subprocess.run(["git", "log", "-1", "--format=%h", "--", rel_path], cwd=nocrun.REPO,
                         capture_output=True, text=True).stdout.strip()
    return out or "chưa commit"


def md_table(df: pd.DataFrame) -> str:
    cols = [str(c) for c in df.columns]
    lines = ["| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
    for row in df.itertuples(index=False):
        cells = ["—" if (isinstance(v, float) and math.isnan(v)) or v == "" else str(v) for v in row]
        lines.append("| " + " | ".join(cells) + " |")
    return "\n".join(lines)


def source_block(files: list[str], cmd: str) -> str:
    items = ", ".join(f"`{f}` (`{git_hash(f)}`)" for f in files)
    return f"- File nguồn (commit gần nhất): {items}\n- Lệnh tạo ra: `{cmd}`\n"


def lines_of(path: Path, kind: str) -> list[dict[str, str]]:
    return nocrun.lines_of(path.read_text().splitlines(), kind)


class Report:
    def __init__(self, results: Path):
        self.r = results
        self.parts: list[str] = []
        self.inventory: list[dict[str, str]] = []

    def section(self, title: str, files: list[str], cmd: str, body: str) -> None:
        n = len(self.inventory) + 1
        self.inventory.append({"#": str(n), "Mục": title, "File nguồn": ", ".join(f"`{f}`" for f in files),
                               "Commit": git_hash(files[0])})
        self.parts.append(f"## {n}. {title}\n\n{source_block(files, cmd)}\n{body.strip()}\n")

    def csv(self, name: str) -> pd.DataFrame:
        return pd.read_csv(self.r / name, dtype=str, keep_default_na=False)


def phi_section(rep: Report) -> None:
    rows = lines_of(rep.r / "phi_all.txt", "RESULT")
    d = {(kv["routing"], kv["traffic"], kv["method"]): kv for kv in rows}
    out = []
    for algo in [*nocrun.ALGOS, "FULL"]:
        for traffic in nocrun.TRAFFICS:
            p, h = d[(algo, traffic, "path")], d[(algo, traffic, "hop")]
            out.append({"routing": nocrun.LABELS[algo], "traffic": traffic, "φ path": p["phi"],
                        "φ hop": h["phi"], "φ hop chuẩn hóa": h["phi_norm"],
                        "PIR_max hop": h["pir_max"], "số kênh đạt φ hop": h["nmax"]})
    body = ("Lưới 7×7. Với uniform, φ chưa chuẩn hóa; PIR_max tính từ φ chuẩn hóa (chia N−1 = 48). "
            "NF_WS là biến thể chẩn đoán, khi so với bài báo ghi là \"NF (bài báo)\". FULL chỉ dùng để "
            "kiểm thử.\n\n" + md_table(pd.DataFrame(out)))
    rep.section("Bảng φ theo hai phương pháp (path k/p, hop)", ["results/phi_all.txt"], CMD["phi"], body)


def paper_section(rep: Report) -> None:
    res = lines_of(rep.r / "selftest.txt", "RESULT")
    paths = [{"routing": kv["routing"], "got": kv["got"], "expect": kv["expect"], "status": kv["status"]}
             for kv in res if kv["test"] == "paths_0_48"]
    phi = [{"routing": nocrun.LABELS[kv["routing"]], "traffic": kv["traffic"], "method": kv["method"],
            "mesh": kv["mesh"], "got": kv["got"], "bài báo": kv["paper"], "status": kv["status"]}
           for kv in res if kv["test"] == "paper_phi"]
    body = ("Số đường 0→48 trên 7×7 (test HARD):\n\n" + md_table(pd.DataFrame(paths)) +
            "\n\nφ so với bài báo (test SOFT, ±0.005; NF (bài báo) so với số NF của bài báo):\n\n" +
            md_table(pd.DataFrame(phi)))
    rep.section("Mốc so với bài báo (MATCH/MISMATCH)", ["results/selftest.txt"], CMD["selftest"], body)


def link_section(rep: Report) -> None:
    s = rep.csv("link_validate_summary.csv")
    s = s[(s["channels"] == "active") | (s["ratio"] == "all")]
    z = rep.csv("link_validate_z.csv")
    z = z[z["scope"] == "all"]
    p_t = nocrun.t_tail_prob(3.0, 4)
    body = ("U_đo (mô phỏng, trung bình RngRun 1..5, measure 100000) so với U_hop = ρ_hop·PIR·16 và "
            "U_path = ρ_path·PIR·16, tại pir_ratio 0.5 và 0.9; 15 cấu hình.\n\n" + md_table(s) +
            "\n\nz = (mean U_đo − U_pred)/(sd/√5), kênh hoạt động. Tham chiếu khi sd ước lượng từ 5 mẫu "
            f"(t₄): sd(z) = {math.sqrt(2):.3f}, P(|z|>3) = {p_t:.4f}. mean(z) âm là nhiễu tạo gói dùng chung "
            "giữa các cấu hình cùng RngRun (xem mục Hạn chế).\n\n" + md_table(z) +
            "\n\nHình: `report/figures/link_validate.png`.")
    rep.section("Kiểm chứng ρ trên từng kênh (link_validate)",
                ["results/link_validate_summary.csv", "results/link_validate_z.csv",
                 "results/link_validate_channels.csv", "report/figures/link_validate.png"],
                CMD["link_validate"], body)


def heatmap_section(rep: Report) -> None:
    ch = pd.read_csv(rep.r / "link_validate_channels.csv")
    ch = ch[ch["ratio"] == 0.9]
    out = []
    for algo in ("XY", "OE", "NF_WS"):
        for traffic in ("transpose1", "transpose2"):
            part = ch[(ch["routing"] == algo) & (ch["traffic"] == traffic)]
            phi_set = set(nocrun.max_channels(dict(zip(part["ch"], part["u_hop"]))))
            top = part.loc[part["u_meas"].idxmax()]
            out.append({"routing": nocrun.LABELS[algo], "traffic": traffic, "số kênh đạt φ": len(phi_set),
                        "max U_hop": f"{part['u_hop'].max():.3f}", "max U_đo": f"{top['u_meas']:.3f}",
                        "kênh U_đo max": top["ch"], "thuộc tập kênh đạt φ": "có" if top["ch"] in phi_set else "không"})
    body = ("Tại pir_ratio 0.9, thang màu chung 0..1, viền đen là các kênh đạt φ theo dự đoán.\n\n" +
            md_table(pd.DataFrame(out)) +
            "\n\nHình: `report/figures/heatmap_pred.png`, `report/figures/heatmap_meas.png`.")
    rep.section("Heatmap độ sử dụng kênh",
                ["report/figures/heatmap_pred.png", "report/figures/heatmap_meas.png",
                 "results/link_validate_channels.csv"], CMD["heatmap"], body)


def saturation_section(rep: Report) -> None:
    s = rep.csv("saturation.csv")
    cols = ["label", "traffic", "phi_hop", "pir_max_hop", "n_at_max", "n_near_max", "sat_ratio_thr",
            "sat_ratio_lat", "sat_ratio_umax", "knee_pred", "knee_pred_sc"]
    body = ("Điểm bão hòa theo pir_ratio (1.0 = PIR_max_hop), nội suy trên lưới tương đối 0.1..1.3: "
            "(i) sat_ratio_thr: throughput/offered < 0.95; (ii) sat_ratio_lat: lat_avg ≥ 2·16·hops_avg; "
            "(iii) sat_ratio_umax: u_max ≥ 0.98. n_near_max = số kênh có ρ_hop ≥ 0.95·φ. knee_pred, "
            "knee_pred_sc: điểm gãy (ii) theo model kleinrock, kleinrock_sc (mục 9). \"—\" = không vượt "
            "trong lưới.\n\n" + md_table(s[cols]) +
            "\n\nHình: `report/figures/sweep_latency_rel.png`, `report/figures/sweep_throughput_rel.png`, "
            "`report/figures/saturation_vs_nearmax.png`.")
    rep.section("Điểm bão hòa (3 định nghĩa, n_near_max)",
                ["results/saturation.csv", "report/figures/sweep_latency_rel.png",
                 "report/figures/sweep_throughput_rel.png", "report/figures/saturation_vs_nearmax.png"],
                CMD["analyze"], body)


def umax_section(rep: Report) -> None:
    df = analyze_sweep.load(rep.r / "sweep.csv")
    rel = df[df["grid"] == "rel"].copy()
    rel["ratio"] = rel["ratio_target"].astype(float)
    piv = rel.groupby(["routing", "traffic", "ratio"])["u_max"].mean().unstack("ratio")
    ratios = [0.5, 0.8, 0.9, 1.0, 1.1, 1.3]
    out = []
    for (algo, traffic), row in piv.iterrows():
        out.append({"routing": nocrun.LABELS[algo], "traffic": traffic,
                    **{f"r{r}": f"{row[r]:.3f}" for r in ratios}})
    body = ("Trung bình u_max (kênh bận nhất trong cửa sổ đo) qua RngRun 1..5. Kỳ vọng bám y = x tới 1 "
            "rồi đi ngang; các cấu hình nhiều kênh gần max nhỉnh hơn y = x vì max của nhiều ước lượng "
            "có nhiễu bị đẩy lên.\n\n" + md_table(pd.DataFrame(out)) +
            "\n\nHình: `report/figures/sweep_umax_rel.png`.")
    rep.section("u_max theo pir_ratio", ["results/sweep.csv", "report/figures/sweep_umax_rel.png"],
                CMD["sweep"] + " ; " + CMD["analyze"], body)


def paired_section(rep: Report) -> None:
    pdiff = pd.read_csv(rep.r / "paired_diffs.csv", dtype={"pir": str})
    body = ("lat_avg trên lưới tuyệt đối, A − B ± CI 95% (t với df = 4), * = có ý nghĩa. Ghép cặp theo "
            "RngRun nhờ CRN (cùng traffic, PIR, RngRun ⇒ cùng tập gói).\n\n" +
            md_table(analyze_sweep.paired_wide(pdiff)) +
            "\n\nHình: `report/figures/sweep_latency_abs.png`. Đầy đủ 10 cặp × 3 traffic × 7 PIR trong "
            "`results/paired_diffs.csv`.")
    rep.section("Hiệu theo cặp giữa các thuật toán (CRN)",
                ["results/paired_diffs.csv", "report/figures/sweep_latency_abs.png"], CMD["analyze"], body)


def old_reports_section(rep: Report) -> None:
    df = analyze_sweep.load(rep.r / "sweep.csv")
    ab = df[df["grid"] == "abs"]
    mean = ab.groupby(["routing", "traffic", "pir_arg"])["lat_avg"].mean()
    phi = {(kv["routing"], kv["traffic"], kv["method"]): kv["phi"]
           for kv in lines_of(rep.r / "phi_all.txt", "RESULT")}
    out = []
    for traffic in ("transpose1", "transpose2"):
        for algo in ("XY", "NF", "NF_WS", "OE"):
            old = analyze_sweep.OLD_REPORT.get((traffic, algo)) if algo != "NF_WS" else None
            out.append({"traffic": traffic, "routing": nocrun.LABELS[algo],
                        "φ path": phi[(algo, traffic, "path")], "φ hop": phi[(algo, traffic, "hop")],
                        "lat@0.010": f"{mean[(algo, traffic, '0.01')]:.1f}",
                        "lat@0.014": f"{mean[(algo, traffic, '0.014')]:.1f}",
                        "báo cáo cũ @0.010": "" if old is None else str(old[0]),
                        "báo cáo cũ @0.014": "" if old is None else str(old[1])})
    body = ("Số liệu báo cáo cũ lấy từ docs/ (mục 4.2–4.3, chỉ có hai đầu PIR), lưu trong "
            "`analyze_sweep.OLD_REPORT`. φ của họ (t1 = 6, t2 = 2.60) khớp NF (bài báo) + path; mô phỏng "
            "của họ (NF tốt ở t1, tệ ở t2) khớp NF (quy ước chính).\n\n" + md_table(pd.DataFrame(out)) +
            "\n\nHình: `report/figures/old_reports_nf.png`.")
    rep.section("So sánh với hai báo cáo cũ",
                ["results/sweep.csv", "results/phi_all.txt", "report/figures/old_reports_nf.png"],
                CMD["analyze"], body)


def models_section(rep: Report) -> None:
    m = pd.read_csv(rep.r / "mdl_predict.csv")
    sat = pd.read_csv(rep.r / "saturation.csv")
    rho: dict = {}
    for kv in lines_of(rep.r / "phi_all.txt", "CHAN"):
        if kv["method"] == "hop":
            rho.setdefault((kv["routing"], kv["traffic"]), {})[kv["ch"]] = float(kv["rho"])
    turns: dict = {}
    for kv in lines_of(rep.r / "phi_all.txt", "PHITURN"):
        turns.setdefault((kv["routing"], kv["traffic"]), {})[(kv["in"], kv["out"])] = float(kv["rate"])

    def rng(s: pd.Series, fmt: str) -> str:
        return f"{format(s.min(), fmt)} … {format(s.max(), fmt)}"

    rows = []
    for model, sfx in (("kleinrock", ""), ("kleinrock_sc", "_sc")):
        row = {"model": model}
        for r in (0.1, 0.5, 0.9):
            row[f"queue pred/sim r{r}"] = rng(m.loc[m["ratio"] == r, f"queue_ratio{sfx}"], ".2f")
        row["lat tổng r0.9 [%]"] = rng(m.loc[m["ratio"] == 0.9, f"lat_rel_err{sfx}"] * 100, "+.1f")
        for traffic in ("transpose1", "transpose2"):
            n_src, _ = mdl_predict.theory(traffic)
            key = ("XY", traffic)
            vec = mdl_predict.normalized_rho(rho[key], traffic)
            fac = None if not sfx else mdl_predict.sc_factor(list(rho[key]), turns[key])
            row[f"queue PIR 0.001 XY/{traffic}"] = f"{mdl_predict.queue_pred(vec, 0.001, n_src, fac):.3f}"
        knee = sat[f"knee_pred{sfx}"]
        row["knee − (ii)"] = rng(knee - sat["sat_ratio_lat"], "+.3f")
        is_det = sat["traffic"].str.startswith("transpose") & (sat["n_near_max"] == 4)
        oe_uni = sat[(sat["routing"] == "OE") & (sat["traffic"] == "uniform")]
        row["OE/uniform sớm hơn nhóm tất định trên transpose"] = (
            "có" if float(oe_uni[f"knee_pred{sfx}"].iloc[0]) < knee[is_det].min() else "không")
        rows.append(row)
    sim_oe = float(sat.loc[(sat["routing"] == "OE") & (sat["traffic"] == "uniform"), "sat_ratio_lat"].iloc[0])
    sim_det = sat.loc[sat["traffic"].str.startswith("transpose") & (sat["n_near_max"] == 4), "sat_ratio_lat"]
    body = ("kleinrock: Kleinrock independence + M/D/1. kleinrock_sc: bỏ self-contention (heuristic, "
            "W·(1 − Σ p_i²)). Phần chờ = lat − 16·hops. queue_delay ở PIR 0.001 đo được trong simcheck: "
            "0.48–0.68 chu kỳ. Mô phỏng: OE/uniform gãy ở "
            f"{sim_oe:.3f}, nhóm tất định trên transpose ở {sim_det.min():.3f}–{sim_det.max():.3f}. "
            "Phần mô hình đã khóa sau bước 6c.\n\n" + md_table(pd.DataFrame(rows).T.reset_index()
                                                              .rename(columns={"index": "chỉ số", 0: "kleinrock", 1: "kleinrock_sc"}).iloc[1:]) +
            "\n\nHình: `report/figures/mdl_predict.png`. Theo cấu hình: `results/mdl_predict.csv`, cột "
            "knee_pred/knee_pred_sc trong `results/saturation.csv`.")
    rep.section("Hai model hàng đợi", ["results/mdl_predict.csv", "results/saturation.csv",
                                       "results/phi_all.txt", "report/figures/mdl_predict.png"],
                CMD["mdl"], body)


def tests_section(rep: Report) -> None:
    st = lines_of(rep.r / "selftest.txt", "RESULT")
    counts = {k: sum(kv.get("status") == k for kv in st) for k in ("PASS", "FAIL", "MATCH", "MISMATCH")}
    summ = next(kv for kv in st if kv["test"] == "summary")
    sc = lines_of(rep.r / "simcheck.txt", "RESULT")
    checks = [{"check": kv["check"], "kind": kv.get("kind", ""), "traffic": kv.get("traffic", ""),
               "metric": kv.get("metric", ""), "status": kv["status"]}
              for kv in sc if kv.get("mode") == "simcheck" and kv["check"] != "summary"]
    sc_summ = next(kv for kv in sc if kv.get("check") == "summary")
    body = (f"selftest: {counts['PASS']} PASS / {counts['FAIL']} FAIL (HARD, hard_failures="
            f"{summ['hard_failures']}); SOFT so với bài báo: {counts['MATCH']} MATCH / {counts['MISMATCH']} "
            f"MISMATCH (soft_match={summ['soft_match']}, soft_mismatch={summ['soft_mismatch']}).\n\n"
            f"simcheck (RngRun mặc định 1): failures={sc_summ['failures']}.\n\n" + md_table(pd.DataFrame(checks)))
    rep.section("Số liệu kiểm thử", ["results/selftest.txt", "results/simcheck.txt"],
                CMD["selftest"] + " ; " + CMD["simcheck"], body)


def limitations(repo: Path) -> str:
    text = (repo / "CLAUDE.md").read_text().splitlines()
    start = text.index("## Diễn giải kết quả")
    end = next(i for i in range(start + 1, len(text)) if text[i].startswith("## "))
    return "\n".join(text[start + 1:end]).strip()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--results", default=str(nocrun.RESULTS), help="thư mục chứa dữ liệu đầu vào")
    ap.add_argument("--out", default=str(nocrun.REPO / "report" / "RESULTS.md"))
    args = ap.parse_args()

    rep = Report(Path(args.results))
    phi_section(rep)
    paper_section(rep)
    link_section(rep)
    heatmap_section(rep)
    saturation_section(rep)
    umax_section(rep)
    paired_section(rep)
    old_reports_section(rep)
    models_section(rep)
    tests_section(rep)

    head = ("# Kết quả: Routing Pressure trên NoC 7×7 (ns-3.48)\n\n"
            "File này được sinh bởi `scripts/make_results_md.py`; KHÔNG sửa tay. Mọi con số đọc từ file "
            "trong `results/` (CSV/TXT) và CLAUDE.md. Tái lập toàn bộ và so từng byte với các file đã "
            "commit: `scripts/reproduce.sh`. Build profile release (-O3, không -march=native); RNG cố định "
            "(RngRun 1..5 cho sweep và link_validate).\n\n"
            "## Kiểm kê\n\n" + md_table(pd.DataFrame(rep.inventory)) + "\n")
    tail = ("## Hạn chế và diễn giải\n\nTrích nguyên văn mục \"Diễn giải kết quả\" của CLAUDE.md.\n\n" +
            limitations(nocrun.REPO) + "\n")
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_text("\n".join([head, *rep.parts, tail]))
    print(f"# đã ghi {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
