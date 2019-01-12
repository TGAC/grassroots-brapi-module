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
 * brapi_trial.c
 *
 *  Created on: 8 Jan 2019
 *      Author: billy
 */


#include "brapi_trial.h"
#include "schema_keys.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"
#include "time_util.h"

#include "brapi_module.h"


static json_t *ConvertGrassrootsTrialToBrapi (const json_t *grassroots_json_p);



int IsTrialCall (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	int res = 0;
	const char *signature_s = "trials";

	if (strcmp (api_call_s, signature_s) == 0)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = -1;

			if (params_p)
				{
					bool success_flag = true;
					SharedType value;

					InitSharedType (&value);
					value.st_boolean_value = true;

					if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Search Studies", NULL, NULL, value, PL_ALL))
						{
							if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Get all Studies", NULL, NULL, value, PL_ALL))
								{
									apr_pool_t *pool_p = req_p -> pool;
									const char *active_s = GetParameterValue (req_params_p, "active", pool_p);
									const char *crop_name_s = GetParameterValue (req_params_p, "commonCropName", pool_p);
									const char *location_s = GetParameterValue (req_params_p, "locationDbId", pool_p);

									if (active_s && (strcmp (active_s, "true") == 0))
										{
											struct tm current_time;

											success_flag = false;

											if (GetCurrentTime (&current_time))
												{
													char *current_date_s = GetTimeAsString (&current_time, false);

													if (current_date_s)
														{
															if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_TIME, "Active on date", NULL, NULL, value, PL_ALL))
																{
																	success_flag = true;
																}

															FreeCopiedString (current_date_s);
														}
												}

										}		/* if (active_s && (strcmp (active_s, "true" == 0))) */

									if (success_flag)
										{
											res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsTrialToBrapi);
										}

								}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Search Experimental Areas", NULL, NULL, value, PL_ALL)) */


					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}
	else
		{
			signature_s = "trials/";
			size_t l = strlen (signature_s);

			if (strncmp (api_call_s, signature_s, l) == 0)
				{
					res = -1;
				}
		}

	return res;
}


static json_t *ConvertGrassrootsTrialToBrapi (const json_t *grassroots_json_p)
{
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			json_t *brapi_trial_p = json_object ();

			if (brapi_trial_p)
				{

					return brapi_trial_p;
				}		/* if (brapi_trial_p) */

		}		/* if (grassroots_data_p) */

	return NULL;
}
