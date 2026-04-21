#pragma once
#include <iostream>

#include "dtype.h"

namespace verilog {

template <class T>
::std::ostream &SaveContent(::std::ostream &os, const T &rhs) {
  static_assert(::std::is_trivially_copyable_v<T>);
  os.write(reinterpret_cast<const char *>(&rhs), sizeof(T));
  return os;
};

template <class T>
::std::istream &LoadContent(::std::istream &ist, T &rhs) {
  static_assert(::std::is_trivially_copyable_v<T>);
  ist.read(reinterpret_cast<char *>(&rhs), sizeof(T));
  return ist;
};

template <class T>
::std::ostream &SaveHexString(::std::ostream &os, const T &rhs) {
  static_assert(verilog::is_dtype_v<T>);
  os << to_hex(pack(rhs));
  return os;
}

template <class T>
::std::istream &LoadHexString(::std::istream &ist, T &rhs) {
  static_assert(verilog::is_dtype_v<T>);
  verilog::vuint<verilog::bits<T>()> buf;
  ::std::string line;
  ::std::getline(ist, line);
  from_string(buf, line, 16);
  verilog::unpack(rhs, buf);
  return ist;
}

template <unsigned num_bit_>
void ToByteArray(uint8_t *buf, const size_t len,
                 verilog::vint<false, num_bit_> v) {
  for (size_t i = len; i > 0; --i) {
    buf[i - 1] = v.value() & 0xff;
    v >>= 8;
  }
}

template <unsigned num_bit_>
void FromByteArray(verilog::vint<false, num_bit_> &v, const uint8_t *const buf,
                   const size_t &len) {
  for (size_t i = 0; i < len; i++) {
    v <<= 8;
    auto b = static_cast<verilog::vuint<8>>(buf[i]);
    v.template SetSlice<0, 8>(b);
  }
}

}  // namespace verilog