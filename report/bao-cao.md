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

# Danh mục hình, bảng và đoạn mã {-}

## Danh mục hình {-}

<!-- DANH-MUC-HINH -->

## Danh mục bảng {-}

<!-- DANH-MUC-BANG -->

## Danh mục đoạn mã {-}

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

# Thiết kế và cài đặt

## Kiến trúc tổng thể

### Nguyên tắc "một hàm định tuyến"

Phân tích các báo cáo trước cho thấy một nguồn lỗi dễ mắc khi tái hiện [1]: bộ tính routing pressure và router mô
phỏng dùng hai bản cài đặt khác nhau của cùng một thuật toán, nên số liệu tĩnh và số liệu mô phỏng không còn nói về
cùng một thứ (mục 4.5). Để loại bỏ khả năng này, toàn bộ chương trình chỉ có một hàm định tuyến `routeSet`. Hàm nhận
thuật toán, tọa độ nút hiện tại, nút nguồn và nút đích, rồi trả về tập cổng hợp lệ dưới dạng bitmask theo thứ tự E, W,
N, S. Việc chọn một cổng trong tập đó (selection) được tách thành một hàm riêng. Hình 3.1 cho thấy hai nhánh của chương
trình cùng gọi `routeSet`: nhánh phân tích tĩnh (cây tìm kiếm, routing pressure, CDG) và nhánh mô phỏng ns-3 (router).

<!-- nguồn: report/figures/diag_arch.png, scripts/diagrams.py, 5a6f060 -->

![Hình 3.1: Kiến trúc chương trình: phân tích tĩnh và mô phỏng ns-3 cùng gọi một hàm định tuyến routeSet](figures/diag_arch.png){width=90%}

Đoạn mã 3.1 trích phần chính của `routeSet`. Mọi thuật toán trừ OE chỉ cần dấu của $\Delta x$ và $\Delta y$; OE được
tách thành hàm riêng vì cần thêm cột của nút hiện tại, nút nguồn và nút đích. Tập cổng rỗng nghĩa là gói đã tới đích.

<!-- nguồn: noc/routing.cc, 950decd -->

::: {custom-style="Table Caption"}
Đoạn mã 3.1: Hàm định tuyến duy nhất routeSet (noc/routing.cc, rút gọn)
:::

```cpp
DirSet
routeSet(Algo algo, Coord cur, Coord src, Coord dst)
{
    const int dx = dst.x - cur.x;
    const int dy = dst.y - cur.y; // dy < 0: cần đi N (y giảm)
    if (dx == 0 && dy == 0)
    {
        return 0;
    }
    const DirSet ex = dx > 0 ? Bit(Dir::E) : (dx < 0 ? Bit(Dir::W) : 0);
    const DirSet ey = dy > 0 ? Bit(Dir::S) : (dy < 0 ? Bit(Dir::N) : 0);

    switch (algo)
    {
    case Algo::XY:
        return ex != 0 ? ex : ey;
    ...
    case Algo::NF: {
        // Đi hết các hướng âm (W, N) trước, sau đó mới đi hướng dương (E, S).
        const DirSet neg = (dx < 0 ? Bit(Dir::W) : 0) | (dy < 0 ? Bit(Dir::N) : 0);
        return neg != 0 ? neg : (ex | ey);
    }
    ...
    case Algo::OE:
        return OddEven(cur, src, dst, ey);
    ...
```

### Tổ chức mã nguồn

Mã C++ nằm trong thư mục `noc/` và được biên dịch thành một chương trình ns-3 duy nhất. Phần lõi (lưới, định tuyến,
traffic, cây tìm kiếm, routing pressure, CDG) là C++ thuần, không phụ thuộc ns-3; chỉ các module mô phỏng dùng API của
ns-3. Các script Python trong `scripts/` chạy chương trình song song, tổng hợp kết quả thành CSV, vẽ hình và kiểm tra
tính tái lập. Bảng 3.1 liệt kê các module.

<!-- nguồn: noc/noc.cc, scripts/reproduce.sh, 5a6f060 -->

Table: Bảng 3.1: Các module của chương trình

