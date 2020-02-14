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
 * brapi_study.c
 *
 *  Created on: 13 Nov 2018
 *      Author: billy
 */

#include "bson.h"
#include "mongodb_tool.h"

#include "brapi_study.h"

#include "schema_keys.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"
#include "time_util.h"

#include "brapi_module.h"
#include "brapi_location.h"

#include "study.h"
#include "streams.h"
#include "study_jobs.h"

#include "boolean_parameter.h"
#include "time_parameter.h"


static json_t *ConvertGrassrootsStudyToBrapi (const json_t *grassroots_json_p);

static bool SetStudyActivity (const json_t *grassroots_data_p, json_t *brapi_response_p);

static bool SetStudyCurrentCrop (const json_t *grassroots_data_p, json_t *brapi_response_p);

static bool AddAdditionalInfo (const json_t *grassroots_data_p, json_t *brapi_response_p);

static bool AddParentTrialData (const json_t *grassroots_data_p, json_t *brapi_response_p);

/*
	commonCropName
	Common name for the crop associated with this study
	String

	studyTypeDbId
	Filter based on study type unique identifier
	String

	programDbId
	Program filter to only return studies associated with given program id.
	String

	locationDbId
	Filter by location
	String

	seasonDbId
	Filter by season or year
	String

	trialDbId
	Filter by trial
	String

	studyDbId
	Filter by study DbId
	String

	active
	Filter active status true/false.
	String

	sortBy
	Name of the field to sort by.
	String

	sortOrder
	Sort order direction. Ascending/Descending.
	String

	page
	Which result page is requested. The page indexing starts at 0 (the first page is 'page'= 0). Default is 0.
	String

	pageSize
	The size of the pages to be returned. Default is 1000.
	String

	Authorization
	HTTP HEADER - Token used for Authorization
	String
 */

int IsStudyCall (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	int res = 0;
	const char *signature_s = "studies";

	if (strcmp (api_call_s, signature_s) == 0)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = -1;

			if (params_p)
				{
					bool params_success_flag = false;
					bool value = true;

					if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, STUDY_SEARCH_STUDIES.npt_name_s, NULL, NULL, &value, PL_ALL))
						{
							const char *study_id_s = apr_table_get (req_params_p, "studyDbId");

							/*
							 * Is the search for a specific study?
							 */
							if (study_id_s)
								{
									if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, STUDY_ID.npt_type, STUDY_ID.npt_name_s, NULL, NULL, study_id_s, PL_ALL))
										{
											params_success_flag = true;
										}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

								}		/* if (study_id_s) */
							else
								{
									apr_pool_t *pool_p = req_p -> pool;
									const char *active_s = GetParameterValue (req_params_p, "active", pool_p);
									const char *crop_name_s = GetParameterValue (req_params_p, "commonCropName", pool_p);

									if (active_s && (strcmp (active_s, "true") == 0))
										{
											struct tm current_time;

											if (GetCurrentTime (&current_time))
												{
													if (EasyCreateAndAddTimeParameterToParameterSet (NULL, params_p, NULL, STUDY_SEARCH_ACTIVE_DATE.npt_name_s, NULL, NULL, &current_time, PL_ALL))
														{
															params_success_flag = true;
														}
												}

										}		/* if (active_s && (strcmp (active_s, "true" == 0))) */
									else
										{
											params_success_flag = true;
										}

								}

						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, STUDY_SEARCH_STUDIES.npt_type, STUDY_SEARCH_STUDIES.npt_name_s, NULL, NULL, value, PL_ALL)) */


					if (params_success_flag)
						{
							params_p -> ps_current_level = PL_ADVANCED;
							res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsStudyToBrapi);
						}

					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}
	else
		{
			signature_s = "studies/";
			size_t l = strlen (signature_s);

			if (strncmp (api_call_s, signature_s, l) == 0)
				{
					res = -1;

				}
		}

	return res;
}



