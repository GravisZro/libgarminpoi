#include <data_types.h>

namespace garmin
{
  // typecasted io
  std::istream& operator>>(std::istream& is, record_id_t& data)
    { return is >> reinterpret_cast<uint16_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const record_id_t& data)
    { return os << reinterpret_cast<const uint16_t&>(data); }

  std::istream& operator>>(std::istream& is, region_t& data)
    { return is >> reinterpret_cast<uint16_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const region_t& data)
    { return os << reinterpret_cast<const uint16_t&>(data); }

  std::istream& operator>>(std::istream& is, codepage_t& data)
    { return is >> reinterpret_cast<uint16_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const codepage_t& data)
    { return os << reinterpret_cast<const uint16_t&>(data); }

  std::istream& operator>>(std::istream& is, alert_trigger_t& data)
    { return is >> reinterpret_cast<uint8_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const alert_trigger_t& data)
    { return os << reinterpret_cast<const uint8_t&>(data); }

  std::istream& operator>>(std::istream& is, alert_source_t& data)
    { return is >> reinterpret_cast<uint8_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const alert_source_t& data)
    { return os << reinterpret_cast<const uint8_t&>(data); }

  std::istream& operator>>(std::istream& is, audio_clips_t& data)
    { return is >> reinterpret_cast<uint8_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const audio_clips_t& data)
    { return os << reinterpret_cast<const uint8_t&>(data); }

  std::istream& operator>>(std::istream& is, audio_format_t& data)
    { return is >> reinterpret_cast<uint8_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const audio_format_t& data)
    { return os << reinterpret_cast<const uint8_t&>(data); }

  std::istream& operator>>(std::istream& is, flags_t& data)
    { return is >> reinterpret_cast<uint16_t&>(data); }

  std::ostream& operator<<(std::ostream& os, const flags_t& data)
    { return os << reinterpret_cast<const uint16_t&>(data); }

  // basic type io
  std::istream& operator>>(std::istream& is,  coord_t<24>& data)
  {
    uint32_t tmp = 0;
    is >> tmp;
    data = double(tmp) / 360 * (2 << 24);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const coord_t<24>& data)
  {
    union
    {
      uint8_t val8[4];
      uint32_t val32;
    };
    val32 = double(data) * 360 / (2 << 24);
    return os << val8[0] << val8[1] << val8[2];
  }

  std::istream& operator>>(std::istream& is, coord_t<32>& data)
  {
    uint32_t tmp = 0;
    is >> tmp;
    data = double(tmp) / 360 * (uint64_t(2) << 32);
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const coord_t<32>& data)
  {
    return os << uint32_t(double(data) * 360 / (uint64_t(2) << 32));
  }

  template<int bits>
  std::istream& operator>>(std::istream& is, coord_pair_t<bits>& data)
    { return is >> data.longitude >> data.longitude; }

  template<int bits>
  std::ostream& operator<<(std::ostream& os, const coord_pair_t<bits>& data)
    { return os << data.longitude << data.longitude; }


  template<int sz>
  std::istream& operator>>(std::istream& is, char data[sz])
  {
    for(int i = 0; i < sz; ++i)
      is >> data[i];
    return is;
  }

  template<int sz>
  std::ostream& operator<<(std::ostream& os, const char data[sz])
  {
    for(int i = 0; i < sz; ++i)
      os << data[i];
    return os;
  }

  std::istream& operator>>(std::istream& is, timestamp_t& data)
  {
    uint32_t input = 0;
    is >> input;
    if(input == 0xFFFFFFFF)
      input = 0;

    data = timestamp_t(std::chrono::duration<uint64_t>(input + unix_time_offset));
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const timestamp_t& data)
  {
    return os << data.time_since_epoch().count() - unix_time_offset;
  }

  // record io

  std::istream& operator>>(std::istream& is, record_header_t& data)
  {
    data.extended_byte_length.reset();

    is >> data.type
       >> data.header_flags;

    if(data.header_flags.bit3)
      is >> data.extended_byte_length;

    is >> data.byte_length;
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record_header_t& data)
  {
    data.header_flags.bit3 = data.extended_byte_length.has_value();

    os << data.type
       << data.header_flags
       << data.extended_byte_length
       << data.byte_length;

    return os;
  }


  std::istream& operator>>(std::istream& is, garmin_header_t& data)
  {
    return is >> data.header()
              >> data.magic
              >> data.version
              >> data.timestamp
              >> data.flags
              >> data.name;
  }

  std::ostream& operator<<(std::ostream& os, const garmin_header_t& data)
  {
    data.byte_length = data.statics_size + data.name.byte_count();
    return os << data.header()
              << data.magic
              << data.version
              << data.timestamp
              << data.flags
              << data.name;
  }


  std::istream& operator>>(std::istream& is, poi_header_t& data)
  {
    return is >> data.header()
              >> data.magic
              >> data.version
              >> data.codepage
              >> data.extended_type;
  }

