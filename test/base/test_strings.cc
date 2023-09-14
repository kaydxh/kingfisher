#include <gtest/gtest.h>

#include <iostream>

#include "strings/strings.h"

using namespace kingfisher;
using namespace kingfisher::strings;

class test_Strings : public testing::Test {
 public:
  test_Strings() {}
  ~test_Strings() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

TEST_F(test_Strings, IsNumber) {
  struct TestCases {
    std::string str;
    bool expected;
  };

  std::vector<TestCases> testCases = {{"123", true}, {"12a", false}};

  for (auto testCase : testCases) {
    auto isNum = IsNumber(testCase.str);
    std::cout << "isNum: " << isNum << std::endl;
    ASSERT_TRUE(isNum == testCase.expected);
  }
}

TEST_F(test_Strings, ParseInt64) {
  struct TestCases {
    std::string str;
    int64_t expected;
  };

  std::vector<TestCases> testCases = {{"123", 123}, {"-123", -123}};

  for (auto testCase : testCases) {
    int64_t result;
    auto ret = ParseInt64(result, testCase.str);
    ASSERT_TRUE(ret == 0);
    std::cout << "result of ParseInt64: " << result << std::endl;
    ASSERT_TRUE(result == testCase.expected);
  }
}

TEST_F(test_Strings, HasPrefix) {
  struct TestCases {
    std::string str;
    std::string prefix;
    bool caseSenstitive;
    bool expected;
  };

  std::vector<TestCases> testCases = {{"abcde", "ab", true, true},
                                      {"abce", "abced", true, false},
                                      {"abce", "AB", true, false},
                                      {"abce", "AB", false, true}};

  for (auto testCase : testCases) {
    auto has =
        HasPrefix(testCase.str, testCase.prefix, testCase.caseSenstitive);
    ASSERT_TRUE(has == testCase.expected);
    std::cout << "result of HasPrefix: " << has << std::endl;
    ASSERT_TRUE(has == testCase.expected);
  }
}

TEST_F(test_Strings, TrimLeft) {
  struct TestCases {
    std::string str;
    std::string prefix;
    std::string expected;
  };

  std::vector<TestCases> testCases = {{"aabbcc", "aa", "bbcc"},
                                      {"aabbcc", "a", "bbcc"}};

  for (auto testCase : testCases) {
    auto result = TrimLeft(testCase.str, testCase.prefix);
    std::cout << "result of TrimLeft: " << result << std::endl;
    ASSERT_TRUE(result == testCase.expected);
  }
}

TEST_F(test_Strings, TrimRight) {
  struct TestCases {
    std::string str;
    std::string suffix;
    std::string expected;
  };

  std::vector<TestCases> testCases = {{"aabbcc", "cc", "aabb"},
                                      {"aabbcc", "c", "aabb"}};

  for (auto testCase : testCases) {
    auto result = TrimRight(testCase.str, testCase.suffix);
    std::cout << "result of TrimRight: " << result << std::endl;
    ASSERT_TRUE(result == testCase.expected);
  }
}

TEST_F(test_Strings, Split) {
  std::string filepath = "./tmp/test";
  std::vector<std::string> path_nodes = Split(filepath, '/');

  std::cout << "path nodes: " << std::endl;
  for (auto &node : path_nodes) {
    std::cout << node << std::endl;
  }
}

TEST_F(test_Strings, SplitToInt64) {
  std::string filepath = "10,20,30,40";
  std::vector<int64_t> values = SplitToInt64(filepath, ',');

  std::cout << "values: " << std::endl;
  for (auto &v : values) {
    std::cout << v << std::endl;
  }
}

TEST_F(test_Strings, ToNumbers) {
  std::string str(
      "\x00\x00\x80\x3F\x00\x00\xC0\x40\x00\x00\x00\x00\x00\x00\xF0\x41", 16);
  std::vector<float> values = ToNumbers<float>(str);

  std::cout << "values: " << std::endl;
  for (auto &v : values) {
    std::cout << v << " ";
  }
  std::cout << endl;
}
