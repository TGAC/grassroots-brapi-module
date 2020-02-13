/*
** Copyright 2014-2018 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * brapi_location.c
 *
 *  Created on: 9 Nov 2018
 *      Author: billy
 */

#include "bson.h"

#include "brapi_location.h"

#include "address.h"
#include "schema_keys.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"
#include "mongodb_tool.h"
#include "brapi_module.h"

#include "study.h"
#include "location_jobs.h"

#include "boolean_parameter.h"
#include "string_parameter.h"


static bool SetValidString (json_t *json_p, const char *key_s, const char *value_s);


static json_t *ConvertGrassrootsLocationToBrapi (const json_t *grassroots_json_p);




int IsLocationCall (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	int res = 0;
	const char *signature_s = "locations";

	if (strcmp (api_call_s, signature_s) == 0)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = -1;

			if (params_p)
				{
					bool value = true;

					if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, LOCATION_GET_ALL_LOCATIONS.npt_name_s, NULL, NULL, &value, PL_ALL))
						{
							params_p -> ps_current_level = PL_ADVANCED;
							res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsLocationToBrapi);
						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, LOCATION_GET_ALL_LOCATIONS.npt_type, LOCATION_GET_ALL_LOCATIONS.npt_name_s, NULL, NULL, value, PL_ALL)) */

					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}
	else
		{
			signature_s = "locations/";
			size_t l = strlen (signature_s);

			if (strncmp (api_call_s, signature_s, l) == 0)
				{
					const char *location_id_s = api_call_s + l;

					if (strlen (location_id_s) > 0)
						{
							ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

							res = -1;

							if (params_p)
								{
									if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, LOCATION_ID.npt_type, LOCATION_ID.npt_name_s, NULL, NULL, location_id_s, PL_ALL))
										{
											params_p -> ps_current_level = PL_ADVANCED;
											res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsLocationToBrapi);
										}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

									FreeParameterSet (params_p);
								}		/* if (params_p) */


						}
					else
						{
							res = -1;
						}
				}
		}

	return res;
}


bool GetMinimalLocationData (const json_t *grassroots_json_p, char **name_ss, char **db_id_ss)
{
	const json_t *address_p = json_object_get (grassroots_json_p, ST_LOCATION_S);

	if (address_p)
		{
			bson_oid_t id;

			if (GetMongoIdFromJSON (address_p, &id))
				{
					const char *name_s = GetJSONString (address_p, "name");

					if (name_s)
						{
							char *id_s = GetBSONOidAsString (&id);

							if (id_s)
								{
									char *copied_name_s = EasyCopyToNewString (name_s);

									if (copied_name_s)
										{
											*name_ss = copied_name_s;
											*db_id_ss = id_s;

											return true;
										}

									FreeCopiedString (id_s);
								}
						}
				}
		}

	return false;
}



/*


Implemented by: Germinate

Get a list of locations.

    The countryCode is as per ISO_3166-1_alpha-3 spec.

    altitude is in meters.

Note: Consider revising to describe polygon lat/lan points and check if adopting http://geojson.org/ is worth doing for v1.

Response Fields
Field 	Type 	Description
data 	array[object]
abbreviation 	string 	An abbreviation which represents this location
additionalInfo 	object 	Additional arbitrary info
altitude 	number 	The altitude of this location
countryCode 	string 	ISO_3166-1_alpha-3 spec
countryName 	string 	The full name of the country where this location is
documentationURL 	string (uri) 	A URL to the human readable documentation of this object
instituteAddress 	string 	The street address of the institute representing this location
instituteName 	string 	each institute/laboratory can have several experimental field
latitude 	number 	The latitude of this location
locationDbId 	string 	string identifier
locationName 	string 	A human readable name for this location
locationType 	string 	The type of location this represents (ex. Breeding Location, Storage Location, etc)
longitude 	number 	the longitude of this location
 */

static json_t *ConvertGrassrootsLocationToBrapi (const json_t *grassroots_json_p)
{
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			const json_t *src_address_p = json_object_get (grassroots_data_p, ST_LOCATION_S);

			if (src_address_p)
				{
					Address *address_p = GetAddressFromJSON (src_address_p);

					if (address_p)
						{
							json_t *brapi_address_p = json_object ();

							if (brapi_address_p)
								{
									if (SetValidString (brapi_address_p, "locationType", "Breeding Location"))
										{
											if (SetValidString (brapi_address_p, "countryCode", address_p -> ad_country_code_s))
												{
													if (SetValidString (brapi_address_p, "countryName", address_p -> ad_country_s))
														{
															if (SetValidString (brapi_address_p, "locationName", address_p -> ad_name_s))
																{
																	char *address_s = GetAddressAsString (address_p);

																	if (address_s)
																		{
																			if (SetValidString (brapi_address_p, "instituteAddress", address_s))
																				{
																					bool success_flag = true;
																					char *id_s = GetObjectIdString (grassroots_data_p);

																					if (id_s)
																						{
																							success_flag = SetValidString (brapi_address_p, "locationDbId", id_s);
																							FreeCopiedString (id_s);
																						}

																					if (success_flag)
																						{
																							if (address_p -> ad_gps_centre_p)
																								{
																									if (SetJSONReal (brapi_address_p, "latitude", address_p -> ad_gps_centre_p -> co_x))
																										{
																											if (SetJSONReal (brapi_address_p, "longitude", address_p -> ad_gps_centre_p -> co_y))
																												{
																													success_flag = true;
																												}
																										}
																								}
																							else
																								{
																									success_flag = true;
																								}

																							if (success_flag)
																								{
																									if (address_p -> ad_elevation_p)
																										{
																											success_flag = SetJSONReal (brapi_address_p, "altitude", * (address_p -> ad_elevation_p));
																										}

																									if (success_flag)
																										{
																											return brapi_address_p;
																										}

																								}

																						}		/* if (success_flag) */

																				}		/* if (SetValidString (brapi_address_p, "instituteAddress", address_s)) */

																			FreeCopiedString (address_s);
																		}		/* if (address_s) */


																}		/* if (SetValidString (brapi_address_p, "locationName", address_p -> ad_name_s)) */

														}		/* if (SetValidString (brapi_address_p, "countryName", address_p -> ad_country_s)) */

												}		/* if (SetValidString (brapi_address_p, "countryCode", address_p -> ad_country_code_s)) */

										}
									else		/* if (SetValidString (brapi_address_p, "locationType", "Breeding Location")) */
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, brapi_address_p, "Failed to set locationType to \"Breeding Location\"");
										}

									json_decref (brapi_address_p);
								}		/* if (brapi_address_p) */

							FreeAddress (address_p);
						}		/* if (address_p) */

				}		/* if (src_address_p) */

		}		/* if (grassroots_data_p) */


	return NULL;
}


static bool SetValidString (json_t *json_p, const char *key_s, const char *value_s)
{
	bool success_flag = true;

	if (value_s)
		{
			success_flag = SetJSONString (json_p, key_s, value_s);
		}

	return success_flag;
}



