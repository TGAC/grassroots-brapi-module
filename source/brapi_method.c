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
 * brapi_method.c
 *
 *  Created on: 22 Jan 2020
 *      Author: billy
 */


#include "brapi_method.h"

#include "bson.h"
#include "mongodb_tool.h"

#include "schema_keys.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"
#include "time_util.h"

#include "brapi_module.h"

#include "streams.h"

#include "boolean_parameter.h"



static json_t *ConvertGrassrootsMethodToBrapi (const json_t *grassroots_json_p);


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

int IsMethodCall (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	int res = -1;
	const char *signature_s = "methods";
	size_t l = strlen (signature_s);

	if (strncmp (api_call_s, signature_s, l) == 0)
		{
			if (* (api_call_s + l) == '/')
				{
					const char *id_s = api_call_s + l;

					if (strlen (id_s) > 0)
						{
							ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

							if (params_p)
								{
									FreeParameterSet (params_p);
								}		/* if (params_p) */

						}

				}
			else
				{
					ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

					if (params_p)
						{
							bool value = true;

							if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, "Get all Locations", NULL, NULL, &value, PL_ALL))
								{
									params_p -> ps_current_level = PL_ADVANCED;
									res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsMethodToBrapi);
								}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

							FreeParameterSet (params_p);
						}		/* if (params_p) */
				}
		}
	else
		{
			res = 0;
		}


	return res;
}


static json_t *ConvertGrassrootsMethodToBrapi (const json_t *grassroots_json_p)
{
	json_t *brapi_json_p = NULL;


	return brapi_json_p;
}

