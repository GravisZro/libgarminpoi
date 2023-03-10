#include <data_types.h>

#include <cassert>

namespace garmin
{

  template<typename T>
  inline uint32_t record_size(const any_record_t& data)
  {
    auto& record = std::get<T>(data);

    return record.header_size() +
        record.data_size() +
        record.children_size();
  }

  uint32_t record_size(const any_record_t& data)
  {
    switch(data.index())
    {
      case GarminHeader:      return record_size<garmin_header_t      >(data);
      case POIHeader:         return record_size<poi_header_t         >(data);
      case Waypoint:          return record_size<waypoint_t           >(data);
      case Alert:             return record_size<alert_t              >(data);
      case BitmapReference:   return record_size<bitmap_reference_t   >(data);
      case Bitmap:            return record_size<bitmap_t             >(data);
      case CategoryReference: return record_size<category_reference_t >(data);
      case Category:          return record_size<category_t           >(data);
      case Area:              return record_size<area_t               >(data);
      case POIGroup:          return record_size<poi_group_t          >(data);
      case Comment:           return record_size<comment_t            >(data);
      case Address:           return record_size<address_t            >(data);
      case Contact:           return record_size<contact_t            >(data);
      case ImageFile:         return record_size<image_file_t         >(data);
      case Description:       return record_size<description_t        >(data);
      case Record15:          return record_size<record15_t           >(data);
      case Record16:          return record_size<record16_t           >(data);
      case Copyright:         return record_size<copyright_t          >(data);
      case AudioFile:         return record_size<audio_file_t         >(data);
      case SpeedCamera:       return record_size<speed_camera_t       >(data);
      case Record20:          return record_size<record20_t           >(data);
      case Index:             return record_size<index_t              >(data);
      case Record22:          return record_size<record22_t           >(data);
      case Record23:          return record_size<record23_t           >(data);
      case Record24:          return record_size<record24_t           >(data);
      case Record25:          return record_size<record25_t           >(data);
      case Record26:          return record_size<record26_t           >(data);
      case Record27:          return record_size<record27_t           >(data);
      default:
        assert(false);
        break;
    }
    return 0;
  }

  uint32_t record_header_t::children_size(void) const
  {
    uint32_t total = 0;
    for(const auto& child : child_records)
      total += record_size(child);
    return total;
  }

  uint32_t bitmap_t::calc_data_size(void) const
  {
    return statics_size() +
        image_data.size() +
        (palette_data.size() * sizeof(uint32_t)) +
        mask_data.size();
  }

  uint32_t poi_group_t::calc_data_size(void) const
  {
    uint32_t total = source.byte_count();
    for(auto& area : areas)
    {
      total += area.calc_data_size() +
               area.children_size();
    }
    return total;
  }

  uint32_t address_t::calc_data_size(void) const
  {
    return statics_size() +
        (city         ? city->byte_count()        : 0) +
        (country      ? country->byte_count()     : 0) +
        (state        ? state->byte_count()       : 0) +
        (postal_code  ? postal_code->byte_count() : 0) +
        (street_name  ? street_name->byte_count() : 0) +
        (building_id  ? building_id->byte_count() : 0);
  }

  uint32_t contact_t::calc_data_size(void) const
  {
    return statics_size() +
        (phone1 ? phone1->byte_count()  : 0) +
        (phone2 ? phone2->byte_count()  : 0) +
        (fax    ? fax->byte_count()     : 0) +
        (email  ? email->byte_count()   : 0) +
        (URL    ? URL->byte_count()     : 0);
  }


  uint32_t copyright_t::calc_data_size(void) const
  {
    return statics_size() +
        data_source.byte_count() +
        copyright_notice.byte_count() +
        (device_model ? device_model->byte_count() : 0) +
        (image_files.size() * sizeof(image_file_data_t)) +
        (Unknown30 ? sizeof(uint16_t) : 0);
  }

}
