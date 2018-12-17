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


static json_t *ConvertGrassrootsStudyToBrapi (const json_t *grassroots_json_p);

static bool SetStudyActivity (const json_t *grassroots_data_p, json_t *brapi_response_p);

static bool CopyJSONStringValue (const json_t *src_p, const char *src_key_s, json_t *dest_p, const char *dest_key_s);

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
					bool success_flag = true;
					SharedType value;

					InitSharedType (&value);
					value.st_boolean_value = true;

					if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Search Experimental Areas", NULL, NULL, value, PL_ALL))
						{
							if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Get all Experimental Areas", NULL, NULL, value, PL_ALL))
								{
									apr_pool_t *pool_p = req_p -> pool;
									const char *active_s = GetParameterValue (req_params_p, "active", pool_p);
									const char *crop_name_s = GetParameterValue (req_params_p, "commonCropName", pool_p);

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
											res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsStudyToBrapi);
										}

								}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Search Experimental Areas", NULL, NULL, value, PL_ALL)) */


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
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			json_t *brapi_study_p = json_object ();

			if (brapi_study_p)
				{
					if (SetStudyActivity (grassroots_data_p, brapi_study_p))
						{
							return brapi_study_p;
						}		/* if (SetStudyActivity (grassroots_data_p, brapi_study_p)) */

					json_decref (brapi_study_p);
				}		/* if (brapi_study_p) */

		}		/* if (grassroots_data_p) */

	return NULL;
}


static bool SetStudyActivity (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;
	struct tm current_time;

	if (GetCurrentTime (&current_time))
		{
			const char *start_time_s = GetJSONString (grassroots_data_p, "sowing_date");

			if (start_time_s)
				{
					struct tm *start_time_p = GetTimeFromString (start_time_s);

					if (start_time_p)
						{
							const char *active_s = NULL;
							struct tm *end_time_p = NULL;
							const char *end_time_s;

							end_time_s = GetJSONString (grassroots_data_p, "harvest_date");

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
													if (CopyJSONStringValue (grassroots_data_p, "so:name", brapi_response_p, "studyName"))
														{
															success_flag = true;
														}
												}

										}		/* if ((!end_time_s) || (SetJSONString (brapi_response_p, "startDate", end_time_s))) */

								}		/* if (SetJSONString (brapi_response_p, "startDate", start_time_s)) */

						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to convert \"%s\" to a time", start_time_s);
						}

				}		/* if (time_s) */

		}		/* if (GetCurrentTime (&current_time)) */

	return success_flag;
}


static bool AddDataLinks (const json_t *grassroots_data_p, json_t *brapi_response_p)
{
	bool success_flag = false;


	return success_flag;
}


static bool CopyJSONStringValue (const json_t *src_p, const char *src_key_s, json_t *dest_p, const char *dest_key_s)
{
	bool success_flag = false;
	const char *value_s = GetJSONString (src_p, src_key_s);

	if (value_s)
		{
			if (SetJSONString (dest_p, dest_key_s, value_s))
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


