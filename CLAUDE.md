# NoC Routing Pressure (ns-3) — đồ án Lập trình mạng

Tái hiện Tang, Lin, Palesi, "Routing Pressure: A Channel-Related and Traffic-Aware
Metric of Routing Algorithm", IEEE TPDS 26(3), 2015 (docs/). Hai báo cáo cũ trong
docs/ chỉ để tham khảo các lỗi đã mắc, không sao chép.

## Build / run
- ns-3 ở ~/ns-3-dev (ns-3.48). noc/ được symlink vào scratch/noc → chương trình `noc`.
- Build: `cd ~/ns-3-dev && ./ns3 build noc`
- Chạy: `./ns3 run --no-build "noc --mode=phi --routing=OE --traffic=transpose1"`
- Cờ ẩn `--selftest-force-fail`: thêm một test HARD luôn fail (selftest phải thoát mã 1), dùng để kiểm tra exit code.
- Build profile: **release** (`./ns3 configure --build-profile=release --enable-examples`): -O3, không
  -march=native. KHÔNG dùng `optimized`: trong ns-3.48 nó bật NS3_NATIVE_OPTIMIZATIONS
  (-march=native -mtune=native), có thể sinh FMA làm kết quả lệch từng byte giữa các máy.
  Binary release không có hậu tố: `build/scratch/noc/ns3.48-noc`. NS_ASSERT/NS_LOG tắt, NS_ABORT vẫn chạy.
  Đã kiểm: selftest, simcheck, phi và các dòng RESULT mẫu giống hệt từng byte so với profile default.
  Quay lại profile default: `./ns3 configure --build-profile=default --enable-examples`.
- C++ theo chuẩn mặc định của ns-3.48 (CMake của ns-3 đặt tối thiểu C++23), không thêm thư viện ngoài.
  Python dùng ~/noc-routing-pressure/.venv.

## Quy ước BẮT BUỘC (không tự đổi)
- Lưới W×H (mặc định 7×7). Gốc góc trên-trái, x tăng về Đông, y tăng về Nam, id = y*W + x.
- Kênh = link có hướng u→v.
- Chỉ định tuyến tối thiểu (minimal).
- NF: hướng âm = West, North; cấm turn E→N và S→W.
- OE theo Chiu (2000): cột đánh số từ 0; cột chẵn cấm E→N, E→S; cột lẻ cấm N→W, S→W. North = y giảm.
- Routing function trả về TẬP cổng hợp lệ; selection (chọn 1 cổng) tách riêng.
  Bộ tính φ và router mô phỏng phải gọi CÙNG một hàm routing, không được có 2 bản cài đặt.
- Traffic: uniform; transpose1 (x,y)→(N−1−y, N−1−x); transpose2 (x,y)→(y,x).
  Node tự map vào chính nó thì KHÔNG phát gói.
- 1 chu kỳ = 1 ns; frate = 0.5 flit/chu kỳ; len = 8 flit ⇒ mỗi gói chiếm link đúng 16 chu kỳ.
  Frame trên dây = 32 B (gồm 2 B header PPP + NoCHeader + payload), DataRate 16 Gbps, channel delay 0.

## Hai cách tính φ (phải có cả hai)
- path: chia đều theo số đường (k/p).
- hop: chia đều theo nhánh tại mỗi node (như Fig. 9 bài báo, tương đương router chọn ngẫu nhiên).
- Uniform: in φ chưa chuẩn hóa; khi suy PIR_max thì mỗi cặp có trọng số 1/(N−1).
- PIR_max = 0.0625 / φ.

## Mốc kiểm thử (từ bài báo)
- Số đường 0→48 trên 7×7: fully adaptive 924, OE 84, XY 1.
- φ transpose1: XY 6, OE 4.81, NF 6. φ transpose2: XY 6, OE 4.81, NF 2.41.
- 3×3 uniform, XY: φ chưa chuẩn hóa = 6.
- Channel dependency graph của XY, YX, NF, OE phải không có chu trình.
Nếu không khớp: KHÔNG chỉnh code để ép ra số. Báo cáo biến thể nào cho ra số nào.

## Phát hiện
- NF/transpose2 theo quy ước chính: transpose2 (x,y)→(y,x) cho dx = y−x và dy = x−y luôn trái dấu,
  nên mọi cặp là SW hoặc NE. Với các cặp này NF (âm = W, N) là tất định (SW: W rồi S; NE: N rồi E).
  Vì vậy mọi channel pressure là số nguyên và φ(NF, transpose2) KHÔNG THỂ bằng 2.41.
  Mốc 2.41 không đạt được dưới quy ước chính; đây không phải lỗi code.
