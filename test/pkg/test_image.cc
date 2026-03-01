#include <gtest/gtest.h>

#ifdef ENABLE_GRAPHICS_MAGICK
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "core/scope_guard.h"
#include "cv/image/image.h"
#include "cv/image/image_error.h"

using namespace kingfisher;
using namespace kingfisher::kcv;

//./kingfisher_base_test --gtest_filter=test_Image.*
//./kingfisher_base_test --gtest_filter=test_Image.DecodeImage
class test_Image : public testing::Test {
 public:
  test_Image() {}
  ~test_Image() {}

  virtual void SetUp(void) {
    kingfisher::kcv::Image::GlobalInit();
  }

  virtual void TearDown(void) {}
};

// ==================== 基础功能测试 ====================

TEST_F(test_Image, DecodeImageFile) {
  std::string imageFile = "./testdata/1.jpg";

  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImageFile(imageFile, opts, mat);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(mat.empty());
  ::cv::imwrite("./output.BGRColorSpace.png", mat);
}

TEST_F(test_Image, DecodeImage) {
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImage(content, opts, mat);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(mat.empty());
  ::cv::imwrite("./output.BGRColorSpace.png", mat);
}

TEST_F(test_Image, RotateImage) {
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::RotateImage(content, -45, mat);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(mat.empty());
  ::cv::imwrite("./output.rotate.png", mat);
}

TEST_F(test_Image, RotateImageMat) {
  std::string imageFile = "./testdata/1.jpg";

  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImageFile(imageFile, opts, mat);
  EXPECT_EQ(ret, 0);

  ::cv::Mat rotated_mat;
  ret = kingfisher::kcv::Image::RotateImage(mat, 90, rotated_mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.rotate.png", rotated_mat);
}

TEST_F(test_Image, ResizeImage) {
  std::string imageFile = "./testdata/kay2.png";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::ResizeImage(content, 520, 765, false, mat);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(mat.empty());
  ::cv::imwrite("./output.resize.jpg", mat);
}

TEST_F(test_Image, CropImage) {
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  kingfisher::kcv::Rect rect;
  rect.set_x(10);
  rect.set_y(10);
  rect.set_width(100);
  rect.set_height(200);
  int ret = kingfisher::kcv::Image::CropImage(content, rect, mat);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(mat.empty());
  ::cv::imwrite("./output.crop.png", mat);
}

TEST_F(test_Image, DumpImageFileToBytes) {
  std::string imageFile = "./testdata/1.jpg";
  std::string imageBytes = "./testdata/1_bytes.txt";

  int ret = kingfisher::kcv::Image::DumpImageFileToBytes(imageFile, imageBytes);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, AdaptiveWatermarkFill) {
  std::string image = "/data/home/kayxhding/workspace/github.com/kaydxh/kingfisher/test/testdata/1.jpg";
  std::string logo = "/data/home/kayxhding/workspace/github.com/kaydxh/kingfisher/test/testdata/1.jpg";
  cv::Mat image_mat = cv::imread(image, cv::IMREAD_UNCHANGED);
  cv::Mat logo_mat = cv::imread(logo, cv::IMREAD_UNCHANGED);

  // 在(100,50)位置创建200x150区域（允许负坐标和越界）
  kingfisher::kcv::Image::AdaptiveWatermarkFill(image_mat, logo_mat, cv::Rect(100, 50, 200, 150),
                       cv::INTER_LANCZOS4, 0.7);
  ::cv::imwrite("/data/home/kayxhding/workspace/github.com/kaydxh/kingfisher/test/testdata/output.mark.png", image_mat);
}

// ==================== 边界条件测试 ====================

TEST_F(test_Image, DecodeImage_EmptyData) {
  std::string emptyData = "";
  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  int ret = kingfisher::kcv::Image::DecodeImage(emptyData, opts, mat);
  EXPECT_NE(ret, 0);  // 应该失败
  EXPECT_TRUE(mat.empty());
}

TEST_F(test_Image, DecodeImage_InvalidData) {
  std::string invalidData = "this is not an image";
  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  int ret = kingfisher::kcv::Image::DecodeImage(invalidData, opts, mat);
  EXPECT_NE(ret, 0);  // 应该失败
}

TEST_F(test_Image, DecodeImageFile_NotFound) {
  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::DecodeImageFile("/nonexistent/path.jpg", mat);
  EXPECT_NE(ret, 0);  // 应该失败
}

TEST_F(test_Image, ResizeImage_ZeroSize) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ResizeImage(input, 0, 0, false, output);
  EXPECT_NE(ret, 0);  // 无效尺寸应该失败
}

