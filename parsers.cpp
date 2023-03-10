#include "parsers.h"

#include <type_traits>
#include <iomanip>

#include <cassert>

#define ISTREAM_DEBUG_START \
  assert(is.good()); \
  size_t start = input_pos(is); \
  size_t end = start + data.data_size();

#define ISTREAM_DEBUG_START_EXT \
  assert(is.good()); \
  size_t start = input_pos(is); \
  size_t end = start + data.end_of_record;

#define ISTREAM_DEBUG_END \
  size_t pos = is.tellg(); \
  if(pos < end) \
  { \
    std::cout << uint32_t(end - pos) << " bytes not parsed in type " << uint32_t(data.type) << std::endl; \
    is.seekg(end - pos, std::ios_base::cur); \
  } \
  else { assert(pos == end); }

namespace garmin
{
  // helpers

  static size_t input_pos (std::istream& is) { return size_t(is.tellg()); }
  //static size_t output_pos(std::ostream& os) { return size_t(os.tellp()); }

  void parse_records(std::istream& is, record_header_t& data, ssize_t bytes_remaining)
  {
    static uint32_t depth = 0;
    ++depth;
    std::cout << "lowering to depth: " << std::to_string(depth) << std::endl;
    while(bytes_remaining > 0)
    {
      assert(is.good());
      size_t before = input_pos(is);
      is >> data.child_records.emplace_back();
      bytes_remaining -= input_pos(is) - before;
    }
    --depth;
    std::cout << "raising to depth: " << std::to_string(depth) << std::endl;
  }

  void parse_records(std::istream& is, record_header_t& data)
  {
    if(is.good() &&
       data.ext_data_size() &&
       data.type != Address &&
       data.type != Contact &&
       data.type != AudioFile)
      parse_records(is, data, data.ext_data_size());
  }

  // enums

  template <typename T, std::enable_if_t<std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint8_t>, bool> = true>
  std::istream& operator>>(std::istream& is, T& data)
    {
    assert(is.good());
    return is.get(reinterpret_cast<char&>(data)); }

  template <typename T, std::enable_if_t<std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint8_t>, bool> = true>
  std::ostream& operator<<(std::ostream& os, const T& data)
    { return os.put(reinterpret_cast<const char&>(data)); }

  template <typename T, std::enable_if_t<std::is_enum_v<T> && !std::is_same_v<std::underlying_type_t<T>, uint8_t>, bool> = true>
  std::istream& operator>>(std::istream& is, T& data)
  {
    assert(is.good());
    uint16le_t input;
    is >> input;
    data = static_cast<T>(static_cast<uint16_t>(input));
    return is;
  }

  template <typename T, std::enable_if_t<std::is_enum_v<T> && !std::is_same_v<std::underlying_type_t<T>, uint8_t>, bool> = true>
  std::ostream& operator<<(std::ostream& os, const T& data)
    { return os << uint16le_t(data); }

  // helper types
  std::istream& operator>>(std::istream& is, flags_t& data)
    { return is >> data.byte0 >> data.byte1; }

  std::ostream& operator<<(std::ostream& os, const flags_t& data)
    { return os << data.byte0 << data.byte1; }

  std::istream& operator>>(std::istream& is,  coord_t<24>& data)
  {
    uint32le_t val = 0;
    is >> val[0] >> val[1] >> val[2] >> val[3] >> val[4] >> val[5];
    data = double(val) / 360 * (2 << 24);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const coord_t<24>& data)
  {
    uint32le_t val = double(data) * 360 / (2 << 24);
    return os << val[0] << val[1] << val[2] << val[3] << val[4] << val[5];
  }

  std::istream& operator>>(std::istream& is, coord_t<32>& data)
  {
    assert(is.good());
    uint32le_t tmp = 0;
    is >> tmp;
    data = double(tmp) / 360 * (uint64_t(2) << 32);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const coord_t<32>& data)
  {
    return os << uint32le_t(double(data) * 360 / (uint64_t(2) << 32));
  }

  template<int bits>
  std::istream& operator>>(std::istream& is, coord_pair_t<bits>& data)
    { return is >> data.longitude >> data.longitude; }

  template<int bits>
  std::ostream& operator<<(std::ostream& os, const coord_pair_t<bits>& data)
    { return os << data.longitude << data.longitude; }


  std::istream& operator>>(std::istream& is, timestamp_t& data)
  {
    uint32le_t input = 0;
    is >> input;
    if(input == 0xFFFFFFFF)
      input = 0;

    data = timestamp_t(std::chrono::duration<uint64_t>(input + unix_time_offset));
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const timestamp_t& data)
  {
    return os << uint32le_t(data.time_since_epoch().count() - unix_time_offset);
  }


