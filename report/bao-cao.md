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

# Cơ sở lý thuyết

## Network-on-Chip

### Kiến trúc NoC và lưới hai chiều

Một NoC gồm các router và các link nối giữa chúng. Mỗi router có năm cổng: bốn cổng nối với láng giềng theo các hướng
Đông (E), Tây (W), Bắc (N), Nam (S) và một cổng cục bộ nối với PE. Gói đi vào một router qua một cổng vào, được thuật
toán định tuyến chỉ định cổng ra và rời router qua link tương ứng [6].

Đề tài dùng lưới hai chiều W×H với quy ước tọa độ giống [1]. Gốc tọa độ ở góc trên-trái; trục x tăng về phía Đông,
trục y tăng về phía Nam. Nút có tọa độ $(x, y)$ được đánh số $\mathrm{id} = y \cdot W + x$. Mỗi link vật lý giữa hai
nút láng giềng gồm hai kênh (channel) có hướng; kênh từ nút u sang nút v được ký hiệu "u-v". Hình 2.1 minh họa quy ước
này trên lưới 4×4. Số kênh của lưới W×H là

$$|C| = 2\,\bigl[\,W(H-1) + H(W-1)\,\bigr],$$

nên lưới 7×7 dùng trong đề tài có 168 kênh. Đề tài chỉ xét định tuyến tối thiểu (minimal routing): mọi gói đi theo
một đường ngắn nhất, nên số hop từ nguồn tới đích bằng khoảng cách Manhattan $|\Delta x| + |\Delta y|$.

<!-- nguồn: report/figures/diag_mesh.png, scripts/diagrams.py, 5a6f060 -->

![Hình 2.1: Quy ước tọa độ, số hiệu nút và tên kênh trên lưới 4×4 (gốc trên-trái, y tăng về phía Nam)](figures/diag_mesh.png){width=75%}

### Gói, flit và tham số thời gian

Trong NoC, một gói được chia thành các flit (flow control unit), đơn vị nhỏ nhất mà router điều khiển luồng. Đề tài
giữ nguyên các tham số của [1]: mỗi gói dài $\mathrm{len} = 8$ flit và một kênh truyền $\mathrm{frate} = 0{,}5$ flit
mỗi chu kỳ. Như vậy mỗi gói chiếm một kênh trong $\mathrm{len}/\mathrm{frate} = 16$ chu kỳ, và một kênh truyền được
tối đa một gói mỗi 16 chu kỳ.

Tải đưa vào mạng được đo bằng tốc độ phát gói PIR: xác suất mỗi nút tạo một gói mới trong một chu kỳ. Đích của gói do
mẫu lưu lượng quyết định. Đề tài dùng ba mẫu của [1] trên lưới vuông N×N:

- uniform: đích được chọn đều trong $N^2 - 1$ nút còn lại;
- transpose1: nút $(x, y)$ gửi tới $(N-1-y,\; N-1-x)$;
- transpose2: nút $(x, y)$ gửi tới $(y, x)$.

Với hai mẫu transpose, các nút nằm trên đường chéo được ánh xạ vào chính nó và không phát gói. Trên lưới 7×7 mỗi mẫu
transpose có 42 luồng nguồn–đích, mỗi nguồn chỉ có một đích cố định.

### Store-and-forward và wormhole

Có hai kỹ thuật chuyển mạch thường gặp trong NoC [6]:

- **Store-and-forward (SAF, lưu-rồi-chuyển):** router nhận trọn một gói vào buffer rồi mới chuyển gói sang kênh kế
  tiếp. Khi mạng không tải, độ trễ của một gói bằng số hop nhân với thời gian truyền một gói qua một kênh, tức
  $16 \cdot \mathrm{hops}$ chu kỳ với các tham số ở trên. Mỗi router cần buffer đủ chứa nguyên gói.