TEST_F(test_Image, ResizeImage_NegativeSize) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ResizeImage(input, -10, -10, false, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, ResizeImage_EmptyInput) {
  cv::Mat emptyMat;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ResizeImage(emptyMat, 100, 100, false, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, CropImage_EmptyInput) {
  cv::Mat emptyMat;
  kingfisher::kcv::Rect rect;
  rect.set_x(0);
  rect.set_y(0);
  rect.set_width(50);
  rect.set_height(50);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::CropImage(emptyMat, rect, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, CropImage_OutOfBounds) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  kingfisher::kcv::Rect rect;
  rect.set_x(200);
  rect.set_y(200);
  rect.set_width(50);
  rect.set_height(50);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::CropImage(input, rect, output);
  EXPECT_NE(ret, 0);  // 完全越界应该失败
}

// ==================== 新增核心功能测试 ====================

TEST_F(test_Image, EncodeImage_JPEG) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  std::string output;
  int ret = kingfisher::kcv::Image::EncodeImage(input, ".jpg", 90, output);
  EXPECT_EQ(ret, 0);
  EXPECT_GT(output.size(), 0u);
}

TEST_F(test_Image, EncodeImage_PNG) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  std::string output;
  int ret = kingfisher::kcv::Image::EncodeImage(input, ".png", 3, output);
  EXPECT_EQ(ret, 0);
  EXPECT_GT(output.size(), 0u);
}

TEST_F(test_Image, EncodeImage_EmptyMat) {
  cv::Mat emptyMat;
  std::string output;
  int ret = kingfisher::kcv::Image::EncodeImage(emptyMat, ".jpg", 90, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, EncodeImage_WithOpts) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  kingfisher::kcv::EncodeOptions opts;
  opts.set_format("jpeg");
  opts.set_quality(85);
  std::string output;
  int ret = kingfisher::kcv::Image::EncodeImage(input, opts, output);
  EXPECT_EQ(ret, 0);
  EXPECT_GT(output.size(), 0u);
}

TEST_F(test_Image, ConvertFormat_JpegToPng) {
  // 先准备一个 JPEG 数据
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  std::string jpegData;
  int ret = kingfisher::kcv::Image::EncodeImage(input, ".jpg", 90, jpegData);
  EXPECT_EQ(ret, 0);

  // 转换为 PNG
  std::string pngData;
  ret = kingfisher::kcv::Image::ConvertFormat(jpegData, ".png", 3, pngData);
  EXPECT_EQ(ret, 0);
  EXPECT_GT(pngData.size(), 0u);
}

TEST_F(test_Image, ConvertFormat_EmptyInput) {
  std::string emptyData = "";
  std::string output;
  int ret = kingfisher::kcv::Image::ConvertFormat(emptyData, ".png", 3, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, ConvertColorSpace_BGR2GRAY) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ConvertColorSpace(
      input, kingfisher::kcv::BGRColorSpace, kingfisher::kcv::GRAYColorSpace, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.channels(), 1);
}

TEST_F(test_Image, ConvertColorSpace_BGR2RGB) {
  cv::Mat input(100, 100, CV_8UC3, cv::Scalar(10, 20, 30));
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ConvertColorSpace(
      input, kingfisher::kcv::BGRColorSpace, kingfisher::kcv::RGBColorSpace, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.channels(), 3);
  // 验证 BGR→RGB 通道交换：B=10, G=20, R=30 → R=10, G=20, B=30
  EXPECT_EQ(output.at<cv::Vec3b>(0, 0)[0], 30);  // R
  EXPECT_EQ(output.at<cv::Vec3b>(0, 0)[2], 10);  // B
}

