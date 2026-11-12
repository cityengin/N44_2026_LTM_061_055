<!-- Diễn giải kết quả của đề tài. scripts/make_results_md.py trích nguyên văn mục dưới đây vào
     report/RESULTS.md (phần "Hạn chế và diễn giải"). Sửa ở đây, rồi chạy lại make_results_md.py. -->

## Diễn giải kết quả
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