- **Wormhole (chuyển mạch lỗ sâu):** flit đầu giữ chỗ trên từng kênh, các flit sau đi theo kiểu đường ống, nên độ trễ
  khi không tải xấp xỉ số hop cộng thời gian truyền một gói. Buffer mỗi cổng chỉ chứa vài flit. Khi flit đầu bị chặn,
  gói nằm trải dài trên nhiều router và vẫn giữ các kênh đã chiếm. Buffer đầy chặn kênh phía trước nó, nên nghẽn lan
  dần ngược về phía nguồn; hiện tượng này gọi là backpressure (áp lực ngược).

Bài báo [1] mô phỏng bằng Noxim [8] với router wormhole, buffer bốn flit mỗi cổng, không có kênh ảo, và chiến lược lựa
chọn cổng ngẫu nhiên. Đề tài dùng mô hình SAF ở mức gói với hàng đợi không giới hạn. Mô hình này không có backpressure,
nên độ trễ tuyệt đối không so trực tiếp được với [1]. Đại lượng được so sánh là ngưỡng tải mà tại đó mạng bắt đầu nghẽn.
Hệ quả của lựa chọn này được thảo luận ở chương 5.

## Deadlock và turn model

### Deadlock và đồ thị phụ thuộc kênh

Deadlock xảy ra khi một tập gói chờ nhau theo vòng tròn: mỗi gói giữ một kênh và chờ một kênh đang bị gói kế tiếp
trong vòng giữ. Dally và Seitz [4] mô hình hóa quan hệ này bằng đồ thị phụ thuộc kênh (channel dependency graph, CDG)
$D = (C, E)$. Mỗi đỉnh là một kênh. Có cạnh $(c_i, c_j) \in E$ khi thuật toán định tuyến cho phép một gói đang dùng
kênh $c_i$ yêu cầu ngay kênh $c_j$. Nếu CDG không có chu trình thì không thể hình thành vòng chờ, nên thuật toán không
có deadlock [4].

Với thuật toán thích ứng, tập phụ thuộc phải gồm mọi cặp kênh liên tiếp trên mọi đường mà thuật toán cho phép. Đề
tài xây CDG đúng theo cách này: liệt kê mọi đường hợp lệ của mọi cặp nguồn–đích rồi thêm cạnh giữa hai kênh liên tiếp
trên mỗi đường. Nhờ đó CDG chỉ chứa các phụ thuộc thực sự xảy ra, kể cả với OE là thuật toán phụ thuộc vào nút nguồn.

### Turn model

Glass và Ni [3] nhận xét rằng trên lưới 2D có tám hướng rẽ 90°. Tám hướng rẽ này tạo thành hai chu trình trừu tượng,
một theo chiều kim đồng hồ (E→S, S→W, W→N, N→E) và một ngược chiều kim đồng hồ (E→N, N→W, W→S, S→E). Mọi vòng chờ trên
lưới đều phải dùng đủ bốn hướng rẽ của một trong hai chu trình đó. Vì vậy, để tránh deadlock, cần cấm ít nhất một
hướng rẽ trong mỗi chu trình; tuy nhiên không phải mọi cách chọn hai hướng rẽ đều đủ để CDG không có chu trình [3].
Cách tiếp cận cấm rẽ này gọi là turn model (mô hình rẽ). Hình 2.2 cho thấy các hướng rẽ bị cấm của từng thuật toán
dùng trong đề tài.

<!-- nguồn: report/figures/diag_turns.png, scripts/diagrams.py, noc/routing.cc, 5a6f060 -->

![Hình 2.2: Tám hướng rẽ trên lưới 2D và các hướng rẽ bị cấm của XY, YX, NF theo hai quy ước, OE ở cột chẵn và cột lẻ](figures/diag_turns.png){width=95%}

### Các thuật toán XY, YX, NF và OE

**XY và YX.** XY (định tuyến theo thứ tự chiều) đi hết độ lệch theo trục x rồi mới đi theo trục y, nên cấm mọi hướng
rẽ từ trục y sang trục x. YX làm ngược lại. Cả hai là thuật toán tất định: mỗi cặp nguồn–đích có đúng một đường.