  // record header helpers

  std::istream& operator>>(std::istream& is, record_header_t& data)
  {
    assert(is.good());
    if(data.end_of_record == UINT32_MAX) // if record header isn't initialized
    {
      data.end_of_data.reset();

      is >> data.type
         >> data.header_flags
         >> data.end_of_record;

      if(data.header_flags.bit3)
        is >> data.end_of_data;

      if(is.good())
      {
        std::cout << "data:" << std::endl
                  << "  type: " << std::to_string(uint16_t(data.type)) << std::endl
                  << "  header_flags: " << std::setw(2) << uint16_t(data.header_flags.byte0)
                                        << std::setw(2) << uint16_t(data.header_flags.byte1) << std::endl
                  << "  data size: " << data.data_size() << std::endl
                  << "  ext data size: " << data.ext_data_size() << std::endl;
      }

    }
    return is;
  }


  std::ostream& operator<<(std::ostream& os, const record_header_t& data)
  {
    if(data.child_records.size())
    {
      data.end_of_data = data.end_of_record;
      data.end_of_record += data.children_size();
    }

    data.header_flags.bit3 = data.end_of_data.has_value();

    return os << data.type
              << data.header_flags
              << data.end_of_record
              << data.end_of_data;
  }

  // record io
  std::istream& operator>>(std::istream& is, garmin_header_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.magic[0] >> data.magic[1] >> data.magic[2] >> data.magic[3] >> data.magic[4] >> data.magic[5]
       >> data.version[0] >> data.version[1]
       >> data.timestamp
       >> data.flags
       >> data.name;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const garmin_header_t& data)
  {
    data.end_of_data = data.calc_data_size();
    os << data.header()
      << data.magic[0] << data.magic[1] << data.magic[2] << data.magic[3] << data.magic[4] << data.magic[5]
      << data.version[0] << data.version[1]
      << data.timestamp
      << data.flags
      << data.name;
    return os;
  }


  std::istream& operator>>(std::istream& is, poi_header_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
              >> data.magic[0] >> data.magic[1] >> data.magic[2] >> data.magic[3] >> data.magic[4] >> data.magic[5]
              >> data.version[0] >> data.version[1]
              >> data.codepage
              >> data.extended_type;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const poi_header_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.magic[0] << data.magic[1] << data.magic[2] << data.magic[3] << data.magic[4] << data.magic[5]
              << data.version[0] << data.version[1]
              << data.codepage
              << data.extended_type;
  }


  std::istream& operator>>(std::istream& is, waypoint_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
              >> data.coordinates
              >> data.reserved
              >> data.flags
              >> data.shortname;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const waypoint_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.coordinates
              << data.reserved
              << data.flags
              << data.shortname;
  }


  std::istream& operator>>(std::istream& is, alert_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
              >> data.proximity
              >> data.velocity
              >> data.Unknown6
              >> data.Unknown7
              >> data.enabled
              >> data.trigger
              >> data.symbol_id
              >> data.source;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const alert_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.proximity
              << data.velocity
              << data.Unknown6
              << data.Unknown7
              << uint8_t(data.enabled)
              << data.trigger
              << data.symbol_id
              << data.source;
  }


  std::istream& operator>>(std::istream& is, bitmap_reference_t& data)
  {
    ISTREAM_DEBUG_START

    data.Unknown8.reset();
    is >> data.header()
       >> data.bitmap_id;

    if(data.data_size() >= 4)
      is >> data.Unknown8.value();

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const bitmap_reference_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.bitmap_id
              << data.Unknown8;
  }