  std::ostream& operator<<(std::ostream& os, const poi_header_t& data)
  {
    data.byte_length = data.statics_size;
    return os << data.header()
              << data.magic
              << data.version
              << data.codepage
              << data.extended_type;
  }


  std::istream& operator>>(std::istream& is, waypoint_t& data)
  {
    return is >> data.header()
              >> data.coordinates
              >> data.reserved
              >> data.flags
              >> data.shortname;
  }

  std::ostream& operator<<(std::ostream& os, const waypoint_t& data)
  {
    data.byte_length = data.statics_size + data.shortname.byte_count();
    return os << data.header()
              << data.coordinates
              << data.reserved
              << data.flags
              << data.shortname;
  }


  std::istream& operator>>(std::istream& is, alert_t& data)
  {
    return is >> data.header()
              >> data.proximity
              >> data.velocity
              >> data.Unknown6
              >> data.Unknown7
              >> data.enabled
              >> data.trigger
              >> data.symbol_id
              >> data.source;
  }

  std::ostream& operator<<(std::ostream& os, const alert_t& data)
  {
    data.byte_length = data.statics_size;
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
    data.Unknown8.reset();
    is >> data.header()
       >> data.bitmap_id;

    if(data.header().byte_length >= 4)
      is >> data.Unknown8.value();
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const bitmap_reference_t& data)
  {
    data.byte_length = data.statics_size + (data.Unknown8 ? sizeof(uint16_t) : 0);

    return os << data.header()
              << data.bitmap_id
              << data.Unknown8;
  }


  std::istream& operator>>(std::istream& is, bitmap_t& data)
  {
    uint32_t image_byte_length = 0;
    uint32_t palette_size = 0;

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

    if(data.flags.bit0)
    {
      uint32_t mask_size = (data.header().byte_length + 6) -
                           data.palette_offset -
                           palette_size;
      data.mask_data.resize(mask_size);
      is.read(reinterpret_cast<char*>(data.mask_data.data()), mask_size);
    }

    return is;
  }

  std::ostream& operator<<(std::ostream& os, const bitmap_t& data)
  {
    uint32_t image_byte_length = data.image_data.size();
    uint32_t palette_size = data.palette_data.size();

    data.byte_length = data.statics_size +
                       image_byte_length +
                       (palette_size * sizeof(uint32_t)) +
                       data.mask_data.size();

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
    return is >> data.header()
              >> data.category_id;
  }

  std::ostream& operator<<(std::ostream& os, const category_reference_t& data)
  {
    data.byte_length = data.statics_size;
    return os << data.header()
              << data.category_id;
  }

  std::istream& operator>>(std::istream& is, category_t& data)
  {
    return is >> data.header()
              >> data.category_id
              >> data.name;
  }

  std::ostream& operator<<(std::ostream& os, const category_t& data)
  {
    data.byte_length = data.statics_size + data.name.byte_count();
    return os << data.header()
              << data.category_id
              << data.name;
  }

  std::istream& operator>>(std::istream& is, area_t& data)
  {
    return is >> data.header()
              >> data.coordinates_max
              >> data.coordinates_min
              >> data.reserved
              >> data.flags
              >> data.unknown;
  }

  std::ostream& operator<<(std::ostream& os, const area_t& data)
  {
    data.byte_length = data.statics_size;
    return os << data.header()
              << data.coordinates_max
              << data.coordinates_min
              << data.reserved
              << data.flags
              << data.unknown;
  }


  std::istream& operator>>(std::istream& is, poi_group_t& data)
  {
    data.areas.clear();
    is >> data.header()
       >> data.source;
    ssize_t remaining_bytes = data.byte_length - data.source.byte_count();
    while(remaining_bytes > 0)
    {
      area_t area;
      is >> area;
      remaining_bytes -= area.header_size() + area.byte_length;
      data.areas.push_back(area);
    }
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const poi_group_t& data)
  {
    data.byte_length = data.statics_size;

    for(auto& area : data.areas)
      data.byte_length += area.header_size() + area.byte_length;

    os << data.header()
       << data.source;

    for(auto& area : data.areas)
      os << area;

    return os;
  }

  std::istream& operator>>(std::istream& is, comment_t& data)
  {
    return is >> data.header()
              >> data.text;
  }

  std::ostream& operator<<(std::ostream& os, const comment_t& data)
  {
    data.byte_length = data.statics_size + data.text.byte_count();
    return os << data.header()
              << data.text;
  }

  std::istream& operator>>(std::istream& is, address_t& data)
  {
     data.city.reset();
     data.country.reset();
     data.state.reset();
     data.postal_code.reset();
     data.street_name.reset();
     data.building_id.reset();

    is >> data.header()
       >> data.header_flags
       >> reinterpret_cast<uint16_t&>(data.have);

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

    return is;
  }