TEST_F(test_Image, ConvertColorSpace_SameColorSpace) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ConvertColorSpace(
      input, kingfisher::kcv::BGRColorSpace, kingfisher::kcv::BGRColorSpace, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, ConvertColorSpace_EmptyInput) {
  cv::Mat emptyMat;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ConvertColorSpace(
      emptyMat, kingfisher::kcv::BGRColorSpace, kingfisher::kcv::GRAYColorSpace, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, ResizeMat_KeepRatio) {
  cv::Mat input = cv::Mat::ones(200, 400, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::ResizeImage(input, 100, 100, true, output);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(output.empty());
}

TEST_F(test_Image, CropMat) {
  cv::Mat input = cv::Mat::ones(200, 300, CV_8UC3) * 128;
  kingfisher::kcv::Rect rect;
  rect.set_x(10);
  rect.set_y(10);
  rect.set_width(50);
  rect.set_height(50);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::CropImage(input, rect, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.cols, 50);
  EXPECT_EQ(output.rows, 50);
}

TEST_F(test_Image, CenterCropImage) {
  cv::Mat input = cv::Mat::ones(200, 300, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::CenterCropImage(input, 100, 100, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.cols, 100);
  EXPECT_EQ(output.rows, 100);
}

TEST_F(test_Image, CenterCropImage_LargerThanInput) {
  cv::Mat input = cv::Mat::ones(50, 50, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::CenterCropImage(input, 200, 200, output);
  EXPECT_EQ(ret, 0);
  // 裁剪尺寸应自动限制为图像大小
  EXPECT_LE(output.cols, input.cols);
  EXPECT_LE(output.rows, input.rows);
}

TEST_F(test_Image, FlipImage_Horizontal) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::FlipImage(input, kingfisher::kcv::FLIP_HORIZONTAL, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, FlipImage_Vertical) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::FlipImage(input, kingfisher::kcv::FLIP_VERTICAL, output);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, FlipImage_Both) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::FlipImage(input, kingfisher::kcv::FLIP_BOTH, output);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, FlipImage_EmptyInput) {
  cv::Mat emptyMat;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::FlipImage(emptyMat, kingfisher::kcv::FLIP_HORIZONTAL, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, HConcat) {
  cv::Mat img1 = cv::Mat::ones(100, 50, CV_8UC3) * 100;
  cv::Mat img2 = cv::Mat::ones(100, 80, CV_8UC3) * 200;
  std::vector<cv::Mat> images = {img1, img2};
  cv::Mat output;
  int ret = kingfisher::kcv::Image::HConcat(images, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.rows, 100);
  EXPECT_EQ(output.cols, 50 + 80);
}

TEST_F(test_Image, HConcat_DifferentHeights) {
  cv::Mat img1 = cv::Mat::ones(100, 50, CV_8UC3) * 100;
  cv::Mat img2 = cv::Mat::ones(200, 80, CV_8UC3) * 200;
  std::vector<cv::Mat> images = {img1, img2};
  cv::Mat output;
  int ret = kingfisher::kcv::Image::HConcat(images, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.rows, 200);  // 统一到最大高度
}

TEST_F(test_Image, HConcat_EmptyList) {
  std::vector<cv::Mat> images;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::HConcat(images, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, VConcat) {
  cv::Mat img1 = cv::Mat::ones(50, 100, CV_8UC3) * 100;
  cv::Mat img2 = cv::Mat::ones(80, 100, CV_8UC3) * 200;
  std::vector<cv::Mat> images = {img1, img2};
  cv::Mat output;
  int ret = kingfisher::kcv::Image::VConcat(images, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.cols, 100);
  EXPECT_EQ(output.rows, 50 + 80);
}

TEST_F(test_Image, Overlay_Normal) {
  cv::Mat dest = cv::Mat::zeros(200, 200, CV_8UC3);
  cv::Mat src = cv::Mat::ones(50, 50, CV_8UC3) * 255;
  int ret = kingfisher::kcv::Image::Overlay(dest, src, 10, 10, 0.5);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, Overlay_NegativePosition) {
  cv::Mat dest = cv::Mat::zeros(200, 200, CV_8UC3);
  cv::Mat src = cv::Mat::ones(50, 50, CV_8UC3) * 255;
  int ret = kingfisher::kcv::Image::Overlay(dest, src, -10, -10, 0.7);
  EXPECT_EQ(ret, 0);  // 负坐标应正常处理
}

TEST_F(test_Image, Overlay_Overflow) {
  cv::Mat dest = cv::Mat::zeros(100, 100, CV_8UC3);
  cv::Mat src = cv::Mat::ones(50, 50, CV_8UC3) * 255;
  int ret = kingfisher::kcv::Image::Overlay(dest, src, 80, 80, 1.0);
  EXPECT_EQ(ret, 0);  // 超出边界应自动裁剪
}

TEST_F(test_Image, Overlay_NoOverlap) {
  cv::Mat dest = cv::Mat::zeros(100, 100, CV_8UC3);
  cv::Mat src = cv::Mat::ones(50, 50, CV_8UC3) * 255;
  int ret = kingfisher::kcv::Image::Overlay(dest, src, 200, 200, 1.0);
  EXPECT_EQ(ret, 0);  // 无重叠区域应安全返回
}

TEST_F(test_Image, AnnotateImage_Mat) {
  cv::Mat input = cv::Mat::zeros(200, 400, CV_8UC3);
  kingfisher::kcv::AnnotateOptions opts;
  opts.set_font_size(1.0);
  opts.set_color_r(255);
  opts.set_color_g(255);
  opts.set_color_b(255);
  opts.set_thickness(2);
  opts.set_anti_alias(true);
  int ret = kingfisher::kcv::Image::AnnotateImage(input, "Hello World",
                                                    cv::Point(50, 100), opts);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, AnnotateImage_EmptyText) {
  cv::Mat input = cv::Mat::zeros(200, 200, CV_8UC3);
  kingfisher::kcv::AnnotateOptions opts;
  int ret = kingfisher::kcv::Image::AnnotateImage(input, "", cv::Point(50, 100), opts);
  EXPECT_EQ(ret, 0);  // 空文本应安全返回
}

TEST_F(test_Image, WriteImage_WithParams) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 95};
  int ret = kingfisher::kcv::Image::WriteImage(input, "/tmp/test_quality.jpg", params);
  EXPECT_EQ(ret, 0);
}

// ==================== 进阶功能测试 ====================

TEST_F(test_Image, Thumbnail_FromMat) {
  cv::Mat input = cv::Mat::ones(800, 1200, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::Thumbnail(input, 300, output);
  EXPECT_EQ(ret, 0);
  EXPECT_LE(output.cols, 300);
  EXPECT_LE(output.rows, 300);
  // 长边应等于 maxDimension
  EXPECT_EQ(std::max(output.cols, output.rows), 300);
}

TEST_F(test_Image, Thumbnail_AlreadySmall) {
  cv::Mat input = cv::Mat::ones(100, 80, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::Thumbnail(input, 300, output);
  EXPECT_EQ(ret, 0);
  // 图像已小于 maxDimension，应保持原尺寸
  EXPECT_EQ(output.cols, input.cols);
  EXPECT_EQ(output.rows, input.rows);
}

TEST_F(test_Image, Thumbnail_InvalidParam) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat output;
  int ret = kingfisher::kcv::Image::Thumbnail(input, 0, output);
  EXPECT_NE(ret, 0);

  ret = kingfisher::kcv::Image::Thumbnail(input, -100, output);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, GaussianBlur) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::GaussianBlur(input, output, 5, 1.0);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, MeanBlur) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::MeanBlur(input, output, 5);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, MedianBlur) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::MedianBlur(input, output, 5);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, BilateralFilter) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::BilateralFilter(input, output, 9, 75, 75);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, BlurImage_Unified) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  kingfisher::kcv::FilterOptions opts;
  opts.set_blur_type(kingfisher::kcv::BLUR_GAUSSIAN);
  opts.set_kernel_size(5);
  opts.set_sigma(1.0);
  int ret = kingfisher::kcv::Image::BlurImage(input, opts, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, Blur_EmptyInput) {
  cv::Mat emptyMat;
  cv::Mat output;
  EXPECT_NE(kingfisher::kcv::Image::GaussianBlur(emptyMat, output), 0);
  EXPECT_NE(kingfisher::kcv::Image::MeanBlur(emptyMat, output), 0);
  EXPECT_NE(kingfisher::kcv::Image::MedianBlur(emptyMat, output), 0);
  EXPECT_NE(kingfisher::kcv::Image::BilateralFilter(emptyMat, output), 0);
}

