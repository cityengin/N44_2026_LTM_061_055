# Kết quả: Routing Pressure trên NoC 7×7 (ns-3.48)

File này được sinh bởi `scripts/make_results_md.py`; KHÔNG sửa tay. Mọi con số đọc từ file trong `results/` (CSV/TXT) và docs/dien-giai.md. Tái lập toàn bộ và so từng byte với các file đã commit: `scripts/reproduce.sh`. Build profile release (-O3, không -march=native); RNG cố định (RngRun 1..5 cho sweep và link_validate).

## Kiểm kê

| # | Mục | File nguồn | Commit |
|---|---|---|---|
| 1 | Bảng φ theo hai phương pháp (path k/p, hop) | `results/phi_all.txt` | c7f4ea5 |
| 2 | Mốc so với bài báo (MATCH/MISMATCH) | `results/selftest.txt` | c7f4ea5 |
| 3 | Kiểm chứng ρ trên từng kênh (link_validate) | `results/link_validate_summary.csv`, `results/link_validate_z.csv`, `results/link_validate_channels.csv`, `report/figures/link_validate.png` | aede5ad |
| 4 | Heatmap độ sử dụng kênh | `report/figures/heatmap_pred.png`, `report/figures/heatmap_meas.png`, `results/link_validate_channels.csv` | 2a1afa5 |
| 5 | Điểm bão hòa (3 định nghĩa, n_near_max) | `results/saturation.csv`, `report/figures/sweep_latency_rel.png`, `report/figures/sweep_throughput_rel.png`, `report/figures/saturation_vs_nearmax.png` | 73a6e1c |
| 6 | u_max theo pir_ratio | `results/sweep.csv`, `report/figures/sweep_umax_rel.png` | c7f4ea5 |
| 7 | Hiệu theo cặp giữa các thuật toán (CRN) | `results/paired_diffs.csv`, `report/figures/sweep_latency_abs.png` | 42a2883 |
| 8 | So sánh với hai báo cáo cũ | `results/sweep.csv`, `results/phi_all.txt`, `report/figures/old_reports_nf.png` | c7f4ea5 |
| 9 | Hai model hàng đợi | `results/mdl_predict.csv`, `results/saturation.csv`, `results/phi_all.txt`, `report/figures/mdl_predict.png` | c7f4ea5 |
| 10 | Số liệu kiểm thử | `results/selftest.txt`, `results/simcheck.txt` | c7f4ea5 |

## 1. Bảng φ theo hai phương pháp (path k/p, hop)

- File nguồn (commit gần nhất): `results/phi_all.txt` (`c7f4ea5`)
- Lệnh tạo ra: `~/ns-3-dev/build/scratch/noc/ns3.48-noc --mode=phi --channels=1 --turns=1 > results/phi_all.txt`

Lưới 7×7. Với uniform, φ chưa chuẩn hóa; PIR_max tính từ φ chuẩn hóa (chia N−1 = 48). NF_WS là biến thể chẩn đoán, khi so với bài báo ghi là "NF (bài báo)". FULL chỉ dùng để kiểm thử.

| routing | traffic | φ path | φ hop | φ hop chuẩn hóa | PIR_max hop | số kênh đạt φ hop |
|---|---|---|---|---|---|---|
| XY | uniform | 84.000000 | 84.000000 | 1.750000 | 0.035714 | 56 |
| XY | transpose1 | 6.000000 | 6.000000 | 6.000000 | 0.010417 | 4 |
| XY | transpose2 | 6.000000 | 6.000000 | 6.000000 | 0.010417 | 4 |
| YX | uniform | 84.000000 | 84.000000 | 1.750000 | 0.035714 | 56 |
| YX | transpose1 | 6.000000 | 6.000000 | 6.000000 | 0.010417 | 4 |
| YX | transpose2 | 6.000000 | 6.000000 | 6.000000 | 0.010417 | 4 |
| NF (quy ước chính) | uniform | 111.064646 | 111.750000 | 2.328125 | 0.026846 | 2 |
| NF (quy ước chính) | transpose1 | 2.598990 | 2.406250 | 2.406250 | 0.025974 | 12 |
| NF (quy ước chính) | transpose2 | 6.000000 | 6.000000 | 6.000000 | 0.010417 | 4 |
| NF (bài báo) | uniform | 111.064646 | 111.750000 | 2.328125 | 0.026846 | 2 |
| NF (bài báo) | transpose1 | 6.000000 | 6.000000 | 6.000000 | 0.010417 | 4 |
| NF (bài báo) | transpose2 | 2.598990 | 2.406250 | 2.406250 | 0.025974 | 12 |
| OE | uniform | 111.371429 | 118.687500 | 2.472656 | 0.025276 | 2 |
| OE | transpose1 | 4.361905 | 4.812500 | 4.812500 | 0.012987 | 1 |
| OE | transpose2 | 4.361905 | 4.812500 | 4.812500 | 0.012987 | 1 |
| FULL | uniform | 112.208947 | 105.750000 | 2.203125 | 0.028369 | 8 |
| FULL | transpose1 | 2.598990 | 2.406250 | 2.406250 | 0.025974 | 12 |
| FULL | transpose2 | 2.598990 | 2.406250 | 2.406250 | 0.025974 | 12 |