  std::istream& operator>>(std::istream& is, bitmap_t& data)
  {
    ISTREAM_DEBUG_START

    uint32le_t image_byte_length = 0;
    uint32le_t palette_size = 0;

    is >> data.header()
       >> data.bitmap_id
       >> data.height
       >> data.width
       >> data.line_length
       >> data.bits_per_pixel
       >> data.reserved0
       >> image_byte_length
       >> data.image_offset
       >> palette_size
       >> data.transparent_color
       >> data.reserved1
       >> data.flags
       >> data.palette_offset;

    data.image_data.resize(image_byte_length);
    data.palette_data.resize(palette_size);

    is.read(reinterpret_cast<char*>(data.image_data.data()), image_byte_length);

    if(palette_size)
      is.read(reinterpret_cast<char*>(data.palette_data.data()), palette_size * sizeof(uint32_t));

//  if(data.flags.bit0)

    uint32_t mask_size = data.data_size() -
                         data.statics_size() -
                         image_byte_length -
                         (palette_size * sizeof(uint32_t));
    data.mask_data.resize(mask_size);
    is.read(reinterpret_cast<char*>(data.mask_data.data()), mask_size);

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const bitmap_t& data)
  {
    uint32le_t image_byte_length = data.image_data.size();
    uint32le_t palette_size = data.palette_data.size();

    data.end_of_data = data.calc_data_size();

    data.flags.bit0 = !data.mask_data.empty();

    os << data.header()
       << data.bitmap_id
       << data.height
       << data.width
       << data.line_length
       << data.bits_per_pixel
       << data.reserved0
       << image_byte_length
       << data.image_offset
       << palette_size
       << data.transparent_color
       << data.reserved1
       << data.flags
       << data.palette_offset;

    if(image_byte_length)
      os.write(reinterpret_cast<const char*>(data.image_data.data()), image_byte_length);

    if(palette_size)
      os.write(reinterpret_cast<const char*>(data.palette_data.data()), palette_size * sizeof(uint32_t));

    if(data.flags.bit0)
      os.write(reinterpret_cast<const char*>(data.mask_data.data()), data.mask_data.size());

    return os;
  }

  std::istream& operator>>(std::istream& is, category_reference_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.category_id;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const category_reference_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.category_id;
  }

  std::istream& operator>>(std::istream& is, category_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
              >> data.category_id
              >> data.name;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const category_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.category_id
              << data.name;
  }

  std::istream& operator>>(std::istream& is, area_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
              >> data.coordinates_max
              >> data.coordinates_min
              >> data.reserved
              >> data.flags
              >> data.unknown;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const area_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.coordinates_max
              << data.coordinates_min
              << data.reserved
              << data.flags
              << data.unknown;
  }


  std::istream& operator>>(std::istream& is, poi_group_t& data)
  {
    ISTREAM_DEBUG_START

    data.areas.clear();
    is >> data.header()
       >> data.source;

    parse_records(is, data, data.data_size() - data.source.byte_count());
    for(auto& area : data.child_records)
      data.areas.emplace_back(std::get<area_t>(area));
    data.child_records.clear();

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const poi_group_t& data)
  {
    data.end_of_data = data.calc_data_size();

    os << data.header()
       << data.source;

    for(auto& area : data.areas)
      os << area;

    return os;
  }

  std::istream& operator>>(std::istream& is, comment_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.text;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const comment_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.text;
  }

  std::istream& operator>>(std::istream& is, address_t& data)
  {
    ISTREAM_DEBUG_START

    data.city.reset();
    data.country.reset();
    data.state.reset();
    data.postal_code.reset();
    data.street_name.reset();
    data.building_id.reset();

    is >> data.header()
       >> data.header_flags
       >> data.have.byte0 >> data.have.byte1;

    if(data.have.city)
      is >> data.city;
    if(data.have.country)
      is >> data.country;
    if(data.have.state)
      is >> data.state;
    if(data.have.postal_code)
      is >> data.postal_code;
    if(data.have.street_name)
      is >> data.street_name;
    if(data.have.building_id)
      is >> data.building_id;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const address_t& data)
  {
    data.end_of_data = data.calc_data_size();

    data.have.city        = data.city.has_value();
    data.have.country     = data.country.has_value();
    data.have.state       = data.state.has_value();
    data.have.postal_code = data.postal_code.has_value();
    data.have.street_name = data.street_name.has_value();
    data.have.building_id = data.building_id.has_value();

    os << data.header()
       << data.header_flags
       << data.have.byte0 << data.have.byte1
       << data.city
       << data.country
       << data.state
       << data.postal_code
       << data.street_name
       << data.building_id;

    return os;
  }

  std::istream& operator>>(std::istream& is, contact_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.header_flags
       >> data.have.byte0 >> data.have.byte1;

    data.phone1.reset();
    data.phone2.reset();
    data.fax.reset();
    data.email.reset();
    data.URL.reset();

    if(data.have.phone1)
      is >> data.phone1;
    if(data.have.phone2)
      is >> data.phone2;
    if(data.have.fax)
      is >> data.fax;
    if(data.have.email)
      is >> data.email;
    if(data.have.URL)
      is >> data.URL;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const contact_t& data)
  {
    data.end_of_data = data.calc_data_size();

    data.have.phone1 = data.phone1.has_value();
    data.have.phone2 = data.phone2.has_value();
    data.have.fax    = data.fax.has_value();
    data.have.email  = data.email.has_value();
    data.have.URL    = data.URL.has_value();

    return os << data.header()
       << data.have.byte0 << data.have.byte1
       << data.phone1
       << data.phone2
       << data.fax
       << data.email
       << data.URL;
  }

