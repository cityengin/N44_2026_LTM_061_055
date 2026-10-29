# NoC Routing Pressure (ns-3.48)

> **Đồ án môn Lập trình mạng**  
> Tái hiện, kiểm chứng và phân tích nghiên cứu:  
> **Tang, Lin, Palesi (IEEE TPDS 2015)**: *"Routing Pressure: A Channel-Related and Traffic-Aware Metric of Routing Algorithm"* ([docs/tang2015.pdf](docs/tang2015.pdf)).

---

## 📌 Giới thiệu

Đồ án xây dựng mô hình mô phỏng mạng trên chip (**Network-on-Chip — NoC**) cấu trúc lưới 2 chiều (**2D Mesh 7×7**) trên nền tảng mô phỏng mạng sự kiện rời rạc **ns-3 (bản 3.48)** viết bằng C++23. Dự án tập trung tính toán, đánh giá và kiểm chứng thực nghiệm chỉ số **Áp lực định tuyến (Routing Pressure — $\phi$)** và **Áp lực kênh (Channel Pressure — $\rho$)** đối với các thuật toán định tuyến:
* **Tất định (Deterministic):** XY, YX
* **Thích ứng bán phần (Partially Adaptive):** Negative-First (NF), Odd-Even (OE)
* **Thích ứng toàn phần (Fully Adaptive minimal):** FULL (dùng đối sánh kiểm thử)

trên các mô hình lưu lượng chuẩn: `uniform`, `transpose1`, `transpose2`.

Dự án đồng thời làm rõ bản chất lý thuyết, giải mã các điểm mâu thuẫn số liệu và khắc phục toàn diện các sai sót từ các báo cáo trước đó.

---

## 🔬 Các phát hiện & Đóng góp cốt lõi

1. **Phân định hai phương pháp tính $\phi$ (`path` vs `hop`):**
   * **`path` ($k/p$):** Chia đều xác suất theo số đường đi tối thiểu giữa cặp nguồn–đích ($k$ đường qua kênh / $p$ tổng số đường).
   * **`hop` (tại mỗi chặng):** Chia đều xác suất rẽ nhánh tại từng router trung gian (tương ứng với router chọn ngẫu nhiên đều 1 cổng hợp lệ).
   * *Thực nghiệm:* Đối chiếu 5 040 mẫu kênh (`link_validate.py`) chứng minh độ sử dụng kênh đo được trong ns-3 khớp hoàn hảo với phương pháp `hop` ($R^2 = 0,99986$, slope $\approx 0,999$, RMSE $\approx 0,0029$). Báo cáo cũ tính theo `path` nên bị lệch với mô phỏng và bài báo.

2. **Giải mã nghịch lý quy ước Negative-First (NF vs `NF_WS`):**
   * Quy ước Turn Model chuẩn (Glass & Ni 1994, Chiu 2000): hướng âm là Tây (W) và Bắc (N). Dưới quy ước này với `transpose2`, NF là tất định nên áp lực kênh luôn là số nguyên (không thể đạt giá trị $2,41$ như bài báo).
   * Dự án phát hiện simulator Noxim trong bài báo Tang 2015 coi hướng âm là Tây (W) và Nam (S). Bằng phép đối xứng gương qua trục $y$, dự án bổ sung biến thể chẩn đoán **`NF_WS`** (ghi nhãn *"NF (bài báo)"*), khớp chính xác 100% mọi số liệu của bài báo gốc.

3. **Nguyên tắc "Một hàm định tuyến duy nhất" (`routeSet`):**
   * [noc/routing.cc](noc/routing.cc) cung cấp hàm duy nhất `routeSet()`. Cả bộ tính áp lực $\phi$, bộ kiểm tra chu trình đồ thị phụ thuộc kênh CDG (phát hiện deadlock) và Router mô phỏng trong ns-3 đều gọi chung hàm này, triệt tiêu nguy cơ sai lệch giữa lý thuyết và mô phỏng.

4. **Mô hình hàng đợi giải tích cải tiến `kleinrock_sc`:**
   * Mở rộng mô hình Kleinrock M/D/1 bằng cách loại trừ hiện tượng tự xung đột (*self-contention*) dựa trên thông lượng rẽ nhánh (turn flows), dự đoán chính xác thứ tự điểm gãy độ trễ (latency knee) của toàn bộ 15 cấu hình mô phỏng.

5. **Quy chuẩn tái lập 100% từng byte (Byte-level Reproducibility):**
   * Sử dụng build profile `release` (-O3, không `-march=native`), gán seed/stream RNG logic cố định, áp dụng kỹ thuật Common Random Numbers (CRN) so sánh hiệu theo cặp. Script [scripts/reproduce.sh](scripts/reproduce.sh) cho phép tái lập toàn bộ dữ liệu từ đầu và so sánh khớp từng byte với kết quả đã công bố.

