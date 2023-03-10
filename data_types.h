#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <cstring>
#include <chrono>
#include <optional>
#include <map>
#include <vector>
#include <iostream>
#include <variant>

#include <type_traits>

#include <endian_types.h>

namespace garmin
{
  enum bool_t : uint8_t
  {
    False = 0,
    True,
  };


  static_assert(sizeof(bool) == 1, "wut?");
  static_assert(sizeof(std::underlying_type<bool_t>) == 1, "oh snap");
  static_assert(std::is_same_v<std::underlying_type_t<bool_t>, uint8_t>, "oh snap");


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
    None = 0,
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
  constexpr uint64_t unix_time_offset = 7304 /* days*/ * (24 * 60 * 60 /* seconds per day */); // 631065600 seconds between UNIX epoch (1970-01-01) and Garmin epoch (1989-12-31).


  struct flags_t
  {
    union
    {
      struct
      {
        uint8_t bit0 : 1;
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;
      };
      uint8_t byte0;
    };

    union
    {
      struct
      {
        uint8_t bit8 : 1;
        uint8_t bit9 : 1;
        uint8_t bitA : 1;
        uint8_t bitB : 1;
        uint8_t bitC : 1;
        uint8_t bitD : 1;
        uint8_t bitE : 1;
        uint8_t bitF : 1;
      };
      uint8_t byte1;
    };
  };


  struct garmin_header_t;
  struct poi_header_t;
  struct waypoint_t;
  struct alert_t;
  struct bitmap_reference_t;
  struct bitmap_t;
  struct category_reference_t;
  struct category_t;
  struct area_t;
  struct poi_group_t;
  struct comment_t;
  struct address_t;
  struct contact_t;
  struct image_file_t;
  struct description_t;
  struct record15_t;
  struct record16_t;
  struct copyright_t;
  struct audio_file_t;
  struct speed_camera_t;
  struct record20_t;
  struct index_t;
  struct record22_t;
  struct record23_t;
  struct record24_t;
  struct record25_t;
  struct record26_t;
  struct record27_t;

  using any_record_t = std::variant<garmin_header_t, poi_header_t, waypoint_t, alert_t,
                                    bitmap_reference_t, bitmap_t, category_reference_t,
                                    category_t, area_t, poi_group_t, comment_t, address_t,
                                    contact_t, image_file_t, description_t, record15_t,
                                    record16_t, copyright_t, audio_file_t, speed_camera_t,
                                    record20_t, index_t, record22_t, record23_t, record24_t,
                                    record25_t, record26_t, record27_t>;

  struct record_header_t
  {
    record_header_t(const record_header_t& header)
      : type(header.type),
        header_flags(header.header_flags),
        end_of_record(header.end_of_record),
        end_of_data(header.end_of_data)
    {}

    record_header_t(const record_id_t t = End, std::initializer_list<record_id_t> ct = {})
      : type(t),
        end_of_record(UINT32_MAX),
        children_types(ct)
    {}
    virtual ~record_header_t(void) = default;

    record_header_t& header(void) { return *this; }
    const record_header_t& header(void) const { return *this; }
    uint32_t header_size(void) const { return end_of_data ? 12 : 8; }
    virtual uint32_t statics_size(void) const { return 0; }
    virtual uint32_t calc_data_size(void) const { return statics_size(); }
    uint32_t children_size(void) const;



    record_id_t type;
    mutable flags_t header_flags; // seen values: 0, 8 and 0x18
      // bit3: record has auxiliary data
      // bit4: Only seen in FileFormat '01' on Header2 and POI Group records. Seems to indicate that record 23 and record 24 are present

    uint32_t data_size(void) const { return end_of_data.value_or(end_of_record); }
    uint32_t aux_data_size(void) const { return end_of_data ? end_of_record - end_of_data.value() : 0; }

    mutable uint32le_t end_of_record;
    mutable std::optional<uint32le_t> end_of_data;
    const std::vector<record_id_t> children_types;
    std::vector<any_record_t> child_records;
  };

  template<typename size_type, typename data_type>
  struct vector_t : std::vector<data_type>
  {
    size_type byte_count(void) const { return sizeof(size_type) + bytes_held(); }
    size_type bytes_held(void) const { return std::vector<data_type>::size() * sizeof(data_type); }
  };

  template<typename localized_type>
  struct localized_t : std::map<uint16_t, localized_type>
  {
    uint32_t byte_count(void) const
    {
      uint32_t total = sizeof(uint32_t);
      for(const auto& pair : *this)
        total += sizeof(uint16_t) + pair.second.byte_count();
      return total;
    }
  };


