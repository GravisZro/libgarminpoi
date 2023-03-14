#include "endian_types.h"


std::istream& operator>>(std::istream& is,       alien_endian_t<uint16_t>& data) { return is >> data[0] >> data[1]; }
std::ostream& operator<<(std::ostream& os, const alien_endian_t<uint16_t>& data) { return os << data[0] << data[1]; }
std::istream& operator>>(std::istream& is,       alien_endian_t<uint32_t>& data) { return is >> data[0] >> data[1] >> data[2] >> data[3]; }
std::ostream& operator<<(std::ostream& os, const alien_endian_t<uint32_t>& data) { return os << data[0] << data[1] << data[2] << data[3]; }
std::istream& operator>>(std::istream& is,       alien_endian_t<uint64_t>& data) { return is >> data[0] >> data[1] >> data[2] >> data[3] >> data[4] >> data[5] >> data[6] >> data[7]; }
std::ostream& operator<<(std::ostream& os, const alien_endian_t<uint64_t>& data) { return os << data[0] << data[1] << data[2] << data[3] << data[4] << data[5] << data[6] << data[7]; }