**Negative-first (NF).** NF [3] đi hết các hướng âm trước rồi mới đi các hướng dương. Tại mỗi nút, nếu gói còn phải đi
theo một hướng âm thì tập cổng hợp lệ là các hướng âm còn lại; ngược lại, tập cổng gồm mọi hướng dương còn lại. Theo
hệ tọa độ ở mục 2.1.1, hướng âm của trục x là Tây và hướng âm của trục y là Bắc (y giảm), nên NF cấm hai hướng rẽ từ
dương sang âm là E→N và S→W. Đề tài gọi đây là NF theo quy ước chính. Với các cặp mà đích nằm về phía Tây-Bắc hoặc
Đông-Nam của nguồn, NF thích ứng hoàn toàn; với các cặp Đông-Bắc hoặc Tây-Nam, NF chỉ có một đường.

Để giải thích các mốc của [1] (mục 4.1), đề tài cài thêm một biến thể coi Tây và Nam là hướng âm, cấm E→S và N→W. Biến
thể này được ghi nhãn "NF (bài báo)" trong mọi bảng và hình so sánh với [1]. Hai quy ước là ảnh gương của nhau qua phép
lật trục y ($y \mapsto N-1-y$).

**Odd-even (OE).** Mô hình rẽ chẵn–lẻ của Chiu [2] không cấm cố định hai hướng rẽ trên toàn lưới mà phân bố lệnh cấm
theo cột, đánh số từ 0. Ở cột chẵn, gói không được rẽ E→N và E→S; ở cột lẻ, gói không được rẽ N→W và S→W. Hàm định
tuyến của OE phụ thuộc cả nút hiện tại, nút đích lẫn cột của nút nguồn. Cách phân bố này giúp mức thích ứng không bị
dồn về một vùng của lưới như NF.

**Fully adaptive (FULL).** Thuật toán cho phép mọi hướng tối thiểu. Đề tài chỉ dùng nó làm đối chứng: số đường của nó
là cận trên, và CDG của nó phải có chu trình.

Bảng 2.1 tóm tắt các thuật toán. Bộ tự kiểm thử của đề tài xác nhận CDG của XY, YX, cả hai quy ước NF và OE đều không
có chu trình, còn CDG của FULL có chu trình.

<!-- nguồn: results/selftest.txt, noc/routing.cc, c7f4ea5 -->

Table: Bảng 2.1: Các thuật toán định tuyến trên lưới 7×7: hướng rẽ bị cấm, tổng số đường trên 2 352 cặp nguồn–đích và CDG

| Thuật toán | Hướng rẽ bị cấm | Loại | Tổng số đường | Số cạnh CDG | CDG có chu trình |
|---|---|---|---|---|---|
| XY | N→E, N→W, S→E, S→W | tất định | 2 352 | 284 | không |
| YX | E→N, E→S, W→N, W→S | tất định | 2 352 | 284 | không |
| NF (quy ước chính) | E→N, S→W | thích ứng một phần | 26 394 | 356 | không |
| NF (bài báo) | E→S, N→W | thích ứng một phần | 26 394 | 356 | không |
| OE | cột chẵn: E→N, E→S; cột lẻ: N→W, S→W | thích ứng một phần | 12 432 | 356 | không |
| FULL (đối chứng) | không cấm | thích ứng hoàn toàn | 50 436 | 428 | có |

## Degree of adaptiveness

Theo định nghĩa được dùng trong [1], mức độ thích ứng của một cặp nguồn–đích là số đường ngắn nhất mà thuật toán cho
phép, và mức độ thích ứng của thuật toán là tổng đại lượng đó trên mọi cặp. Với FULL, cặp có độ lệch
$(\Delta x, \Delta y)$ có

$$p_{\mathrm{FULL}} = \binom{|\Delta x| + |\Delta y|}{|\Delta x|}$$

đường; cặp 0→48 trên lưới 7×7 có $\binom{12}{6} = 924$ đường. Cột "Tổng số đường" của Bảng 2.1 chính là mức độ thích
ứng của từng thuật toán trên lưới 7×7.