## 2. Mốc so với bài báo (MATCH/MISMATCH)

- File nguồn (commit gần nhất): `results/selftest.txt` (`c7f4ea5`)
- Lệnh tạo ra: `~/ns-3-dev/build/scratch/noc/ns3.48-noc --mode=selftest > results/selftest.txt`

Số đường 0→48 trên 7×7 (test HARD):

| routing | got | expect | status |
|---|---|---|---|
| FULL | 924 | 924 | PASS |
| OE | 84 | 84 | PASS |
| XY | 1 | 1 | PASS |

φ so với bài báo (test SOFT, ±0.005; NF (bài báo) so với số NF của bài báo):

| routing | traffic | method | mesh | got | bài báo | status |
|---|---|---|---|---|---|---|
| XY | transpose1 | path | 7x7 | 6.000000 | 6 | MATCH |
| XY | transpose1 | hop | 7x7 | 6.000000 | 6 | MATCH |
| OE | transpose1 | path | 7x7 | 4.361905 | 4.81 | MISMATCH |
| OE | transpose1 | hop | 7x7 | 4.812500 | 4.81 | MATCH |
| NF (quy ước chính) | transpose1 | path | 7x7 | 2.598990 | 6 | MISMATCH |
| NF (quy ước chính) | transpose1 | hop | 7x7 | 2.406250 | 6 | MISMATCH |
| NF (bài báo) | transpose1 | path | 7x7 | 6.000000 | 6 | MATCH |
| NF (bài báo) | transpose1 | hop | 7x7 | 6.000000 | 6 | MATCH |
| XY | transpose2 | path | 7x7 | 6.000000 | 6 | MATCH |
| XY | transpose2 | hop | 7x7 | 6.000000 | 6 | MATCH |
| OE | transpose2 | path | 7x7 | 4.361905 | 4.81 | MISMATCH |
| OE | transpose2 | hop | 7x7 | 4.812500 | 4.81 | MATCH |
| NF (quy ước chính) | transpose2 | path | 7x7 | 6.000000 | 2.41 | MISMATCH |
| NF (quy ước chính) | transpose2 | hop | 7x7 | 6.000000 | 2.41 | MISMATCH |
| NF (bài báo) | transpose2 | path | 7x7 | 2.598990 | 2.41 | MISMATCH |
| NF (bài báo) | transpose2 | hop | 7x7 | 2.406250 | 2.41 | MATCH |
| XY | uniform | path | 3x3 | 6.000000 | 6 | MATCH |
| XY | uniform | hop | 3x3 | 6.000000 | 6 | MATCH |

## 3. Kiểm chứng ρ trên từng kênh (link_validate)