  using vector16_t = vector_t<uint16_t, uint8_t>;
  using vector32_t = vector_t<uint32_t, uint8_t>;
  using lstring_t = localized_t<vector16_t>;

  struct garmin_header_t : record_header_t
  {
    garmin_header_t(const record_header_t& header) : record_header_t(header) { }
    garmin_header_t(void)
      : record_header_t(GarminHeader, { Record15 })
    {
      memcpy(magic, "GRMREC", 6);
      memcpy(version, "01", 2);
    }
    uint32_t statics_size(void) const { return 14; }


    char magic[6];    // "GRMREC"
    char version[2];  // "00" or "01"
    timestamp_t timestamp;
    flags_t flags;
      // bit0: obfustication enabled
    vector16_t name;
  };

  struct poi_header_t : record_header_t
  {
    poi_header_t(const record_header_t& header) : record_header_t(header) { }
    poi_header_t(void)
      : record_header_t(POIHeader, { Copyright })
    {
      memcpy(magic, "POI\0\0\0", 6);
      memcpy(version, "01", 2);
    }

    uint32_t statics_size(void) const { return 12; }

    char magic[6];    // "POI\0\0\0"
    char version[2];  // "00" or "01"
    codepage_t codepage;
    record_id_t auxiliary_type; // 0 for none, 17 for record_t::Copyright type
  };

  struct waypoint_t : record_header_t
  {
    waypoint_t(const record_header_t& header) : record_header_t(header) { }
    waypoint_t(void) : record_header_t(Waypoint, { CategoryReference, BitmapReference, Alert, Comment, Address, Contact, ImageFileSet, Description, Record26 }) { }

    uint32_t statics_size(void) const { return 11; }
    uint32_t calc_data_size(void) const { return statics_size() + shortname.byte_count(); }

    coords32_t coordinates;
    uint8_t reserved; // 0

    flags_t flags;
      // bit0: could be “Alert record in auxiliary data”
      // bit1: unknown
      // bit4: unknown
      // bit8: unknown (always 1)
    lstring_t shortname;
  };

  struct alert_t : record_header_t
  {
    alert_t(const record_header_t& header) : record_header_t(header) { }
    alert_t(void) : record_header_t(Alert, { Record16, Record27 }) { }

    uint32_t statics_size(void) const { return 12; }

    uint16le_t proximity; // measured in meters
    uint16le_t velocity;  // measured in 100x meters / second, 0 = none
    uint16le_t Unknown6;  // Seen values 0 (dorognoe_radio.gpi) and 0x100
    uint16le_t Unknown7;  // Seen values 0 (dorognoe_radio.gpi) and 0x100

    bool_t enabled;
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
    bitmap_reference_t(const record_header_t& header) : record_header_t(header) { }
    bitmap_reference_t(void) : record_header_t(BitmapReference) { }

    uint32_t statics_size(void) const { return 2; }
    uint32_t calc_data_size(void) const { return statics_size() + (Unknown8 ? 2 : 0); }

    uint16le_t bitmap_id; // points to the Bitmap record with this ID number
    std::optional<uint16le_t> Unknown8; // Optional, =2
  };

  struct bitmap_t : record_header_t
  {
    bitmap_t(const record_header_t& header) : record_header_t(header) { }
    bitmap_t(void) : record_header_t(Bitmap) { }

    uint32_t statics_size(void) const { return 36; }
    uint32_t calc_data_size(void) const;

    uint16le_t bitmap_id;
    uint16le_t height;
    uint16le_t width;
    uint16le_t line_length; // in bytes
    uint16le_t bits_per_pixel; // 4/8 (paletted) or 24/32? (rgb/argb?)
    uint16le_t reserved0; // 0
    uint32le_t image_offset; // 44 bytes (from start of record)
    uint32le_t transparent_color;
    uint16le_t reserved1; // 0
    mutable flags_t flags; // seen value 0, 1 and 0x0100 (D0743030F.gpi)
      // bit0: has mask
      // bit8: unknown

    uint32le_t palette_offset; // "image_byte_length" + 44 (from start of record)
    std::vector<uint8_t> image_data; // "image_byte_length" bytes
    std::vector<uint32le_t> palette_data; // "palette_size" * 4 bytes
    std::vector<uint8_t> mask_data; // unknown byte length
  };


  struct category_reference_t : record_header_t
  {
    category_reference_t(const record_header_t& header) : record_header_t(header) { }
    category_reference_t(void) : record_header_t(CategoryReference) { }

    uint32_t statics_size(void) const { return 2; }

    uint16le_t category_id; // points to the Category record with this ID number
  };

