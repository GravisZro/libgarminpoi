#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <cstring>
#include <chrono>
#include <optional>
#include <string_view>
#include <map>
#include <vector>
#include <bitset>
#include <iostream>
#include <array>
#include <initializer_list>
#include <cassert>


namespace garmin
{
  enum record_id_t : uint16_t
  {
    GarminHeader = 0,
    POIHeader,
    Waypoint,
    Alert,
    BitmapReference,
    Bitmap,
    CategoryReference,
    Category,
    Area,
    POIGroup,
    Comment,
    Address,
    Contact,
    ImageFile,
    Description,
    Record15,  // 15
    Record16,  // 16
    Copyright,
    AudioFile,
    SpeedCamera,
    Record20, // 20
    Index,
    Record22, // 22
    Record23,  // 23
    Record24,  // 24
    Record25, // 25
    Record26,  // 26
    Record27,  // 27

    // All versions
    End = 0xFFFF,

     // 0 or more of each
    BitmapSet = 32,
    CategorySet,
    AreaSet,
    POIGroupSet,
    ImageFileSet,
    AudioFileSet,
    WaypointSpeedCameraSet, // set of waypoints or a set of speed cameras
  };

  enum codepage_t : uint16_t
  {
    Thai                = 0x036A, // Thai (Windows)
    ChineseTraditional  = 0x03B6, // Chinese Traditional (Big5)
    CentralEuropean     = 0x04E2, // Central European (Windows)
    Cyrillic            = 0x04E3, // Cyrillic (Windows)
    WesternEuropean     = 0x04E4, // Western European (Windows)
    Unicode             = 0xFDE9, // Unicode (UTF-8)
  };

  enum alert_trigger_t : uint8_t
  {
    proximity = 0, // 360° alert
    along_road, // values 1 and 2 (in 2011_DOC_Camping_Sites_Update.gpi)
    tour_guide,
  };


  enum alert_source_t : uint8_t
  {
    symbol    = 0x00, // exact nature is uncertain
    internal  = 0x10, // internal audio clip
    media     = 0x20, // audio clip in media record
  };

  enum audio_clips_t : uint8_t
  {
    beep = 0,
    tone,
    triple_beep,
    silence,
    plonk,
    double_plonk,
  };

  enum audio_format_t : uint8_t
  {
    WAV = 0,
    MP3,
  };

  enum region_t : uint8_t
  {
    None,
    UnitedKingdom_Ireland,
    Netherlands,
    France,
    Belgium_Luxemburg,
    Australia_NewZealand,
    Spain_Portugal,
    Italy_Slovenia,
    Austria_Germany,
    Nordics,
    EasternEurope,
    Greece,
    NorthAmerica,
    Russia,
    SouthAfrica,
    MiddleEast,

    AlsoNone = 0xFF,
  };

  template <int bits>
  struct coord_t
  {
    double data;
    coord_t<bits>& operator = (double other) { data = other; return *this; }
    operator double(void) const { return data; }
  };

  template <int bits>
  struct coord_pair_t
  {
    coord_t<bits> latitude;
    coord_t<bits> longitude;
  };

  using coords24_t = coord_pair_t<24>;
  using coords32_t = coord_pair_t<32>;

  using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<uint64_t>>;
  constexpr uint64_t unix_time_offset = 7304 /* days*/ * (24 * 60 * 60 /* seconds per day */); // seconds between UNIX epoch (1970-01-01) and Garmin epoch (1989-12-31).


  struct flags_t
  {
    uint16_t bit0 : 1;
    uint16_t bit1 : 1;
    uint16_t bit2 : 1;
    uint16_t bit3 : 1;
    uint16_t bit4 : 1;
    uint16_t bit5 : 1;
    uint16_t bit6 : 1;
    uint16_t bit7 : 1;
    uint16_t bit8 : 1;
    uint16_t bit9 : 1;
    uint16_t bitA : 1;
    uint16_t bitB : 1;
    uint16_t bitC : 1;
    uint16_t bitD : 1;
    uint16_t bitE : 1;
    uint16_t bitF : 1;
  };

  struct record_header_t
  {
    record_header_t(const uint32_t statics_sz, const record_id_t t, std::initializer_list<record_id_t> c = {})
      : type(t),
        statics_size(statics_sz),
        byte_length(UINT32_MAX),
        children(c)
    {}

