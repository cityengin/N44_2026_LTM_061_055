#pragma once

namespace noc
{

// Chạy các mốc kiểm thử, in mỗi test một dòng RESULT.
// Trả về số test HARD bị fail (test REPORT không tính).
// forceFail: thêm một test HARD luôn fail, dùng để kiểm tra exit code.
int RunSelfTest(bool forceFail = false);

} // namespace noc
