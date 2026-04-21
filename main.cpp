#include "problem/src.hpp"
#include <bits/stdc++.h>
using namespace std;
using namespace sjtu;

// The OJ harness expects an executable named `code`.
// There is no strict I/O spec here; we implement a simple self-check
// to exercise the any_ptr features.
int main() {
  // Basic checks similar to provided examples
  any_ptr a = make_any_ptr(1);
  any_ptr b = a;
  a.unwrap<int>() = 2;
  if (b.unwrap<int>() != 2) return 1;

  b = new std::string;
  b.unwrap<std::string>() = "Hello, world!";
  if (b.unwrap<std::string>() != std::string("Hello, world!")) return 2;

  any_ptr v = make_any_ptr<std::vector<int>>(1, 2, 3);
  if (v.unwrap<std::vector<int>>().size() != 3) return 3;

  // Optional: read stdin passthrough (do nothing) to be safe
  // but ensure a clean exit
  return 0;
}

