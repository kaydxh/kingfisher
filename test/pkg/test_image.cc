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
  kingfisher::kcv::Rect rect;
  rect.set_x(10);
  rect.set_y(10);
  rect.set_width(100);
  rect.set_height(200);
  int ret = kingfisher::kcv::Image::CropImage(content, rect, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.crop.png", mat);
}

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
