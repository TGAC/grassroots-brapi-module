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

#define ALLOCATE_FIELD_TRIAL_TAGS (1)
#include "field_trial.h"

#define ALLOCATE_FIELD_TRIAL_CONSTANTS (1)
#include "field_trial_jobs.h"

#include "programme.h"

#include "boolean_parameter.h"
#include "string_parameter.h"

#include "apr_strings.h"


static json_t *ConvertGrassrootsTrialToBrapi (const json_t *grassroots_json_p, request_rec *req_p, ModBrapiConfig *config_p);

static APIStatus DoTrialsSearch (const char *id_s, request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p);

static bool AddActive (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddAdditionalInfo (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddCommonCropName (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddContacts (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddDatasetAuthorships (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddDocumentationURL (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddEndDate (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddExternalReferences (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddProgrammeDbId (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddProgrammeName (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddPublications (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddStartDate (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static char *GetAndAddTrialDbId (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddTrialDescription (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddTrialName (const json_t *grassroots_trial_p, json_t *brapi_trial_p);

static bool AddTrialPUI (const json_t *grassroots_trial_p, json_t *brapi_trial_p, const char *id_s, request_rec *req_p, ModBrapiConfig *config_p);



APIStatus GetAllTrials (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p)
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



APIStatus GetTrialByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p)
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


static json_t *ConvertGrassrootsTrialToBrapi (const json_t *grassroots_json_p, request_rec *req_p, ModBrapiConfig *config_p)
{
	json_t *brapi_response_p = NULL;
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			brapi_response_p = json_object ();

			if (brapi_response_p)
				{
					if (AddActive (grassroots_data_p, brapi_response_p))
						{
							if (AddAdditionalInfo (grassroots_data_p, brapi_response_p))
								{
									if (AddCommonCropName (grassroots_data_p, brapi_response_p))
										{
											if (AddContacts (grassroots_data_p, brapi_response_p))
												{
													if (AddDatasetAuthorships (grassroots_data_p, brapi_response_p))
														{
															if (AddDocumentationURL (grassroots_data_p, brapi_response_p))
																{
																	if (AddEndDate (grassroots_data_p, brapi_response_p))
																		{
																			if (AddExternalReferences (grassroots_data_p, brapi_response_p))
																				{
																					const json_t *parent_programme_p = json_object_get (grassroots_data_p, FT_PARENT_PROGRAM_S);

																					if (parent_programme_p)
																						{
																							if (AddProgrammeDbId (parent_programme_p, brapi_response_p))
																								{
																									if (AddProgrammeName (parent_programme_p, brapi_response_p))
																										{
																											if (AddPublications (grassroots_data_p, brapi_response_p))
																												{
																													if (AddStartDate (grassroots_data_p, brapi_response_p))
																														{
																															char *id_s = GetAndAddTrialDbId (grassroots_data_p, brapi_response_p);

																															if (id_s)
																																{
																																	if (AddTrialName (grassroots_data_p, brapi_response_p))
																																		{
																																			if (AddTrialDescription (grassroots_data_p, brapi_response_p))
																																				{
																																					if (AddTrialPUI (grassroots_data_p, brapi_response_p, id_s, req_p, config_p))
																																						{
																																							return brapi_response_p;
																																						}

																																				}		/* if (AddTrialDescription (grassroots_data_p, brapi_response_p)) */

																																		}		/* if (AddTrialName (grassroots_data_p, brapi_response_p)) */

																																	FreeBSONOidString (id_s);
																																}		/* if (id_s) */

																														}		/* if (AddStartDate (grassroots_data_p, brapi_response_p)) */

																												}		/* if (AddPublications (grassroots_data_p, brapi_response_p)) */

																										}		/* if (AddProgrammeName (grassroots_data_p, brapi_response_p)) */

																								}		/* if (AddProgrammeDbId (grassroots_data_p, brapi_response_p)) */

																						}		/* if (parent_programme_p) */


																				}		/* if (AddExternalReferences (grassroots_data_p, brapi_response_p)) */

																		}		/* if (AddEndDate (grassroots_data_p, brapi_response_p)) */

																}		/* if (AddDocumentationURL (grassroots_data_p, brapi_response_p)) */

														}		/* if (AddDatasetAuthorships (grassroots_data_p, brapi_response_p)) */

												}		/* if (AddContacts (grassroots_data_p, brapi_response_p)) */

										}		/* if (AddCommonCropName (grassroots_data_p, brapi_response_p)) */

								}		/* if (AddAdditionalInfo (grassroots_data_p, brapi_response_p)) */

						}		/* if (AddActive (grassroots_data_p, brapi_response_p)) */

					json_decref (brapi_response_p);
				}		/* if (brapi_response_p) */

		}		/* if (grassroots_data_p) */

	return NULL;
}




static bool AddActive (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "active"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddAdditionalInfo (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "additionalInfo"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddCommonCropName (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "commonCropName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddContacts (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "contacts"))
		{
			success_flag = true;
		}

	return success_flag;
}



static bool AddDatasetAuthorships (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "datasetAuthorships"))
		{
			success_flag = true;
		}

	return success_flag;

}

static bool AddDocumentationURL (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "documentationURL"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddExternalReferences (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "externalReferences"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddEndDate (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "endDate"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddProgrammeDbId (const json_t *parent_programme_p, json_t *brapi_trial_p)
{
	bool success_flag = false;
	bson_oid_t id;

	if (GetMongoIdFromJSON (parent_programme_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_trial_p, "programmeDbId", id_s))
						{
							success_flag = true;
						}

					FreeBSONOidString (id_s);
				}
		}

	return success_flag;
}

static bool AddProgrammeName (const json_t *parent_programme_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (parent_programme_p, PR_NAME_S, brapi_trial_p, "programmeName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddPublications (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "publications"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddStartDate (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "startDate"))
		{
			success_flag = true;
		}

	return success_flag;
}


static char *GetAndAddTrialDbId (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bson_oid_t id;

	if (GetMongoIdFromJSON (grassroots_trial_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_trial_p, "trialDbId", id_s))
						{
							return id_s;
						}

					FreeBSONOidString (id_s);
				}
		}

	return NULL;
}


static bool AddTrialDescription (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_trial_p, "trialDescription"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddTrialName (const json_t *grassroots_trial_p, json_t *brapi_trial_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_trial_p, FT_NAME_S, brapi_trial_p, "trialName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddTrialPUI (const json_t *grassroots_trial_p, json_t *brapi_trial_p, const char *trial_id_s, request_rec *req_p, ModBrapiConfig *config_p)
{
	bool success_flag = false;
	char *url_s = NULL;

	if ((config_p -> mbc_grassroots_frontend_url_s) && (config_p -> mbc_grassroots_frontend_trials_path_s))
		{
			url_s = apr_pstrcat (req_p -> pool, config_p -> mbc_grassroots_frontend_url_s, config_p -> mbc_grassroots_frontend_trials_path_s, trial_id_s, NULL);
		}

	if (url_s)
		{
			if (SetJSONString (brapi_trial_p, "trialPUI", url_s))
				{
					success_flag = true;
				}
		}
	else
		{
			if (SetJSONNull (brapi_trial_p, "trialPUI"))
				{
					success_flag = true;
				}
		}

	return success_flag;
}
