#include <gtest/gtest.h>

#ifdef ENABLE_GRAPHICS_MAGICK
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

#include "core/scope_guard.h"
#include "cv/image.h"

using namespace kingfisher;
using namespace kingfisher::cv;

//./kingfisher_base_test  --gtest_filter=test_Sha256.*
class test_Image : public testing::Test {
 public:
  test_Image() {}
  ~test_Image() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Image, DecodeImage) {
  std::string imageFile = "./testdata/1.jpg";
  std::ifstream stream(imageFile, std::ios::in | std::ios::binary);
  std::string content{std::istreambuf_iterator<char>(stream), {}};

  ::cv::Mat mat;
  int ret = kingfisher::cv::Image::DecodeImage(
      content, kingfisher::cv::Image::RGBColorSpace, mat);
  EXPECT_EQ(ret, 0);
  ::cv::imwrite("./output.1.jpg", mat);
}

#endif
