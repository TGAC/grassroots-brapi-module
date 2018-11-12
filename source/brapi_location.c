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

#include "brapi_location.h"

#include "address.h"
#include "schema_keys.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"

#include "brapi_module.h"


static int GetLocations (request_rec *req_p, apr_table_t *params_p);

static int GetLocationsByDbId (request_rec *req_p, apr_table_t *params_p);

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
					SharedType value;

					InitSharedType (&value);
					value.st_boolean_value = true;

					if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Get all Locations", NULL, NULL, value, PL_ALL))
						{
							json_t *grassroots_request_p = GetGrassrootsRequest (params_p);

							if (grassroots_request_p)
								{
									ModBrapiConfig *config_p = ap_get_module_config (req_p -> per_dir_config, &grassroots_brapi_module);
									Connection *connection_p = AllocateWebServerConnection (config_p -> mbc_grassroots_url_s);

									if (connection_p)
										{
											json_t *grassroots_response_p = MakeRemoteJsonCall (grassroots_request_p, connection_p);

											if (grassroots_response_p)
												{
													const json_t *service_results_p = json_object_get (grassroots_response_p, SERVICE_RESULTS_S);

													if (service_results_p)
														{
															if (json_is_array (service_results_p))
																{
																	/*
																	 * We should just have a single service
																	 */
																	if (json_array_size (service_results_p) == 1)
																		{
																			/* Get the job status */
																			OperationStatus status = OS_ERROR;
																			service_results_p = json_array_get (service_results_p, 0);

																			const char *value_s = GetJSONString (service_results_p, SERVICE_STATUS_S);

																			if (value_s)
																				{
																					status = GetOperationStatusFromString (value_s);
																				}
																			else
																				{
																					int i;
																					/* Get the job status */

																					if (GetJSONInteger(service_results_p, SERVICE_STATUS_VALUE_S, &i))
																						{
																							if ((i > OS_LOWER_LIMIT) && (i < OS_UPPER_LIMIT))
																								{
																									status = (OperationStatus) i;
																								}
																						}
																				}

																			if ((status == OS_PARTIALLY_SUCCEEDED) || (status == OS_SUCCEEDED))
																				{
																					const json_t *job_results_p = json_object_get (service_results_p, JOB_RESULTS_S);

																					if (job_results_p)
																						{
																							json_t *brapi_results_p = json_array ();

																							if (brapi_results_p)
																								{
																									json_t *response_p = NULL;
																									size_t current_page = 1;
																									size_t page_size;
																									size_t total_count;
																									size_t total_pages = 1;

																									if (json_is_array (job_results_p))
																										{
																											const size_t num_results = json_array_size (job_results_p);
																											size_t i = 0;

																											for (i = 0; i < num_results; ++ i)
																												{
																													const json_t *grassroots_result_p = json_array_get (job_results_p, i);
																													json_t *brapi_result_p = ConvertGrassrootsLocationToBrapi (grassroots_result_p);

																													if (brapi_result_p)
																														{
																															if (json_array_append_new (brapi_results_p, brapi_result_p) == 0)
																																{

																																}		/* if (json_array_append_new (brapi_results_p, brapi_result_p) == 0) */
																															else
																																{
																																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, brapi_result_p, "Failed to add result");
																																	json_decref (brapi_result_p);
																																}
																														}
																													else
																														{
																															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, grassroots_result_p, "Failed to create brapi_result_p");
																														}
																												}

																										}		/* if (json_is_array (job_results_p)) */

																									total_count = json_array_size (brapi_results_p);
																									page_size = json_array_size (brapi_results_p);

																									response_p = CreateResponseJSONForResult (brapi_results_p, current_page, page_size, total_count, total_pages);

																									if (response_p)
																										{
																											char *response_s = json_dumps (response_p, JSON_INDENT (2));

																											if (response_s)
																												{
																													ap_rputs (response_s, req_p);
																													ap_set_content_type (req_p, "application/json");

																													res = 1;
																													free (response_s);
																												}		/* if (repsonse_s) */
																											else
																												{
																													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed tp get string representation");
																												}


																											json_decref (response_p);
																										}		/* if (response_p) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_results_p, "CreateResponseJSONForResult failed");
																										}

																								}		/* if (brapi_results_p) */

																						}		/* if (job_results_p) */

																				}		/* if ((status == OS_PARTIALLY_SUCCEEDED) || (status == OS_SUCCEEDED)) */

																		}		/* if (json_array_size (service_results_p) == 1) */

																}		/* if (json_is_array (service_results_p)) */

														}		/* if (results_p) */


													json_decref (grassroots_response_p);
												}		/* if (grassroots_response_p) */

											FreeConnection (connection_p);
										}		/* if (connection_p) */

									json_decref (grassroots_request_p);
								}		/* if (grassroots_request_p) */

						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}
	else
		{
			signature_s = "locations/";
			size_t l = strlen (signature_s);

			if (strncmp (api_call_s, signature_s, l) == 0)
				{
					res = -1;

				}
		}

	return res;
}


static json_t *ConvertGrassrootsLocationToBrapi (const json_t *grassroots_json_p)
{
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			const json_t *src_address_p = json_object_get (grassroots_data_p, "address");

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


static int GetLocations (request_rec *req_p, apr_table_t *params_p)
{
	int res = 0;

	return res;
}


static int GetLocationsByDbId (request_rec *req_p, apr_table_t *params_p)
{
	int res = 0;

	return res;
}