  struct category_t : record_header_t
  {
    category_t(const record_header_t& header) : record_header_t(header) { }
    category_t(void) : record_header_t(Category, { BitmapReference }) { }

    uint32_t statics_size(void) const { return 2; }
    uint32_t calc_data_size(void) const { return statics_size() + name.byte_count(); }

    uint16le_t category_id; // points to the Category record with this ID number
    lstring_t name;
  };

  struct area_t : record_header_t
  {
    area_t(const record_header_t& header) : record_header_t(header) { }
    area_t(void) : record_header_t(Area, { AreaSet, WaypointSpeedCameraSet }) { }

    uint32_t statics_size(void) const { return 23; }

    coords32_t coordinates_max;
    coords32_t coordinates_min;
    uint32le_t reserved; // 0

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
    poi_group_t(const record_header_t& header) : record_header_t(header) { }
    poi_group_t(void) : record_header_t(POIGroup, { CategorySet, BitmapSet, AudioFileSet, Record23, Record24 }) { }

    uint32_t calc_data_size(void) const;

    lstring_t source;
    std::vector<area_t> areas; // n x Record List of area records (type 8).
  };

  struct comment_t : record_header_t
  {
    comment_t(const record_header_t& header) : record_header_t(header) { }
    comment_t(void) : record_header_t(Comment) { }

    uint32_t calc_data_size(void) const { return text.byte_count(); }
    lstring_t text;
  };

  struct address_t : record_header_t
  {
    address_t(const record_header_t& header) : record_header_t(header) { }
    address_t(void) : record_header_t(Address) { }

    uint32_t statics_size(void) const { return 2; }
    uint32_t calc_data_size(void) const;

    struct have_t
    {
      uint8_t byte0;
      union
      {
        struct
        {
          uint8_t city        : 1;
          uint8_t country     : 1;
          uint8_t state       : 1;
          uint8_t postal_code : 1;
          uint8_t street_name : 1;
          uint8_t building_id : 1;
        };
        uint8_t byte1;
      };
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
    contact_t(const record_header_t& header) : record_header_t(header) { }
    contact_t(void) : record_header_t(Contact) { }

    uint32_t statics_size(void) const { return 2; }
    uint32_t calc_data_size(void) const;

    struct have_t
    {
      uint8_t byte0;
      union
      {
        struct
        {
          uint8_t phone1 : 1;
          uint8_t phone2 : 1;
          uint8_t fax    : 1;
          uint8_t email  : 1;
          uint8_t URL    : 1;
        };
        uint8_t byte1;
      };
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
    image_file_t(const record_header_t& header) : record_header_t(header) { }
    image_file_t(void) : record_header_t(ImageFile) { }

    uint32_t statics_size(void) const { return 5; }
    uint32_t calc_data_size(void) const { return statics_size() + image_data.byte_count(); }

    uint8_t unknown;
    vector32_t image_data;
  };

  struct description_t : record_header_t
  {
    description_t(const record_header_t& header) : record_header_t(header) { }
    description_t(void) : record_header_t(Description) { }
    uint32_t statics_size(void) const { return 1; }
    uint32_t calc_data_size(void) const { return statics_size() + text.byte_count(); }

    uint8_t unknown; // Seen values: 1, 5, 50 (color/style)
    lstring_t text;
  };

  struct record15_t : record_header_t
  {
    record15_t(const record_header_t& header) : record_header_t(header) { }
    record15_t(void) : record_header_t(Record15) { }

    uint32_t statics_size(void) const { return 5; }
    uint32_t calc_data_size(void) const { return statics_size() + (unknown ? 1 : 0); }

    uint16le_t map_id;
    uint8_t product_id;
    region_t region_id;
    uint8_t vendor_id;
    std::optional<uint8_t> unknown;
  };

  struct record16_t : record_header_t
  {
    record16_t(const record_header_t& header) : record_header_t(header) { }
    record16_t(void) : record_header_t(Record16) { }
    uint32_t statics_size(void) const { return 2; }
    uint32_t calc_data_size(void) const { return statics_size() + (points.size() * sizeof(uint32_t) * 3); }

    struct point3d_t
    {
      coords32_t location;
      uint32le_t unknown; // altitude?
    };

    vector_t<uint16_t, point3d_t> points;
  };


  struct copyright_t : record_header_t
  {
    copyright_t(const record_header_t& header) : record_header_t(header) { }
    copyright_t(void) : record_header_t(Copyright) { }
    uint32_t statics_size(void) const { return 8; }

    uint32_t calc_data_size(void) const;