Theo thước đo này, NF thích ứng hơn OE (26 394 so với 12 432 đường). Tuy nhiên, với mẫu transpose1, routing pressure
của NF (bài báo) là 6, lớn hơn của OE là 4,8125 (mục 4.1), tức NF làm kênh cổ chai chịu tải nặng hơn. Mức độ thích ứng
cộng dồn mọi cặp nguồn–đích, kể cả những cặp không có lưu lượng, và không cho biết các đường dồn vào kênh nào. Đây là
lý do [1] đề xuất một thước đo gắn với kênh và với mẫu lưu lượng.

## Routing pressure

### Channel pressure và routing pressure

Xét một mẫu lưu lượng gồm $M$ luồng; luồng thứ $i$ đi từ nguồn $s_i$ tới đích $d_i$ với tốc độ $\mathrm{PIR}_i$. Gọi
$\sigma_i(c)$ là tỉ lệ gói của luồng $i$ đi qua kênh $c$. Channel pressure của kênh $c$ là tổng lưu lượng mà mọi luồng
dồn lên kênh đó:

$$\rho(c) = \sum_{i=1}^{M} \sigma_i(c)\,\mathrm{PIR}_i .$$

Khi mọi nguồn có cùng tốc độ PIR, $\rho(c) = \mathrm{PIR} \cdot \hat\rho(c)$ với

$$\hat\rho(c) = w \sum_{i=1}^{M} \sigma_i(c), \qquad \varphi = \max_{c \in C} \hat\rho(c),$$

trong đó $w = 1$ với hai mẫu transpose (mỗi nguồn chỉ có một đích) và $w = 1/(N^2-1)$ với uniform (mỗi nguồn chia đều
lưu lượng cho $N^2-1$ đích). Đại lượng $\varphi$ là routing pressure: channel pressure lớn nhất, tính trên một đơn vị
PIR [1]. Với uniform, đề tài in cả giá trị chưa chuẩn hóa ($w = 1$) để so với các mốc kiểm thử và giá trị đã chuẩn hóa
để tính ngưỡng tải. Ví dụ, XY với uniform trên lưới 7×7 có routing pressure chưa chuẩn hóa 84 và đã chuẩn hóa 1,75.

### Ngưỡng tải PIR~max~

Một kênh truyền tối đa $\mathrm{frate}/\mathrm{len}$ gói mỗi chu kỳ. Độ sử dụng (utilization) của kênh $c$, tức tỉ lệ
thời gian kênh bận, là

$$U(c) = \frac{\mathrm{len}}{\mathrm{frate}}\,\hat\rho(c)\,\mathrm{PIR} = 16\,\hat\rho(c)\,\mathrm{PIR}.$$

Mạng ổn định khi mọi kênh có $U(c) < 1$. Điều kiện chặt nhất đến từ kênh có $\hat\rho$ lớn nhất, nên [1] suy ra

$$\mathrm{PIR} < \mathrm{PIR}_{\max} = \frac{\mathrm{frate}}{\mathrm{len} \cdot \varphi} = \frac{0{,}0625}{\varphi}.$$

Ví dụ, XY với transpose1 có $\varphi = 6$ nên PIR~max~ = 0,010417; OE với transpose1 có $\varphi$ = 4,8125 nên PIR~max~ =
0,012987. Đề tài dùng tỉ số pir_ratio = PIR/PIR~max~ để đặt các cấu hình có routing pressure khác nhau lên cùng một
thang: tại pir_ratio = 1, kênh cổ chai được dự đoán bận 100% thời gian.

### Hai cách hiểu "chia đều"

Định nghĩa ở trên chưa nói $\sigma_i(c)$ được tính thế nào. Bài báo [1] giả thiết lưu lượng của mỗi nguồn được "chia
đều", nhưng câu này có hai cách hiểu, cho kết quả khác nhau với thuật toán thích ứng.

**Chia theo đường (path).** Lưu lượng chia đều cho $p_i$ đường hợp lệ. Nếu $k_i(c)$ trong số đó đi qua kênh $c$ thì

$$\sigma_i^{\mathrm{path}}(c) = \frac{k_i(c)}{p_i}.$$

Đây là công thức được viết trong phần định nghĩa của [1]. Nó tương ứng với việc nguồn chọn trước cả đường đi, mỗi đường
với cùng xác suất.