---

## 📂 Cấu trúc thư mục

```
noc-routing-pressure/
├── CLAUDE.md                    # Quy ước kỹ thuật, đặc tả mốc kiểm thử và quy trình Git
├── README.md                    # Tài liệu giới thiệu dự án
├── docs/                        # Tài liệu tham khảo gốc (Tang 2015 và các báo cáo cũ)
├── noc/                         # Mã nguồn C++ mô phỏng NoC trên ns-3 (symlink vào scratch/noc)
│   ├── topology.{h,cc}          # Lưới 2D Mesh (ánh xạ ID, tọa độ, kênh có hướng)
│   ├── routing.{h,cc}           # Hàm routeSet() minimal cho XY, YX, NF, NF_WS, OE, FULL
│   ├── searchtree.{h,cc}        # Duyệt cây đường đi tối thiểu, tính xác suất rẽ nhánh
│   ├── phi.{h,cc}               # Tính áp lực kênh rho, áp lực định tuyến phi, turn flows
│   ├── cdg.{h,cc}               # Channel Dependency Graph & thuật toán kiểm tra chu trình
│   ├── selection.{h,cc}         # Bộ chọn cổng ngẫu nhiên đều (SelectUniform)
│   ├── noc-header.{h,cc}        # Header ns-3 tùy biến (10 B: src, dst, seq, hops)
│   ├── timestamp-tag.{h,cc}     # Tag ns-3 lưu thời điểm phát gói (0 wire overhead)
│   ├── noc-router.{h,cc}        # NocRouter nhận & chuyển tiếp gói tin qua NetDevice
│   ├── noc-sim.{h,cc}           # Điều phối mô phỏng ns-3, phát Bernoulli, đo thống kê
│   ├── selftest.{h,cc}          # Suite kiểm thử tự động toàn diện (>50 test cases)
│   └── noc.cc                   # Điểm vào main() hỗ trợ các cờ chế độ
├── scripts/                     # Bộ công cụ tự động hóa & phân tích số liệu (Python)
│   ├── nocrun.py                # Wrapper thực thi binary ns-3, parse dòng RESULT/SIMCHAN
│   ├── link_validate.py         # Kiểm chứng tương quan U_đo với U_hop vs U_path
│   ├── sweep.py                 # Quét lưới tham số đa luồng (PIR ratio & PIR abs)
│   ├── analyze_sweep.py         # Tính điểm bão hòa, phân tích hiệu cặp CRN, vẽ đồ thị
│   ├── mdl_predict.py           # Mô hình hàng đợi M/D/1 Kleinrock & Kleinrock SC
│   ├── heatmap.py               # Biểu đồ nhiệt tải kênh lý thuyết vs thực tế
│   ├── diagrams.py              # Sinh các sơ đồ kiến trúc, turn model, vòng đời gói
│   ├── make_results_md.py       # Tự động kết xuất report/RESULTS.md từ kết quả
│   ├── reproduce.sh             # Tái lập toàn bộ thí nghiệm từ đầu, so sánh từng byte
│   ├── check_report.py          # Linter kiểm tra tính toàn vẹn số liệu của báo cáo
│   └── build_report.sh          # Xuất báo cáo Pandoc từ Markdown sang Word docx
├── results/                     # Toàn bộ dữ liệu thực nghiệm đã kiểm chứng (CSV, TXT)
│   ├── phi_all.txt, selftest.txt, simcheck.txt
│   ├── link_validate_{summary, channels, z}.csv
│   └── sweep.csv, saturation.csv, paired_diffs.csv, mdl_predict.csv
└── report/                      # Báo cáo học thuật hoàn chỉnh
    ├── bao-cao.docx             # Báo cáo đồ án định dạng Word
    ├── bao-cao.md               # Bản thảo báo cáo khoa học 6 chương (~11 381 từ)
    ├── RESULTS.md               # Bảng kiểm kê số liệu thực nghiệm độc lập
    ├── template.docx            # Mẫu định dạng tài liệu Word (TNR 13, giãn dòng 1.5)
    └── figures/                 # Toàn bộ 15 biểu đồ và hình vẽ minh họa
```

---

## 🛠️ Cài đặt & Biên dịch

### Yêu cầu môi trường
* Hệ điều hành Linux (khuyến nghị Ubuntu 22.04 / 24.04).
* C++23 (GCC $\ge$ 13 hoặc Clang $\ge$ 17).
* **ns-3 phiên bản 3.48** được cài đặt tại `~/ns-3-dev`.
* Python $\ge$ 3.10 với môi trường ảo tại `~/noc-routing-pressure/.venv`.