| Module | Trách nhiệm |
|---|---|
| `topology` | Lưới W×H, chuyển đổi id ↔ tọa độ, láng giềng, danh sách kênh có hướng |
| `routing` | `routeSet`: tập cổng hợp lệ của XY, YX, NF, NF (bài báo), OE, FULL |
| `traffic` | Sinh luồng uniform, transpose1, transpose2; hệ số chuẩn hóa $w$ |
| `searchtree` | `ForEachPath`: duyệt mọi đường hợp lệ của một cặp, kèm xác suất theo nhánh |
| `phi` | Channel pressure theo path và hop, φ, PIR~max~, lưu lượng theo hướng rẽ |
| `cdg` | Dựng CDG từ mọi đường hợp lệ, tìm chu trình |
| `noc-header`, `timestamp-tag` | Header 10 byte được tuần tự hóa vào gói; Tag ghi thời điểm tạo gói |
| `selection` | Chọn đều một cổng trong tập cổng |
| `noc-router` | Router: nhận gói qua callback, gọi `routeSet` và selection, gửi qua NetDevice |
| `noc-sim` | Dựng mạng ns-3, sinh traffic, gắn trace, thống kê; kiểm tra `simcheck` |
| `selftest` | Bộ tự kiểm thử |
| `noc.cc` | Chương trình chính: đọc tham số dòng lệnh, chọn chế độ phi, selftest, sim, simcheck |
| `scripts/*.py`, `reproduce.sh` | Chạy song song, tổng hợp CSV, vẽ hình, mô hình hàng đợi, tái lập |

Mỗi chế độ in kết quả thành các dòng máy đọc được dạng `RESULT,key=value,...`, nhờ đó script Python không phải phân
tích văn bản tự do và mọi con số trong báo cáo đều truy được về một dòng output cụ thể.

## Bộ tính routing pressure và kiểm tra deadlock

### Cây tìm kiếm

Hàm `ForEachPath` cài đặt trực tiếp cây tìm kiếm của [1] bằng duyệt theo chiều sâu (DFS). Xuất phát từ nút nguồn, tại
mỗi nút hàm gọi `routeSet` để lấy tập cổng, rồi lần lượt đi theo từng cổng cho tới khi gặp nút đích. Mỗi lá của cây là
một đường hợp lệ. Cùng với dãy nút của đường, hàm trả về xác suất của đường theo cách chia theo nhánh, tức tích các
$1/|R(v)|$ dọc đường. Nếu tại một nút chưa phải đích mà tập cổng rỗng, chương trình dừng với lỗi, vì điều đó nghĩa là
thuật toán định tuyến bị cài sai. Với lưới 7×7, cặp xa nhất có 924 đường với FULL, nên việc duyệt hết mọi đường của
mọi cặp vẫn chạy nhanh.

### Channel pressure, routing pressure và PIR~max~

Từ một lần duyệt, bộ tính cộng đồng thời hai đại lượng cho mỗi kênh: $k_i(c)/p_i$ (cách chia theo đường) và tổng xác
suất của các đường đi qua kênh (cách chia theo nhánh). Cộng qua mọi luồng của mẫu lưu lượng và nhân hệ số $w$ cho ra
$\hat\rho(c)$ theo hai cách; giá trị lớn nhất là $\varphi$, và PIR~max~ = 0,0625/φ. Chế độ `--mode=phi` in một dòng
`RESULT` cho mỗi tổ hợp thuật toán, mẫu lưu lượng và cách chia. Tùy chọn `--channels` in thêm channel pressure của
từng kênh (dòng `CHAN`), còn `--turns` in lưu lượng theo nhánh từ mỗi cổng vào sang mỗi kênh ra (dòng `PHITURN`), dùng
cho mô hình hàng đợi ở mục 4.4.

### Kiểm tra CDG

Hàm `CheckCdg` dùng lại `ForEachPath` để liệt kê mọi đường của mọi cặp nguồn–đích và thêm cạnh giữa hai kênh liên tiếp
trên mỗi đường. Chu trình được tìm bằng DFS ba màu (trắng: chưa thăm; xám: đang trên ngăn xếp; đen: đã xong); gặp một
đỉnh xám nghĩa là có chu trình, và chương trình in chu trình đó làm bằng chứng. FULL được dùng làm đối chứng âm: nếu
bộ dò chu trình không tìm ra chu trình của FULL thì chính bộ dò bị sai.

## Mô hình mô phỏng trên ns-3

### Mô phỏng sự kiện rời rạc

