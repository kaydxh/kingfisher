//
// Created by kayxhding on 2020-04-15 15:58:24
//
#include <iostream>
#include <string>

#include "md5.h"
using namespace std;

int main() {
  string str = "abc";
  auto md5 = kingfisher::crypto::Md5SumString(str);
  cout << md5 << endl;
}