  std::istream& operator>>(std::istream& is, image_file_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.unknown
       >> data.image_data;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const image_file_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.unknown
              << data.image_data;
  }

  std::istream& operator>>(std::istream& is, description_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.unknown
       >> data.text;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const description_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.unknown
              << data.text;
  }

  std::istream& operator>>(std::istream& is, audio_file_t& data)
  {
    ISTREAM_DEBUG_START_EXT

    is >> data.header()
       >> data.audio_id
       >> data.format;

    if(data.end_of_record)
      is >> data.audio_data;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const audio_file_t& data)
  {
    data.end_of_data = data.calc_data_size();
    data.end_of_record = data.audio_data.byte_count();
    return os << data.header()
              << data.audio_id
              << data.format
              << data.audio_data;
  }


  std::istream& operator>>(std::istream& is, record15_t& data)
  {
    ISTREAM_DEBUG_START

    data.unknown.reset();
    is >> data.header()
       >> data.map_id
       >> data.product_id
       >> data.region_id
       >> data.vendor_id;
    if(data.data_size() > data.statics_size())
      is >> data.unknown;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record15_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.map_id
              << data.product_id
              << data.region_id
              << data.vendor_id
              << data.unknown;
  }

  std::istream& operator>>(std::istream& is, record16_t& data)
  {
    ISTREAM_DEBUG_START

    is >> data.header()
       >> data.points;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record16_t& data)
  {
    data.end_of_data = data.calc_data_size();
    return os << data.header()
              << data.points;
  }

  std::istream& operator>>(std::istream& is, copyright_t& data)
  {
    ISTREAM_DEBUG_START

    data.Unknown30.reset();
    is >> data.header()
       >> data.have.byte0 >> data.have.byte1 >> data.have.byte2 >> data.have.byte3
       >> data.unknown0
       >> data.unknown1
       >> data.data_source
       >> data.copyright_notice;

    if(data.have.device_model)
      is >> data.device_model;
    if(data.have.image_files)
    {
//      is >> data.image_files;
    }
    if(data.have.Unknown30)
      is >> data.Unknown30;

    ISTREAM_DEBUG_END
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const copyright_t& data)
  {
    data.end_of_data = data.calc_data_size();
    data.have.device_model = data.device_model.has_value();
    data.have.image_files = 0;//data.image_files.has_value();
    data.have.Unknown30 = data.Unknown30.has_value();
    os << data.header()
       << data.have.byte0 << data.have.byte1 << data.have.byte2 << data.have.byte3
       << data.unknown0
       << data.unknown1
       << data.data_source
       << data.copyright_notice
       << data.device_model
//       << data.image_files
       << data.Unknown30;

    return os;
  }

  std::istream& operator>>(std::istream& is, index_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const index_t& data)
  {
    (void)data;
    return os;
  }


  std::istream& operator>>(std::istream& is, record20_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record20_t& data)
  {
    (void)data;
    return os;
  }

  std::istream& operator>>(std::istream& is, record22_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record22_t& data)
  {
    (void)data;
    return os;
  }

  std::istream& operator>>(std::istream& is, record23_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record23_t& data)
  {
    (void)data;
    return os;
  }

  std::istream& operator>>(std::istream& is, record24_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record24_t& data)
  {
    (void)data;
    return os;
  }

  std::istream& operator>>(std::istream& is, record25_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record25_t& data)
  {
    (void)data;
    return os;
  }

  std::istream& operator>>(std::istream& is, record26_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record26_t& data)
  {
    (void)data;
    return os;
  }

  std::istream& operator>>(std::istream& is, record27_t& data)
  {
    assert(is.good());
    is.seekg(data.end_of_record, std::ios_base::cur);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record27_t& data)
  {
    (void)data;
    return os;
  }


