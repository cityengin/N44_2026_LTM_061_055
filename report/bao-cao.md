---
lang: vi-VN
---

<!--
Nguồn của báo cáo. Xuất docx: scripts/build_report.sh (kiểm tra bằng scripts/check_report.py trước).
Quy ước: số thập phân dùng dấu phẩy, hàng nghìn tách bằng U+00A0; mọi số thập phân phải khớp
report/RESULTS.md hoặc results/*, hoặc có <!- - suy-ra: … - -> ngay trước đoạn; mỗi hình/bảng/đoạn mã có
<!- - nguồn: file, commit - -> ngay trước.
-->

::: {custom-style="Cover Text"}
[ĐIỀN: TÊN TRƯỜNG]

[ĐIỀN: TÊN KHOA]
:::

::: {custom-style="Cover Title"}
BÁO CÁO ĐỒ ÁN LẬP TRÌNH MẠNG
:::

::: {custom-style="Cover Text"}
ĐỀ TÀI

ROUTING PRESSURE: TÁI HIỆN VÀ KIỂM CHỨNG THƯỚC ĐO ÁP LỰC ĐỊNH TUYẾN CHO NETWORK-ON-CHIP BẰNG NS-3
:::

::: {custom-style="Cover Info"}
Sinh viên thực hiện: [ĐIỀN: tên SV, MSSV]

Lớp học phần: [ĐIỀN: lớp học phần]

Giảng viên hướng dẫn: [ĐIỀN: GVHD]
:::

::: {custom-style="Cover Text"}
[ĐIỀN: địa điểm, năm]
:::

# Tóm tắt {-}

[Sẽ viết sau khi hoàn thành chương 6.]

# Bảng phân công công việc {-}

[ĐIỀN nếu làm theo nhóm]

| STT | Thành viên | Công việc | Mức đóng góp |
|---|---|---|---|
| [ĐIỀN] | [ĐIỀN] | [ĐIỀN] | [ĐIỀN] |

<!-- MUC-LUC -->

# Danh mục hình {-}

<!-- DANH-MUC-HINH -->

# Danh mục bảng {-}

<!-- DANH-MUC-BANG -->

# Danh mục đoạn mã {-}

<!-- DANH-MUC-DOAN-MA -->

# Thuật ngữ và chữ viết tắt {-}

| Thuật ngữ, viết tắt | Tiếng Anh | Giải thích |
|---|---|---|
| NoC | Network-on-Chip | Mạng trên chip: các router nối với nhau bằng link điểm–điểm, thay cho bus dùng chung |
| PE | processing element | Phần tử xử lý (lõi, bộ nhớ…) gắn với một router |
| Lưới 2D | 2D mesh | Topology lưới W×H; mỗi router nối với tối đa bốn láng giềng |
| Kênh | channel | Link có hướng u→v giữa hai router láng giềng |
| Gói, flit | packet, flow control unit | Gói gồm nhiều flit; flit là đơn vị điều khiển luồng |
| PIR | packet injection rate | Tốc độ phát gói: xác suất một nút tạo một gói trong mỗi chu kỳ |
| PIR~max~ | — | PIR lớn nhất mà kênh chịu tải nặng nhất chưa quá tải |
| φ | routing pressure | Áp lực định tuyến: channel pressure lớn nhất trong mạng |
| ρ | channel pressure | Áp lực kênh: tổng phần lưu lượng mà mọi cặp nguồn–đích dồn lên một kênh |
| pir_ratio | — | Tỉ số PIR/PIR~max~ |
| — | degree of adaptiveness | Mức độ thích ứng: số đường ngắn nhất mà thuật toán cho phép |
| — | traffic pattern | Mẫu lưu lượng: quy tắc chọn đích của các nguồn (uniform, transpose1, transpose2) |
| — | deterministic, adaptive routing | Định tuyến tất định (một đường cho mỗi cặp), thích ứng (nhiều đường) |
| — | minimal routing | Định tuyến tối thiểu: chỉ đi theo đường ngắn nhất |
| — | selection function | Hàm lựa chọn: chọn một cổng trong tập cổng mà thuật toán định tuyến trả về |
| — | deadlock | Khóa chết: các gói chờ tài nguyên của nhau theo vòng tròn |
| CDG | channel dependency graph | Đồ thị phụ thuộc kênh |
| — | turn model | Mô hình rẽ: cấm một số hướng rẽ để không thể tạo chu trình |
| XY, YX | dimension-order routing | Định tuyến theo thứ tự chiều: đi hết trục X rồi trục Y (hoặc ngược lại) |
| NF | negative-first | Định tuyến "âm trước": đi hết các hướng âm rồi mới đi hướng dương |
| OE | odd-even turn model | Mô hình rẽ chẵn–lẻ: luật cấm rẽ khác nhau ở cột chẵn và cột lẻ |
| — | search tree | Cây tìm kiếm: liệt kê mọi đường hợp lệ của một cặp nguồn–đích |
| SAF | store-and-forward | Lưu-rồi-chuyển: router nhận trọn gói rồi mới chuyển tiếp |
| — | wormhole switching | Chuyển mạch lỗ sâu: chuyển tiếp từng flit, một gói trải dài qua nhiều router |
| — | backpressure | Áp lực ngược: buffer đầy chặn kênh phía trước, nghẽn lan dần về nguồn |
| DES | discrete-event simulation | Mô phỏng sự kiện rời rạc |
| PPP | Point-to-Point Protocol | Giao thức điểm–điểm; PointToPointNetDevice của ns-3 thêm header PPP 2 byte |
| — | NetDevice | Lớp trừu tượng giao diện mạng của ns-3 |
| — | Header, Tag | Header được tuần tự hóa vào gói (tăng số byte trên dây); Tag là siêu dữ liệu gắn kèm, không truyền trên dây |
| CRN | common random numbers | Dùng chung số ngẫu nhiên giữa các cấu hình để so sánh theo cặp |
| U | utilization | Độ sử dụng kênh: tỉ lệ thời gian kênh bận truyền |
| M/D/1 | — | Hàng đợi có luồng đến Poisson, thời gian phục vụ cố định, một server |
| R² | coefficient of determination | Hệ số xác định của hồi quy tuyến tính |
| — | saturation | Bão hòa: mạng không nhận thêm được tải, độ trễ tăng không giới hạn |

# Giới thiệu

## Bối cảnh

Khi số lõi xử lý tích hợp trên một chip tăng lên hàng chục hoặc hàng trăm, bus dùng chung trở thành nút thắt cả về
băng thông lẫn khả năng mở rộng. Network-on-Chip (NoC – mạng trên chip) giải quyết vấn đề này bằng cách thay bus bằng
một mạng chuyển mạch gói: mỗi phần tử xử lý (processing element, PE) gắn với một router, các router nối với nhau bằng
những link điểm–điểm, và dữ liệu được chia thành các gói (packet) để truyền qua mạng [6]. Topology được dùng nhiều nhất
là lưới hai chiều (2D mesh) vì cấu trúc đều, dây nối ngắn và dễ bố trí trên mặt phẳng chip.

Trong NoC, thuật toán định tuyến (routing algorithm) quyết định tập đường đi mà một gói được phép dùng từ nút nguồn tới
nút đích. Thuật toán định tuyến ảnh hưởng trực tiếp tới độ trễ, thông lượng và mức tải mà mạng chịu được trước khi
nghẽn. Nó cũng phải bảo đảm không xảy ra deadlock (khóa chết), tức tình trạng các gói chờ tài nguyên của nhau theo vòng
tròn và không gói nào tiến được [4]. Số thuật toán định tuyến không có deadlock trên lưới 2D là rất lớn [2], [3], vì vậy
cần những thước đo cho phép so sánh các thuật toán ngay ở mức phân tích, trước khi phải chạy những mô phỏng chi tiết
tốn nhiều thời gian.

## Hạn chế của degree of adaptiveness

Thước đo thường được dùng là degree of adaptiveness (mức độ thích ứng): số đường đi ngắn nhất mà thuật toán cho phép
giữa một cặp nguồn–đích, hoặc tổng của đại lượng này trên mọi cặp [1], [2]. Trực giác đằng sau thước đo là thuật toán
càng cho phép nhiều đường thì càng có nhiều lựa chọn để tránh vùng nghẽn. Trên lưới 7×7, cặp từ góc trên-trái (nút 0)
tới góc dưới-phải (nút 48) có tất cả 924 đường ngắn nhất; thuật toán odd-even (OE) cho phép 84 đường trong số đó, còn
thuật toán tất định XY chỉ cho phép đúng một đường. Các con số này được bộ kiểm thử của đề tài tính lại và khớp với [1].

Tuy nhiên, mức độ thích ứng có ba hạn chế:

- Nó chỉ đếm số đường mà không xét các đường đó chồng lên nhau ở những kênh nào. Nhiều đường khác nhau vẫn có thể cùng
  đi qua một vài kênh chung, nên mức thích ứng cao chưa chắc đã phân tán được tải.
- Nó không phụ thuộc mẫu lưu lượng (traffic pattern), tức quy tắc chọn đích của các nguồn, trong khi hiệu năng của cùng
  một thuật toán có thể rất khác nhau giữa các mẫu lưu lượng.
- Nó không chỉ ra kênh nào sẽ nghẽn đầu tiên, nên không suy ra được mức tải tối đa mà mạng chịu được.

## Routing pressure

Để khắc phục các hạn chế trên, Tang, Lin và Palesi [1] đề xuất routing pressure (áp lực định tuyến). Với một thuật toán
và một mẫu lưu lượng cho trước, mỗi nguồn được giả thiết chia đều lượng gói của mình cho các đường mà thuật toán cho
phép. Phần lưu lượng mà mỗi cặp nguồn–đích dồn lên một kênh được cộng qua mọi cặp, tạo thành channel pressure (áp lực
kênh) của kênh đó. Routing pressure là channel pressure lớn nhất trong mạng, tức áp lực lên kênh chịu tải nặng nhất.
Khi tốc độ phát gói (packet injection rate, PIR) tăng, kênh này quá tải đầu tiên, nên routing pressure cho phép ước
lượng PIR lớn nhất mà mạng chịu được trước khi nghẽn, ký hiệu PIR~max~. Thước đo này vừa gắn với từng kênh vừa phụ
thuộc mẫu lưu lượng, nên khắc phục được cả ba hạn chế nêu ở mục trước.

Bài báo [1] tính routing pressure của các thuật toán XY, negative-first (NF) và OE trên lưới 7×7 với hai mẫu lưu lượng
transpose. Cùng một thuật toán có thể chịu áp lực rất khác nhau giữa hai mẫu: NF có routing pressure bằng 6 với
transpose1 nhưng chỉ 2,41 với transpose2, còn OE có 4,81 với cả hai mẫu. Các tác giả kiểm chứng thước đo bằng mô phỏng
trên Noxim [8], một bộ mô phỏng NoC chính xác theo chu kỳ, với router chuyển mạch wormhole.

Khi tái hiện [1], đề tài gặp ba điểm chưa rõ ràng:

- Công thức định nghĩa channel pressure trong [1] chia đều lưu lượng theo số đường, nhưng ví dụ tính bằng cây tìm kiếm
  trong cùng bài báo lại chia đều theo nhánh tại mỗi nút. Hai cách này cho kết quả khác nhau với thuật toán thích ứng.
- Theo hệ tọa độ của [1] (gốc ở góc trên-trái, trục Y hướng về phía Nam), hướng âm của NF là Tây và Bắc. Tuy nhiên, các
  mốc của NF trong [1] chỉ tái hiện được khi coi Tây và Nam là hướng âm.
- Một báo cáo trước về cùng đề tài [9] cho các giá trị routing pressure và kết quả mô phỏng không khớp với [1].

## Mục tiêu của đề tài

Đề tài đặt ra ba mục tiêu:

1. Tái hiện cách tính channel pressure, routing pressure và PIR~max~ của [1] trên lưới 7×7, với các thuật toán XY, YX,
   NF, OE và ba mẫu lưu lượng uniform, transpose1, transpose2.
2. Kiểm chứng các giá trị dự đoán bằng mô phỏng trên ns-3 [7]: so độ sử dụng đo được trên từng kênh với channel
   pressure, và so ngưỡng bão hòa với PIR~max~.
3. Giải thích các sai khác: giữa hai cách hiểu "chia đều", giữa hai quy ước hướng âm của NF, và giữa [1] với báo cáo
   trước [9].

## Đóng góp của đề tài

- Cài đặt bộ tính routing pressure theo cả hai cách hiểu "chia đều" (theo đường và theo nhánh), dùng chung một hàm định
  tuyến với router mô phỏng. Cách chia theo nhánh, kết hợp với quy ước NF coi Tây và Nam là hướng âm, tái hiện mọi mốc
  của [1]: OE cho 4,8125 so với 4,81 của bài báo, NF với transpose2 cho 2,40625 so với 2,41. Cách chia theo đường cho
  OE chỉ 4,361905.
- Chỉ ra bằng lập luận đối xứng rằng với quy ước NF theo đúng hệ tọa độ của [1] (Tây và Bắc là hướng âm), mốc 2,41 của
  transpose2 không thể đạt được; hai quy ước cho kết quả đổi chỗ giữa transpose1 và transpose2.
- Xây dựng mô hình mô phỏng store-and-forward (lưu-rồi-chuyển) ở mức gói trên ns-3, dùng trực tiếp API NetDevice, kênh
  điểm–điểm và một header tự định nghĩa. Độ sử dụng đo được trên từng kênh khớp với dự đoán theo nhánh với R² = 0,9998,
  trong khi dự đoán theo đường chỉ đạt R² = 0,964.
- Phân tích ngưỡng bão hòa của 15 cấu hình (5 thuật toán, tính cả hai quy ước NF, nhân với 3 mẫu lưu lượng) theo ba
  định nghĩa, đồng thời nêu rõ giới hạn của kết quả: với store-and-forward và hàng đợi không giới hạn, việc bão hòa xảy
  ra tại PIR~max~ gần như là hệ quả của bảo toàn lưu lượng. Hai mô hình hàng đợi giải tích được dùng để giải thích vị
  trí điểm gãy của độ trễ.
- Giải thích sai khác của báo cáo trước [9]: routing pressure trong [9] khớp với cách chia theo đường, còn kết quả mô
  phỏng khớp với NF theo một quy ước hướng âm khác với quy ước dùng khi tính routing pressure.
- Toàn bộ kết quả được sinh lại từ đầu bằng một script và trùng khớp từng byte với dữ liệu đã lưu; mã nguồn có bộ tự
  kiểm thử đi kèm.

## Cấu trúc báo cáo

Chương 2 trình bày cơ sở lý thuyết: NoC, deadlock và turn model, các thuật toán định tuyến, định nghĩa routing pressure
và hai cách hiểu "chia đều". Chương 3 mô tả thiết kế và cài đặt, gồm bộ tính routing pressure, kiểm tra deadlock, mô
hình mô phỏng trên ns-3, chiến lược kiểm thử và quy trình tái lập. Chương 4 trình bày kết quả: routing pressure tĩnh,
kiểm chứng tải trên từng kênh, quét tải, điểm gãy của độ trễ cùng hai mô hình hàng đợi, và phân tích sai khác của báo
cáo trước. Chương 5 thảo luận ý nghĩa và hạn chế của các kết quả. Chương 6 kết luận và nêu hướng phát triển.

# Tài liệu tham khảo {-}

::: {custom-style="Bibliography"}
[1] M. Tang, X. Lin, and M. Palesi, "Routing pressure: A channel-related and traffic-aware metric of routing
algorithm," *IEEE Trans. Parallel Distrib. Syst.*, vol. 26, no. 3, pp. 891–901, Mar. 2015.

[2] G.-M. Chiu, "The odd-even turn model for adaptive routing," *IEEE Trans. Parallel Distrib. Syst.*, vol. 11,
no. 7, pp. 729–738, Jul. 2000.

[3] C. J. Glass and L. M. Ni, "The turn model for adaptive routing," in *Proc. 19th Annu. Int. Symp. Computer
Architecture (ISCA)*, 1992, pp. 278–287.

[4] W. J. Dally and C. L. Seitz, "Deadlock-free message routing in multiprocessor interconnection networks," *IEEE
Trans. Comput.*, vol. C-36, no. 5, pp. 547–553, May 1987.

[5] L. Kleinrock, *Communication Nets: Stochastic Message Flow and Delay*. New York, NY, USA: McGraw-Hill, 1964.

[6] W. J. Dally and B. Towles, *Principles and Practices of Interconnection Networks*. San Francisco, CA, USA: Morgan
Kaufmann, 2004.

[7] ns-3 Network Simulator, phiên bản 3.48. [Online]. Available: https://www.nsnam.org

[8] V. Catania, A. Mineo, S. Monteleone, M. Palesi, and D. Patti, "Noxim: An open, extensible and cycle-accurate
network on chip simulator," in *Proc. IEEE 26th Int. Conf. Application-specific Systems, Architectures and Processors
(ASAP)*, Toronto, ON, Canada, 2015, pp. 162–163, doi: 10.1109/ASAP.2015.7245728.

[9] Nguyễn Hữu Duy, Hà Lê Minh Hiếu, and Nguyễn Việt Hoàng, "Routing pressure – Chỉ số đánh giá thuật toán định tuyến
dựa trên kênh và lưu lượng dùng NS-3," Báo cáo Lập trình mạng, Khoa Điện tử – Viễn thông, Trường Đại học Bách khoa –
Đại học Đà Nẵng, Đà Nẵng, 2025.

[10] L. Kleinrock, *Queueing Systems, Vol. I: Theory*. New York, NY, USA: Wiley, 1975.
:::
