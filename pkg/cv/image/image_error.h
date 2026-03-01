
#ifndef KINGFISHER_PKG_CV_IMAGE_ERROR_H_
#define KINGFISHER_PKG_CV_IMAGE_ERROR_H_

namespace kingfisher {
namespace kcv {

/**
 * @brief 图像处理模块错误码枚举
 *
 * 规范：
 * - 0 表示成功
 * - 负数表示错误
 * - 不同类别的错误码使用不同的起始值
 */
enum ImageError {
  // 成功
  kImageOk = 0,

  // 通用错误 (-1 ~ -99)
  kImageUnknownError = -1,         // 未知错误
  kImageInvalidParam = -2,         // 无效参数
  kImageMemoryError = -3,          // 内存分配失败
  kImageNotInitialized = -4,       // 未初始化（GlobalInit 未调用）

  // 解码/编码错误 (-100 ~ -199)
  kImageDecodeError = -100,        // 解码失败
  kImageEncodeError = -101,        // 编码失败
  kImageUnsupportedFormat = -102,  // 不支持的格式
  kImageInvalidData = -103,        // 无效的图像数据（空或损坏）

  // 文件 I/O 错误 (-200 ~ -299)
  kImageFileNotFound = -200,       // 文件未找到
  kImageFileOpenError = -201,      // 文件打开失败
  kImageFileReadError = -202,      // 文件读取失败
  kImageFileWriteError = -203,     // 文件写入失败

  // 图像操作错误 (-300 ~ -399)
  kImageEmptyInput = -300,         // 输入图像为空
  kImageSizeMismatch = -301,       // 图像尺寸不匹配
  kImageTypeMismatch = -302,       // 图像类型不匹配
  kImageInvalidRegion = -303,      // 无效的裁剪/操作区域
  kImageUnsupportedOperation = -304, // 不支持的操作

  // 颜色空间错误 (-400 ~ -499)
  kImageUnsupportedColorSpace = -400,  // 不支持的颜色空间转换
  kImageChannelMismatch = -401,        // 通道数不匹配
};

/**
 * @brief 将错误码转换为可读的错误消息
 */
inline const char* ImageErrorToString(ImageError err) {
  switch (err) {
    case kImageOk:                    return "success";
    case kImageUnknownError:          return "unknown error";
    case kImageInvalidParam:          return "invalid parameter";
    case kImageMemoryError:           return "memory allocation error";
    case kImageNotInitialized:        return "not initialized";
    case kImageDecodeError:           return "decode error";
    case kImageEncodeError:           return "encode error";
    case kImageUnsupportedFormat:     return "unsupported format";
    case kImageInvalidData:           return "invalid image data";
    case kImageFileNotFound:          return "file not found";
    case kImageFileOpenError:         return "file open error";
    case kImageFileReadError:         return "file read error";
    case kImageFileWriteError:        return "file write error";
    case kImageEmptyInput:            return "empty input image";
    case kImageSizeMismatch:          return "image size mismatch";
    case kImageTypeMismatch:          return "image type mismatch";
    case kImageInvalidRegion:         return "invalid region";
    case kImageUnsupportedOperation:  return "unsupported operation";
    case kImageUnsupportedColorSpace: return "unsupported color space";
    case kImageChannelMismatch:       return "channel count mismatch";
    default:                          return "unknown error code";
  }
}

}  // namespace kcv
}  // namespace kingfisher

#endif
