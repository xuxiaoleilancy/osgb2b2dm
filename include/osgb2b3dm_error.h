#pragma once

#include <string>
#include <stdexcept>
#include <system_error>

namespace osgb2b3dm {

// 错误码枚举
enum class ErrorCode {
    // 文件操作错误 (1000-1999)
    FILE_NOT_FOUND = 1000,
    FILE_READ_ERROR = 1001,
    FILE_WRITE_ERROR = 1002,
    FILE_PERMISSION_DENIED = 1003,
    FILE_ALREADY_EXISTS = 1004,
    FILE_FORMAT_INVALID = 1005,

    // OSG 相关错误 (2000-2999)
    OSG_LOAD_ERROR = 2000,
    OSG_SAVE_ERROR = 2001,
    OSG_INVALID_NODE = 2002,
    OSG_INVALID_GEOMETRY = 2003,
    OSG_INVALID_MATERIAL = 2004,
    OSG_INVALID_TEXTURE = 2005,
    OSG_INVALID_ANIMATION = 2006,
    OSG_INVALID_INSTANCE = 2007,

    // 转换过程错误 (3000-3999)
    CONVERSION_FAILED = 3000,
    INVALID_INPUT = 3001,
    INVALID_OUTPUT = 3002,
    MEMORY_ALLOCATION_FAILED = 3003,
    DATA_PROCESSING_ERROR = 3004,
    VALIDATION_FAILED = 3005,

    // 材质和纹理错误 (4000-4999)
    MATERIAL_PROCESSING_ERROR = 4000,
    TEXTURE_PROCESSING_ERROR = 4001,
    TEXTURE_NOT_FOUND = 4002,
    TEXTURE_FORMAT_UNSUPPORTED = 4003,
    MATERIAL_PROPERTY_INVALID = 4004,

    // 动画相关错误 (5000-5999)
    ANIMATION_PROCESSING_ERROR = 5000,
    ANIMATION_DATA_INVALID = 5001,
    ANIMATION_CHANNEL_INVALID = 5002,
    ANIMATION_SAMPLER_INVALID = 5003,
    ANIMATION_TARGET_INVALID = 5004,

    // 实例化相关错误 (6000-6999)
    INSTANCE_PROCESSING_ERROR = 6000,
    INSTANCE_DATA_INVALID = 6001,
    INSTANCE_TRANSFORM_INVALID = 6002,
    INSTANCE_GROUP_INVALID = 6003,

    // 系统错误 (9000-9999)
    SYSTEM_ERROR = 9000,
    UNKNOWN_ERROR = 9999
};

// 错误类别
class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "osgb2b3dm";
    }

    std::string message(int ev) const override {
        switch (static_cast<ErrorCode>(ev)) {
            // 文件操作错误
            case ErrorCode::FILE_NOT_FOUND:
                return "File not found";
            case ErrorCode::FILE_READ_ERROR:
                return "Error reading file";
            case ErrorCode::FILE_WRITE_ERROR:
                return "Error writing file";
            case ErrorCode::FILE_PERMISSION_DENIED:
                return "Permission denied";
            case ErrorCode::FILE_ALREADY_EXISTS:
                return "File already exists";
            case ErrorCode::FILE_FORMAT_INVALID:
                return "Invalid file format";

            // OSG 相关错误
            case ErrorCode::OSG_LOAD_ERROR:
                return "Error loading OSG file";
            case ErrorCode::OSG_SAVE_ERROR:
                return "Error saving OSG file";
            case ErrorCode::OSG_INVALID_NODE:
                return "Invalid OSG node";
            case ErrorCode::OSG_INVALID_GEOMETRY:
                return "Invalid OSG geometry";
            case ErrorCode::OSG_INVALID_MATERIAL:
                return "Invalid OSG material";
            case ErrorCode::OSG_INVALID_TEXTURE:
                return "Invalid OSG texture";
            case ErrorCode::OSG_INVALID_ANIMATION:
                return "Invalid OSG animation";
            case ErrorCode::OSG_INVALID_INSTANCE:
                return "Invalid OSG instance";

            // 转换过程错误
            case ErrorCode::CONVERSION_FAILED:
                return "Conversion failed";
            case ErrorCode::INVALID_INPUT:
                return "Invalid input";
            case ErrorCode::INVALID_OUTPUT:
                return "Invalid output";
            case ErrorCode::MEMORY_ALLOCATION_FAILED:
                return "Memory allocation failed";
            case ErrorCode::DATA_PROCESSING_ERROR:
                return "Data processing error";
            case ErrorCode::VALIDATION_FAILED:
                return "Validation failed";

            // 材质和纹理错误
            case ErrorCode::MATERIAL_PROCESSING_ERROR:
                return "Material processing error";
            case ErrorCode::TEXTURE_PROCESSING_ERROR:
                return "Texture processing error";
            case ErrorCode::TEXTURE_NOT_FOUND:
                return "Texture not found";
            case ErrorCode::TEXTURE_FORMAT_UNSUPPORTED:
                return "Unsupported texture format";
            case ErrorCode::MATERIAL_PROPERTY_INVALID:
                return "Invalid material property";

            // 动画相关错误
            case ErrorCode::ANIMATION_PROCESSING_ERROR:
                return "Animation processing error";
            case ErrorCode::ANIMATION_DATA_INVALID:
                return "Invalid animation data";
            case ErrorCode::ANIMATION_CHANNEL_INVALID:
                return "Invalid animation channel";
            case ErrorCode::ANIMATION_SAMPLER_INVALID:
                return "Invalid animation sampler";
            case ErrorCode::ANIMATION_TARGET_INVALID:
                return "Invalid animation target";

            // 实例化相关错误
            case ErrorCode::INSTANCE_PROCESSING_ERROR:
                return "Instance processing error";
            case ErrorCode::INSTANCE_DATA_INVALID:
                return "Invalid instance data";
            case ErrorCode::INSTANCE_TRANSFORM_INVALID:
                return "Invalid instance transform";
            case ErrorCode::INSTANCE_GROUP_INVALID:
                return "Invalid instance group";

            // 系统错误
            case ErrorCode::SYSTEM_ERROR:
                return "System error";
            case ErrorCode::UNKNOWN_ERROR:
                return "Unknown error";
            default:
                return "Unknown error code";
        }
    }
};

// 获取错误类别实例
const ErrorCategory& errorCategory();

// 创建错误码
std::error_code make_error_code(ErrorCode e);

// 异常类
class Error : public std::runtime_error {
public:
    Error(ErrorCode code, const std::string& message = "")
        : std::runtime_error(message.empty() ? errorCategory().message(static_cast<int>(code)) : message)
        , code_(code) {}

    ErrorCode code() const { return code_; }

private:
    ErrorCode code_;
};

} // namespace osgb2b3dm

// 注册错误码
namespace std {
    template<>
    struct is_error_code_enum<osgb2b3dm::ErrorCode> : true_type {};
} 