**Chia theo nhánh (hop).** Tại mỗi nút trên đường đi, gói chọn đều một cổng trong tập cổng $R(v)$ mà thuật toán cho
phép. Một đường $\pi$ được chọn với xác suất bằng tích các xác suất rẽ nhánh dọc đường, và

$$\sigma_i^{\mathrm{hop}}(c) = \sum_{\pi \in P_i,\ c \in \pi} \;\prod_{v \in \pi,\ v \ne d_i} \frac{1}{|R(v)|}.$$

Đây là cách tính trên cây tìm kiếm trong ví dụ của [1]: tổng phần chia ở mỗi tầng của cây bằng 1. Hai cách cho kết quả
giống nhau với thuật toán tất định ($p_i = 1$) và khác nhau khi cây tìm kiếm không cân bằng.

Xét cặp 0→8 trên lưới 3×3 với một thuật toán thích ứng hoàn toàn. Cặp này có $\binom{4}{2} = 6$ đường. Kênh 1-2 chỉ
nằm trên một đường (0→1→2→5→8), nên theo cách chia theo đường
$\sigma^{\mathrm{path}}(\text{1-2}) = 1/6$. Theo cách chia theo nhánh, gói phải chọn Đông ở nút 0 rồi chọn Đông ở nút
1, nên $\sigma^{\mathrm{hop}}(\text{1-2}) = \tfrac12 \cdot \tfrac12 = \tfrac14$. Ngược lại, kênh 1-4 nằm trên hai đường,
nên $\sigma^{\mathrm{path}}(\text{1-4}) = 2/6 = 1/3$, trong khi $\sigma^{\mathrm{hop}}(\text{1-4}) = \tfrac14$. Cách chia
theo đường dồn tải vào giữa lưới, nơi có nhiều đường đi qua; cách chia theo nhánh dồn tải ra mép nhiều hơn.

<!-- nguồn: results/example_3x3.txt, 5a6f060 -->

Hình 2.3 áp dụng cả hai cách cho toàn bộ mẫu transpose1 trên lưới 3×3 với NF theo quy ước chính. Trên lưới này cả sáu
cặp đều thích ứng hoàn toàn. Theo cách chia theo đường, routing pressure là 0,833333 và đạt tại 8 kênh quanh nút trung
tâm; theo cách chia theo nhánh, routing pressure là 0,75 và đạt tại 16 kênh. Không chỉ giá trị mà cả vị trí kênh cổ
chai cũng thay đổi theo cách hiểu.

<!-- nguồn: report/figures/diag_path_hop.png, results/example_3x3.txt, 5a6f060 -->

![Hình 2.3: Channel pressure của từng kênh trên lưới 3×3, NF (quy ước chính), transpose1: (a) chia theo đường, (b) chia theo nhánh](figures/diag_path_hop.png){width=100%}

### Cách chia theo nhánh và router chọn cổng ngẫu nhiên

Router trong Noxim [8] (theo cấu hình của [1]) và router trong mô phỏng của đề tài đều chọn đều một cổng trong tập cổng
hợp lệ tại mỗi hop, độc lập với các hop trước. Khi đó xác suất một gói của luồng $i$ đi qua kênh $c$ đúng bằng
$\sigma_i^{\mathrm{hop}}(c)$. Theo tính tuyến tính của kỳ vọng, số gói đi qua kênh $c$ trên một đơn vị thời gian có kỳ
vọng $\mathrm{PIR} \cdot \hat\rho^{\mathrm{hop}}(c)$, bất kể hàng đợi ở các router diễn biến thế nào, miễn là mạng còn
ổn định. Vì vậy với router chọn ngẫu nhiên, cách chia theo nhánh là giá trị dự đoán đúng cho tải của từng kênh, còn cách
chia theo đường chỉ đúng khi nguồn tự chọn trước cả đường đi (định tuyến tại nguồn). Chương 4 kiểm chứng nhận định này
bằng mô phỏng, và chương 5 bàn về hệ quả của nó đối với ý nghĩa của ngưỡng PIR~max~.

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