TEST_F(test_Image, Blur_EvenKernelSize) {
  // 偶数核大小应自动校正为奇数
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::GaussianBlur(input, output, 4, 0);
  EXPECT_EQ(ret, 0);  // 应自动校正为5并成功
}

TEST_F(test_Image, Sharpen) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::Sharpen(input, output, 1.5, 3);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, AdjustBrightnessContrast) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::AdjustBrightnessContrast(input, output, 50, 1.2);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.size(), input.size());
}

TEST_F(test_Image, AdjustBrightnessContrast_EmptyInput) {
  cv::Mat emptyMat;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::AdjustBrightnessContrast(emptyMat, output, 0, 1.0);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, Grayscale_BGR) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::Grayscale(input, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.channels(), 1);
}

TEST_F(test_Image, Grayscale_AlreadyGray) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC1) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::Grayscale(input, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.channels(), 1);
}

TEST_F(test_Image, ComputeImageHash_AHash) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  uint64_t hash;
  int ret = kingfisher::kcv::Image::ComputeImageHash(input, kingfisher::kcv::HASH_AVERAGE, hash);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, ComputeImageHash_PHash) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  uint64_t hash;
  int ret = kingfisher::kcv::Image::ComputeImageHash(input, kingfisher::kcv::HASH_PERCEPTUAL, hash);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, ComputeImageHash_DHash) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  uint64_t hash;
  int ret = kingfisher::kcv::Image::ComputeImageHash(input, kingfisher::kcv::HASH_DIFFERENCE, hash);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, HammingDistance_SameHash) {
  uint64_t hash = 0x1234567890abcdef;
  int distance = kingfisher::kcv::Image::HammingDistance(hash, hash);
  EXPECT_EQ(distance, 0);
}