ns-3 [7] là bộ mô phỏng mạng theo sự kiện rời rạc (discrete-event simulation): trạng thái hệ thống chỉ thay đổi tại các
thời điểm có sự kiện, và bộ lập lịch `Simulator` lấy lần lượt sự kiện có thời điểm nhỏ nhất ra thực thi. Đề tài quy
ước một chu kỳ của NoC bằng 1 ns thời gian mô phỏng. Các sự kiện chính gồm: một nút tạo gói, một thiết bị bắt đầu và
kết thúc truyền một khung, và một gói tới thiết bị ở nút kế tiếp. Hình 3.2 mô tả vòng đời của một gói qua các lời gọi
hàm và sự kiện đó, cùng với trục thời gian của một lần chạy.

<!-- nguồn: report/figures/diag_ns3.png, scripts/diagrams.py, 5a6f060 -->

![Hình 3.2: Mô hình ns-3: (a) vòng đời của một gói, (b) trục thời gian warmup, measure, drain](figures/diag_ns3.png){width=100%}

### Nút, NetDevice và kênh điểm–điểm

Mỗi router là một `Node` của ns-3. Mỗi cặp nút láng giềng được nối bằng một link `PointToPoint` hai chiều, tương ứng
hai kênh có hướng của lưới. Link có tốc độ 16 Gbps, độ trễ lan truyền 0 và khoảng cách giữa hai khung bằng 0. Mỗi
thiết bị có hàng đợi `DropTailQueue` rất lớn (100 000 gói), nên trong thực tế hàng đợi không bao giờ tràn và mô hình
tương đương hàng đợi không giới hạn.

Đề tài không cài Internet stack (IP, UDP) vì NoC không cần định địa chỉ IP; router gửi gói trực tiếp xuống tầng
NetDevice bằng `NetDevice::Send`. Vì `PointToPointNetDevice` chỉ chấp nhận các giao thức mà nó ánh xạ được sang trường
protocol của PPP, gói được gửi với mã giao thức `0x0800` (IPv4) dù nội dung không phải IP. Để nhận gói, router đăng ký
một callback bằng `SetReceiveCallback`. Callback này phải được đăng ký sau khi `PointToPointHelper::Install` gắn thiết
bị vào nút, vì `Node::AddDevice` ghi đè callback nhận đã có; vì vậy router chỉ được gắn vào thiết bị (hàm
`AttachDevice` trong Đoạn mã 3.4) sau khi toàn bộ link đã được dựng. Đoạn mã 3.2 trình bày phần dựng link.

<!-- nguồn: noc/noc-sim.cc, 433b8a8 -->

::: {custom-style="Table Caption"}
Đoạn mã 3.2: Dựng các link điểm–điểm giữa các nút láng giềng (noc/noc-sim.cc)
:::

```cpp
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("16Gbps"));
    p2p.SetDeviceAttribute("InterframeGap", TimeValue(Seconds(0)));
    p2p.SetChannelAttribute("Delay", TimeValue(Seconds(0)));
    p2p.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("100000p"));

    // devs[id][dir]: device trên node id nối tới hàng xóm theo hướng dir (id logic = y·W + x).
    std::vector<std::array<Ptr<NetDevice>, kNumLinkDirs>> devs(n);
    for (int u = 0; u < n; ++u)
    {
        for (Dir d : {Dir::E, Dir::S}) // mỗi link PPP hai chiều chỉ cài một lần
        {
            const int v = mesh.Neighbor(u, d);
            if (v < 0)
            {
                continue;
            }
            NetDeviceContainer pair = p2p.Install(nodes.Get(u), nodes.Get(v));
            devs[u][static_cast<int>(d)] = pair.Get(0);
            devs[v][static_cast<int>(Opposite(d))] = pair.Get(1);
        }
    }
```

### Định dạng khung và tuần tự hóa header

Để một gói chiếm kênh đúng 16 chu kỳ như mô hình của [1], kích thước khung trên dây được chọn sao cho thời gian truyền
bằng 16 ns: $32\ \text{byte} \times 8 / 16\ \text{Gbps} = 16\ \text{ns}$. Khung 32 byte gồm header PPP 2 byte do
`PointToPointNetDevice` thêm vào, header riêng của đề tài `NoCHeader` 10 byte và 20 byte payload (Bảng 3.2).

