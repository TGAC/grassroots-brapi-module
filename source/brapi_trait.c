/*
** Copyright 2014-2020 The Earlham Institute
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
 * brapi_trait.c
 *
 *  Created on: 23 Jan 2020
 *      Author: billy
 */


#include "brapi_trait.h"
#include "brapi_module.h"

#include "parameter_set.h"

#include "treatment_jobs.h"

#include "string_parameter.h"
#include "boolean_parameter.h"


static json_t *ConvertGrassrootsTraitToBrapi (const json_t *grassroots_json_p);




int IsTraitCall (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	int res = -1;
	const char *signature_s = "traits";
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
									if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, TR_TRAIT_ID.npt_type, TR_TRAIT_ID.npt_name_s, NULL, NULL, id_s, PL_ALL))
										{
											params_p -> ps_current_level = PL_ADVANCED;
											res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsTraitToBrapi);
										}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */

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
									res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsTraitToBrapi);
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




static json_t *ConvertGrassrootsTraitToBrapi (const json_t *grassroots_json_p)
{
	json_t *brapi_response_p = NULL;

	return brapi_response_p;
}
