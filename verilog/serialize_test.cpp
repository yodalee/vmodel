// Direct include
#include "serialize.h"
// C system headers
// C++ standard library headers
#include <array>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
// Other libraries' .h files.
#include <gtest/gtest.h>
// Your project's .h files.
#include "verilog/dtype.h"
using namespace std;
using namespace verilog;
// typedef varray<vuint<10>, 5> Arr1;
// typedef varray<vuint<3>, 2, 3> Arr2;

TEST(TestFromBytes, basic) {
  uint8_t arr[16];
  vuint<16 * 8> varr, golden;
  for (uint16_t i = 0; i < 16; i++) {
    arr[i] = i;
  }

  // Complete copy
  FromByteArray(varr, arr, 16);
  from_hex(golden, "0x000102030405060708090A0B0C0D0E0F");
  EXPECT_EQ(varr, golden);

  // Accummulation
  varr = 0;  // reset
  FromByteArray(varr, arr, 3);
  from_hex(golden, "0x000102");
  EXPECT_EQ(varr, golden);
  FromByteArray(varr, arr + 3, 9);
  from_hex(golden, "0x000102030405060708090A0B");
  EXPECT_EQ(varr, golden);
  FromByteArray(varr, arr + 12, 4);
  from_hex(golden, "0x000102030405060708090A0B0C0D0E0F");
  EXPECT_EQ(varr, golden);
}

TEST(TestToBytes, basic) {
  uint8_t arr[16];
  vuint<16 * 8> varr;
  from_hex(varr, "0x000102030405060708090A0B0C0D0E0F");

  // Complete copy
  ToByteArray(arr, 16, varr);
  for (uint16_t i = 0; i < 16; i++) {
    EXPECT_EQ(arr[i], i);
  }

  // Partial copy
  memset(arr, 0, sizeof arr);  // reset
  ToByteArray(arr, 3, varr);
  EXPECT_EQ(arr[0], 0x0D);
  EXPECT_EQ(arr[1], 0x0E);
  EXPECT_EQ(arr[2], 0x0F);

  memset(arr, 0, sizeof arr);  // reset
  ToByteArray(arr, 6, varr);
  EXPECT_EQ(arr[0], 0x0A);
  EXPECT_EQ(arr[1], 0x0B);
  EXPECT_EQ(arr[2], 0x0C);
  EXPECT_EQ(arr[3], 0x0D);
  EXPECT_EQ(arr[4], 0x0E);
  EXPECT_EQ(arr[5], 0x0F);
}