<!-- nguồn: noc/noc-header.cc, noc/noc-header.h, results/selftest.txt, c7f4ea5 -->

Table: Bảng 3.2: Bố cục khung 32 byte trên dây

| Thành phần | Trường | Byte | Ý nghĩa |
|---|---|---|---|
| Header PPP | protocol | 2 | ns-3 ánh xạ `0x0800` sang mã PPP của IPv4 |
| NoCHeader | src | 2 | id nút nguồn |
| | dst | 2 | id nút đích |
| | seq | 4 | số thứ tự, dùng để ghép gói với thời điểm tạo |
| | hops | 2 | số hop đã đi, tăng 1 ở mỗi router nhận |
| Payload | — | 20 | phần đệm để khung đủ 32 byte |
| Tổng | | 32 | thời gian truyền 16 ns ở 16 Gbps |

`NoCHeader` kế thừa lớp `ns3::Header`, nên phải tự cài các hàm tuần tự hóa: `GetSerializedSize` cho biết số byte,
`Serialize` ghi các trường vào bộ đệm của gói theo thứ tự byte mạng (big-endian), và `Deserialize` đọc lại theo đúng
thứ tự đó (Đoạn mã 3.3). Thời điểm tạo gói không được ghi vào header mà gắn vào gói dưới dạng `Tag` (`CreationTimeTag`).
Tag là siêu dữ liệu đi kèm đối tượng gói trong bộ mô phỏng, không được truyền trên dây, nên không làm tăng kích thước
khung. Mỗi khung bắt đầu truyền đều được kiểm tra ở trace `PhyTxBegin`: nếu kích thước khác 32 byte, chương trình dừng.

<!-- nguồn: noc/noc-header.cc, c69dda5 -->

::: {custom-style="Table Caption"}
Đoạn mã 3.3: Tuần tự hóa NoCHeader (noc/noc-header.cc)
:::

```cpp
uint32_t
NoCHeader::GetSerializedSize() const
{
    return 2 + 2 + 4 + 2;
}

void
NoCHeader::Serialize(ns3::Buffer::Iterator start) const
{
    start.WriteHtonU16(m_src);
    start.WriteHtonU16(m_dst);
    start.WriteHtonU32(m_seq);
    start.WriteHtonU16(m_hops);
}

uint32_t
NoCHeader::Deserialize(ns3::Buffer::Iterator start)
{
    m_src = start.ReadNtohU16();
    m_dst = start.ReadNtohU16();
    m_seq = start.ReadNtohU32();
    m_hops = start.ReadNtohU16();
    return GetSerializedSize();
}
```

### Router

Router (`NocRouter`) xử lý gói theo hai hàm (Đoạn mã 3.4). Khi thiết bị nhận xong một khung, ns-3 gọi `Receive`. Hàm
này tách `NoCHeader` khỏi gói, tăng số hop và kiểm tra đích. Nếu gói đã tới đích, router đọc `CreationTimeTag` để tính
độ trễ và giao gói cho bộ thống kê. Nếu chưa, router gắn lại header và gọi `Forward`. `Forward` gọi `routeSet` với tọa
độ của nút hiện tại, nguồn và đích lấy từ header, chọn một cổng bằng `SelectUniform`, rồi gửi gói qua thiết bị tương
ứng. Nếu kênh đang bận, gói nằm trong hàng đợi của thiết bị cho tới khi kênh rảnh; vì thiết bị chỉ bắt đầu truyền khi
đã nhận trọn khung, router hoạt động theo kiểu store-and-forward.

<!-- nguồn: noc/noc-router.cc, 32f4e9a -->

::: {custom-style="Table Caption"}
Đoạn mã 3.4: Nhận và chuyển tiếp gói trong NocRouter (noc/noc-router.cc, rút gọn)
:::

