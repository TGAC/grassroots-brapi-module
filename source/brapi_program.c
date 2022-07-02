/*
 * brapi_program.c
 *
 *  Created on: 2 Jul 2022
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

#include "programme.h"
#include "programme_jobs.h"

#include "boolean_parameter.h"
#include "string_parameter.h"


static json_t *ConvertGrassrootsProgrammeToBrapi (const json_t *grassroots_json_p);

static bool AddAbbreviation (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddAdditionalInfo (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddCommonCropName (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddDocumentationURL (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddExternalReferences (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddFundingInformation (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

/*
 * "leadPersonDbId",
 * "leadPersonName",
 *
 */
static bool AddLeadPerson (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddObjective (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddProgrammeDbId (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddProgrammeName (const json_t *grassroots_programme_p, json_t *brapi_programme_p);

static bool AddProgrammeType (const json_t *grassroots_programme_p, json_t *brapi_programme_p);




int IsProgramCall (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p)
{
	int res = 0;
	const char *signature_s = "programs";
	const char *id_s = NULL;

	if (strcmp (api_call_s, signature_s) == 0)
		{
			id_s = "*";
		}
	else
		{
			signature_s = "programs/";
			size_t l = strlen (signature_s);

			if (strncmp (api_call_s, signature_s, l) == 0)
				{
					const char *programme_id_s = api_call_s + l;

					if (strlen (programme_id_s) > 0)
						{
							id_s = programme_id_s;
						}
					else
						{
							res = -1;
						}
				}
		}

	if (id_s)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = -1;

			if (params_p)
				{
					bool success_flag = true;
					const char *all_ids_s = "*";

					if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, PROGRAMME_SEARCH.npt_type, PROGRAMME_SEARCH.npt_name_s, NULL, NULL, all_ids_s, PL_ALL))
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
									res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsProgrammeToBrapi);
								}


						}		/* if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, PROGRAMME_SEARCH.npt_type, PROGRAMME_SEARCH.npt_name_s, NULL, NULL, all_ids_s, PL_ALL)) */

					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}		/* if (id_s) */
	else
		{
			res = 0;
		}

	return res;
}


static json_t *ConvertGrassrootsProgrammeToBrapi (const json_t *grassroots_json_p)
{
	json_t *brapi_response_p = NULL;
	const json_t *grassroots_data_p = json_object_get (grassroots_json_p, RESOURCE_DATA_S);

	if (grassroots_data_p)
		{
			brapi_response_p = json_object ();

			if (brapi_response_p)
				{
					bool success_flag = false;

					if (AddAbbreviation (grassroots_data_p, brapi_response_p))
						{
							if (AddAdditionalInfo (grassroots_data_p, brapi_response_p))
								{
									if (AddCommonCropName (grassroots_data_p, brapi_response_p))
										{
											if (AddDocumentationURL (grassroots_data_p, brapi_response_p))
												{
													if (AddExternalReferences (grassroots_data_p, brapi_response_p))
														{
															if (AddFundingInformation (grassroots_data_p, brapi_response_p))
																{
																	if (AddLeadPerson (grassroots_data_p, brapi_response_p))
																		{
																			if (AddObjective (grassroots_data_p, brapi_response_p))
																				{
																					if (AddProgrammeDbId (grassroots_data_p, brapi_response_p))
																						{
																							if (AddProgrammeName (grassroots_data_p, brapi_response_p))
																								{
																									if (AddProgrammeType (grassroots_data_p, brapi_response_p))
																										{
																											return brapi_response_p;
																										}		/* if (AddProgrammeType (grassroots_data_p, brapi_response_p)) */

																								}		/* if (AddProgrammeName (grassroots_data_p, brapi_response_p)) */

																						}		/* if (AddProgrammeDbId (grassroots_data_p, brapi_response_p)) */

																				}		/* if (AddObjective (grassroots_data_p, brapi_response_p)) */

																		}		/* if (AddLeadPerson (grassroots_data_p, brapi_response_p)) */

																}		/* if (AddFundingInformation (grassroots_data_p, brapi_response_p)) */

														}		/* if (AddExternalReferences (grassroots_data_p, brapi_response_p)) */

												}		/* if (AddDocumentationURL (grassroots_data_p, brapi_response_p)) */

										}		/* if (AddCommonCropName (grassroots_data_p, brapi_response_p)) */

								}		/* if (AddAdditionalInfo (grassroots_data_p, brapi_response_p)) */

						}		/* if (AddAbbreviation (grassroots_data_p, brapi_response_p)) */

					json_decref (brapi_response_p);
				}		/* if (brapi_response_p) */

		}		/* if (grassroots_data_p) */

	return NULL;
}


static bool AddAbbreviation (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_programme_p, PR_ABBREVIATION_S, brapi_programme_p, "abbreviation"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddAdditionalInfo (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_programme_p, "additionalInfo"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddCommonCropName (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	return success_flag;
}


static bool AddDocumentationURL (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_programme_p, PR_DOCUMENTATION_URL_S, brapi_programme_p, "documentationURL"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddExternalReferences (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_programme_p, "externalReferences"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddFundingInformation (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_programme_p, "fundingInformation"))
		{
			success_flag = true;
		}

	return success_flag;
}


/*
 * "leadPersonDbId",
 * "leadPersonName",
 *
 */
static bool AddLeadPerson (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;
	const json_t *pi_p = json_object_get (grassroots_programme_p, PR_PI_S);

	if (pi_p)
		{
			if (CopyJSONStringValue (pi_p, PE_NAME_S, brapi_programme_p, "leadPersonName"))
				{
					if (CopyJSONStringValue (pi_p, PE_EMAIL_S, brapi_programme_p, "leadPersonDbId"))
						{
							success_flag = true;
						}
				}
		}		/* if (pi_p) */

	return success_flag;
}


static bool AddObjective (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_programme_p, PR_OBJECTIVE_S, brapi_programme_p, "objective"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddProgrammeDbId (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	bson_oid_t id;

	if (GetMongoIdFromJSON (grassroots_programme_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_programme_p, "programDbId", id_s))
						{
							success_flag = true;
						}

					FreeBSONOidString (id_s);
				}
		}

	return success_flag;
}



static bool AddProgrammeName (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_programme_p, PR_NAME_S, brapi_programme_p, "programmeName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddProgrammeType (const json_t *grassroots_programme_p, json_t *brapi_programme_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_programme_p, "programType"))
		{
			success_flag = true;
		}

	return success_flag;
}