TEST_F(test_Image, HammingDistance_DifferentHash) {
  uint64_t hash1 = 0xFFFFFFFFFFFFFFFF;
  uint64_t hash2 = 0x0000000000000000;
  int distance = kingfisher::kcv::Image::HammingDistance(hash1, hash2);
  EXPECT_EQ(distance, 64);
}

TEST_F(test_Image, ComputeSSIM_SameImage) {
  cv::Mat img = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  double ssim = kingfisher::kcv::Image::ComputeSSIM(img, img);
  EXPECT_GT(ssim, 0.99);  // 相同图像 SSIM 应接近 1.0
}

TEST_F(test_Image, ComputeSSIM_EmptyImage) {
  cv::Mat empty;
  cv::Mat img = cv::Mat::ones(100, 100, CV_8UC3);
  double ssim = kingfisher::kcv::Image::ComputeSSIM(empty, img);
  EXPECT_LT(ssim, 0);  // 空图像应返回负数
}

TEST_F(test_Image, ComputeSSIM_SizeMismatch) {
  cv::Mat img1 = cv::Mat::ones(100, 100, CV_8UC3);
  cv::Mat img2 = cv::Mat::ones(200, 200, CV_8UC3);
  double ssim = kingfisher::kcv::Image::ComputeSSIM(img1, img2);
  EXPECT_LT(ssim, 0);  // 尺寸不同应返回负数
}

TEST_F(test_Image, CompareHistogram_SameImage) {
  cv::Mat img = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  double result = kingfisher::kcv::Image::CompareHistogram(
      img, img, kingfisher::kcv::COMPARE_CORRELATION);
  EXPECT_GT(result, 0.99);  // 相关性应接近 1.0
}

TEST_F(test_Image, CompareHistogram_EmptyImage) {
  cv::Mat empty;
  cv::Mat img = cv::Mat::ones(100, 100, CV_8UC3);
  double result = kingfisher::kcv::Image::CompareHistogram(empty, img);
  EXPECT_LT(result, 0);
}

TEST_F(test_Image, ReadExifInfo) {
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  if (!stream.is_open()) {
    GTEST_SKIP() << "Test image not found";
  }
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  kingfisher::kcv::ExifInfo exifInfo;
  int ret = kingfisher::kcv::Image::ReadExifInfo(content, exifInfo);
  EXPECT_EQ(ret, 0);
  EXPECT_GT(exifInfo.image_width(), 0);
  EXPECT_GT(exifInfo.image_height(), 0);
}

TEST_F(test_Image, ReadExifInfo_EmptyData) {
  std::string emptyData;
  kingfisher::kcv::ExifInfo exifInfo;
  int ret = kingfisher::kcv::Image::ReadExifInfo(emptyData, exifInfo);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, BatchDecodeImages) {
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  if (!stream.is_open()) {
    GTEST_SKIP() << "Test image not found";
  }
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  std::vector<std::string> imageDatas = {content, content, content};
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  std::vector<cv::Mat> outputs;
  int ret = kingfisher::kcv::Image::BatchDecodeImages(imageDatas, opts, outputs);
  EXPECT_EQ(ret, 3);  // 3张全部成功
  EXPECT_EQ(outputs.size(), 3u);
  for (const auto &mat : outputs) {
    EXPECT_FALSE(mat.empty());
  }
}

