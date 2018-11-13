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

#include "brapi_module.h"


static json_t *ConvertGrassrootsStudyToBrapi (const json_t *grassroots_json_p);



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
					SharedType value;

					InitSharedType (&value);
					value.st_boolean_value = true;

					if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PT_BOOLEAN, "Get all Experimental Areas", NULL, NULL, value, PL_ALL))
						{
							struct tm *time_p = NULL;
							apr_pool_t *pool_p = req_p -> pool;
							const char *value_s = GetParameterValue (req_params_p, "active", pool_p);

							if (value_s && (strcmp (value_s, "true" == 0)))
								{

								}		/* if (value_s && (strcmp (value_s, "true" == 0))) */
							else if ((value_s = GetParameterValue (req_params_p, "active", pool_p)) && (strcmp (value_s, "true" == 0)))
								{

								}


							res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsStudyToBrapi);

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
      "totalCount": 3,
      "totalPages": 2
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
        "endDate": "2014-01-01",
        "locationDbId": "1",
        "locationName": "Location 1",
        "name": "Study 1",
        "programDbId": "1",
        "programName": "Program 1",
        "seasons": [
          "fall 2011",
          "winter 2012"
        ],
        "startDate": "2013-01-01",
        "studyDbId": "1001",
        "studyType": "Yield study",
        "trialDbId": "101",
        "trialName": "Peru Yield Trial 1"
      },
      {
        "active": "true",
        "additionalInfo": {
          "publications": "pmid:23643517318968"
        },
        "endDate": "2015-01-01",
        "locationDbId": "1",
        "locationName": "Location 1",
        "name": "Study 2",
        "programDbId": "1",
        "programName": "Program 1",
        "seasons": [
          "winter 2012"
        ],
        "startDate": "2014-01-01",
        "studyDbId": "1002",
        "studyType": "Yield study",
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


		}		/* if (grassroots_data_p) */


	return NULL;
}

