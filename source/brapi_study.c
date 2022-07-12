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


#define ALLOCATE_STUDY_TAGS (1)
#include "study.h"

#define ALLOCATE_STUDY_JOB_CONSTANTS (1)
#include "study_jobs.h"

#include "streams.h"

#include "boolean_parameter.h"
#include "time_parameter.h"

#include "apr_strings.h"



static json_t *ConvertGrassrootsStudyToBrapi (const json_t *grassroots_json_p, request_rec *req_p, ModBrapiConfig *config_p);

static APIStatus DoStudySearch (const char *id_s, request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p);

static bool AddActive (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddAdditionalInfo (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddCommonCropName (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddContacts (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddDataLinks (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddDocumentationURL (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddEndDate (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddEnvironmentParameters (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddExperimentalDesign (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddExternalReferences (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddGrowthFacility (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddLastUpdate (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddLicense (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddLocationDetails (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddObservationLevels (const json_t *grassroots_study_p, json_t *brapi_study_p);

static char *GetAndAddStudyDbId (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddObservationVariableDbIds (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddSeasons(const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddStartDate (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddStudyCode (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddStudyDbId (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddStudyDescription (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddStudyName (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddStudyPUI (const json_t *grassroots_study_p, json_t *brapi_study_p, const char *study_id_s, request_rec *req_p, ModBrapiConfig *config_p);

static bool AddStudyType (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddTrialDbId (const json_t *grassroots_trial_p, json_t *brapi_study_p);

static bool AddTrialName (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddTrialDetails (const json_t *grassroots_study_p, json_t *brapi_study_p);

static bool AddPerson (const json_t *grassroots_study_p, const char * const grassroots_key_s, json_t *brapi_contacts_p, const char * const brapi_type_s);

static bool AddDate (const json_t *grassroots_study_p, const char * const grassroots_key_s, json_t *brapi_study_p, const char *brapi_key_s);



APIStatus GetAllStudies (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p)
{
	APIStatus res = AS_IGNORED;
	const char *signature_s = "studies";

	if (strcmp (api_call_s, signature_s) == 0)
		{
			ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

			res = AS_FAILED;

			if (params_p)
				{
					bool value = true;

					if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, STUDY_SEARCH_STUDIES.npt_name_s, NULL, NULL, &value, PL_ALL))
						{
							apr_pool_t *pool_p = req_p -> pool;
							const char *active_s = GetParameterValue (req_params_p, "active", pool_p);
							const char *crop_name_s = GetParameterValue (req_params_p, "commonCropName", pool_p);


							params_p -> ps_current_level = PL_ADVANCED;
							res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsStudyToBrapi);

						}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, STUDY_SEARCH_STUDIES.npt_type, STUDY_SEARCH_STUDIES.npt_name_s, NULL, NULL, value, PL_ALL)) */

					FreeParameterSet (params_p);
				}		/* if (params_p) */

		}

	return res;
}


APIStatus GetStudyByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p)
{
	APIStatus res = AS_IGNORED;

	const char *signature_s = "studies/";
	size_t l = strlen (signature_s);

	if (strncmp (api_call_s, signature_s, l) == 0)
		{
			const char *id_s = api_call_s + l;

			if (strlen (id_s) > 0)
				{
					ParameterSet *params_p = AllocateParameterSet (NULL, NULL);

					res = -1;

					if (params_p)
						{
							bool value = true;

							if (EasyCreateAndAddBooleanParameterToParameterSet (NULL, params_p, NULL, STUDY_SEARCH_STUDIES.npt_name_s, NULL, NULL, &value, PL_ALL))
								{
									if (EasyCreateAndAddStringParameterToParameterSet (NULL, params_p, NULL, STUDY_ID.npt_type, STUDY_ID.npt_name_s, NULL, NULL, id_s, PL_ALL))
										{
											params_p -> ps_current_level = PL_ADVANCED;
											res = DoGrassrootsCall (req_p, params_p, ConvertGrassrootsStudyToBrapi);
										}		/* if (EasyCreateAndAddParameterToParameterSet (NULL, params_p, NULL, PA_TYPE_BOOLEAN_S, "Get all Locations", NULL, NULL, value, PL_ALL)) */
								}

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



static json_t *ConvertGrassrootsStudyToBrapi (const json_t *grassroots_json_p, request_rec *req_p, ModBrapiConfig *config_p)
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
													if (AddDataLinks (grassroots_data_p, brapi_response_p))
														{
															if (AddDocumentationURL (grassroots_data_p, brapi_response_p))
																{
																	if (AddEndDate (grassroots_data_p, brapi_response_p))
																		{
																			if (AddEnvironmentParameters (grassroots_data_p, brapi_response_p))
																				{
																					if (AddExperimentalDesign (grassroots_data_p, brapi_response_p))
																						{
																							if (AddExternalReferences (grassroots_data_p, brapi_response_p))
																								{
																									if (AddGrowthFacility (grassroots_data_p, brapi_response_p))
																										{
																											if (AddLastUpdate (grassroots_data_p, brapi_response_p))
																												{
																													if (AddLicense (grassroots_data_p, brapi_response_p))
																														{
																															if (AddLocationDetails (grassroots_data_p, brapi_response_p))
																																{
																																	if (AddObservationLevels (grassroots_data_p, brapi_response_p))
																																		{
																																			char *id_s = GetAndAddStudyDbId (grassroots_data_p, brapi_response_p);

																																			if (id_s)
																																				{
																																					if (AddObservationVariableDbIds (grassroots_data_p, brapi_response_p))
																																						{
																																							if (AddSeasons (grassroots_data_p, brapi_response_p))
																																								{
																																									if (AddStartDate (grassroots_data_p, brapi_response_p))
																																										{
																																											if (AddStudyCode (grassroots_data_p, brapi_response_p))
																																												{
																																													if (AddStudyDbId (grassroots_data_p, brapi_response_p))
																																														{
																																															if (AddStudyDescription (grassroots_data_p, brapi_response_p))
																																																{
																																																	if (AddStudyName (grassroots_data_p, brapi_response_p))
																																																		{
																																																			if (AddStudyPUI (grassroots_data_p, brapi_response_p, id_s, req_p, config_p))
																																																				{
																																																					if (AddStudyType (grassroots_data_p, brapi_response_p))
																																																						{
																																																							if (AddTrialDetails (grassroots_data_p, brapi_response_p))
																																																								{
																																																									return brapi_response_p;

																																																								}		/* if (AddTrialDetails (grassroots_data_p, brapi_response_p)) */

																																																						}		/* if (AddStudyType (grassroots_data_p, brapi_response_p)) */

																																																				}		/* if (AddStudyPUI (grassroots_data_p, brapi_response_p)) */

																																																		}		/* if (AddStudyName (grassroots_data_p, brapi_response_p)) */

																																																}		/* if (AddStudyDescription (grassroots_data_p, brapi_response_p)) */

																																														}		/* if (AddStudyDbId (grassroots_data_p, brapi_response_p)) */

																																												}		/* if (AddStudyCode (grassroots_data_p, brapi_response_p)) */

																																										}		/* if (AddStartDate (grassroots_data_p, brapi_response_p)) */

																																								}		/* if (AddSeasons (grassroots_data_p, brapi_response_p)) */

																																						}		/* if (AddObservationVariableDbIds (grassroots_data_p, brapi_response_p)) */

																																					FreeBSONOidString (id_s);
																																				}		/* if (id_s) */

																																		}		/* if (AddObservationLevels (grassroots_data_p, brapi_response_p)) */

																																}		/* if (AddLocationDetails (grassroots_data_p, brapi_response_p)) */

																														}		/* if (AddLicense (grassroots_data_p, brapi_response_p)) */

																												}		/* if (AddLastUpdate (grassroots_data_p, brapi_response_p)) */

																										}		/* if (AddGrowthFacility (grassroots_data_p, brapi_response_p)) */

																								}		/* if (AddExternalReferences (grassroots_data_p, brapi_response_p)) */

																						}		/* if (AddExperimentalDesign (grassroots_data_p, brapi_response_p)) */

																				}		/* if (AddEnvironmentParameters (grassroots_data_p, brapi_response_p)) */

																		}		/* if (AddEndDate (grassroots_data_p, brapi_response_p)) */

																}		/* if (AddDocumentationURL (grassroots_data_p, brapi_response_p)) */

														}		/* if (AddDataLinks (grassroots_data_p, brapi_response_p)) */

												}		/* if (AddContacts (grassroots_data_p, brapi_response_p)) */

										}		/* if (AddCommonCropName (grassroots_data_p, brapi_response_p)) */

								}		/* if (AddAdditionalInfo (grassroots_data_p, brapi_response_p)) */

						}		/* if (AddActive (grassroots_data_p, brapi_response_p)) */

					json_decref (brapi_response_p);
				}		/* if (brapi_response_p) */

		}		/* if (grassroots_data_p) */

	return NULL;
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

									FreeBSONOidString (id_s);
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


static bool AddAdditionalInfo (const json_t *grassroots_data_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "additionalInfo"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddActive (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;
	struct tm current_time;

	if (GetCurrentTime (&current_time))
		{
			const uint32 current_year = ((uint32) current_time.tm_year) + 1900;
			uint32 sowing_year = 0;
			const char * const KEY_S = "active";

			if (GetJSONUnsignedInteger (grassroots_study_p, ST_SOWING_YEAR_S, &sowing_year))
				{
					bool active_flag = false;

					if (current_year >= sowing_year)
						{
							uint32 harvest_year = 0;

							if (GetJSONUnsignedInteger (grassroots_study_p, ST_HARVEST_YEAR_S, &harvest_year))
								{
									if (current_year <= harvest_year)
										{
											active_flag = true;
										}
								}
							else
								{
									active_flag = true;
								}

						}

					if (SetJSONBoolean (brapi_study_p, KEY_S, active_flag))
						{
							success_flag = true;
						}
				}
			else
				{
					if (SetJSONNull (brapi_study_p, KEY_S))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}




static bool AddCommonCropName (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;
	const json_t *crop_p = json_object_get (grassroots_study_p, ST_CURRENT_CROP_S);

	if (crop_p)
		{
			if (CopyJSONStringValue (crop_p, CR_NAME_S, brapi_study_p, "commonCropName"))
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


static bool AddContacts (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	json_t *contacts_p = json_array ();

	if (contacts_p)
		{
			if (AddPerson (grassroots_study_p, ST_CURATOR_S, contacts_p, "curator"))
				{
					if (AddPerson (grassroots_study_p, ST_CONTACT_S, contacts_p, "contact"))
						{
							if (json_object_set_new (brapi_study_p, "contacts", contacts_p) == 0)
								{
									return true;
								}
						}

				}

			json_decref (contacts_p);
		}

	return false;
}


static bool AddDataLinks (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "dataLinks"))
		{
			success_flag = true;
		}

	return success_flag;

}

static bool AddDocumentationURL (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_study_p, ST_DATA_LINK_S, brapi_study_p, "documentationURL"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddEndDate (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	return AddDate (grassroots_study_p, ST_HARVEST_YEAR_S, brapi_study_p, "endDate");
}


static bool AddEnvironmentParameters (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "environmentParameters"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddExperimentalDesign (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_study_p, ST_DESIGN_S, brapi_study_p, "experimentalDesign"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddExternalReferences (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "externalReferences"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddGrowthFacility (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "growthFacility"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddLastUpdate (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "lastUpdate"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddLicense (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	return SetJSONString (brapi_study_p, "license", "toronto");
}



/*
 * locationDbId and locationName
 */
static bool AddLocationDetails (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;
	const char * const ID_KEY_S = "locationDbId";
	const char * const NAME_KEY_S = "locationName";
	const char *name_s = NULL;
	char *id_s = NULL;
	const json_t *address_p = json_object_get (grassroots_study_p, ST_LOCATION_S);

	if (address_p)
		{
			bson_oid_t id;

			if (GetMongoIdFromJSON (address_p, &id))
				{
					id_s = GetBSONOidAsString (&id);

					if (id_s)
						{
							if (SetJSONString (brapi_study_p, ID_KEY_S, id_s))
								{
									if (CopyJSONStringValue (address_p, LO_NAME_S, brapi_study_p, NAME_KEY_S))
										{
											success_flag = true;
										}
								}

							FreeBSONOidString (id_s);
						}
				}

		}		/* if (address_p) */
	else
		{
			if (SetJSONNull (brapi_study_p, ID_KEY_S))
				{
					if (SetJSONNull (brapi_study_p, NAME_KEY_S))
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


static bool AddObservationLevels (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "observationLevels"))
		{
			success_flag = true;
		}

	return success_flag;
}


static char *GetAndAddStudyDbId (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bson_oid_t id;

	if (GetMongoIdFromJSON (grassroots_study_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_study_p, "studyDbId", id_s))
						{
							return id_s;
						}

					FreeBSONOidString (id_s);
				}
		}

	return NULL;
}


static bool AddObservationVariableDbIds (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "observationVariableDbIds"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddSeasons(const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "seasons"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddStartDate (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	return AddDate (grassroots_study_p, ST_SOWING_YEAR_S, brapi_study_p, "startDate");
}


static bool AddStudyCode (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "studyCode"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddStudyDbId (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bson_oid_t id;

	if (GetMongoIdFromJSON (grassroots_study_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_study_p, "studyDbId", id_s))
						{
							return id_s;
						}

					FreeBSONOidString (id_s);
				}
		}

	return NULL;
}


static bool AddStudyDescription (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_study_p, ST_DESCRIPTION_S, brapi_study_p, "studyDescription"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddStudyName (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_study_p, ST_NAME_S, brapi_study_p, "studyName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddStudyPUI (const json_t *grassroots_study_p, json_t *brapi_study_p, const char *study_id_s, request_rec *req_p, ModBrapiConfig *config_p)
{
	bool success_flag = false;
	char *url_s = NULL;
	const char * const key_s = "studyPUI";

	if ((config_p -> mbc_grassroots_frontend_url_s) && (config_p -> mbc_grassroots_frontend_studies_path_s))
		{
			url_s = apr_pstrcat (req_p -> pool, config_p -> mbc_grassroots_frontend_url_s, config_p -> mbc_grassroots_frontend_studies_path_s, study_id_s, NULL);
		}

	if (url_s)
		{
			if (SetJSONString (brapi_study_p, key_s, url_s))
				{
					success_flag = true;
				}
		}
	else
		{
			if (SetJSONNull (brapi_study_p, key_s))
				{
					success_flag = true;
				}
		}

	return success_flag;

}

static bool AddStudyType (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (SetJSONNull (brapi_study_p, "studyType"))
		{
			success_flag = true;
		}

	return success_flag;
}



static bool AddTrialDetails (const json_t *grassroots_study_p, json_t *brapi_study_p)
{
	const json_t *grassroots_trial_p = json_object_get (grassroots_study_p, ST_PARENT_FIELD_TRIAL_S);

	if (grassroots_trial_p)
		{
			if (AddTrialDbId (grassroots_trial_p, brapi_study_p))
				{
					if (AddTrialName (grassroots_trial_p, brapi_study_p))
						{
							return true;
						}
				}
		}

	return false;
}



static bool AddTrialDbId (const json_t *grassroots_trial_p, json_t *brapi_study_p)
{
	bool success_flag = false;
	bson_oid_t id;

	if (GetMongoIdFromJSON (grassroots_trial_p, &id))
		{
			char *id_s = GetBSONOidAsString (&id);

			if (id_s)
				{
					if (SetJSONString (brapi_study_p, "trialDbId", id_s))
						{
							success_flag  = true;
						}

					FreeBSONOidString (id_s);
				}
		}

	return success_flag;
}


static bool AddTrialName (const json_t *grassroots_trial_p, json_t *brapi_study_p)
{
	bool success_flag = false;

	if (CopyJSONStringValue (grassroots_trial_p, FT_NAME_S, brapi_study_p, "trialName"))
		{
			success_flag = true;
		}

	return success_flag;
}


static bool AddPerson (const json_t *grassroots_study_p, const char * const grassroots_key_s, json_t *brapi_contacts_p, const char * const brapi_type_s)
{
	bool success_flag = false;
	const json_t *grassroots_person_p = json_object_get (grassroots_study_p, grassroots_key_s);

	if (grassroots_person_p)
		{
			const char *name_s = GetJSONString (grassroots_person_p, PE_NAME_S);

			if (name_s)
				{
					const char *email_s = GetJSONString (grassroots_person_p, PE_EMAIL_S);

					if (email_s)
						{
							json_t *brapi_person_p = json_object ();

							if (brapi_person_p)
								{
									if (SetJSONNull (brapi_person_p, "contactDbId"))
										{
											if (SetJSONString (brapi_person_p, "email", email_s))
												{
													if (SetJSONNull (brapi_person_p, "instituteName"))
														{
															if (SetJSONString (brapi_person_p, "name", name_s))
																{
																	if (SetJSONNull (brapi_person_p, "orcid"))
																		{
																			if (SetJSONNull (brapi_person_p, brapi_type_s))
																				{
																					if (json_array_append_new (brapi_contacts_p, brapi_person_p) == 0)
																						{
																							success_flag = true;
																						}

																				}
																		}
																}

														}
												}

										}

									if (!success_flag)
										{
											json_decref (brapi_person_p);
										}
								}
						}
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}



static bool AddDate (const json_t *grassroots_study_p, const char * const grassroots_key_s, json_t *brapi_study_p, const char *brapi_key_s)
{
	bool success_flag = false;
	uint32 date = 0;


	if (GetJSONUnsignedInteger (grassroots_study_p, grassroots_key_s, &date))
		{
			if (SetJSONInteger (brapi_study_p, brapi_key_s, date))
				{
					success_flag = true;
				}
		}
	else
		{
			if (SetJSONNull (brapi_study_p, brapi_key_s))
				{
					success_flag = true;
				}

		}

	return success_flag;
}