    record_header_t& header(void) { return *this; }
    const record_header_t& header(void) const { return *this; }
    uint32_t header_size(void) const { return extended_byte_length ? 12 : 8; }

    record_id_t type;
    mutable flags_t header_flags; // seen values: 0, 8 and 0x18
      // bit3: record has extended data
      // bit4: Only seen in FileFormat '01' on Header2 and POI Group records. Seems to indicate that record 23 and record 24 are present

    std::optional<uint32_t> extended_byte_length;
    const uint32_t statics_size;
    mutable uint32_t byte_length;

    const std::vector<record_id_t> children;
  };

  template<typename size_type, typename data_type>
  struct vector_t : std::vector<data_type>
  {
    size_type byte_count(void) const { return sizeof(size_type) + (std::vector<data_type>::size() * sizeof(data_type)); }
  };

  template<typename localized_type>
  struct localized_t : std::map<char[2], localized_type>
  {
    uint32_t byte_count(void) const
    {
      uint32_t total = sizeof(uint32_t);
      for(const auto& pair : *this)
        total += 2 + pair.second.byte_count();
      return total;
    }
  };


  using vector16_t = vector_t<uint16_t, uint8_t>;
  using vector32_t = vector_t<uint32_t, uint8_t>;
  using lstring_t = localized_t<vector16_t>;


  struct garmin_header_t : record_header_t
  {
    garmin_header_t(void)
      : record_header_t(14, GarminHeader, { Record15 })
    {
      memcpy(magic, "GRMREC", 6);
      memcpy(version, "01", 2);
    }

    char magic[6];    // "GRMREC"
    char version[2];  // "00" or "01"
    timestamp_t timestamp;
    flags_t flags;
      // bit0: obfustication
    vector16_t name;
  };

  struct poi_header_t : record_header_t
  {
    poi_header_t(void)
      : record_header_t(12, POIHeader, { Copyright })
    {
      memcpy(magic, "POI\0\0\0", 6);
      memcpy(version, "01", 2);
    }

    char magic[6];    // "POI\0\0\0"
    char version[2];  // "00" or "01"
    codepage_t codepage;
    record_id_t extended_type; // 0 for none, 17 for record_t::Copyright type
  };

  struct waypoint_t : record_header_t
  {
    waypoint_t(void)
      : record_header_t(11, Waypoint, { CategoryReference, BitmapReference, Alert, Comment, Address, Contact, ImageFileSet, Description, Record26 }) { }

    coords32_t coordinates;
    uint8_t reserved; // 0

    flags_t flags;
      // bit0: could be “Alert record in extra data”
      // bit1: unknown
      // bit4: unknown
      // bit8: unknown (always 1)
    lstring_t shortname;
  };

  struct alert_t : record_header_t
  {
    alert_t(void)
      : record_header_t(12, Alert, { Record16, Record27 }) { }

    uint16_t proximity; // measured in meters
    uint16_t velocity;  // measured in 100x meters / second, 0 = none
    uint16_t Unknown6;  // Seen values 0 (dorognoe_radio.gpi) and 0x100
    uint16_t Unknown7;  // Seen values 0 (dorognoe_radio.gpi) and 0x100

    bool enabled;
    alert_trigger_t trigger;
    union
    {
      uint8_t symbol_id;
      audio_clips_t internal_id;
      uint8_t media_id;
    };
    alert_source_t source;
  };

  struct bitmap_reference_t : record_header_t
  {
    bitmap_reference_t(void)
      : record_header_t(2, BitmapReference) { }

    uint16_t bitmap_id; // points to the Bitmap record with this ID number
    std::optional<uint16_t> Unknown8; // Optional, =2
  };

  struct bitmap_t : record_header_t
  {
    bitmap_t(void)
      : record_header_t(36, Bitmap) { }

    uint16_t bitmap_id;
    uint16_t height;
    uint16_t width;
    uint16_t line_length; // in bytes
    uint16_t bits_per_pixel; // 4/8 (paletted) or 24/32? (rgb/argb?)
    uint16_t reserved0; // 0
    uint32_t image_offset; // 44 bytes (from start of record)
    uint32_t transparent_color;
    uint16_t reserved1; // 0
    mutable flags_t flags; // seen value 0, 1 and 0x0100 (D0743030F.gpi)
      // bit0: has mask
      // bit8: unknown

