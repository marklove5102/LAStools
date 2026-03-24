/*
===============================================================================

  FILE:  crscheck.hpp

  CONTENTS:

    Functions to validate whether the GEOTIFF tags specify a valid geo-coding
    and (someday) whether they are in agreement to the OGC WKT string.

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com

  COPYRIGHT:

    (c) 2007-2015, martin isenburg, rapidlasso - tools to catch reality

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING.txt file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    8 July 2015 -- look-up name of unknown ESPG codes in list by Loren Dawe
    3 July 2014 -- allowing unknown ESPG codes to pass CRS check with warning
    8 September 2013 -- it started raining on the beach after TREEMAPS meeting

===============================================================================
*/
#ifndef CRS_CHECK_HPP
#define CRS_CHECK_HPP

#include "lasutility.hpp"
#include "lasvalidate_result.hpp"
#include "geoprojectionconverter.hpp"

class CRSprojectionEllipsoid
{
public:
  I32 id = -1;
  std::string name;
  F64 equatorial_radius = 0.0;
  F64 polar_radius = 0.0;
  F64 eccentricity_squared = 0.0;
  F64 inverse_flattening = 0.0;
  F64 eccentricity_prime_squared = 0.0;
  F64 eccentricity = 0.0;
  F64 eccentricity_e1 = 0.0;
};

class CRSprojectionParameters
{
public:
  I32 type = 0;
  std::string name;
  I16 geokey = 0;
  CRSprojectionParameters() { type = -1; geokey = 0; };
};

class CRSprojectionParametersUTM : public CRSprojectionParameters
{
public:
  I32 utm_zone_number;
  std::string utm_zone_letter;
  BOOL utm_northern_hemisphere;
  I32 utm_long_origin;
  CRSprojectionParametersUTM() {
    utm_zone_number = -1;
    utm_long_origin = -1;
    utm_northern_hemisphere = FALSE;
  };
};

class CRSprojectionParametersLCC : public CRSprojectionParameters
{
public:
  F64 lcc_false_easting_meter = 0.0;
  F64 lcc_false_northing_meter = 0.0;
  F64 lcc_lat_origin_degree = 0.0;
  F64 lcc_long_meridian_degree = 0.0;
  F64 lcc_first_std_parallel_degree = 0.0;
  F64 lcc_second_std_parallel_degree = 0.0;
  F64 lcc_lat_origin_radian = 0.0;
  F64 lcc_long_meridian_radian = 0.0;
  F64 lcc_first_std_parallel_radian = 0.0;
  F64 lcc_second_std_parallel_radian = 0.0;
  F64 lcc_n = 0.0;
  F64 lcc_aF = 0.0;
  F64 lcc_rho0 = 0.0;
};

class CRSprojectionParametersTM : public CRSprojectionParameters
{
public:
  F64 tm_false_easting_meter = 0.0;
  F64 tm_false_northing_meter = 0.0;
  F64 tm_lat_origin_degree = 0.0;
  F64 tm_long_meridian_degree = 0.0;
  F64 tm_scale_factor = 0.0;
  F64 tm_lat_origin_radian = 0.0;
  F64 tm_long_meridian_radian = 0.0;
  F64 tm_ap = 0.0;
  F64 tm_bp = 0.0;
  F64 tm_cp = 0.0;
  F64 tm_dp = 0.0;
  F64 tm_ep = 0.0;
};

class CRScheck
{
public:
  void check(std::string& description);
  CRScheck(const LASheader* lasheader, GeoProjectionConverter* geoprojectionconverter, ValidationResult& results, BOOL no_CRS_fail);
  ~CRScheck();

private:
  U32 coordinate_units[2];
  U32 elevation_units[2];
  U32 vertical_epsg[2];
  CRSprojectionEllipsoid* ellipsoids[2];
  CRSprojectionParameters* projections[2];
  const LASheader* lasheader;
  GeoProjectionConverter* geoprojectionconverter;
  std::ostringstream problem_oss;
  std::ostringstream note_oss;
  ValidationResult& results;
  BOOL no_CRS_fail;

  void set_coordinates_in_survey_feet(const BOOL from_geokeys);
  void set_coordinates_in_feet(const BOOL from_geokeys);
  void set_coordinates_in_meter(const BOOL from_geokeys);
  void set_elevation_in_survey_feet(const BOOL from_geokeys);
  void set_elevation_in_feet(const BOOL from_geokeys);
  void set_elevation_in_meter(const BOOL from_geokeys);
  BOOL set_ellipsoid(const I32 ellipsoid_id, const BOOL from_geokeys, std::string* description=nullptr);
  void set_projection(CRSprojectionParameters* projection, const BOOL from_geokeys);
  BOOL set_no_projection(const BOOL from_geokeys, std::string* description);
  BOOL set_latlong_projection(const BOOL from_geokeys, std::string* description);
  BOOL set_longlat_projection(const BOOL from_geokeys, std::string* description);
  BOOL set_ecef_projection(const BOOL from_geokeys, std::string* description);
  BOOL set_utm_projection(const std::string& zone, const BOOL from_geokeys, std::string* description);
  BOOL set_utm_projection(const I32 zone, const BOOL northern, const BOOL from_geokeys, std::string* description);
  BOOL set_mga_projection(const I32 zone, const BOOL northern, const BOOL from_geokeys, std::string* description);
  void set_lambert_conformal_conic_projection(const F64 falseEasting, const F64 falseNorthing, const F64 latOriginDegree, const F64 longMeridianDegree, const F64 firstStdParallelDegree, const F64 secondStdParallelDegree, const BOOL from_geokeys, std::string* description);
  void set_transverse_mercator_projection(const F64 falseEasting, const F64 falseNorthing, const F64 latOriginDegree, const F64 longMeridianDegree, const F64 scaleFactor, const BOOL from_geokeys, std::string* description);
  BOOL set_state_plane_nad27_lcc(const std::string& zone, const BOOL from_geokeys, std::string& description);
  BOOL set_state_plane_nad83_lcc(const std::string& zone, const BOOL from_geokeys, std::string& description);
  BOOL set_state_plane_nad27_tm(const std::string& zone, const BOOL from_geokeys, std::string& description);
  BOOL set_state_plane_nad83_tm(const std::string& zone, const BOOL from_geokeys, std::string& description);
  BOOL set_coordinates_from_ProjLinearUnitsGeoKey(U16 value);
  BOOL set_elevation_from_VerticalUnitsGeoKey(U16 value);
  BOOL set_vertical_from_VerticalCSTypeGeoKey(U16 value);
  BOOL set_projection_from_ProjectedCSTypeGeoKey(const U16 value, std::string& description);

  BOOL check_geokeys(std::string& description);
};

#endif
