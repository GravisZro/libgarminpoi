#ifndef PARSERS_H
#define PARSERS_H

#include "record_types.h"
#include <cassert>


namespace garmin
{
  std::istream& operator>>(std::istream& is, any_record_t& data);
  std::ostream& operator<<(std::ostream& os, const any_record_t& data);


  template<typename type>
  std::istream& operator>>(std::istream& is, std::optional<type>& data)
  {
    if(!data)
      data.emplace();
    is >> data.value();
    return is;
  }

  template<typename type>
  std::ostream& operator<<(std::ostream& os, const std::optional<type>& data)
  {
    if(data)
      os << data.value();
    return os;
  }

  template<typename size_type, typename data_type>
  std::istream& operator>>(std::istream& is, vector_t<size_type, data_type>& vector)
  {
    little_endian_t<size_type> length = 0;
    is >> length;
    vector.resize(length);
    is.read(reinterpret_cast<char*>(vector.data()), vector.bytes_held());
    return is;
  }

  template<typename size_type, typename data_type>
  std::ostream& operator<<(std::ostream& os, const vector_t<size_type, data_type>& vector)
  {
    os << little_endian_t<size_type>(vector.size());
    os.write(reinterpret_cast<const char*>(vector.data()), vector.bytes_held());
    return os;
  }

  template<typename localized_type>
  std::istream& operator>>(std::istream& is, localized_t<localized_type>& data)
  {
    data.clear();
    uint32le_t remaining_bytes = 0;
    is >> remaining_bytes;
    while(remaining_bytes > 0)
    {
      char key[2];
      localized_type value;
      is >> key[0] >> key[1] >> value;
      data.insert(std::make_pair((key[0] << 8) | key[1], value));
      remaining_bytes -= sizeof(uint16_t) + value.byte_count();
    }
    return is;
  }

  template<typename localized_type>
  std::ostream& operator<<(std::ostream& os, const localized_t<localized_type>& data)
  {
    uint32_t byte_length = 0;
    for(const auto& pair : data)
      byte_length += sizeof(uint16_t)
                  +  pair.second.size();
    os << uint32le_t(byte_length);
    for(const auto& pair : data)
      os << char(pair.first >> 8) << char(pair.first & 0xFF) << pair.second;
    return os;
  }

  std::istream& operator>>(std::istream& is, record_header_t& data);
  std::ostream& operator<<(std::ostream& os, const record_header_t& data);

  std::istream& operator>>(std::istream& is, garmin_header_t& data);
  std::ostream& operator<<(std::ostream& os, const garmin_header_t& data);

  std::istream& operator>>(std::istream& is, poi_header_t& data);
  std::ostream& operator<<(std::ostream& os, const poi_header_t& data);

  std::istream& operator>>(std::istream& is, point_t& data);
  std::ostream& operator<<(std::ostream& os, const point_t& data);

  std::istream& operator>>(std::istream& is, alert_t& data);
  std::ostream& operator<<(std::ostream& os, const alert_t& data);

  std::istream& operator>>(std::istream& is, bitmap_reference_t& data);
  std::ostream& operator<<(std::ostream& os, const bitmap_reference_t& data);

  std::istream& operator>>(std::istream& is, bitmap_t& data);
  std::ostream& operator<<(std::ostream& os, const bitmap_t& data);

  std::istream& operator>>(std::istream& is, category_reference_t& data);
  std::ostream& operator<<(std::ostream& os, const category_reference_t& data);

  std::istream& operator>>(std::istream& is, category_t& data);
  std::ostream& operator<<(std::ostream& os, const category_t& data);

  std::istream& operator>>(std::istream& is, area_t& data);
  std::ostream& operator<<(std::ostream& os, const area_t& data);

  std::istream& operator>>(std::istream& is, poi_group_t& data);
  std::ostream& operator<<(std::ostream& os, const poi_group_t& data);

  std::istream& operator>>(std::istream& is, comment_t& data);
  std::ostream& operator<<(std::ostream& os, const comment_t& data);

  std::istream& operator>>(std::istream& is, address_t& data);
  std::ostream& operator<<(std::ostream& os, const address_t& data);

  std::istream& operator>>(std::istream& is, contact_t& data);
  std::ostream& operator<<(std::ostream& os, const contact_t& data);

  std::istream& operator>>(std::istream& is, image_file_t& data);
  std::ostream& operator<<(std::ostream& os, const image_file_t& data);

  std::istream& operator>>(std::istream& is, description_t& data);
  std::ostream& operator<<(std::ostream& os, const description_t& data);

  std::istream& operator>>(std::istream& is, audio_file_t& data);
  std::ostream& operator<<(std::ostream& os, const audio_file_t& data);

  std::istream& operator>>(std::istream& is, record15_t& data);
  std::ostream& operator<<(std::ostream& os, const record15_t& data);

  std::istream& operator>>(std::istream& is, record16_t& data);
  std::ostream& operator<<(std::ostream& os, const record16_t& data);

  std::istream& operator>>(std::istream& is, copyright_t& data);
  std::ostream& operator<<(std::ostream& os, const copyright_t& data);

  std::istream& operator>>(std::istream& is, index_t& data);
  std::ostream& operator<<(std::ostream& os, const index_t& data);
} // namespace garmin

#endif // PARSERS_H