- Giả thuyết (đã kiểm chứng ở bước 3, xem kết quả bên dưới): bài báo (Noxim) coi hướng âm = W, S.
  Lật y → N−1−y biến transpose1 ↔ transpose2 và NF(W,N) ↔ NF(W,S), còn XY và OE không đổi.
  Do đó φ(NF,t1) = φ(NF_WS,t2) và φ(NF,t2) = φ(NF_WS,t1). Đẳng thức này đúng tới từng kênh
  (test `mirror_channel_pressure`).
- Quyết định: thêm `NF_WS` (âm = W, S; cấm E→S, N→W) làm một Algo đầy đủ. Nó dùng chung hàm routing
  cho φ, CDG và sim (`--routing=NF_WS`). `NF` mặc định vẫn theo quy ước chính. Mọi output của NF_WS
  ghi `variant=diagnostic`. Không đổi mốc kiểm thử ở trên.
- Kết quả bước 3 (7×7, `noc --mode=phi`):

  | routing | traffic | path | hop | bài báo |
  |---|---|---|---|---|
  | XY | t1 / t2 | 6 / 6 | 6 / 6 | 6 / 6 |
  | OE | t1 / t2 | 4.361905 / 4.361905 | 4.8125 / 4.8125 | 4.81 / 4.81 |
  | NF (quy ước chính) | t1 / t2 | 2.598990 / 6 | 2.40625 / 6 | 6 / 2.41 |
  | NF_WS (chẩn đoán) | t1 / t2 | 6 / 2.598990 | 6 / 2.40625 | 6 / 2.41 (của NF) |
  | XY, 3×3 uniform (chưa chuẩn hóa) | — | 6 | 6 | 6 |

  - Phương pháp **hop** kết hợp **NF_WS** khớp mọi số của bài báo trong sai số ±0.005
    (4.8125 → 4.81, 2.40625 → 2.41). Nhiều khả năng bài báo dùng hop (Fig. 9) và NF với âm = W, S.
  - Phương pháp path cho OE = 4.36 và NF_WS t2 = 2.60, đúng bằng số trong hai báo cáo cũ.
    Vậy hai báo cáo cũ đã dùng path (k/p) chứ không dùng hop.
  - NF theo quy ước chính cho t1/t2 đảo so với bài báo, đúng như dự đoán từ đối xứng.
- Khi so với bài báo (bảng, đồ thị, báo cáo), NF_WS được ghi nhãn **"NF (bài báo)"**. Dòng RESULT vẫn
  giữ `routing=NF_WS,variant=diagnostic`; script Python đổi nhãn khi vẽ.
- ρ_hop là giá trị dự đoán chuẩn cho mô phỏng: PIR_max, pir_ratio và U_hop đều tính từ ρ_hop.
  ρ_path chỉ để tham khảo.
- Bước 5 (mô phỏng, 7×7): U_đo theo kênh so với U_hop có R² 0.9998, slope 0.999, RMSE 0.0033 (≈ sàn
  nhiễu); so với U_path chỉ R² 0.964, sai số tới 0.38 (OE). Latency gãy và u_max chạm 1 tại
  pir_ratio ≈ 1 ở cả 15 cấu hình, tức φ_hop dự đoán đúng ngưỡng nghẽn.
- Hai báo cáo cũ: φ của họ khớp NF_WS + path (t1 = 6, t2 = 2.60), còn mô phỏng của họ (NF tốt ở t1,
  tệ ở t2) khớp NF theo quy ước chính. Vậy họ đã dùng hai định nghĩa NF khác nhau cho φ và cho
  mô phỏng, đúng lỗi mà quy ước "một hàm routing duy nhất" ngăn chặn.

## Mô phỏng (--mode=sim, --mode=simcheck)
- Mô hình store-and-forward ở mức gói, queue DropTail 100000p, khác Noxim (wormhole, buffer 4 flit).
  Vì vậy latency tuyệt đối khác bài báo; cái cần so là ngưỡng nghẽn với PIR_max = 0.0625/φ_hop.
- Link PPP: DataRate 16 Gbps, Delay 0, InterframeGap 0. NoCHeader 10 B (src u16, dst u16, seq u32,
  hops u16), payload = 32 − 2 − 10 = 20 B. Mọi frame được assert bằng 32 B ở trace PhyTxBegin.