/*

	GRASSROOTS

	{
  "so:name": "DFW TKNIL Set 2",
  "address": {
    "_id": {
      "$oid": "5bcdc8f8618dc26d670434d3"
    },
    "order": 0,
    "address": {
      "Address": {
        "@type": "PostalAddress",
        "name": "Mrs Salih's field",
        "streetAddress": "Watton Road",
        "addressLocality": "Bawburgh",
        "addressRegion": "Norfolk",
        "addressCountry": "GB",
        "postalCode": "NR9 3LQ"
      },
      "location": {
        "centre": {
          "@type": "so:GeoCoordinates",
          "latitude": 52.625714000000002,
          "longitude": 1.185354
        }
      }
    }
  },
  "sowing_date": "2017-11-10",
  "harvest_date": "2018-08-04",
  "_id": {
    "$oid": "5bcdc979618dc26d682e4a52"
  },
  "@context": {
    "so:": "http://schema.org/",
    "co:": "http://www.cropontology.org/terms/"
  }
}

	BRAPI

{
  "metadata": {
    "datafiles": [],
    "pagination": {
      "currentPage": 0,
      "pageSize": 2,
      "totalCount": 2,
      "totalPages": 1
    },
    "status": []
  },
  "result": {
    "data": [
      {
        "active": "true",
        "additionalInfo": {
          "studyObjective": "Increase yield"
        },
        "commonCropName": "Tomatillo",
        "documentationURL": "https://brapi.org",
        "endDate": "2014-01-01",
        "locationDbId": "1",
        "locationName": "Location 1",
        "name": "Study 1",
        "programDbId": "1",
        "programName": "Program 1",
        "seasons": [
          {
            "season": "fall",
            "seasonDbId": "1",
            "year": "2011"
          },
          {
            "season": "winter",
            "seasonDbId": "2",
            "year": "2012"
          }
        ],
        "startDate": "2013-01-01",
        "studyDbId": "1001",
        "studyName": "Study 1",
        "studyType": "Yield study",
        "studyTypeDbId": "2",
        "studyTypeName": "Yield study",
        "trialDbId": "101",
        "trialName": "Peru Yield Trial 1"
      },
      {
        "active": "true",
        "additionalInfo": {
          "publications": "pmid:23643517318968"
        },
        "commonCropName": "Tomatillo",
        "documentationURL": "https://brapi.org",
        "endDate": "2015-01-01",
        "locationDbId": "1",
        "locationName": "Location 1",
        "name": "Study 2",
        "programDbId": "1",
        "programName": "Program 1",
        "seasons": [
          {
            "season": "winter",
            "seasonDbId": "2",
            "year": "2012"
          }
        ],
        "startDate": "2014-01-01",
        "studyDbId": "1002",
        "studyName": "Study 2",
        "studyType": "Yield study",
        "studyTypeDbId": "2",
        "studyTypeName": "Yield study",
        "trialDbId": "101",
        "trialName": "Peru Yield Trial 1"
      }
    ]
  }
}
 */