- File nguồn (commit gần nhất): `results/link_validate_summary.csv` (`aede5ad`), `results/link_validate_z.csv` (`aede5ad`), `results/link_validate_channels.csv` (`c7f4ea5`), `report/figures/link_validate.png` (`aede5ad`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python link_validate.py`

U_đo (mô phỏng, trung bình RngRun 1..5, measure 100000) so với U_hop = ρ_hop·PIR·16 và U_path = ρ_path·PIR·16, tại pir_ratio 0.5 và 0.9; 15 cấu hình.

| predictor | ratio | channels | n | r2 | slope | intercept | rmse | max_abs_err | max_at |
|---|---|---|---|---|---|---|---|---|---|
| hop | 0.5 | active | 1972 | 0.999537 | 0.999514 | -0.000418777 | 0.00285666 | 0.011077 | OE/transpose2/r0.5/16-23 |
| hop | 0.9 | active | 1972 | 0.999762 | 0.999227 | -0.000311005 | 0.00368745 | 0.015633 | NF_WS/transpose2/r0.9/22-15 |
| hop | all | all | 5040 | 0.999862 | 0.998945 | -0.000172891 | 0.00291773 | 0.015633 | NF_WS/transpose2/r0.9/22-15 |
| hop | all | active | 3944 | 0.999777 | 0.999313 | -0.000358628 | 0.00329832 | 0.015633 | NF_WS/transpose2/r0.9/22-15 |
| path | 0.5 | active | 1972 | 0.952895 | 0.961164 | 0.00982553 | 0.0288126 | 0.212681 | OE/transpose2/r0.5/33-26 |
| path | 0.9 | active | 1972 | 0.953305 | 0.960993 | 0.0180727 | 0.0516162 | 0.383492 | OE/transpose1/r0.9/19-26 |
| path | all | all | 5040 | 0.977899 | 0.980587 | 0.00519944 | 0.0369764 | 0.383492 | OE/transpose1/r0.9/19-26 |
| path | all | active | 3944 | 0.964154 | 0.969784 | 0.0106843 | 0.0417995 | 0.383492 | OE/transpose1/r0.9/19-26 |

z = (mean U_đo − U_pred)/(sd/√5), kênh hoạt động. Tham chiếu khi sd ước lượng từ 5 mẫu (t₄): sd(z) = 1.414, P(|z|>3) = 0.0399. mean(z) âm là nhiễu tạo gói dùng chung giữa các cấu hình cùng RngRun (xem mục Hạn chế).

| predictor | scope | n | n_sd0 | mean_z | se_mean_z | sd_z | frac_abs_gt3 |
|---|---|---|---|---|---|---|---|
| hop | all | 3944 | 0 | -0.262867 | 0.0222412 | 1.39678 | 0.0425963 |
| path | all | 3944 | 0 | -0.921066 | 0.286968 | 18.0219 | 0.44929 |

Hình: `report/figures/link_validate.png`.

## 4. Heatmap độ sử dụng kênh

- File nguồn (commit gần nhất): `report/figures/heatmap_pred.png` (`2a1afa5`), `report/figures/heatmap_meas.png` (`2a1afa5`), `results/link_validate_channels.csv` (`c7f4ea5`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python heatmap.py`

Tại pir_ratio 0.9, thang màu chung 0..1, viền đen là các kênh đạt φ theo dự đoán.

| routing | traffic | số kênh đạt φ | max U_hop | max U_đo | kênh U_đo max | thuộc tập kênh đạt φ |
|---|---|---|---|---|---|---|
| XY | transpose1 | 4 | 0.900 | 0.904 | 42-35 | có |
| XY | transpose2 | 4 | 0.900 | 0.900 | 47-48 | có |
| OE | transpose1 | 1 | 0.900 | 0.906 | 19-26 | có |
| OE | transpose2 | 1 | 0.900 | 0.897 | 33-26 | có |
| NF (bài báo) | transpose1 | 4 | 0.900 | 0.904 | 42-35 | có |
| NF (bài báo) | transpose2 | 12 | 0.900 | 0.909 | 17-16 | có |

Hình: `report/figures/heatmap_pred.png`, `report/figures/heatmap_meas.png`.

## 5. Điểm bão hòa (3 định nghĩa, n_near_max)

- File nguồn (commit gần nhất): `results/saturation.csv` (`73a6e1c`), `report/figures/sweep_latency_rel.png` (`42a2883`), `report/figures/sweep_throughput_rel.png` (`42a2883`), `report/figures/saturation_vs_nearmax.png` (`2a1afa5`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python analyze_sweep.py`

Điểm bão hòa theo pir_ratio (1.0 = PIR_max_hop), nội suy trên lưới tương đối 0.1..1.3: (i) sat_ratio_thr: throughput/offered < 0.95; (ii) sat_ratio_lat: lat_avg ≥ 2·16·hops_avg; (iii) sat_ratio_umax: u_max ≥ 0.98. n_near_max = số kênh có ρ_hop ≥ 0.95·φ. knee_pred, knee_pred_sc: điểm gãy (ii) theo model kleinrock, kleinrock_sc (mục 9). "—" = không vượt trong lưới.

| label | traffic | phi_hop | pir_max_hop | n_at_max | n_near_max | sat_ratio_thr | sat_ratio_lat | sat_ratio_umax | knee_pred | knee_pred_sc |
|---|---|---|---|---|---|---|---|---|---|---|
| NF (quy ước chính) | transpose1 | 2.4062 | 0.0260 | 12 | 12 | 1.1119 | 0.9044 | 0.9563 | 0.8660 | 0.9365 |
| NF (quy ước chính) | transpose2 | 6.0000 | 0.0104 | 4 | 4 | 1.2019 | 0.9937 | 0.9842 | 0.8933 | 0.9902 |
| NF (quy ước chính) | uniform | 2.3281 | 0.0268 | 2 | 12 | 1.1476 | 0.9038 | 0.9749 | 0.8478 | 0.9375 |
| NF (bài báo) | transpose1 | 6.0000 | 0.0104 | 4 | 4 | 1.2095 | 0.9968 | 0.9890 | 0.8933 | 0.9902 |
| NF (bài báo) | transpose2 | 2.4062 | 0.0260 | 12 | 12 | 1.1177 | 0.9036 | 0.9599 | 0.8660 | 0.9365 |
| NF (bài báo) | uniform | 2.3281 | 0.0268 | 2 | 12 | 1.1447 | 0.9024 | 0.9712 | 0.8478 | 0.9375 |
| OE | transpose1 | 4.8125 | 0.0130 | 1 | 1 | 1.2931 | 1.0074 | 0.9975 | 0.9755 | 0.9932 |
| OE | transpose2 | 4.8125 | 0.0130 | 1 | 1 | — | 1.0089 | 0.9926 | 0.9755 | 0.9932 |
| OE | uniform | 2.4727 | 0.0253 | 2 | 4 | 1.2545 | 0.9503 | 0.9791 | 0.9187 | 0.9806 |
| XY | transpose1 | 6.0000 | 0.0104 | 4 | 4 | 1.2137 | 1.0006 | 0.9898 | 0.8933 | 0.9902 |
| XY | transpose2 | 6.0000 | 0.0104 | 4 | 4 | 1.2041 | 0.9954 | 0.9873 | 0.8933 | 0.9902 |
| XY | uniform | 1.7500 | 0.0357 | 56 | 56 | 1.0503 | 0.8220 | 0.9621 | 0.7490 | 0.8748 |
| YX | transpose1 | 6.0000 | 0.0104 | 4 | 4 | 1.2136 | 1.0028 | 0.9938 | 0.8933 | 0.9902 |
| YX | transpose2 | 6.0000 | 0.0104 | 4 | 4 | 1.2067 | 0.9985 | 0.9864 | 0.8933 | 0.9902 |
| YX | uniform | 1.7500 | 0.0357 | 56 | 56 | 1.0506 | 0.8224 | 0.9554 | 0.7490 | 0.8748 |

Hình: `report/figures/sweep_latency_rel.png`, `report/figures/sweep_throughput_rel.png`, `report/figures/saturation_vs_nearmax.png`.

## 6. u_max theo pir_ratio

- File nguồn (commit gần nhất): `results/sweep.csv` (`c7f4ea5`), `report/figures/sweep_umax_rel.png` (`42a2883`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python sweep.py ; cd scripts && ../.venv/bin/python analyze_sweep.py`

Trung bình u_max (kênh bận nhất trong cửa sổ đo) qua RngRun 1..5. Kỳ vọng bám y = x tới 1 rồi đi ngang; các cấu hình nhiều kênh gần max nhỉnh hơn y = x vì max của nhiều ước lượng có nhiễu bị đẩy lên.

| routing | traffic | r0.5 | r0.8 | r0.9 | r1.0 | r1.1 | r1.3 |
|---|---|---|---|---|---|---|---|
| NF (quy ước chính) | transpose1 | 0.524 | 0.846 | 0.954 | 1.000 | 1.000 | 1.000 |
| NF (quy ước chính) | transpose2 | 0.510 | 0.799 | 0.898 | 0.996 | 1.000 | 1.000 |
| NF (quy ước chính) | uniform | 0.512 | 0.822 | 0.928 | 0.998 | 1.000 | 1.000 |
| NF (bài báo) | transpose1 | 0.505 | 0.798 | 0.896 | 0.990 | 1.000 | 1.000 |
| NF (bài báo) | transpose2 | 0.530 | 0.834 | 0.950 | 1.000 | 1.000 | 1.000 |
| NF (bài báo) | uniform | 0.522 | 0.836 | 0.935 | 0.998 | 1.000 | 1.000 |
| OE | transpose1 | 0.494 | 0.774 | 0.880 | 0.983 | 1.000 | 1.000 |
| OE | transpose2 | 0.488 | 0.798 | 0.902 | 0.986 | 1.000 | 1.000 |
| OE | uniform | 0.501 | 0.819 | 0.912 | 0.998 | 1.000 | 1.000 |
| XY | transpose1 | 0.510 | 0.789 | 0.891 | 0.990 | 1.000 | 1.000 |
| XY | transpose2 | 0.511 | 0.794 | 0.894 | 0.992 | 1.000 | 1.000 |
| XY | uniform | 0.542 | 0.845 | 0.947 | 1.000 | 1.000 | 1.000 |
| YX | transpose1 | 0.499 | 0.792 | 0.888 | 0.986 | 1.000 | 1.000 |
| YX | transpose2 | 0.500 | 0.800 | 0.901 | 0.992 | 1.000 | 1.000 |
| YX | uniform | 0.538 | 0.851 | 0.955 | 1.000 | 1.000 | 1.000 |

Hình: `report/figures/sweep_umax_rel.png`.

## 7. Hiệu theo cặp giữa các thuật toán (CRN)

- File nguồn (commit gần nhất): `results/paired_diffs.csv` (`42a2883`), `report/figures/sweep_latency_abs.png` (`42a2883`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python analyze_sweep.py`

lat_avg trên lưới tuyệt đối, A − B ± CI 95% (t với df = 4), * = có ý nghĩa. Ghép cặp theo RngRun nhờ CRN (cùng traffic, PIR, RngRun ⇒ cùng tập gói).

| traffic | pir | OE − XY | NF (bài báo) − XY | OE − NF (bài báo) | NF (quy ước chính) − XY |
|---|---|---|---|---|---|
| transpose1 | 0.009 | -7.5 ± 2.7* | 0.7 ± 1.5 | -8.2 ± 2.7* | -12.1 ± 2.2* |
| transpose1 | 0.01 | -25.2 ± 8.9* | 1.6 ± 6.3 | -26.8 ± 10.9* | -32.3 ± 8.6* |
| transpose1 | 0.011 | -275.9 ± 78.6* | 9.5 ± 24.8 | -285.4 ± 100.0* | -285.9 ± 79.5* |
| transpose1 | 0.012 | -816.6 ± 105.2* | 0.8 ± 37.9 | -817.4 ± 122.8* | -834.0 ± 108.5* |
| transpose1 | 0.013 | -1498.5 ± 139.1* | 25.3 ± 73.3 | -1523.8 ± 105.5* | -1545.9 ± 152.2* |
| transpose1 | 0.014 | -2261.3 ± 125.9* | 26.8 ± 65.5 | -2288.1 ± 104.8* | -2448.7 ± 152.1* |
| transpose1 | 0.015 | -2957.6 ± 112.2* | 22.1 ± 59.0 | -2979.7 ± 127.2* | -3413.1 ± 127.3* |
| transpose2 | 0.009 | -7.2 ± 1.6* | -12.1 ± 1.9* | 4.9 ± 0.4* | 0.0 ± 1.5 |
| transpose2 | 0.01 | -28.2 ± 3.8* | -36.1 ± 3.9* | 7.9 ± 0.9* | -3.4 ± 2.2* |
| transpose2 | 0.011 | -309.6 ± 33.7* | -321.6 ± 32.7* | 11.9 ± 2.3* | 2.3 ± 30.1 |
| transpose2 | 0.012 | -860.2 ± 61.3* | -880.5 ± 60.0* | 20.2 ± 7.2* | 3.7 ± 44.9 |
| transpose2 | 0.013 | -1569.2 ± 124.7* | -1619.5 ± 130.5* | 50.4 ± 17.3* | -22.0 ± 69.3 |
| transpose2 | 0.014 | -2316.4 ± 109.8* | -2513.2 ± 133.3* | 196.8 ± 62.1* | -20.7 ± 89.8 |
| transpose2 | 0.015 | -3034.1 ± 130.3* | -3464.8 ± 126.4* | 430.6 ± 54.0* | -29.3 ± 97.4 |

Hình: `report/figures/sweep_latency_abs.png`. Đầy đủ 10 cặp × 3 traffic × 7 PIR trong `results/paired_diffs.csv`.

## 8. So sánh với hai báo cáo cũ

- File nguồn (commit gần nhất): `results/sweep.csv` (`c7f4ea5`), `results/phi_all.txt` (`c7f4ea5`), `report/figures/old_reports_nf.png` (`42a2883`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python analyze_sweep.py`

Số liệu báo cáo cũ lấy từ docs/ (mục 4.2–4.3, chỉ có hai đầu PIR), lưu trong `analyze_sweep.OLD_REPORT`. φ của họ (t1 = 6, t2 = 2.60) khớp NF (bài báo) + path; mô phỏng của họ (NF tốt ở t1, tệ ở t2) khớp NF (quy ước chính).

| traffic | routing | φ path | φ hop | lat@0.010 | lat@0.014 | báo cáo cũ @0.010 | báo cáo cũ @0.014 |
|---|---|---|---|---|---|---|---|
| transpose1 | XY | 6.000000 | 6.000000 | 126.1 | 2549.7 | 1586 | 2857 |
| transpose1 | NF (quy ước chính) | 2.598990 | 2.406250 | 93.7 | 101.0 | 162 | 797 |
| transpose1 | NF (bài báo) | 6.000000 | 6.000000 | 127.7 | 2576.5 | — | — |
| transpose1 | OE | 4.361905 | 4.812500 | 100.8 | 288.4 | 183 | 695 |
| transpose2 | XY | 6.000000 | 6.000000 | 130.3 | 2614.0 | 1089 | 2026 |
| transpose2 | NF (quy ước chính) | 6.000000 | 6.000000 | 126.9 | 2593.3 | 1642 | 2927 |
| transpose2 | NF (bài báo) | 2.598990 | 2.406250 | 94.2 | 100.8 | — | — |
| transpose2 | OE | 4.361905 | 4.812500 | 102.1 | 297.6 | 185 | 666 |

Hình: `report/figures/old_reports_nf.png`.

## 9. Hai model hàng đợi

- File nguồn (commit gần nhất): `results/mdl_predict.csv` (`c7f4ea5`), `results/saturation.csv` (`73a6e1c`), `results/phi_all.txt` (`c7f4ea5`), `report/figures/mdl_predict.png` (`73a6e1c`)
- Lệnh tạo ra: `cd scripts && ../.venv/bin/python mdl_predict.py`

kleinrock: Kleinrock independence + M/D/1. kleinrock_sc: bỏ self-contention (heuristic, W·(1 − Σ p_i²)). Phần chờ = lat − 16·hops. queue_delay ở PIR 0.001 đo được trong simcheck: 0.48–0.68 chu kỳ. Mô phỏng: OE/uniform gãy ở 0.950, nhóm tất định trên transpose ở 0.994–1.003. Phần mô hình đã khóa sau bước 6c.

| chỉ số | kleinrock | kleinrock_sc |
|---|---|---|
| queue pred/sim r0.1 | 1.64 … 4.74 | 0.89 … 1.09 |
| queue pred/sim r0.5 | 1.56 … 4.29 | 0.86 … 0.96 |
| queue pred/sim r0.9 | 1.36 … 3.72 | 0.67 … 0.83 |
| lat tổng r0.9 [%] | +14.5 … +60.2 | -17.3 … -3.8 |
| queue PIR 0.001 XY/transpose1 | 2.560 | 0.590 |
| queue PIR 0.001 XY/transpose2 | 2.560 | 0.590 |
| knee − (ii) | -0.109 … -0.032 | -0.016 … +0.053 |
| OE/uniform sớm hơn nhóm tất định trên transpose | không | có |

Hình: `report/figures/mdl_predict.png`. Theo cấu hình: `results/mdl_predict.csv`, cột knee_pred/knee_pred_sc trong `results/saturation.csv`.

## 10. Số liệu kiểm thử

- File nguồn (commit gần nhất): `results/selftest.txt` (`c7f4ea5`), `results/simcheck.txt` (`c7f4ea5`)
- Lệnh tạo ra: `~/ns-3-dev/build/scratch/noc/ns3.48-noc --mode=selftest > results/selftest.txt ; ~/ns-3-dev/build/scratch/noc/ns3.48-noc --mode=simcheck > results/simcheck.txt`

selftest: 181 PASS / 0 FAIL (HARD, hard_failures=0); SOFT so với bài báo: 11 MATCH / 7 MISMATCH (soft_match=11, soft_mismatch=7).

simcheck (RngRun mặc định 1): failures=0.

| check | kind | traffic | metric | status |
|---|---|---|---|---|
| low_load_lat_ge_16hops | hard | transpose1 | — | PASS |
| low_load_queue_delay | model | transpose1 | — | PASS |
| low_load_hops | stat | transpose1 | — | PASS |
| low_load_throughput | stat | transpose1 | — | PASS |
| low_load_lat_ge_16hops | hard | transpose2 | — | PASS |
| low_load_queue_delay | model | transpose2 | — | PASS |
| low_load_hops | stat | transpose2 | — | PASS |
| low_load_throughput | stat | transpose2 | — | PASS |
| mirror_nf_t1_vs_nfws_t2 | stat | — | lat_avg | PASS |
| mirror_nf_t1_vs_nfws_t2 | stat | — | throughput | PASS |

## Hạn chế và diễn giải

Trích nguyên văn mục "Diễn giải kết quả" của docs/dien-giai.md.

- Với mô hình store-and-forward + queue vô hạn, việc ngưỡng bão hòa bằng PIR_max gần như là hệ quả trực
  tiếp của bảo toàn lưu lượng (tải kênh = ρ_hop·PIR·16 khi router chọn đều theo hop) cộng với điều kiện
  ổn định (tải < 1 trên mọi kênh). Kết quả đó không phải là bằng chứng độc lập rằng φ là thước đo tốt.
- 5b (U_đo so với U_hop/U_path) kiểm chứng cài đặt (bộ tính φ và mô phỏng nhất quán) và bác bỏ giả
  thiết path (k/p). Nó KHÔNG chứng minh φ đúng trong mọi mô hình router.
- Quan sát (chưa phải quy luật đã kiểm): vị trí "gãy" theo latency (điểm (ii), lat_avg ≥ 2·16·hops_avg)
  giảm khi số kênh gần max tăng (n_near_max = số kênh có ρ_hop ≥ 0.95·φ):
  1 kênh (OE/t1, OE/t2) → 1.007–1.009; 4 kênh (XY, YX, NF/t2, NF_WS/t1) → 0.994–1.003; 12 kênh (NF,
  NF_WS thích ứng) → 0.902–0.904; 56 kênh (XY/YX uniform) → 0.822.
  n_near_max bị lẫn với loại traffic và loại routing: nhóm 4 kênh toàn là routing tất định trên transpose,
  nhóm 12 toàn là NF/NF_WS thích ứng, nhóm 56 là uniform. Vì vậy chưa tách được tác động của số kênh
  khỏi loại traffic. OE/uniform là điểm lệch: 4 kênh nhưng gãy ở 0.950.
  Xem report/figures/saturation_vs_nearmax.png và model ở bước 6b.
- Chưa kiểm: φ khi có backpressure (wormhole, buffer hữu hạn). Khi đó nghẽn ở một kênh lan ngược về
  phía nguồn, nên ngưỡng thực có thể thấp hơn PIR_max.
- Model giải tích (bước 6b, scripts/mdl_predict.py): xấp xỉ **Kleinrock independence + M/D/1**. Mỗi kênh
  là một hàng đợi M/D/1 độc lập (luồng đến Poisson λ = ρ_hop·PIR, phục vụ 16 chu kỳ):
  W = U·16/(2(1−U)), lat_pred = 16·hops_avg + Σ λ·W / Λ.
  - Dự đoán trước khi chạy: model sẽ ước lượng cao, vì trong chuỗi hàng đợi nối tiếp (tandem) với thời
    gian phục vụ cố định, gói ra khỏi một kênh đã cách nhau ≥ 16 chu kỳ, nên ít va nhau ở kênh sau hơn
    so với giả thiết Poisson.
  - Thực tế: đúng dấu, model cao hơn ở mọi cấu hình và mọi pir_ratio. Phần chờ (bỏ 16·hops) bị ước lượng
    cao 1.4–2.1 lần với uniform, routing thích ứng và OE; 3.5–4.7 lần với routing tất định trên transpose
    (XY, YX, NF/t2, NF_WS/t1), nơi luồng đi theo đường cố định nên hiệu ứng giãn gói mạnh nhất. Ở PIR 0.001
    (XY/t1, t2): dự đoán 2.56 chu kỳ so với 0.48–0.68 đo được. Sai số latency tổng: +1–3% ở ratio 0.1,
    tới +14–60% ở ratio 0.9.
  - knee_pred (cột trong saturation.csv) luôn sớm hơn (ii) 0.03–0.11. Model giữ đúng thứ tự giữa các nhóm
    56 kênh (0.749 so với 0.822) < 12 kênh (0.848–0.866 so với 0.902–0.904) < 1 kênh (0.976 so với
    1.007–1.009), nhưng KHÔNG tái hiện điểm lệch OE/uniform: model xếp OE/uniform (0.919) sau nhóm tất
    định trên transpose (0.893), trong khi mô phỏng thì ngược lại (0.950 so với 0.994–1.003). Lý do: model
    ước lượng cao nhiều nhất đúng ở nhóm tất định trên transpose.
- Hiệu chỉnh duy nhất (bước 6c), model **kleinrock_sc** (bỏ self-contention):
  - Cơ chế: gói đến một kênh từ cùng một cổng vào (kênh láng giềng) đã cách nhau ≥ 16 chu kỳ vì kênh đó
    chỉ truyền một gói mỗi 16 chu kỳ, nên chúng không xung đột với nhau. Ví dụ XY/t1: kênh cổ chai 6-13
    nhận toàn bộ lưu lượng từ 5-6 (node 6 tự map nên không phát), nên dù U = 0.9 vẫn không phải chờ.
  - Công thức (HEURISTIC, không suy ra chặt chẽ): W_ch = W_M/D/1(U_ch)·(1 − Σ_i p_i²), với
    p_i = rate(i→ch)/ρ(ch), i chạy trên các kênh vào từ node láng giềng; luồng local KHÔNG nằm trong tổng.
    rate lấy từ `noc --mode=phi --turns=1` (dòng PHITURN, cùng cây tìm kiếm với ρ; test HARD bảo toàn
    turn_in_sum_eq_rho và turn_out_plus_term_eq_rho).
  - So sánh hai model (tỉ số phần chờ pred/sim; điểm gãy so với (ii)):

    | | kleinrock | kleinrock_sc |
    |---|---|---|
    | queue pred/sim, ratio 0.1 | 1.64–4.74 | 0.89–1.09 |
    | queue pred/sim, ratio 0.5 | 1.56–4.29 | 0.86–0.96 |
    | queue pred/sim, ratio 0.9 | 1.36–3.72 | 0.67–0.83 |
    | lat tổng, ratio 0.9 | +14% … +60% | −3.8% … −17.3% |
    | queue_delay PIR 0.001 XY/t1, t2 (đo 0.48–0.68) | 2.56 | 0.59 |
    | knee − (ii) | −0.109 … −0.032 | −0.016 … +0.053 |

  - knee_pred_sc: OE/t1, t2 0.993 (sim 1.007–1.009); tất định trên transpose 0.990 (sim 0.994–1.003);
    OE/uniform 0.981 (sim 0.950); NF/NF_WS thích ứng 0.936–0.938 (sim 0.902–0.904); XY/YX uniform 0.875
    (sim 0.822). Model mới giữ đúng toàn bộ thứ tự điểm gãy của mô phỏng, kể cả xếp OE/uniform sớm hơn
    nhóm tất định trên transpose, nhưng khoảng cách chỉ 0.009 (sim: 0.044–0.053).
  - Sai số còn lại có dấu ngược model cũ: ở tải cao kleinrock_sc ước lượng thấp phần chờ (0.67–0.83),
    nên điểm gãy của các cấu hình nhiều kênh gần max bị dự đoán muộn hơn (+0.03…+0.05).
- **Phần mô hình đã khóa sau 6c**: không hiệu chỉnh thêm công thức hay tham số của model.
