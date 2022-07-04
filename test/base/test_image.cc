#include <gtest/gtest.h>

#ifdef ENABLE_GRAPHICS_MAGICK
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "core/scope_guard.h"
#include "cv/image.h"

using namespace kingfisher;
using namespace kingfisher::kcv;

//./kingfisher_base_test  --gtest_filter=test_Image.*
class test_Image : public testing::Test {
 public:
  test_Image() {}
  ~test_Image() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Image, DecodeImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::DecodeImage(
      content, kingfisher::kcv::BGRColorSpace, true, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.BGRColorSpace.png", mat);
}

TEST_F(test_Image, RotateImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::RotateImage(content, -90, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.rotate.png", mat);
}

TEST_F(test_Image, ResizeImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::ResizeImage(content, 100, 30, true, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.resize.png", mat);
}

TEST_F(test_Image, CropImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::CropImage(
      content, kingfisher::kcv::Rect{10, 10, 1000, 1000}, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.crop.png", mat);
}

//../output/bin/kingfisher_base_test --gtest_filter=test_Image.AnnotateImage
TEST_F(test_Image, AnnotateImage) {
  kingfisher::kcv::Image::GlobalInit();
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::kcv::Image::AnnotateImage(
      content, "1", kingfisher::kcv::Rect{10, 10, 10, 10}, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.annotate.png", mat);
}

#endif