### 1. Thiết lập symlink vào ns-3
Thư mục `noc/` được liên kết vào thư mục `scratch/noc` của ns-3:
```bash
ln -s ~/noc-routing-pressure/noc ~/ns-3-dev/scratch/noc
```

### 2. Biên dịch ns-3 (Build Profile: Release)
> [!IMPORTANT]
> Dự án bắt buộc sử dụng profile `release` (-O3, không dùng `-march=native`) để đảm bảo kết quả số thực nhất quán giữa các CPU khác nhau:
```bash
cd ~/ns-3-dev
./ns3 configure --build-profile=release --enable-examples
./ns3 build noc
```

### 3. Cài đặt môi trường Python
```bash
cd ~/noc-routing-pressure
python3 -m venv .venv
source .venv/bin/activate
pip install numpy pandas matplotlib
```

---

## 🚀 Hướng dẫn chạy

### Chạy kiểm thử tự động (Selftest)
Kiểm tra tính đúng đắn của topo, tính tối thiểu của đường đi, turn model, phát hiện chu trình CDG và so sánh mốc số liệu với bài báo:
```bash
cd ~/ns-3-dev
./ns3 run --no-build "noc --mode=selftest"
```

### Tính toán Routing Pressure ($\phi$)
Tính bảng $\phi$, kênh cổ chai và ngưỡng $PIR_{max}$:
```bash
# Ví dụ chạy Odd-Even với lưu lượng transpose1:
./ns3 run --no-build "noc --mode=phi --routing=OE --traffic=transpose1"

# Xuất đầy đủ áp lực từng kênh và lưu lượng rẽ nhánh:
./ns3 run --no-build "noc --mode=phi --channels=1 --turns=1"
```

### Chạy mô phỏng đơn lẻ
```bash
# Mô phỏng thuật toán XY, lưu lượng transpose1, PIR = 0.01:
./ns3 run --no-build "noc --mode=sim --routing=XY --traffic=transpose1 --pir=0.01"
```

### Kiểm tra tính nhất quán ở tải thấp (`simcheck`)
```bash
./ns3 run --no-build "noc --mode=simcheck"
```

---

## 🔄 Tái lập toàn bộ kết quả thực nghiệm

Chỉ với một lệnh duy nhất, hệ thống sẽ:
1. Kiểm tra build profile release và biên dịch `noc`.
2. Chạy `selftest`, `simcheck`, xuất bảng `phi`.
3. Chạy kiểm chứng tải kênh `link_validate.py`.
4. Chạy quét lưới tham số `sweep.py` và phân tích `analyze_sweep.py`.
5. Tính mô hình hàng đợi `mdl_predict.py`, vẽ biểu đồ nhiệt `heatmap.py` và sơ đồ `diagrams.py`.
6. Sinh bảng kết quả `RESULTS.md` và so sánh từng byte với dữ liệu đã commit:

```bash
cd ~/noc-routing-pressure
./scripts/reproduce.sh
```

---

## 📄 Báo cáo đồ án

Báo cáo học thuật chi tiết 6 chương (~11 381 từ, 15 hình vẽ, 12 bảng biểu số liệu) đã được hoàn thiện tại:
* 📘 **File Word (.docx):** [report/bao-cao.docx](report/bao-cao.docx)
* 📝 **File Markdown nguồn:** [report/bao-cao.md](report/bao-cao.md)
* 📊 **Bảng kiểm kê số liệu:** [report/RESULTS.md](report/RESULTS.md)

Biên dịch lại báo cáo Word từ Markdown (yêu cầu `pandoc`):
```bash
./scripts/build_report.sh
```

---

## 📚 Tài liệu tham khảo

1. L.-S. Tang, H.-Y. Lin, and M. Palesi, *"Routing Pressure: A Channel-Related and Traffic-Aware Metric of Routing Algorithm,"* **IEEE Transactions on Parallel and Distributed Systems (TPDS)**, vol. 26, no. 3, pp. 884–897, Mar. 2015.
2. C. J. Glass and L. M. Ni, *"The Turn Model for Adaptive Routing,"* **Journal of the ACM**, vol. 41, no. 5, pp. 874–902, 1994.
3. G.-M. Chiu, *"The Odd-Even Turn Model for Adaptive Routing,"* **IEEE Transactions on Parallel and Distributed Systems**, vol. 11, no. 7, pp. 729–738, 2000.
4. L. Kleinrock, *Queueing Systems, Volume 2: Computer Applications*, Wiley-Interscience, 1976.