    uint32_t palette_offset; // "image_byte_length" + 44 (from start of record)
    std::vector<uint8_t> image_data; // "image_byte_length" bytes
    std::vector<uint32_t> palette_data; // "palette_size" * 4 bytes
    std::vector<uint8_t> mask_data; // unknown byte length
  };


  struct category_reference_t : record_header_t
  {
    category_reference_t(void)
      : record_header_t(2, CategoryReference) { }

    uint16_t category_id; // points to the Category record with this ID number
  };

  struct category_t : record_header_t
  {
    category_t(void)
      : record_header_t(2, Category, { BitmapReference }) { }

    uint16_t category_id; // points to the Category record with this ID number
    lstring_t name;
  };

  struct area_t : record_header_t
  {
    area_t(void)
      : record_header_t(23, Area, { AreaSet, WaypointSpeedCameraSet }) { }

    coords32_t coordinates_max;
    coords32_t coordinates_min;
    uint32_t reserved; // 0

    flags_t flags;
      // bit0: always set
    uint8_t unknown; // Seen values 0, 1,
    // 2 (3D_Bogenschiessen.gpi),
    // 3 (dorognoe_radio.gpi),
    // 5 (2011_DOC_Camping_Sites_Update.gpi),
    // 8 and 18 (spb_metro_norm.gpi)
  };

  struct poi_group_t : record_header_t
  {
    poi_group_t(void)
      : record_header_t(0, POIGroup, { CategorySet, BitmapSet, AudioFileSet, Record23, Record24 }) { }

    lstring_t source;
    std::vector<area_t> areas; // n x Record List of area records (type 8).
  };

  struct comment_t : record_header_t
  {
    comment_t(void)
      : record_header_t(0, Comment) { }

    lstring_t text;
  };

  struct address_t : record_header_t
  {
    address_t(void)
      : record_header_t(2, Address) { }

    struct have_t
    {
      uint16_t city        : 1;
      uint16_t country     : 1;
      uint16_t state       : 1;
      uint16_t postal_code : 1;
      uint16_t street_name : 1;
      uint16_t building_id : 1;
    } mutable have;
    static_assert(sizeof(have_t) == sizeof(uint16_t), "packing failure");

    std::optional<lstring_t>  city;
    std::optional<lstring_t>  country;
    std::optional<lstring_t>  state;
    std::optional<vector16_t> postal_code;
    std::optional<lstring_t>  street_name;
    std::optional<vector16_t> building_id;
  };

  struct contact_t : record_header_t
  {
    contact_t(void)
      : record_header_t(2, Contact) { }

    struct have_t
    {
      uint16_t phone1 : 1;
      uint16_t phone2 : 1;
      uint16_t fax    : 1;
      uint16_t email  : 1;
      uint16_t URL    : 1;
    } mutable have;
    static_assert(sizeof(have_t) == sizeof(uint16_t), "packing failure");

    std::optional<vector16_t> phone1;
    std::optional<vector16_t> phone2;
    std::optional<vector16_t> fax;
    std::optional<vector16_t> email;
    std::optional<vector16_t> URL;
  };

  struct image_file_t : record_header_t
  {
    image_file_t(void)
      : record_header_t(5, ImageFile) { }
    uint8_t unknown;
    vector32_t image_data;
  };

  struct description_t : record_header_t
  {
    description_t(void)
      : record_header_t(1, Description) { }

    uint8_t unknown; // Seen values: 1, 5, 50 (color/style)
    lstring_t text;
  };


  struct audio_file_t : record_header_t
  {
    audio_file_t(void)
      : record_header_t(3, AudioFile) { }

    uint16_t audio_id;
    audio_format_t format;
    localized_t<vector32_t> audio_data;
  };

  struct record15_t : record_header_t
  {
    record15_t(void)
      : record_header_t(5, Record15) { }

    uint16_t map_id;
    uint8_t product_id;
    region_t region_id;
    uint8_t vendor_id;
    std::optional<uint8_t> unknown;
  };

  struct record16_t : record_header_t
  {
    record16_t(void)
      : record_header_t(2, Record16) { }

    struct point3d_t
    {
      coords32_t location;
      uint32_t unknown; // altitude?
    };

    vector_t<uint16_t, point3d_t> points;
  };