  std::ostream& operator<<(std::ostream& os, const address_t& data)
  {
    data.byte_length = data.statics_size;

    data.have.city        = data.city.has_value();
    data.have.country     = data.country.has_value();
    data.have.state       = data.state.has_value();
    data.have.postal_code = data.postal_code.has_value();
    data.have.street_name = data.street_name.has_value();
    data.have.building_id = data.building_id.has_value();

    if(data.have.city)
      data.byte_length += data.city.value().byte_count();
    if(data.have.country)
      data.byte_length += data.country.value().byte_count();
    if(data.have.state)
      data.byte_length += data.state.value().byte_count();
    if(data.have.postal_code)
      data.byte_length += data.postal_code.value().byte_count();
    if(data.have.street_name)
      data.byte_length += data.street_name.value().byte_count();
    if(data.have.building_id)
      data.byte_length += data.building_id.value().byte_count();

    os << data.header()
       << data.header_flags
       << *reinterpret_cast<const uint16_t*>(&data.have)
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
    is >> data.header()
       >> data.header_flags
       >> reinterpret_cast<uint16_t&>(data.have);

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

    return is;
  }

  std::ostream& operator<<(std::ostream& os, const contact_t& data)
  {
    data.byte_length = data.statics_size;

    data.have.phone1 = data.phone1.has_value();
    data.have.phone2 = data.phone2.has_value();
    data.have.fax    = data.fax.has_value();
    data.have.email  = data.email.has_value();
    data.have.URL    = data.URL.has_value();

    if(data.have.phone1)
      data.byte_length += data.phone1.value().byte_count();
    if(data.have.phone2)
      data.byte_length += data.phone2.value().byte_count();
    if(data.have.fax)
      data.byte_length += data.fax.value().byte_count();
    if(data.have.email)
      data.byte_length += data.email.value().byte_count();
    if(data.have.URL)
      data.byte_length += data.URL.value().byte_count();

    return os << data.header()
       << *reinterpret_cast<const uint16_t*>(&data.have)
       << data.phone1
       << data.phone2
       << data.fax
       << data.email
       << data.URL;
  }

  std::istream& operator>>(std::istream& is, image_file_t& data)
  {
    return is >> data.header()
              >> data.unknown
              >> data.image_data;
  }

  std::ostream& operator<<(std::ostream& os, const image_file_t& data)
  {
    data.byte_length = data.statics_size + data.image_data.byte_count();
    return os << data.header()
              << data.unknown
              << data.image_data;
  }

  std::istream& operator>>(std::istream& is, description_t& data)
  {
    return is >> data.header()
              >> data.unknown
              >> data.text;
  }

  std::ostream& operator<<(std::ostream& os, const description_t& data)
  {
    data.byte_length = data.statics_size + data.text.byte_count();
    return os << data.header()
              << data.unknown
              << data.text;
  }

  std::istream& operator>>(std::istream& is, audio_file_t& data)
  {
    return is >> data.header()
              >> data.audio_id
              >> data.format
              >> data.audio_data;
  }

  std::ostream& operator<<(std::ostream& os, const audio_file_t& data)
  {
    data.byte_length = data.statics_size + data.audio_data.byte_count();
    return os << data.header()
              << data.audio_id
              << data.format
              << data.audio_data;
  }

  std::istream& operator>>(std::istream& is, record15_t& data)
  {
    data.unknown.reset();
    is >> data.header()
       >> data.map_id
       >> data.product_id
       >> data.region_id
       >> data.vendor_id;
    if(data.byte_length > data.statics_size)
      is >> data.unknown;
    return is;
  }

  std::ostream& operator<<(std::ostream& os, const record15_t& data)
  {
    data.byte_length = data.statics_size + (data.unknown ? sizeof(uint8_t) : 0);
    return os << data.header()
              << data.map_id
              << data.product_id
              << data.region_id
              << data.vendor_id
              << data.unknown;
  }

  std::istream& operator>>(std::istream& is, record16_t& data)
  {
    return is >> data.header()
              >> data.points;
  }

  std::ostream& operator<<(std::ostream& os, const record16_t& data)
  {
    data.byte_length = data.statics_size;
    return os << data.header()
              << data.points;
  }

  std::istream& operator>>(std::istream& is, copyright_t& data)
  {
    data.Unknown30.reset();
    is >> data.header()
       >> reinterpret_cast<uint32_t&>(data.have)
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

    return is;
  }

  std::ostream& operator<<(std::ostream& os, const copyright_t& data)
  {
    data.byte_length = data.statics_size;
    data.have.device_model = data.device_model.has_value();
    data.have.image_files = 0;//data.image_files.has_value();
    data.have.Unknown30 = data.Unknown30.has_value();
    os << data.header()
       << reinterpret_cast<uint32_t&>(data.have)
       << data.unknown0
       << data.unknown1
       << data.data_source
       << data.copyright_notice
       << data.device_model
//       << data.image_files
       << data.Unknown30;

    return os;
  }


  std::istream& operator>>(std::istream& is, index_t& data);
  std::ostream& operator<<(std::ostream& os, const index_t& data);
}