  std::istream& operator>>(std::istream& is, any_record_t& data)
  {
    record_header_t record_header;
    if((is >> record_header).good())
    {
      std::cout << "header pos: " << std::setw(4) << uint32_t(uint32_t(is.tellg()) - record_header.header_size()) << std::endl;
      std::cout << "record pos: " << std::setw(4) << uint32_t(is.tellg()) << std::endl;

      switch(record_header.type)
      {
        case GarminHeader:      is >> data.emplace<garmin_header_t      >(record_header); break;
        case POIHeader:         is >> data.emplace<poi_header_t         >(record_header); break;
        case Waypoint:          is >> data.emplace<waypoint_t           >(record_header); break;
        case Alert:             is >> data.emplace<alert_t              >(record_header); break;
        case BitmapReference:   is >> data.emplace<bitmap_reference_t   >(record_header); break;
        case Bitmap:            is >> data.emplace<bitmap_t             >(record_header); break;
        case CategoryReference: is >> data.emplace<category_reference_t >(record_header); break;
        case Category:          is >> data.emplace<category_t           >(record_header); break;
        case Area:              is >> data.emplace<area_t               >(record_header); break;
        case POIGroup:          is >> data.emplace<poi_group_t          >(record_header); break;
        case Comment:           is >> data.emplace<comment_t            >(record_header); break;
        case Address:           is >> data.emplace<address_t            >(record_header); break;
        case Contact:           is >> data.emplace<contact_t            >(record_header); break;
        case ImageFile:         is >> data.emplace<image_file_t         >(record_header); break;
        case Description:       is >> data.emplace<description_t        >(record_header); break;
        case Record15:          is >> data.emplace<record15_t           >(record_header); break;
        case Record16:          is >> data.emplace<record16_t           >(record_header); break;
        case Copyright:         is >> data.emplace<copyright_t          >(record_header); break;
        case AudioFile:         is >> data.emplace<audio_file_t         >(record_header); break;
        case SpeedCamera:       is >> data.emplace<speed_camera_t       >(record_header); break;
        case Record20:          is >> data.emplace<record20_t           >(record_header); break;
        case Index:             is >> data.emplace<index_t              >(record_header); break;
        case Record22:          is >> data.emplace<record22_t           >(record_header); break;
        case Record23:          is >> data.emplace<record23_t           >(record_header); break;
        case Record24:          is >> data.emplace<record24_t           >(record_header); break;
        case Record25:          is >> data.emplace<record25_t           >(record_header); break;
        case Record26:          is >> data.emplace<record26_t           >(record_header); break;
        case Record27:          is >> data.emplace<record27_t           >(record_header); break;
        default:
          break;
      }
      std::cout << "end pos: " << std::setw(4) << is.tellg() << std::endl;
      parse_records(is, record_header);
    }
    return is;
  } // end function

  std::ostream& operator<<(std::ostream& os, const any_record_t& data)
  {
    switch(data.index())
    {
      case GarminHeader:      os << std::get<garmin_header_t      >(data); break;
      case POIHeader:         os << std::get<poi_header_t         >(data); break;
      case Waypoint:          os << std::get<waypoint_t           >(data); break;
      case Alert:             os << std::get<alert_t              >(data); break;
      case BitmapReference:   os << std::get<bitmap_reference_t   >(data); break;
      case Bitmap:            os << std::get<bitmap_t             >(data); break;
      case CategoryReference: os << std::get<category_reference_t >(data); break;
      case Category:          os << std::get<category_t           >(data); break;
      case Area:              os << std::get<area_t               >(data); break;
      case POIGroup:          os << std::get<poi_group_t          >(data); break;
      case Comment:           os << std::get<comment_t            >(data); break;
      case Address:           os << std::get<address_t            >(data); break;
      case Contact:           os << std::get<contact_t            >(data); break;
      case ImageFile:         os << std::get<image_file_t         >(data); break;
      case Description:       os << std::get<description_t        >(data); break;
      case Record15:          os << std::get<record15_t           >(data); break;
      case Record16:          os << std::get<record16_t           >(data); break;
      case Copyright:         os << std::get<copyright_t          >(data); break;
      case AudioFile:         os << std::get<audio_file_t         >(data); break;
      case SpeedCamera:       os << std::get<speed_camera_t       >(data); break;
      case Record20:          os << std::get<record20_t           >(data); break;
      case Index:             os << std::get<index_t              >(data); break;
      case Record22:          os << std::get<record22_t           >(data); break;
      case Record23:          os << std::get<record23_t           >(data); break;
      case Record24:          os << std::get<record24_t           >(data); break;
      case Record25:          os << std::get<record25_t           >(data); break;
      case Record26:          os << std::get<record26_t           >(data); break;
      case Record27:          os << std::get<record27_t           >(data); break;
      default:
        break;
    }
    return os;
  }
}