    struct have_t
    {
      union
      {
        struct
        {
          uint8_t bit00 : 1;
          uint8_t bit01 : 1;
          uint8_t bit02 : 1;
          uint8_t bit03 : 1;
          uint8_t bit04 : 1;
          uint8_t waypoint_index : 1;
          uint8_t bit06 : 1;
          uint8_t seen0: 1;
        };
        uint8_t byte0;
      };
      union
      {
        struct
        {
          uint8_t image_files : 1;
          uint8_t bit09 : 1;
          uint8_t bit0A : 1;
          uint8_t bit0B : 1;
          uint8_t bit0C : 1;
          uint8_t bit0D : 1;
          uint8_t bit0E : 1;
          uint8_t bit0F : 1;
        };
        uint8_t byte1;
      };
      union
      {
        struct
        {
          uint8_t seen1 : 1;
          uint8_t rec23_24 : 1;
          uint8_t bit12 : 1;
          uint8_t bit13 : 1;
          uint8_t Unknown30 : 1;
          uint8_t bit15 : 1;
          uint8_t bit16 : 1;
          uint8_t speed_camera_records : 1;
        };
        uint8_t byte2;
      };
      union
      {
        struct
        {
          uint8_t bit18 : 1;
          uint8_t bit19 : 1;
          uint8_t device_model : 1;
          uint8_t bit1B : 1;
          uint8_t bit1C : 1;
          uint8_t bit1D : 1;
          uint8_t bit1E : 1;
          uint8_t bit1F : 1;
        };
        uint8_t byte3;
      };
    } mutable have;
    static_assert(sizeof(have_t) == sizeof(uint32_t), "packing failure");

    uint16le_t unknown0;
    uint16le_t unknown1;

    lstring_t data_source;
    lstring_t copyright_notice;

    // optional section

    std::optional<vector16_t> device_model;

    struct image_file_data_t
    {
      uint8_t bytes[20];
      // TODO
    };
    std::vector<image_file_data_t> image_files; // always 12 image_file_data_t types?

    std::optional<uint16le_t> Unknown30;
  };

  struct audio_file_t : record_header_t
  {
    audio_file_t(const record_header_t& header) : record_header_t(header) { }
    audio_file_t(void) : record_header_t(AudioFile) { }

    uint32_t statics_size(void) const { return 3; }
    uint32_t calc_data_size(void) const { return statics_size() + audio_data.byte_count(); }

    uint16le_t audio_id;
    audio_format_t format;
    localized_t<vector32_t> audio_data;
  };

  struct speed_camera_t : record_header_t
  {
    speed_camera_t(const record_header_t& header) : record_header_t(header) { }
    speed_camera_t(void) : record_header_t(SpeedCamera) { }

  };

  struct record20_t : record_header_t
  {
    record20_t(const record_header_t& header) : record_header_t(header) { }
    record20_t(void) : record_header_t(Record20) { }
  };

  struct index_t : record_header_t
  {
    index_t(const record_header_t& header) : record_header_t(header) { }
    index_t(void) : record_header_t(Index) { }

    uint32_t statics_size(void) const override { return 58; }

    uint16le_t data_length; // byte_length - 2 (weird)
    uint32le_t index_offset0;
    uint8_t  unknown[32]; // padding?
    uint32le_t index_offset1;
    uint32le_t index_size;
    uint32le_t Unknown36; // 1
    uint32le_t Unknown37; // 0
    uint32le_t Unknown38; // 0
  };


  struct record22_t : record_header_t
  {
    record22_t(const record_header_t& header) : record_header_t(header) { }
    record22_t(void) : record_header_t(Record22) { }
  };


  struct record23_t : record_header_t
  {
    record23_t(const record_header_t& header) : record_header_t(header) { }
    record23_t(void) : record_header_t(Record23) { }
  };


  struct record24_t : record_header_t
  {
    record24_t(const record_header_t& header) : record_header_t(header) { }
    record24_t(void) : record_header_t(Record24) { }
  };


  struct record25_t : record_header_t
  {
    record25_t(const record_header_t& header) : record_header_t(header) { }
    record25_t(void) : record_header_t(Record25) { }
  };


  struct record26_t : record_header_t
  {
    record26_t(const record_header_t& header) : record_header_t(header) { }
    record26_t(void) : record_header_t(Record26) { }
  };


  struct record27_t : record_header_t
  {
    record27_t(const record_header_t& header) : record_header_t(header) { }
    record27_t(void) : record_header_t(Record27) { }
  };


  uint32_t record_size(const any_record_t& data);

} // namespace garmin

#endif // DATA_TYPES_H
