#ifndef ENDIAN_TYPES_H
#define ENDIAN_TYPES_H

#include <iostream>

template<typename T>
struct endian_ops_t
{
  endian_ops_t(T d = 0) : underlying_data(d) {}
  T underlying_data;

  operator T&(void) { return underlying_data; }
  operator const T&(void) const { return underlying_data; }
};


template<typename T>
struct native_endian_t : endian_ops_t<T>
{
  using endian_ops_t<T>::endian_ops_t;

  const uint8_t& operator [](const size_t pos) const
    { return reinterpret_cast<const uint8_t*>(&endian_ops_t<T>::operator const T&())[pos]; }

  uint8_t& operator [](const size_t pos)
    { return reinterpret_cast<uint8_t*>(&endian_ops_t<T>::operator T&())[pos]; }

  friend std::istream& operator>>(std::istream& is, native_endian_t<T>& data)
    { return is.read(reinterpret_cast<char*>(&data.operator T&()), sizeof(T)); }

  friend std::ostream& operator<<(std::ostream& os, const native_endian_t<T>& data)
    { return os.write(reinterpret_cast<const char*>(&data.operator const T&()), sizeof(T)); }
};


template<typename T>
struct alien_endian_t : endian_ops_t<T>
{
  using endian_ops_t<T>::endian_ops_t;

  const uint8_t& operator [](const size_t pos) const
    { return reinterpret_cast<const uint8_t*>(&endian_ops_t<T>::operator const T&())[sizeof(T) - 1 - pos]; }

  uint8_t& operator [](const size_t pos)
    { return reinterpret_cast<uint8_t*>(&endian_ops_t<T>::operator T&())[sizeof(T) - 1 - pos]; }

  template<typename U> friend std::istream& operator>>(std::istream& is,       alien_endian_t<U>& data);
  template<typename U> friend std::ostream& operator<<(std::ostream& os, const alien_endian_t<U>& data);
};

extern std::istream& operator>>(std::istream& is,       alien_endian_t<uint16_t>& data);
extern std::ostream& operator<<(std::ostream& os, const alien_endian_t<uint16_t>& data);
extern std::istream& operator>>(std::istream& is,       alien_endian_t<uint32_t>& data);
extern std::ostream& operator<<(std::ostream& os, const alien_endian_t<uint32_t>& data);
extern std::istream& operator>>(std::istream& is,       alien_endian_t<uint64_t>& data);
extern std::ostream& operator<<(std::ostream& os, const alien_endian_t<uint64_t>& data);

#ifndef __BYTE_ORDER__
#error compiler does not define endianness macros
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  template<typename T> using big_endian_t = alien_endian_t<T>;
  template<typename T> using little_endian_t = native_endian_t<T>;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  template<typename T> using big_endian_t = native_endian_t<T>;
  template<typename T> using little_endian_t = alien_endian_t<T>;
#else
# error are you compiling for a PDP?!
#endif

using uint16be_t  = big_endian_t<uint16_t>;
using uint32be_t  = big_endian_t<uint32_t>;
using uint64be_t  = big_endian_t<uint64_t>;

using int16be_t   = big_endian_t<int16_t>;
using int32be_t   = big_endian_t<int32_t>;
using int64be_t   = big_endian_t<int64_t>;

using uint16le_t  = little_endian_t<uint16_t>;
using uint32le_t  = little_endian_t<uint32_t>;
using uint64le_t  = little_endian_t<uint64_t>;

using int16le_t   = little_endian_t<int16_t>;
using int32le_t   = little_endian_t<int32_t>;
using int64le_t   = little_endian_t<int64_t>;

#endif // ENDIAN_TYPES_H