```cpp
void
NocRouter::AttachDevice(Dir d, ns3::Ptr<ns3::NetDevice> dev)
{
    m_devs[static_cast<int>(d)] = dev;
    dev->SetReceiveCallback(ns3::MakeCallback(&NocRouter::Receive, this));
}
...
    ns3::Ptr<ns3::Packet> p = packet->Copy();
    NoCHeader hdr;
    p->RemoveHeader(hdr);
    hdr.SetHops(hdr.GetHops() + 1);
    if (hdr.GetDst() == m_id)
    {
        CreationTimeTag tag;
        NS_ABORT_MSG_UNLESS(p->PeekPacketTag(tag), "gói thiếu CreationTimeTag");
        m_deliver(hdr, tag.GetCreatedNs());
        return true;
    }
    p->AddHeader(hdr);
    Forward(p, hdr);
    return true;
}
...
    const DirSet s = routeSet(m_algo,
                              m_mesh.ToCoord(m_id),
                              m_mesh.ToCoord(hdr.GetSrc()),
                              m_mesh.ToCoord(hdr.GetDst()));
    const Dir d = SelectUniform(s, m_selectRv);
    ns3::Ptr<ns3::NetDevice> dev = m_devs[static_cast<int>(d)];
    NS_ABORT_MSG_UNLESS(dev, "node " << m_id << " không có link theo hướng " << DirName(d));
    // Send trả false khi queue tràn; drop được đếm qua trace "Drop" của queue.
    dev->Send(packet, dev->GetBroadcast(), 0x0800);
}
```

Số gói bị hủy được đếm qua trace `Drop` của hàng đợi và trace `PhyRxDrop` của thiết bị. Trong mọi lần chạy của đề tài,
số gói bị hủy bằng 0.

### Sinh lưu lượng và số ngẫu nhiên

Mỗi nút phát gói theo quá trình Bernoulli: ở mỗi chu kỳ, nút tạo một gói với xác suất PIR. Thay vì lập lịch một sự kiện
mỗi chu kỳ, chương trình rút khoảng cách tới lần phát kế tiếp từ phân phối hình học, nên số sự kiện tỉ lệ với số gói
chứ không tỉ lệ với số chu kỳ. Bài báo [1] dùng khoảng cách theo phân phối mũ. Hai phân phối có cùng kỳ vọng; hệ số biến
thiên (CV) của phân phối hình học là $\sqrt{1 - \mathrm{PIR}}$, rất gần 1 là CV của phân phối mũ.

<!-- nguồn: results/sweep.csv, c7f4ea5 -->
<!-- suy-ra: PIR lớn nhất trong sweep.csv là 0,0464282 = 1,3 × 0,035714 (XY uniform); CV = √(1 − 0,0464282) = 0,9765 ≈ 0,977 -->

Với PIR lớn nhất trong các lần quét (0,0464282), CV vẫn bằng khoảng 0,977, nên khác biệt giữa hai phân phối là không
đáng kể.

Mỗi router có ba luồng số ngẫu nhiên riêng, đánh theo id logic $\mathrm{id} = y \cdot W + x$ chứ không theo thứ tự tạo
nút của ns-3: luồng $3\,\mathrm{id}$ dùng để chọn cổng, luồng $3\,\mathrm{id}+1$ cho khoảng cách giữa các lần phát, và
luồng $3\,\mathrm{id}+2$ để chọn đích với mẫu uniform. Cùng tham số `--RngRun` thì cùng kết quả. Vì hai luồng sinh lưu
lượng không phụ thuộc thuật toán định tuyến, các cấu hình chạy với cùng mẫu lưu lượng, cùng PIR và cùng RngRun nhận
đúng cùng một tập gói. Kỹ thuật này gọi là common random numbers (CRN, dùng chung số ngẫu nhiên): hiệu độ trễ giữa hai
thuật toán trên cùng RngRun loại được phần lớn nhiễu do sinh lưu lượng, nên khoảng tin cậy của hiệu hẹp hơn nhiều so
với khi chạy độc lập. Đổi lại, mọi cấu hình cùng RngRun chung một nhiễu sinh lưu lượng, nên không được coi các kênh hay
các cấu hình là các mẫu độc lập.

### Cửa sổ đo và các chỉ số

Mỗi lần chạy gồm ba pha (Hình 3.2b). Pha warmup đưa mạng về trạng thái dừng, gói vẫn được tạo nhưng không được thống
kê. Trong cửa sổ đo dài measure chu kỳ, mọi gói được tạo ra đều được ghi lại. Sau cửa sổ đo là pha drain: các nút vẫn
tiếp tục phát để giữ nguyên mức tải, và lần chạy dừng khi mọi gói của cửa sổ đo đã tới đích, hoặc khi hết thời gian
drain. Các lần quét tải dùng warmup 10 000, measure 20 000 và drain 20 000 chu kỳ. Các chỉ số chính gồm:

