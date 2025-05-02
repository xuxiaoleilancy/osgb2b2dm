#include "osgb2b3dm_error.h"

namespace osgb2b3dm {

// 错误类别实例
const ErrorCategory& errorCategory() {
    static ErrorCategory instance;
    return instance;
}

// 创建错误码
std::error_code make_error_code(ErrorCode e) {
    return std::error_code(static_cast<int>(e), errorCategory());
}

} // namespace osgb2b3dm 