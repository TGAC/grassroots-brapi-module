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
 *  Created on: 8 Jan 2019
 *      Author: billy
 */

#include "bson.h"
#include "mongodb_tool.h"


#include "brapi_module.h"

#include "brapi_trial.h"
#include "brapi_study.h"

#include "schema_keys.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"
#include "time_util.h"

#include "country_codes.h"


#define ALLOCATE_COORDINATE_TAGS (1)
#define ALLOCATE_LOCATION_TAGS (1)
#include "location.h"

#define ALLOCATE_LOCATION_JOB_CONSTANTS (1)
#include "location_jobs.h"

#include "programme.h"

#include "boolean_parameter.h"
#include "string_parameter.h"

#include "apr_strings.h"


static json_t *ConvertGrassrootsLocationToBrapi (const json_t *grassroots_json_p, request_rec *req_p, ModBrapiConfig *config_p);

static APIStatus DoLocationsSearch (const char *id_s, request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p);

static bool AddAbbreviation (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddAdditionalInfo (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddCoordinateDescription (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddCoordinateUncertainty (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddCoordinates (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddCountryDetails (const json_t *address_p, json_t *brapi_location_p);

static bool AddDocumentationURL (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddEnvironmentType(const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddExposure (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddExternalReferences (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddInstituteAddress (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddInstituteName (const json_t *grassroots_location_p, json_t *brapi_location_p);

static char *GetAndAddLocationDbId (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddLocationName (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddLocationType (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddParentLocationDbId (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddParentLocationName (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddSiteStatus (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddSlope (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddTopography (const json_t *grassroots_location_p, json_t *brapi_location_p);

static bool AddRealToJSONArray (json_t *array_p, const double value);



APIStatus GetAllLocations (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p)
{
	APIStatus res = AS_IGNORED;
	const char *signature_s = "locations";

	if (strcmp (api_call_s, signature_s) == 0)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = AS_FAILED;

			if (params_p)
				{
					bool success_flag = true;
					bool value = true;

					if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, LOCATION_GET_ALL_LOCATIONS.npt_name_s, NULL, NULL, &value, PL_ALL))
						{
							apr_pool_t *pool_p = req_p -> pool;
							const char *sort_by_s = NULL;
							const char *sort_order_s = NULL;
							const char *page_number_s = NULL;
							const char *page_index_s = NULL;
							const char *location_id_s = GetParameterValue (req_params_p, "locationDbId", pool_p);
							const char *active_s = GetParameterValue (req_params_p, "active", pool_p);
							const char *crop_name_s = GetParameterValue (req_params_p, "commonCropName", pool_p);

							GetSortSearchParameters (req_params_p, &sort_by_s, &sort_order_s, pool_p);

							if (success_flag)
								{
									params_p -> ps_current_level = PL_ADVANCED;
									res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsLocationToBrapi);
								}

						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Search Experimental Areas", NULL, NULL, value, PL_ALL)) */


					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}

	return res;
}



APIStatus GetLocationByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p)
{
	APIStatus res = AS_IGNORED;

	const char *signature_s = "locations/";
	size_t l = strlen (signature_s);

	if (strncmp (api_call_s, signature_s, l) == 0)
		{
			const char *location_id_s = api_call_s + l;

			if (strlen (location_id_s) > 0)
				{
					ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

					res = AS_FAILED;

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
					res = AS_FAILED;
				}
		}

	return res;
}



static json_t *ConvertGrassrootsLocationToBrapi (const json_t *grassroots_json_p, request_rec *req_p, ModBrapiConfig *config_p)
{
	json_t *brapi_response_p = NULL;
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			brapi_response_p = json_object ();

			if (brapi_response_p)
				{
					if (AddAbbreviation (grassroots_data_p, brapi_response_p))
						{
							if (AddAdditionalInfo (grassroots_data_p, brapi_response_p))
								{
									const json_t *address_p = json_object_get (grassroots_data_p, LO_ADDRESS_S);

									if (address_p)
										{
											if (AddCoordinateDescription (grassroots_data_p, brapi_response_p))
												{
													if (AddCoordinateUncertainty (grassroots_data_p, brapi_response_p))
														{
															if (AddCoordinates (address_p, brapi_response_p))
																{
																	if (AddCountryDetails (address_p, brapi_response_p))
																		{
																			if (AddDocumentationURL (grassroots_data_p, brapi_response_p))
																				{
																					if (AddEnvironmentType (grassroots_data_p, brapi_response_p))
																						{
																							if (AddExposure (grassroots_data_p, brapi_response_p))
																								{
																									if (AddExternalReferences (grassroots_data_p, brapi_response_p))
																										{
																											if (AddInstituteAddress (grassroots_data_p, brapi_response_p))
																												{
																													if (AddInstituteName (grassroots_data_p, brapi_response_p))
																														{
																															char *id_s = GetAndAddLocationDbId (grassroots_data_p, brapi_response_p);

																															if (id_s)
																																{
																																	if (AddLocationName (grassroots_data_p, brapi_response_p))
																																		{
																																			if (AddLocationType (grassroots_data_p, brapi_response_p))
																																				{
																																					if (AddParentLocationDbId (grassroots_data_p, brapi_response_p))
																																						{
																																							if (AddParentLocationName (grassroots_data_p, brapi_response_p))
																																								{
																																									if (AddSiteStatus (grassroots_data_p, brapi_response_p))
																																										{
																																											if (AddSlope (grassroots_data_p, brapi_response_p))
																																												{
																																													if (AddTopography (grassroots_data_p, brapi_response_p))
																																														{
																																															return brapi_response_p;
																																														}		/* if (AddTopography (grassroots_data_p, brapi_response_p)) */

																																												}		/* if (AddSlope (grassroots_data_p, brapi_response_p)) */

																																										}		/* if (AddSiteStatus (grassroots_data_p, brapi_response_p)) */

																																								}		/* if (AddParentLocationName (grassroots_data_p, brapi_response_p)) */

																																						}		/* if (AddParentLocationDbId (grassroots_data_p, brapi_response_p)) */

																																				}		/* if (AddLocationType (grassroots_data_p, brapi_response_p)) */

																																		}		/* if (AddLocationName (grassroots_data_p, brapi_response_p)) */

																																	FreeBSONOidString (id_s);
																																}		/* if (id_s) */

																														}		/* if (AddInstituteName (grassroots_data_p, brapi_response_p)) */

																												}		/* if (AddInstituteAddress (grassroots_data_p, brapi_response_p)) */

																										}		/* if (AddExternalReferences (grassroots_data_p, brapi_response_p)) */

																								}		/* if (AddExposure (grassroots_data_p, brapi_response_p)) */

																						}		/* if (AddEnvironmentType (grassroots_data_p, brapi_response_p)) */

																				}		/* if (AddDocumentationURL (grassroots_data_p, brapi_response_p)) */

																		}		/* if (AddCountryDetails (grassroots_data_p, brapi_response_p)) */

																}		/* if (AddCoordinates (grassroots_data_p, brapi_response_p)) */

														}		/* if (AddCoordinateUncertainty (grassroots_data_p, brapi_response_p)) */

												}		/* if (AddCoordinateDescription (grassroots_data_p, brapi_response_p)) */

										}		/* if (address_p) */

								}		/* if (AddAdditionalInfo (grassroots_data_p, brapi_response_p)) */

						}		/* if (AddAbbreviation (grassroots_data_p, brapi_response_p)) */

					json_decref (brapi_response_p);
				}		/* if (brapi_response_p) */

		}		/* if (grassroots_data_p) */

	return NULL;
}




static bool AddAbbreviation (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "abbreviation"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddAdditionalInfo (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "additionalInfo"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddCoordinateDescription (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "coordinateDescription"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddCoordinateUncertainty (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "coordinateUncertainty"))
		{
			success_flag = true;
		}

	return success_flag;
}



/*
	"coordinates": {
			"geometry": {
					"coordinates": [
							-76.506042,
							42.417373,
							123
					],
					"type": "Point"
			},
			"type": "Feature"
	},
 */

static bool AddCoordinates (const json_t *address_p, json_t *brapi_location_p)
{
	const json_t *address_location_p = json_object_get (address_p, AD_LOCATION_S);

	if (address_location_p)
		{
			const json_t *centre_p = json_object_get (address_location_p, AD_CENTRE_LOCATION_S);

			if (centre_p)
				{
					double latitude = 0.0;

					if (GetJSONReal (centre_p, CO_LATITUDE_S, &latitude))
						{
							double longitude = 0.0;

							if (GetJSONReal (centre_p, CO_LONGITUDE_S, &longitude))
								{
									json_t *coordinates_p = NULL;
									bool altitude_flag = false;
									double altitude = 0.0;

									if (GetJSONReal (centre_p, CO_ELEVATION_S, &altitude))
										{
											altitude_flag = true;
										}

									coordinates_p = json_object ();

									if (coordinates_p)
										{
											if (SetJSONString (coordinates_p, "type", "Feature"))
												{
													json_t *geometry_p = json_object ();

													if (geometry_p)
														{
															if (json_object_set_new (coordinates_p, "geometry", geometry_p) == 0)
																{
																	if (SetJSONString (coordinates_p, "type", "Point"))
																		{
																			json_t *coords_array_p = json_array ();

																			if (coords_array_p)
																				{
																					if (json_object_set_new (geometry_p, "coordinates", coords_array_p) == 0)
																						{
																							if (AddRealToJSONArray (coords_array_p, latitude))
																								{
																									if (AddRealToJSONArray (coords_array_p, longitude))
																										{
																											if (altitude_flag)
																												{
																													AddRealToJSONArray (coords_array_p, altitude);
																												}

																											if (json_object_set_new (brapi_location_p, "coordinates", coordinates_p) == 0)
																												{
																													return true;
																												}
																										}

																								}
																						}
																					else
																						{
																							json_decref (coords_array_p);
																						}

																				}		/* if (coords_array_p) */

																		}		/* if (SetJSONString (coordinates_p, "type", "Point")) */


																}		/* if (geometry_p) */
															else
																{
																	json_decref (geometry_p);
																}

														}		/* if (SetJSONString (coordinates_p, "type", "Point")) */

												}		/* if (SetJSONString (coordinates_p, "type", "Feature")) */


											json_decref (coordinates_p);

										}		/* if (coordinates_p) */


								}

						}
				}

		}


	return false;
}




static bool AddCountryDetails (const json_t *address_p, json_t *brapi_location_p)
{
	bool success_flag = false;
	bool done_flag = false;
	const json_t *inner_address_p = json_object_get (address_p, AD_ADDRESS_S);
	const char * const BRAPI_COUNTRY_CODE_S = "countryCode";
	const char * const BRAPI_COUNTRY_NAME_S = "countryName";

	if (inner_address_p)
		{
			const char *country_code_s = GetJSONString (inner_address_p, AD_COUNTRY_S);

			if (country_code_s)
				{
					done_flag = true;

					if (SetJSONString (brapi_location_p, BRAPI_COUNTRY_CODE_S, country_code_s))
						{
							const char *name_s = GetCountryNameFromCode (country_code_s);

							if (name_s)
								{
									if (SetJSONString (brapi_location_p, BRAPI_COUNTRY_NAME_S, name_s))
										{
											success_flag = true;
										}

								}
							else
								{
									if (SetJSONNull (brapi_location_p, BRAPI_COUNTRY_NAME_S))
										{
											success_flag = true;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, brapi_location_p, "Failed to add \"%s\": null", BRAPI_COUNTRY_NAME_S);
										}

								}


						}		/* if (SetJSONString (brapi_location_p, BRAPI_COUNTRY_CODE_S, country_code_s)) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, brapi_location_p, "Failed to add \"%s\": \"%s\"", BRAPI_COUNTRY_CODE_S, country_code_s);
						}
				}		/* if (country_code_s) */
		}


	if (!done_flag)
		{
			if (SetJSONNull (brapi_location_p, BRAPI_COUNTRY_CODE_S))
				{
					if (SetJSONNull (brapi_location_p, BRAPI_COUNTRY_NAME_S))
						{
							success_flag = true;
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, brapi_location_p, "Failed to add \"%s\": null", BRAPI_COUNTRY_CODE_S);
				}
		}

	return success_flag;
}



static bool AddDocumentationURL (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "documentationURL"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddEnvironmentType (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (brapi_location_p, LO_TYPE_S, brapi_location_p, "environmentType"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddExposure (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "exposure"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddExternalReferences (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "externalReferences"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddInstituteAddress (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "instituteAddress"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddInstituteName (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "instituteName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static char *GetAndAddLocationDbId (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bson_oid_t id;

	if (GetMongoIdFromJSON (grassroots_location_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_location_p, "locationDbId", id_s))
						{
							return id_s;
						}

					FreeBSONOidString (id_s);
				}
		}

	return NULL;
}




static bool AddLocationName (const json_t *parent_programme_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (parent_programme_p, PR_NAME_S, brapi_location_p, "locationName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddLocationType (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "locationType"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddParentLocationDbId (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "parentLocationDbId"))
		{
			success_flag = true;
		}

	return success_flag;
}




static bool AddParentLocationName (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "parentLocationName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddSiteStatus (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "siteStatus"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddSlope (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "slope"))
		{
			success_flag = true;
		}

	return success_flag;
}



static bool AddTopography (const json_t *grassroots_location_p, json_t *brapi_location_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_location_p, "topography"))
		{
			success_flag = true;
		}

	return success_flag;
}



static bool AddRealToJSONArray (json_t *array_p, const double value)
{
	json_t *value_p = json_real (value);

	if (value_p)
		{
			if (json_array_append_new (array_p, value_p) == 0)
				{
					return true;
				}

			json_decref (value_p);
		}

	return false;
}

