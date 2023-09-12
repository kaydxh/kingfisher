#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

#include "serializer/djson.h"

using namespace kingfisher;
using namespace kingfisher::core;
using namespace serializer_djson;

class test_DJson : public testing::Test {
 public:
  test_DJson() {}
  ~test_DJson() {}

  virtual void SetUp(void) {}

  virtual void TearDown(void) {}
};

struct Test1 {
  int a;
  std::string b;
  double c;
  std::vector<int> e;
  std::map<std::string, std::string> f;
};

TEST(test_DJson, Struct) {
  // init
  std::vector<std::pair<std::string, JsonBase *> > vec;
  vec.push_back(std::make_pair("a", new JsonObjector<Test1, int>(&Test1::a)));
  vec.push_back(
      std::make_pair("b", new JsonObjector<Test1, std::string>(&Test1::b)));
  vec.push_back(
      std::make_pair("c", new JsonObjector<Test1, double>(&Test1::c)));
  vec.push_back(std::make_pair(
      "e", new JsonObjector<Test1, std::vector<int> >(&Test1::e)));
  vec.push_back(std::make_pair(
      "f",
      new JsonObjector<Test1, std::map<std::string, std::string> >(&Test1::f)));

  Singleton<JsonObjsManager>::Instance().put(typeid(Test1).name(), vec);

  // obj2json
  Test1 t1 = {10, "abc", 3.4, {1, 2, 3, 4}, {{"1", "a"}, {"2", "b"}}};

  std::string js = JsonPraser::toJson(t1);

  // print
  std::cout << js << std::endl;

  // json2Obj
  Test1 t2;
  if (JsonPraser::toObj(t2, js)) {
    // print
    EXPECT_EQ(t2.a, 10);
    EXPECT_EQ(t2.b, "abc");
    EXPECT_EQ(t2.c, 3.4);
    std::cout << "a:" << t2.a << " b:" << t2.b << " c:" << t2.c << std::endl;

    EXPECT_TRUE(t1.e.size() == 4);
    std::cout << "e:";
    for (auto &e : t2.e) {
      std::cout << e << " ";
    }
    std::cout << std::endl;

    EXPECT_TRUE(t1.f.size() == 2);
    std::cout << "f:";
    for (auto &e : t2.f) {
      std::cout << e.first << ":" << e.second << " ";
    }
    std::cout << std::endl;
  } else {
    std::cout << "toObj failed." << std::endl;
  }

  // validJson
  // js = "{\"1\"}";
  js = "eyJhYmMiOiAxfQ==";
  bool validJs = JsonPraser::validJson(js);
  std::cout << js << " is valid json: " << validJs << std::endl;
}

struct Test2 {
  std::string id;
  std::string version;
};

TEST(test_DJson, Array) {
  // init
  std::vector<std::pair<std::string, JsonBase *> > vec;
  vec.push_back(
      std::make_pair("id", new JsonObjector<Test2, std::string>(&Test2::id)));
  vec.push_back(std::make_pair(
      "version", new JsonObjector<Test2, std::string>(&Test2::version)));

  Singleton<JsonObjsManager>::Instance().put(typeid(Test2).name(), vec);

  // obj2json
  std::vector<Test2> t2s;
  t2s.push_back({"id_10", "version1.0"});

  std::string js = JsonPraser::toJson(t2s);

  // print
  std::cout << js << std::endl;

  // json2Obj
  std::vector<Test2> t2s_1;
  if (JsonPraser::toObj(t2s_1, js)) {
    // print
    for (auto &e : t2s_1) {
      std::cout << "id: " << e.id << ",version: " << e.version << std::endl;
    }
    std::cout << std::endl;
  }
}