static json_t *ConvertGrassrootsStudyToBrapi (const json_t *grassroots_json_p)
{
	json_t *brapi_response_p = NULL;
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			brapi_response_p = json_object ();

			if (brapi_response_p)
				{
					bool success_flag = false;

					if (CopyJSONStringValue (grassroots_data_p, "so:name", brapi_response_p, "studyName"))
						{
							bson_oid_t id;

							if (GetMongoIdFromJSON (grassroots_data_p, &id))
								{
									char *id_s = GetBSONOidAsString (&id);

									if (id_s)
										{
											if (SetJSONString (brapi_response_p, "studyDbId", id_s))
												{
													if (SetStudyLocationData (grassroots_data_p, brapi_response_p))
														{
															if (SetStudyCurrentCrop (grassroots_data_p, brapi_response_p))
																{
																	SetStudyActivity (grassroots_data_p, brapi_response_p);

																	AddAdditionalInfo (grassroots_data_p, brapi_response_p);

																	AddParentTrialData (grassroots_data_p, brapi_response_p);

																	success_flag = true;
																}
														}
												}

											FreeCopiedString (id_s);
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


static bool SetStudyActivity (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;
	struct tm current_time;

	if (GetCurrentTime (&current_time))
		{
			const char *start_time_s = GetJSONString (grassroots_data_p, ST_SOWING_DATE_S);
			const char *end_time_s = GetJSONString (grassroots_data_p, ST_HARVEST_DATE_S);

			if (start_time_s)
				{
					struct tm *start_time_p = GetTimeFromString (start_time_s);

					if (start_time_p)
						{
							const char *active_s = NULL;
							struct tm *end_time_p = NULL;

							if (end_time_s)
								{
									end_time_p = GetTimeFromString (end_time_s);
								}

							if (CompareDates (&current_time, start_time_p, true) >= 0)
								{
									if ((!end_time_p) || (CompareDates (&current_time, end_time_p, true) <= 0))
										{
											active_s = "true";
										}
									else
										{
											active_s = "false";
										}
								}

							if (SetJSONString (brapi_response_p, "startDate", start_time_s))
								{
									if ((!end_time_s) || (SetJSONString (brapi_response_p, "endDate", end_time_s)))
										{
											if ((!active_s) || (SetJSONString (brapi_response_p, "active", active_s)))
												{

												}

										}		/* if ((!end_time_s) || (SetJSONString (brapi_response_p, "startDate", end_time_s))) */

								}		/* if (SetJSONString (brapi_response_p, "startDate", start_time_s)) */

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert \"%s\" to a time", start_time_s);
						}

				}		/* if (start_time_s) */

		}		/* if (GetCurrentTime (&current_time)) */

	return success_flag;
}


bool SetStudyLocationData (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;
	char *location_s = NULL;
	char *id_s = NULL;

	if (GetMinimalLocationData (grassroots_data_p, &location_s, &id_s))
		{
			if ((!id_s) || (SetJSONString (brapi_response_p, "locationName", location_s)))
				{
					if ((!location_s) || (SetJSONString (brapi_response_p, "locationDbId", id_s)))
						{
							success_flag = true;
						}
				}

			if (id_s)
				{
					FreeCopiedString (id_s);
				}

			if (location_s)
				{
					FreeCopiedString (location_s);
				}

		}


	return success_flag;
}


static bool SetStudyCurrentCrop (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;
	const json_t *crop_p = json_object_get (grassroots_data_p, ST_CURRENT_CROP_S);

	if (crop_p)
		{
			if (CopyJSONStringValue (crop_p, CR_NAME_S, brapi_response_p, "commonCropName"))
				{
					success_flag = true;
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddParentTrialData (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;
	const json_t *field_trial_p = json_object_get (grassroots_data_p, ST_PARENT_FIELD_TRIAL_S);

	if (field_trial_p)
		{
			if (CopyJSONStringValue (field_trial_p, FT_NAME_S, brapi_response_p, "trialName"))
				{
					bson_oid_t id;

					if (GetMongoIdFromJSON (field_trial_p, &id))
						{
							char *id_s = GetBSONOidAsString (&id);

							if (id_s)
								{
									if (SetJSONString (brapi_response_p, "trialDbId", id_s))
										{
											success_flag = true;
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, brapi_response_p, "Failed set trialDbId to \"%s\"", id_s);
										}

									FreeCopiedString (id_s);
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, field_trial_p, "Failed to copy MongoDB ID to string");
								}
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, field_trial_p, "Failed to get MongoDB ID");
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, field_trial_p, "Failed to copy field trial name \"%s\"", FT_NAME_S);
				}

		}		/* if (field_trial_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, grassroots_data_p, "No parent field trial \"%s\"", ST_PARENT_FIELD_TRIAL_S);
		}

	return success_flag;
}


static bool AddAdditionalInfo (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;
	json_t *additional_info_p = json_object ();

	if (additional_info_p)
		{

			if (!CopyJSONStringValue (grassroots_data_p, ST_DESIGN_S, additional_info_p, ST_DESIGN_S))
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, grassroots_data_p, "Failed to add design to additional info");

				}		/* if (!CopyJSONStringValue (grassroots_data_p, ST_DESIGN_S, additional_info_p, ST_DESIGN_S)) */

			if (!CopyJSONStringValue (grassroots_data_p, ST_DESCRIPTION_S, additional_info_p, ST_DESCRIPTION_S))
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, grassroots_data_p, "Failed to add description to additional info");
				}		/* if (!CopyJSONStringValue (grassroots_data_p, ST_DESCRIPTION_S, additional_info_p, ST_DESCRIPTION_S)) */

			if (!CopyJSONStringValue (grassroots_data_p, ST_GROWING_CONDITIONS_S, additional_info_p, ST_GROWING_CONDITIONS_S))
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, grassroots_data_p, "Failed to add growing conditiuns to additional info");
				}		/* if (!CopyJSONStringValue (grassroots_data_p, ST_GROWING_CONDITIONS_S, additional_info_p, ST_GROWING_CONDITIONS_S)) */

			if (!CopyJSONStringValue (grassroots_data_p, ST_PHENOTYPE_GATHERING_NOTES_S, additional_info_p, ST_PHENOTYPE_GATHERING_NOTES_S))
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, grassroots_data_p, "Failed to add phenotype gathering notes to additional info");
				}		/* if (!CopyJSONStringValue (grassroots_data_p, ST_PHENOTYPE_GATHERING_NOTES_S, additional_info_p, ST_PHENOTYPE_GATHERING_NOTES_S)) */


			if (json_object_size (additional_info_p) > 0)
				{
					if (json_object_set_new (brapi_response_p, "additionalInfo", additional_info_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							json_decref (additional_info_p);
						}
				}
			else
				{
					json_decref (additional_info_p);
					success_flag = true;
				}

		}		/* if (additional_info_p) */


	return success_flag;

}



static bool AddDataLinks (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;


	return success_flag;
}