- Nhận gói bằng SetReceiveCallback, gọi SAU PointToPointHelper::Install (Node::AddDevice ghi đè callback).
- Drop = trace "Drop" của Queue + "PhyRxDrop". Không nối "MacTxDrop" vì nó trùng với Drop của Queue.
- Selection: mỗi router một UniformRandomVariable, chọn đều trong các bit của routeSet theo thứ tự E, W, N, S.
  Stream cố định theo id logic y·W+x (KHÔNG dùng Node::GetId()): 3·id chọn hướng, 3·id+1 khoảng cách
  tạo gói, 3·id+2 đích uniform. Cùng --RngRun thì cùng kết quả.
- Tạo gói: Bernoulli mỗi chu kỳ với xác suất PIR (khoảng cách hình học, thời điểm tạo là số nguyên ns).
  Bài báo dùng phân phối mũ; CV của khoảng cách hình học là √(1−PIR) ≈ 1 nên khác biệt không đáng kể
  với PIR ≤ 0.03.
- Latency đo từ lúc TẠO gói (CreationTimeTag), gồm cả thời gian chờ tại nguồn. Chỉ thống kê các gói tạo
  trong cửa sổ [warmup, warmup+measure) (mặc định 1000 và 20000). Sau đó là pha drain tối đa --drain
  (mặc định 20000), vẫn tiếp tục tạo gói; dừng khi mọi gói của cửa sổ đã tới đích.
- throughput = flit tới đích trong cửa sổ / (W·H·measure); offered = flit tạo trong cửa sổ / (W·H·measure).
  saturated=1 khi còn gói của cửa sổ chưa tới đích; lat_avg_lb tính cả các gói đó với
  latency = thời điểm dừng − thời điểm tạo (cận dưới).
  saturated=0 KHÔNG có nghĩa là chưa bão hòa: drain đủ dài vẫn xả hết backlog (vd OE/t1 ở 1.3·PIR_max:
  saturated=0 nhưng lat_avg ≈ 1500). Xác định bão hòa bằng throughput/offered, lat_avg hoặc u_max.
- U_đo của kênh = số gói bắt đầu truyền trong cửa sổ · 16 / measure; u_max/u_max_ch trong RESULT là kênh
  bận nhất. U_đo ≤ 1 chỉ đảm bảo khi measure chia hết cho 16 (hai lần truyền cách nhau ≥ 16 ns).
- `--mode=simcheck` (chạy lâu hơn, không thuộc selftest, exit 1 nếu FAIL):
  - Tải thấp XY t1/t2 ở PIR 0.001, 4 check: (a, hard) mọi gói lat_i ≥ 16·hops_i;
    (b, model) 0 ≤ lat_avg − 16·hops_avg ≤ 1 chu kỳ; (c, stat) |hops_avg − 224/42| ≤ 4·SE với
    SE = √(8.889/delivered), dùng phương sai lý thuyết; (d, stat) drops = 0, undelivered = 0,
    throughput trong 5% quanh PIR·8·flows/49.
  - NF/t1 so với NF_WS/t2 qua RngRun 1..5 (cố định, không theo --RngRun; α = 0.05; FAIL thì tự chạy
    lại với 1..10).
  - lat_p95 ở tải thấp: chỉ 2/42 nguồn có 12 hop (kỳ vọng 4,76%), nên p95 = 192 khi và chỉ khi số gói
    12 hop vượt ngưỡng nearest-rank (khoảng 5%), ngược lại rơi vào nhóm 10 hop (165–176). Đã kiểm qua
    RngRun 1..10: hàm percentile đúng, bộ tạo traffic không lệch (z gộp ≤ 1,9).

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

## Lưu ý ns-3 đã biết
- Không cài Internet stack. Gửi bằng NetDevice::Send với protocol 0x0800
  (PointToPointNetDevice chỉ map IPv4/IPv6 sang PPP).
