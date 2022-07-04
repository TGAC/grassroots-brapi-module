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

#include "field_trial.h"
#include "field_trial_jobs.h"

#include "boolean_parameter.h"
#include "string_parameter.h"


static json_t *ConvertGrassrootsTrialToBrapi (const json_t *grassroots_json_p);

static bool SetTrialStudiesData (const json_t *grassroots_data_p, json_t *brapi_response_p);

static bool ConvertGrassrootsStudy (const json_t *grassroots_study_p, json_t *brapi_studies_p);



APIStatus GetAllTrials (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	APIStatus res = AS_IGNORED;
	const char *signature_s = "trials";

	if (strcmp (api_call_s, signature_s) == 0)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = AS_FAILED;

			if (params_p)
				{
					bool success_flag = true;
					bool value = true;

					if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, FIELD_TRIAL_SEARCH.npt_name_s, NULL, NULL, &value, PL_ALL))
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
									res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsTrialToBrapi);
								}


						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Search Experimental Areas", NULL, NULL, value, PL_ALL)) */


					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}

	return res;
}



APIStatus GetTrialByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	APIStatus res = AS_IGNORED;

	const char *signature_s = "trials/";
	size_t l = strlen (signature_s);

	if (strncmp (api_call_s, signature_s, l) == 0)
		{
			const char *trial_id_s = api_call_s + l;

			if (strlen (trial_id_s) > 0)
				{
					ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

					res = -1;

					if (params_p)
						{
							if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, FIELD_TRIAL_ID.npt_type, FIELD_TRIAL_ID.npt_name_s, NULL, NULL, trial_id_s, PL_ALL))
								{
									params_p -> ps_current_level = PL_ADVANCED;
									res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsTrialToBrapi);
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


static json_t *ConvertGrassrootsTrialToBrapi (const json_t *grassroots_json_p)
{
	json_t *brapi_response_p = NULL;
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			brapi_response_p = json_object ();

			if (brapi_response_p)
				{
					bool success_flag = false;

					if (CopyJSONStringValue (grassroots_data_p, "so:name", brapi_response_p, "trialName"))
						{
							bson_oid_t id;

							if (GetMongoIdFromJSON (grassroots_data_p, &id))
								{
									char *id_s = GetBSONOidAsString (&id);

									if (id_s)
										{
											if (SetJSONString (brapi_response_p, "trialDbId", id_s))
												{
													if (SetTrialStudiesData (grassroots_data_p, brapi_response_p))
														{
															success_flag = true;
														}
												}

											FreeBSONOidString (id_s);
										}
								}
						}

					if (!success_flag)
						{
							json_decref (brapi_response_p);
							brapi_response_p = NULL;
						}
				}		/* if (brapi_response_p) */

		}		/* if (grassroots_data_p) */

	return brapi_response_p;
}


static bool SetTrialStudiesData (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;

	/*
	   GRASSOOTS
	   ---------
	   "studies": [
    {
      "so:name": "DFW Field Phenotyping Facility - Wheat",
      "min_ph": 0,
      "max_ph": 0,
      "study_design": "Randomized placement of lines within the field with a duplication of as many lines as possible (subject to seed availability). A control line (potentially, WT) will be placed systematically (approx every 9th line) throughout the field to correct for likely spatial heterogeneity.  Note, some consideration of legacy effect (due to to differing previous crops) will be needed in the placement of controls.",
      "phenotype_gathering_notes": "Sponsor to take in-season measurements",
      "address": {
        "_id": {
          "$oid": "5d67a6e824ce205d7f6bbc52"
        },
        "order": 0,
        "name": "Great Field 1/2 Phenotyping Area",
        "address": {
          "Address": {
            "@type": "PostalAddress",
            "name": "Great Field 1/2 Phenotyping Area",
            "addressLocality": "St Albans",
            "addressRegion": "Hertfordshire",
            "addressCountry": "UK",
            "postalCode": "AL5 2GT"
          },
          "location": {
            "centre": {
              "@type": "so:GeoCoordinates",
              "latitude": 51.806477999999998,
              "longitude": -0.36156500000000003
            }
          }
        },
        "@type": "Grassroots:Location"
      },
      "parent_field_trial": {
        "_id": {
          "$oid": "5d5ac41b24ce20420b233228"
        },
        "so:name": "Awaiting project code allocation"
      },
      "_id": {
        "$oid": "5dd80098de68e75a927a826e"
      },
      "number_of_plots": 0,
      "@type": "Grassroots:Study"
    },



		 BRAPI
	   -----
		 "studies": [
		{
			"locationDbId": "1",
			"locationName": "Location 1",
			"studyDbId": "1001",
			"studyName": "Study 1"
		},
		{
			"locationDbId": "1",
			"locationName": "Location 1",
			"studyDbId": "1002",
			"studyName": "Study 2"
		}
	 */

	const json_t *grassroots_studies_p = json_object_get (grassroots_data_p, FT_STUDIES_S);

	if (grassroots_studies_p)
		{
			json_t *brapi_studies_p = json_array ();

			if (brapi_studies_p)
				{
					const size_t num_studies = json_array_size (grassroots_studies_p);
					size_t i;

					for (i = 0; i < num_studies; ++ i)
						{
							const json_t *grassroots_study_p = json_array_get (grassroots_studies_p, i);

							if (!ConvertGrassrootsStudy (grassroots_study_p, brapi_studies_p))
								{
									/* force exit from loop */
									i = num_studies;
								}
						}


					if (num_studies == json_array_size (brapi_studies_p))
						{
							if (json_object_set_new (brapi_response_p, "studies", brapi_studies_p) == 0)
								{
									success_flag = true;
								}
						}
				}



		}		/* if (grassroots_studies_p) */
	else
		{
			/* no studies to add */

			success_flag = true;
		}


	return success_flag;
}


static bool ConvertGrassrootsStudy (const json_t *grassroots_study_p, json_t *brapi_studies_p)
{
	bool success_flag = false;
	json_t *brapi_study_p = json_object ();

	if (brapi_study_p)
		{
			if (CopyJSONStringValue (grassroots_study_p, "so:name", brapi_study_p, "studyName"))
				{
					bson_oid_t id;

					if (GetMongoIdFromJSON (grassroots_study_p, &id))
						{
							char *id_s = GetBSONOidAsString (&id);

							if (id_s)
								{
									if (SetJSONString (brapi_study_p, "studyDbId", id_s))
										{
											if (SetStudyLocationData (grassroots_study_p, brapi_study_p))
												{
													if (json_array_append_new (brapi_studies_p, brapi_study_p) == 0)
														{
															success_flag = true;
														}
												}
										}

									FreeBSONOidString (id_s);
								}
						}
				}

			if (!success_flag)
				{
					json_decref (brapi_study_p);
				}

		}		/* if (brapi_study_p) */

	return success_flag;
}