- độ trễ lat_avg, đo từ lúc tạo gói, nên gồm cả thời gian chờ ở hàng đợi của nút nguồn;
- throughput: số flit tới đích trong cửa sổ đo chia cho số nút và độ dài cửa sổ; offered: số flit được tạo, chuẩn hóa
  tương tự;
- saturated: bằng 1 nếu hết pha drain mà còn gói của cửa sổ chưa tới đích; khi đó lat_avg_lb tính cả các gói này với
  độ trễ lấy bằng thời điểm dừng trừ thời điểm tạo (cận dưới);
- độ sử dụng đo được của từng kênh $U_{\text{đo}}(c)$: số khung bắt đầu truyền trong cửa sổ đo nhân 16 rồi chia cho
  measure; u_max là giá trị lớn nhất trên mọi kênh.

Lưu ý saturated = 0 không có nghĩa là mạng chưa bão hòa: pha drain đủ dài vẫn xả hết hàng đợi dù tải vượt ngưỡng. Vì
vậy chương 4 xác định bão hòa dựa trên throughput/offered, độ trễ và u_max.

## Chiến lược kiểm thử

### Bộ tự kiểm thử

Chế độ `--mode=selftest` chạy toàn bộ bộ tự kiểm thử trong khoảng một giây và trả mã thoát khác 0 nếu có kiểm tra HARD
bị sai. Mọi thay đổi chỉ được đưa lên nhánh chính khi chương trình biên dịch sạch và bộ tự kiểm thử đạt. Các kiểm tra
HARD là những tính chất phải đúng tuyệt đối: bất biến của hàm định tuyến, luật cấm rẽ (kèm đối chứng âm để chắc rằng
bộ kiểm tra bắt được vi phạm), số đường, bảo toàn lưu lượng, đối xứng gương giữa hai quy ước NF, CDG, và các tính chất
của mô phỏng. Ví dụ, một gói đi XY từ nút 0 tới nút 48 qua 12 hop trên mạng rỗng phải có độ trễ đúng 192 chu kỳ, tức
16 chu kỳ mỗi hop; kiểm tra này bắt được ngay các lỗi thừa byte trong khung hoặc lệch đơn vị thời gian. Các kiểm tra
SOFT so kết quả với các mốc của [1] và chỉ in MATCH hoặc MISMATCH, không làm hỏng mã thoát, vì theo nguyên tắc của đề tài,
khi kết quả không khớp với bài báo thì phải báo cáo chứ không được sửa chương trình để ép ra số. Bảng 3.3 tóm tắt bộ
tự kiểm thử.

<!-- nguồn: results/selftest.txt, c7f4ea5 -->

Table: Bảng 3.3: Bộ tự kiểm thử (--mode=selftest)

| Nhóm | Nội dung | Số kiểm tra | Kết quả |
|---|---|---|---|
| Lưới, traffic, khung | id ↔ tọa độ, láng giềng, số kênh từ 1×1 tới 7×7, số luồng, bố cục khung 32 byte, Header và Tag | 13 | 13 PASS |
| Định tuyến và số đường | tập cổng khác rỗng khi chưa tới đích và chỉ gồm hướng tối thiểu, độ dài đường, luật cấm rẽ và đối chứng âm, số đường mọi cặp, số đường 0→48, đối xứng NF | 66 | 66 PASS |
| Channel pressure | tổng xác suất theo nhánh bằng 1, bảo toàn lưu lượng, ρ nguyên với thuật toán tất định, đối xứng gương từng kênh, bảo toàn lưu lượng theo hướng rẽ | 72 | 72 PASS |
| CDG | không chu trình với XY, YX, hai quy ước NF, OE; FULL có chu trình | 12 | 12 PASS |
| Selection và mô phỏng | chọn cổng đều, percentile, một gói 12 hop mất 192 chu kỳ, khung 32 byte, độ trễ ≥ 16·hops, số hop, u_max, tính tất định, khôi phục RngRun | 18 | 18 PASS |
| Mốc của [1] (SOFT) | φ theo path và hop so với các mốc của bài báo | 18 | 11 MATCH, 7 MISMATCH |