  struct copyright_t : record_header_t
  {
    copyright_t(void)
      : record_header_t(8, Copyright) { }

    struct have_t
    {
      uint32_t bit00 : 1;
      uint32_t bit01 : 1;
      uint32_t bit02 : 1;
      uint32_t bit03 : 1;
      uint32_t bit04 : 1;
      uint32_t waypoint_index : 1;
      uint32_t bit06 : 1;
      uint32_t seen0: 1;
      uint32_t image_files : 1;
      uint32_t bit09 : 1;
      uint32_t bit0A : 1;
      uint32_t bit0B : 1;
      uint32_t bit0C : 1;
      uint32_t bit0D : 1;
      uint32_t bit0E : 1;
      uint32_t bit0F : 1;
      uint32_t seen1 : 1;
      uint32_t rec23_24 : 1;
      uint32_t bit12 : 1;
      uint32_t bit13 : 1;
      uint32_t Unknown30 : 1;
      uint32_t bit15 : 1;
      uint32_t bit16 : 1;
      uint32_t speed_camera_records : 1;
      uint32_t bit18 : 1;
      uint32_t bit19 : 1;
      uint32_t device_model : 1;
      uint32_t bit1B : 1;
      uint32_t bit1C : 1;
      uint32_t bit1D : 1;
      uint32_t bit1E : 1;
      uint32_t bit1F : 1;
    } mutable have;
    static_assert(sizeof(have_t) == sizeof(uint32_t), "packing failure");

    uint16_t unknown0;
    uint16_t unknown1;

    lstring_t data_source;
    lstring_t copyright_notice;

    // optional section

    std::optional<vector16_t> device_model;

    struct image_file_data_t
    {
      // TODO
    };
    std::array<image_file_data_t, 12> image_files;

    std::optional<uint16_t> Unknown30;
  };

  struct index_t : record_header_t
  {
    index_t(void)
      : record_header_t(58, Index) { }
    uint16_t record_size; // byte_length - 2 (weird)
    uint32_t index_offset0;
    uint8_t  unknown[32]; // padding?
    uint32_t index_offset1;
    uint32_t index_size;
    uint32_t Unknown36; // 1
    uint32_t Unknown37; // 0
    uint32_t Unknown38; // 0
  };


  template<typename type>
  std::istream& operator>>(std::istream& is, std::optional<type>& data)
  {
    assert(data);
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
    size_type length = 0;
    is >> length;
    vector.resize(length);
    is.read(reinterpret_cast<char*>(vector.data()), vector.byte_count());
    return is;
  }

  template<typename size_type, typename data_type>
  std::ostream& operator<<(std::ostream& os, const vector_t<size_type, data_type>& vector)
  {
    os << size_type(vector.size());
    os.write(reinterpret_cast<const char*>(vector.data()), vector.byte_count());
    return os;
  }

  template<typename localized_type>
  std::istream& operator>>(std::istream& is, localized_t<localized_type>& data)
  {
    data.clear();
    ssize_t remaining_bytes = 0;
    is >> remaining_bytes;
    while(remaining_bytes > 0)
    {
      char key[2];
      localized_type value;
      is >> key >> value;
      data.at(key) = value;
      remaining_bytes -= 2 + value.byte_count();
    }
    return is;
  }

  template<typename localized_type>
  std::ostream& operator<<(std::ostream& os, const localized_t<localized_type>& data)
  {
    uint32_t byte_length = 0;
    for(const auto& pair : data)
      byte_length += sizeof(uint8_t[2])
                  +  pair.second.size();
    os << byte_length;
    for(const auto& pair : data)
      os << pair.first[0] << pair.first[1] << pair.second;
    return os;
  }

  std::istream& operator>>(std::istream& is, record_header_t& data);
  std::ostream& operator<<(std::ostream& os, const record_header_t& data);

  std::istream& operator>>(std::istream& is, garmin_header_t& data);
  std::ostream& operator<<(std::ostream& os, const garmin_header_t& data);

  std::istream& operator>>(std::istream& is, poi_header_t& data);
  std::ostream& operator<<(std::ostream& os, const poi_header_t& data);

  std::istream& operator>>(std::istream& is, waypoint_t& data);
  std::ostream& operator<<(std::ostream& os, const waypoint_t& data);

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

#endif // DATA_TYPES_H
