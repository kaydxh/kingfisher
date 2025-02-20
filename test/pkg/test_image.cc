#include <gtest/gtest.h>

#ifdef ENABLE_GRAPHICS_MAGICK
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "core/scope_guard.h"
#include "cv/image/image.h"

using namespace kingfisher;
using namespace kingfisher::kcv;

//./kingfisher_base_test --gtest_filter=test_Image.*
//./kingfisher_base_test --gtest_filter=test_Image.DecodeImage
class test_Image : public testing::Test {
 public:
  test_Image() {}
  ~test_Image() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Image, DecodeImageFile) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";

  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImageFile(imageFile, opts, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.BGRColorSpace.png", mat);
}

TEST_F(test_Image, DecodeImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImage(content, opts, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.BGRColorSpace.png", mat);
}

TEST_F(test_Image, RotateImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::RotateImage(content, -45, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.rotate.png", mat);
}

TEST_F(test_Image, RotateImageMat) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";

  ::cv::Mat mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImageFile(imageFile, opts, mat);
  EXPECT_EQ(ret, 0);

  ::cv::Mat rotated_mat;
  ret = kingfisher::kcv::Image::RotateImage(mat, 90, rotated_mat);
  ::cv::imwrite("./output.rotate.png", rotated_mat);
}

TEST_F(test_Image, ResizeImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/kay2.png";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::ResizeImage(content, 520, 765, false, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.resize.jpg", mat);
}

TEST_F(test_Image, CropImage) {
  kingfisher::kcv::Image::GlobalInit();
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
  ::cv::imwrite("./output.crop.png", mat);
}

TEST_F(test_Image, DumpImageFileToBytes) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::string imageBytes = "./testdata/1_bytes.txt";

  int ret = kingfisher::kcv::Image::DumpImageFileToBytes(imageFile, imageBytes);
  EXPECT_EQ(ret, 0);
}

TEST_F(test_Image, AdaptiveWatermarkFill) {
  kingfisher::kcv::Image::GlobalInit();
  std::string image = "/data/home/kayxhding/workspace/github.com/kaydxh/kingfisher/test/testdata/1.jpg";
  std::string logo = "/data/home/kayxhding/workspace/github.com/kaydxh/kingfisher/test/testdata/1.jpg";
  cv::Mat image_mat = cv::imread(image, cv::IMREAD_UNCHANGED);
  cv::Mat logo_mat = cv::imread(logo, cv::IMREAD_UNCHANGED);

#if 0
  ::cv::Mat image_mat;
  kingfisher::kcv::DecodeOptions opts;
  opts.set_targetcolorspace(kingfisher::kcv::BGRColorSpace);
  opts.set_auto_orient(true);
  int ret = kingfisher::kcv::Image::DecodeImageFile(image, opts, image_mat);
  EXPECT_EQ(ret, 0);
  ::cv::Mat logo_mat;
  ret = kingfisher::kcv::Image::DecodeImageFile(logo, opts, logo_mat);
  EXPECT_EQ(ret, 0);
#endif

  // 在(100,50)位置创建200x150区域（允许负坐标和越界）
  kingfisher::kcv::Image::AdaptiveWatermarkFill(image_mat, logo_mat, cv::Rect(100, 50, 200, 150),
                       cv::INTER_LANCZOS4, 0.7);
  //EXPECT_EQ(ret, 0);
  ::cv::imwrite("/data/home/kayxhding/workspace/github.com/kaydxh/kingfisher/test/testdata/output.mark.png", image_mat);
}

#if 0
TEST_F(test_Image, WriteImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/test1.jpg";
  kingfisher::kcv::Image::DecodeImage(
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
  ::cv::imwrite("./output.crop.png", mat);
}
#endif

//../output/bin/kingfisher_base_test --gtest_filter=test_Image.AnnotateImage
#if 0
TEST_F(test_Image, AnnotateImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::AnnotateImage(
      content, "hello word", kingfisher::kcv::Point{100, 100}, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.annotate.png", mat);
}
#endif

#endif