Bảy trường hợp MISMATCH đều là các tổ hợp mà đề tài dự đoán trước sẽ lệch: cách chia theo đường với OE và NF, và NF
theo quy ước chính (mục 4.1).

### Kiểm tra mô phỏng ở tải thấp

Chế độ `--mode=simcheck` chạy lâu hơn nên không nằm trong bộ tự kiểm thử. Ở PIR = 0,001, gần như không có xung đột,
nên mô phỏng phải khớp các giá trị tính tay. Có bốn kiểm tra cho mỗi mẫu transpose với XY:

- (hard) mọi gói có độ trễ ≥ 16·hops;
- (model) độ trễ trung bình trừ 16 lần số hop trung bình, tức thời gian chờ trung bình, nằm trong khoảng 0 đến 1 chu kỳ;
- (stat) số hop trung bình khớp giá trị lý thuyết $224/42$ trong phạm vi 4 sai số chuẩn;
- (stat) không có gói bị hủy hay chưa tới đích, và throughput lệch không quá 5% so với giá trị lý thuyết.

Ngoài ra, NF theo quy ước chính với transpose1 phải cho cùng độ trễ và throughput (trong khoảng tin cậy 95%) với NF
(bài báo) với transpose2, vì hai cấu hình là ảnh gương của nhau. Bảng 3.4 cho kết quả.

<!-- nguồn: results/simcheck.txt, c7f4ea5 -->

Table: Bảng 3.4: Kết quả simcheck

| Kiểm tra | Mẫu | Giá trị đo | Ngưỡng | Kết quả |
|---|---|---|---|---|
| độ trễ ≥ 16·hops (min của độ trễ − 16·hops) | transpose1 | 0 | ≥ 0 | PASS |
| | transpose2 | 0 | ≥ 0 | PASS |
| thời gian chờ trung bình (chu kỳ) | transpose1 | 0,540913 | [0, 1] | PASS |
| | transpose2 | 0,627314 | [0, 1] | PASS |
| số hop trung bình (z) | transpose1 | 5,376308 (0,935) | \|z\| ≤ 4 | PASS |
| | transpose2 | 5,351767 (0,399) | \|z\| ≤ 4 | PASS |
| throughput (sai lệch tương đối) | transpose1 | 0,006867 (0,001429) | ≤ 0,05 | PASS |
| | transpose2 | 0,006792 (0,009524) | ≤ 0,05 | PASS |
| đối xứng gương: hiệu lat_avg | NF/t1 so với NF (bài báo)/t2 | 0,501878 | ≤ 1,766331 | PASS |
| đối xứng gương: hiệu throughput | | 0,000217 | ≤ 0,001046 | PASS |

## Tái lập kết quả

Toàn bộ số liệu trong báo cáo được sinh lại từ đầu bằng một lệnh, `scripts/reproduce.sh`. Script chạy lần lượt: biên
dịch, bộ tự kiểm thử, simcheck, tính routing pressure, ví dụ 3×3, kiểm chứng tải từng kênh, quét tải, phân tích bão
hòa, mô hình hàng đợi, vẽ hình và sinh bảng tổng hợp `report/RESULTS.md`. Mọi output được ghi vào một thư mục tạm, sau
đó so từng byte với các file đã lưu trong kho mã; script không bao giờ ghi đè kết quả đã lưu, và báo file cùng dòng đầu
tiên khác nếu có.

Để kết quả giống từng byte giữa các lần chạy, đề tài áp dụng ba biện pháp:

- chương trình được biên dịch ở profile release (`-O3`) mà không bật `-march=native`, vì tối ưu theo từng máy có thể
  sinh lệnh FMA làm thay đổi chữ số cuối của phép tính số thực;
- không ghi thời gian chạy vào file kết quả, và các dòng CSV được sắp xếp theo khóa sau khi các tiến trình song song
  chạy xong;
- dữ liệu được sắp xếp trước mọi phép lấy trung bình, vì thứ tự cộng số thực ảnh hưởng tới chữ số cuối.

Phiên bản kết quả dùng trong báo cáo được đánh dấu bằng tag `v1.0-results` trong kho mã. Các phần bổ sung sau đó (ví dụ
3×3 và các hình minh họa) cũng được đưa vào `reproduce.sh` và được so sánh theo cùng cách.

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
