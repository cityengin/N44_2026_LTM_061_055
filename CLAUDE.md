# NoC Routing Pressure (ns-3) — đồ án Lập trình mạng

Tái hiện Tang, Lin, Palesi, "Routing Pressure: A Channel-Related and Traffic-Aware
Metric of Routing Algorithm", IEEE TPDS 26(3), 2015 (docs/). Hai báo cáo cũ trong
docs/ chỉ để tham khảo các lỗi đã mắc, không sao chép.

## Build / run
- ns-3 ở ~/ns-3-dev (ns-3.48). noc/ được symlink vào scratch/noc → chương trình `noc`.
- Build: `cd ~/ns-3-dev && ./ns3 build noc`
- Chạy: `./ns3 run --no-build "noc --mode=phi --routing=OE --traffic=transpose1"`
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
- Giả thuyết (chưa kiểm chứng bằng số): bài báo (Noxim) coi hướng âm = W, S. Lật y → N−1−y biến
  transpose1 ↔ transpose2 và NF(W,N) ↔ NF(W,S), còn XY và OE không đổi. Do đó dự đoán
  φ(NF,t1) = φ(NF_WS,t2) và φ(NF,t2) = φ(NF_WS,t1).
- Quyết định: thêm `NF_WS` (âm = W, S; cấm E→S, N→W) làm một Algo đầy đủ. Nó dùng chung hàm routing
  cho φ, CDG và sim (`--routing=NF_WS`). `NF` mặc định vẫn theo quy ước chính. Mọi output của NF_WS
  ghi `variant=diagnostic`. Không đổi mốc kiểm thử ở trên. Kết quả so sánh sẽ được ghi vào mục này
  sau bước tính φ.

## Lưu ý ns-3 đã biết
- Không cài Internet stack. Gửi bằng NetDevice::Send với protocol 0x0800
  (PointToPointNetDevice chỉ map IPv4/IPv6 sang PPP).
- Header làm tăng số byte trên dây; Tag thì không. Timestamp dùng Tag.
- Mọi noc/*.cc được biên dịch chung thành một chương trình. CMake của scratch nhận diện file có main
  bằng regex `main[(| (]` trên toàn bộ nội dung file .cc, kể cả comment (ví dụ "remain " cũng khớp).
  CHỈ noc/noc.cc được chứa chuỗi đó; file .h không bị quét.
  Kiểm tra: `grep -l -E "main[(| ]" noc/*.cc` chỉ được ra noc/noc.cc.

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