- Header làm tăng số byte trên dây; Tag thì không. Timestamp dùng Tag.
- Mọi noc/*.cc được biên dịch chung thành một chương trình. CMake của scratch nhận diện file có main
  bằng regex `main[(| (]` trên toàn bộ nội dung file .cc, kể cả comment (ví dụ "remain " cũng khớp).
  CHỈ noc/noc.cc được chứa chuỗi đó; file .h không bị quét.
  Kiểm tra: `grep -l -E "main[(| ]" noc/*.cc` chỉ được ra noc/noc.cc.

## Scripts (Python trong .venv; chạy từ scripts/)
- `nocrun.py`: thư viện chung. Chọn binary theo build profile hiện tại (đọc cmake-cache), từ chối
  profile default trừ khi có --allow-default; ghi đè được bằng NOC_BIN.
- `link_validate.py`: U_đo theo kênh so với U_hop/U_path → results/link_validate_{summary,z}.csv,
  report/figures/link_validate.png.
- `sweep.py`: lưới tương đối (pir_ratio 0.1..1.3) và tuyệt đối (PIR 0.009..0.015), RngRun 1..5, chạy
  song song, chạy tiếp được → results/sweep.csv.
- `analyze_sweep.py`: results/saturation.csv (3 định nghĩa bão hòa, n_at_max, n_near_max),
  results/paired_diffs.csv, hình sweep_*.png, old_reports_nf.png, saturation_vs_nearmax.png.
- `heatmap.py`: heatmap_pred.png (U_hop) và heatmap_meas.png (U_đo) tại pir_ratio 0.9, đọc
  results/link_validate_channels.csv.
- `mdl_predict.py`: hai model "kleinrock" và "kleinrock_sc" so với sweep → results/mdl_predict.csv,
  report/figures/mdl_predict.png. analyze_sweep.py import hàm knee_for của nó để thêm cột knee_pred,
  knee_pred_sc, nên thứ tự chạy là analyze_sweep.py rồi mdl_predict.py. Cần `noc --mode=phi --turns=1`
  (dòng PHITURN: lưu lượng hop từ cổng vào sang kênh ra, uniform đã chia N−1).
- CRN: stream tạo gói (3·id+1, 3·id+2) không phụ thuộc routing, nên cùng (traffic, pir, RngRun) thì cùng
  tập gói. So sánh thuật toán bằng hiệu theo cặp trên lưới tuyệt đối. Hệ quả: mọi cấu hình cùng RngRun
  chung nhiễu tạo gói, KHÔNG coi các kênh hay cấu hình là mẫu độc lập (vd mean z của link_validate âm
  ở RngRun 1..5 vì các run này tạo ít gói hơn kỳ vọng; ở RngRun 6..15 thì dương).

## Cách làm việc
- Luôn đề xuất kế hoạch và danh sách file trước khi viết code; chờ tôi duyệt.
- Thay đổi nhỏ, từng bước, mỗi bước có cách kiểm tra.
- Output máy đọc được: một dòng `RESULT,key=value,...` cho script Python.

## Quy trình Git
Remote: origin = https://github.com/cityengin/noc-routing-pressure.git (HTTPS, xác thực qua gh).
1. Mọi lệnh git chạy trong ~/noc-routing-pressure, KHÔNG BAO GIỜ commit trong ~/ns-3-dev
   (noc/ chỉ được symlink sang đó).
2. Sau mỗi thay đổi hoàn chỉnh (một đơn vị việc có thể kiểm tra được), chạy:
   `cd ~/ns-3-dev && ./ns3 build noc && ./ns3 run --no-build "noc --mode=selftest"`
   Chỉ commit lên main khi build sạch VÀ selftest thoát mã 0.
3. Nếu build hoặc selftest fail mà vẫn cần lưu tiến độ: commit vào nhánh wip/<tên-ngắn>, push nhánh đó,
   không đụng tới main, và báo cho tôi.
4. Commit message dạng "<bước>: <mô tả ngắn>",
   ví dụ "step2: routing XY/YX/NF/NF_WS/OE + invariant tests".
   Phần thân liệt kê các test mới và kết quả (số PASS/FAIL).
5. Trước khi commit: chạy git status, chỉ add các file liên quan. Không commit .venv/, build output,
   __pycache__/, file kết quả lớn hơn 5 MB, hay bất kỳ token/khóa nào.
6. Sau khi commit: git push origin <nhánh hiện tại>. Trong tin nhắn trả lời, báo hash commit và kết quả push.
7. Cấm: git push --force, amend hoặc rebase commit đã push, xóa nhánh, đổi remote.
   Nếu cần làm những việc này thì hỏi tôi trước.
8. Nếu push lỗi (auth, mạng, bị từ chối): dừng lại, báo nguyên văn lỗi,
   không tự thử cách khác để vượt qua.