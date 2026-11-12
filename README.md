<p align="center">
  <img src="banner.svg" alt="NoC Routing Pressure — ns-3.48" width="100%">
</p>

# NoC Routing Pressure (ns-3.48)

> **Đồ án môn Lập trình mạng**  
> Tái hiện, kiểm chứng và phân tích nghiên cứu:  
> **Tang, Lin, Palesi (IEEE TPDS 2015)**: *"Routing Pressure: A Channel-Related and Traffic-Aware Metric of Routing Algorithm"*.

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
   * *Thực nghiệm:* Độ sử dụng đo được trên từng kênh trong ns-3 khớp với dự đoán theo `hop` ($R^2 = 0,9998$, hệ số góc $0,9993$, RMSE $0,0033$), trong khi dự đoán theo `path` chỉ đạt $R^2 = 0,9642$. Báo cáo trước tính theo `path` nên lệch với mô phỏng và bài báo.

2. **Giải mã nghịch lý quy ước Negative-First (NF vs `NF_WS`):**
   * Theo hệ tọa độ của bài báo (gốc trên-trái, y tăng về phía Nam), hướng âm của NF là Tây (W) và Bắc (N). Dưới quy ước này, với `transpose2` NF là tất định nên mọi áp lực kênh là số nguyên, không thể đạt giá trị $2,41$ như bài báo.
   * Mọi mốc của bài báo chỉ tái hiện được khi coi Tây (W) và Nam (S) là hướng âm, nên nhiều khả năng bài báo đã dùng quy ước này. Dự án bổ sung biến thể chẩn đoán **`NF_WS`** (ghi nhãn *"NF (bài báo)"*); kết hợp với cách chia `hop`, nó khớp mọi mốc của bài báo trong sai số làm tròn ±0,005 (OE 4,8125 so với 4,81; NF/transpose2 2,40625 so với 2,41).

3. **Nguyên tắc "Một hàm định tuyến duy nhất" (`routeSet`):**
   * [noc/routing.cc](noc/routing.cc) cung cấp hàm duy nhất `routeSet()`. Cả bộ tính áp lực $\phi$, bộ kiểm tra chu trình đồ thị phụ thuộc kênh CDG (phát hiện deadlock) và Router mô phỏng trong ns-3 đều gọi chung hàm này, triệt tiêu nguy cơ sai lệch giữa lý thuyết và mô phỏng.

4. **Mô hình hàng đợi giải tích cải tiến `kleinrock_sc`:**
   * Mở rộng mô hình Kleinrock M/D/1 bằng cách loại trừ hiện tượng tự xung đột (*self-contention*) dựa trên thông lượng rẽ nhánh (turn flows), dự đoán chính xác thứ tự điểm gãy độ trễ (latency knee) của toàn bộ 15 cấu hình mô phỏng.

5. **Quy chuẩn tái lập 100% từng byte (Byte-level Reproducibility):**
   * Sử dụng build profile `release` (-O3, không `-march=native`), gán seed/stream RNG logic cố định, áp dụng kỹ thuật Common Random Numbers (CRN) so sánh hiệu theo cặp. Toàn bộ dữ liệu trong `results/` được tái lập từ đầu và khớp từng byte (quy trình mô tả ở mục 3.5 của báo cáo).

---

## 📂 Cấu trúc thư mục

```
noc-routing-pressure/
├── README.md                    # Tài liệu giới thiệu dự án
├── banner.svg                   # Banner đồ họa dự án
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
├── results/                     # Toàn bộ dữ liệu thực nghiệm đã kiểm chứng (CSV, TXT)
│   ├── phi_all.txt, selftest.txt, simcheck.txt
│   ├── link_validate_{summary, channels, z}.csv
│   └── sweep.csv, saturation.csv, paired_diffs.csv, mdl_predict.csv
└── report/
    └── bao-cao.docx             # Báo cáo đồ án định dạng Word
```

---

## 🛠️ Cài đặt & Biên dịch

### Yêu cầu môi trường
* Hệ điều hành Linux (khuyến nghị Ubuntu 22.04 / 24.04).
* C++23 (GCC ≥ 13 hoặc Clang ≥ 17).
* **ns-3 phiên bản 3.48** được cài đặt tại `~/ns-3-dev`.

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

## 📄 Báo cáo đồ án

Báo cáo đồ án (6 chương, 15 hình, 17 bảng, 3 giải thuật) ở dạng Word: [report/bao-cao.docx](report/bao-cao.docx).

---

## 📚 Tài liệu tham khảo

1. M. Tang, X. Lin, and M. Palesi, *"Routing Pressure: A Channel-Related and Traffic-Aware Metric of Routing Algorithm,"* **IEEE Transactions on Parallel and Distributed Systems**, vol. 26, no. 3, pp. 891–901, Mar. 2015.
2. C. J. Glass and L. M. Ni, *"The Turn Model for Adaptive Routing,"* in *Proc. 19th Annu. Int. Symp. Computer Architecture (ISCA)*, 1992, pp. 278–287.
3. G.-M. Chiu, *"The Odd-Even Turn Model for Adaptive Routing,"* **IEEE Transactions on Parallel and Distributed Systems**, vol. 11, no. 7, pp. 729–738, 2000.
4. L. Kleinrock, *Queueing Systems, Vol. I: Theory*, Wiley, 1975.