TEST_F(test_Image, BatchDecodeImages_EmptyList) {
  std::vector<std::string> imageDatas;
  kingfisher::kcv::DecodeOptions opts;
  std::vector<cv::Mat> outputs;
  int ret = kingfisher::kcv::Image::BatchDecodeImages(imageDatas, opts, outputs);
  EXPECT_NE(ret, 0);
}

TEST_F(test_Image, BatchResizeImages) {
  std::vector<cv::Mat> inputs = {
    cv::Mat::ones(200, 300, CV_8UC3) * 100,
    cv::Mat::ones(400, 600, CV_8UC3) * 200,
    cv::Mat::ones(100, 150, CV_8UC3) * 50
  };
  std::vector<cv::Mat> outputs;
  int ret = kingfisher::kcv::Image::BatchResizeImages(inputs, 50, 50, false, outputs);
  EXPECT_EQ(ret, 3);
  EXPECT_EQ(outputs.size(), 3u);
  for (const auto &mat : outputs) {
    EXPECT_EQ(mat.cols, 50);
    EXPECT_EQ(mat.rows, 50);
  }
}

TEST_F(test_Image, MatToImage) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  Magick::Image magickImage;
  int ret = kingfisher::kcv::Image::MatToImage(input, magickImage);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(static_cast<int>(magickImage.columns()), 100);
  EXPECT_EQ(static_cast<int>(magickImage.rows()), 100);
}

TEST_F(test_Image, MatToImage_Grayscale) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC1) * 128;
  Magick::Image magickImage;
  int ret = kingfisher::kcv::Image::MatToImage(input, magickImage);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, MatToImage_BGRA) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC4) * 128;
  Magick::Image magickImage;
  int ret = kingfisher::kcv::Image::MatToImage(input, magickImage);
  EXPECT_EQ(ret, 0);
  EXPECT_TRUE(magickImage.matte());  // 应启用透明通道
}

TEST_F(test_Image, MatToImage_EmptyInput) {
  cv::Mat emptyMat;
  Magick::Image magickImage;
  int ret = kingfisher::kcv::Image::MatToImage(emptyMat, magickImage);
  EXPECT_NE(ret, 0);
}

// ==================== 错误码测试 ====================

TEST_F(test_Image, ErrorCodeToString) {
  EXPECT_STREQ(kingfisher::kcv::ImageErrorToString(kingfisher::kcv::kImageOk), "success");
  EXPECT_STREQ(kingfisher::kcv::ImageErrorToString(kingfisher::kcv::kImageDecodeError), "decode error");
  EXPECT_STREQ(kingfisher::kcv::ImageErrorToString(kingfisher::kcv::kImageEmptyInput), "empty input image");
  EXPECT_STREQ(kingfisher::kcv::ImageErrorToString(kingfisher::kcv::kImageFileNotFound), "file not found");
}

// ==================== 旋转任意角度测试 ====================

TEST_F(test_Image, RotateImage_ArbitraryAngle) {
  cv::Mat input = cv::Mat::ones(200, 300, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::RotateImage(input, 45, output);
  EXPECT_EQ(ret, 0);
  EXPECT_FALSE(output.empty());
  // 45度旋转后图像应变大
  EXPECT_GT(output.cols, input.cols);
  EXPECT_GT(output.rows, input.rows);
}

TEST_F(test_Image, RotateImage_ZeroAngle) {
  cv::Mat input = cv::Mat::ones(100, 100, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::RotateImage(input, 0, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.cols, input.cols);
  EXPECT_EQ(output.rows, input.rows);
}

TEST_F(test_Image, RotateImage_180) {
  cv::Mat input = cv::Mat::ones(100, 200, CV_8UC3) * 128;
  cv::Mat output;
  int ret = kingfisher::kcv::Image::RotateImage(input, 180, output);
  EXPECT_EQ(ret, 0);
  EXPECT_EQ(output.cols, input.cols);
  EXPECT_EQ(output.rows, input.rows);
}

#endif
