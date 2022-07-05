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
 * brapi_module.c
 *
 *  Created on: 9 Nov 2018
 *      Author: billy
 */

#include "httpd.h"
#include "util_script.h"
#include "apr_escape.h"

#include "brapi_module.h"
#include "mongodb_tool.h"

#include "brapi_location.h"
#include "brapi_program.h"
#include "brapi_study.h"
#include "brapi_trait.h"
#include "brapi_trial.h"

#include "parameter_set.h"
#include "connection.h"
#include "json_tools.h"
#include "operation.h"
#include "json_util.h"
#include "string_utils.h"


/*
 * Ensure field trial named constants are defined
 */
#define	ALLOCATE_CROP_TAGS (1)
#define	ALLOCATE_FIELD_TRIAL_TAGS (1)
#define ALLOCATE_FIELD_TRIAL_CONSTANTS (1)
#define	ALLOCATE_STUDY_TAGS (1)
#define ALLOCATE_STUDY_JOB_CONSTANTS (1)
#define ALLOCATE_LOCATION_JOB_CONSTANTS (1)
#define ALLOCATE_MEASURED_VARIABLE_CONSTANTS (1)



#include "crop.h"
#include "field_trial.h"
#include "field_trial_jobs.h"
#include "study.h"
#include "study_jobs.h"
#include "location.h"
#include "location_jobs.h"
#include "measured_variable_jobs.h"


typedef APIStatus (*process_req_fn) (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p);


/*
 * STATIC FUNCTION DECLARATIONS
 */

static int BrapiHandler (request_rec *req_p);

static void RegisterHooks (apr_pool_t *pool_p);


static void *MergeGrassrootsBrapiDirectoryConfig (apr_pool_t *pool_p, void *base_config_p, void *new_config_p);


static void *MergeGrassrootsBrapiServerConfig (apr_pool_t *pool_p, void *base_config_p, void *vhost_config_p);


static void *CreateGrassrootsBrapiServerConfig (apr_pool_t *pool_p, server_rec *server_p);


static void *CreateGrassrootsBrapiDirectoryConfig (apr_pool_t *pool_p, char *context_s);


static ModBrapiConfig *CreateConfig (apr_pool_t *pool_p, server_rec *server_p);


static const char *SetGrassrootsURL (cmd_parms *cmd_p, void *cfg_p, const char *arg_s);


static json_t *CreateMetadataResponse (const size_t current_page, const size_t page_size, const size_t total_count, const size_t total_pages);


/*
 * STATIC VARIABLES
 */

static const command_rec s_grassroots_brapi_directives [] =
		{
				AP_INIT_TAKE1 ("GrassrootsURL", SetGrassrootsURL, NULL, ACCESS_CONF, "The url for the Grassroots controller to call"),
				{ NULL }
		};


static const char * const S_BRAPI_API_S = "/brapi/v1/";


/*
 * GRASSROOTS BRAPI MODULE
 */


/* Define our module as an entity and assign a function for registering hooks  */
module AP_MODULE_DECLARE_DATA grassroots_brapi_module =
		{
				STANDARD20_MODULE_STUFF,
				CreateGrassrootsBrapiDirectoryConfig,   	// Per-directory configuration handler
				MergeGrassrootsBrapiDirectoryConfig,   	// Merge handler for per-directory configurations
				CreateGrassrootsBrapiServerConfig,				// Per-server configuration handler
				MergeGrassrootsBrapiServerConfig,				// Merge handler for per-server configurations
				s_grassroots_brapi_directives,			// Any directives we may have for httpd
				RegisterHooks    					// Our hook registering function
		};


/*
 {
 	"services": [{
 		"so:name": "DFWFieldTrial search service",
 		"start_service": true,
 		"parameter_set": {
 			"parameters": [{
 				"param": "Get all Locations",
 				"current_value": true,
 				"grassroots_type": "xsd:boolean",
 			}]
 		}
 	}]
 }
 */


json_t *GetGrassrootsRequest (ParameterSet *params_p)
{
	json_t *req_p = json_object ();

	if (req_p)
		{
			json_t *services_array_p = json_array ();

			if (services_array_p)
				{
					if (json_object_set_new (req_p, SERVICES_NAME_S, services_array_p) == 0)
						{
							json_t *service_p = json_object ();

							if (service_p)
								{
									if (json_array_append_new (services_array_p, service_p) == 0)
										{
											if (SetJSONString (service_p, SERVICE_NAME_S, "Search Field Trials"))
												{
													if (SetJSONBoolean (service_p, SERVICE_RUN_S, true))
														{
															SchemaVersion *sv_p = AllocateSchemaVersion (CURRENT_SCHEMA_VERSION_MAJOR, CURRENT_SCHEMA_VERSION_MINOR);

															if (sv_p)
																{
																	json_t *param_set_json_p = GetParameterSetAsJSON (params_p, sv_p, false);

																	FreeSchemaVersion (sv_p);

																	if (param_set_json_p)
																		{
																			if (json_object_set_new (service_p, PARAM_SET_KEY_S, param_set_json_p) == 0)
																				{
																					return req_p;
																				}
																			else
																				{
																					json_decref (param_set_json_p);
																				}

																		}		/* if (param_set_json_p) */

																}		/* if (sv_p) */

														}		/* if (SetJSONBoolean (service_p, SERVICE_RUN_S, true)) */

												}		/* if (SetJSONString (service_p, SERVICE_NAME_S, "DFWFieldTrial search service")) */

										}		/* if (json_array_append_new (services_array_p, service_p) == 0) */
									else
										{
											json_decref (service_p);
										}

								}		/* if (service_p) */



						}		/* if (json_object_set_new (req_p, SERVICES_NAME_S, services_array_p) == 0) */
					else
						{
							json_decref (services_array_p);
						}

				}		/* if (services_array_p) */


			json_decref (req_p);
		}		/* if (req_p) */

	return NULL;
}



/* The handler function for our module.
 * This is where all the fun happens!
 */
static int BrapiHandler (request_rec *req_p)
{
	int res = DECLINED;

	/* First off, we need to check if this is a call for the grassroots handler.
	 * If it is, we accept it and do our things, it not, we simply return DECLINED,
	 * and Apache will try somewhere else.
	 */
	if ((req_p -> handler) && (strcmp (req_p -> handler, "grassroots-brapi-handler") == 0))
		{
			if (req_p -> method_number == M_GET)
				{
					char *api_call_s = strstr (req_p -> uri, S_BRAPI_API_S);

					if (api_call_s)
						{
							APIStatus success = AS_IGNORED;
							apr_table_t *params_p = NULL;
							process_req_fn brapi_functions [] =
								{
									GetAllLocations,
									GetLocationByID,
									GetAllProgrammes,
									GetProgrammeByID,
									GetAllTrials,
									GetTrialByID,
									NULL
								};
							process_req_fn *current_brapi_fn_p = brapi_functions;

							/* jump to the rest api call string */
							api_call_s += strlen (S_BRAPI_API_S);

							ap_args_to_table (req_p, &params_p);


							while ((current_brapi_fn_p != NULL) && (success == AS_IGNORED))
								{
									success = (*current_brapi_fn_p) (req_p, api_call_s, params_p);

									switch (success)
										{
											case AS_IGNORED:
												++ current_brapi_fn_p;
												break;

											case AS_SUCCEEDED:
												res = OK;
												current_brapi_fn_p = NULL;
												break;

											case AS_FAILED:
												res = HTTP_INTERNAL_SERVER_ERROR;
												current_brapi_fn_p = NULL;
												break;

										}

								}
						}

				}		/* if (req_p -> method_number == M_GET) */

		}		/* if ((req_p -> handler) && (strcmp (req_p -> handler, "brapi-handler") == 0)) */

	return res;
}


APIStatus DoGrassrootsCall (request_rec *req_p, ParameterSet *params_p, json_t * (*convert_grassroots_to_brapi_fn) (const json_t *grassroots_result_p))
{
	APIStatus res = AS_FAILED;
	json_t *grassroots_request_p = GetGrassrootsRequest (params_p);

	if (grassroots_request_p)
		{
			ModBrapiConfig *config_p = ap_get_module_config (req_p -> per_dir_config, &grassroots_brapi_module);
			Connection *connection_p = AllocateWebServerConnection (config_p -> mbc_grassroots_url_s, CM_MEMORY);

			if (connection_p)
				{
					json_t *grassroots_response_p = MakeRemoteJsonCall (grassroots_request_p, connection_p);

					if (grassroots_response_p)
						{
							const json_t *service_results_p = json_object_get (grassroots_response_p, SERVICE_RESULTS_S);

							if (service_results_p)
								{
									if (json_is_array (service_results_p))
										{
											/*
											 * We should just have a single service
											 */
											if (json_array_size (service_results_p) == 1)
												{
													/* Get the job status */
													OperationStatus status = OS_ERROR;
													service_results_p = json_array_get (service_results_p, 0);

													const char *value_s = GetJSONString (service_results_p, SERVICE_STATUS_S);

													if (value_s)
														{
															status = GetOperationStatusFromString (value_s);
														}
													else
														{
															json_int_t i;
															/* Get the job status */

															if (GetJSONInteger(service_results_p, SERVICE_STATUS_VALUE_S, &i))
																{
																	if ((i > OS_LOWER_LIMIT) && (i < OS_UPPER_LIMIT))
																		{
																			status = (OperationStatus) i;
																		}
																}
														}

													if ((status == OS_PARTIALLY_SUCCEEDED) || (status == OS_SUCCEEDED))
														{
															const json_t *job_results_p = json_object_get (service_results_p, JOB_RESULTS_S);

															if (job_results_p)
																{
																	json_t *brapi_results_p = json_array ();

																	if (brapi_results_p)
																		{
																			json_t *response_p = NULL;
																			size_t current_page = 1;
																			size_t page_size;
																			size_t total_count;
																			size_t total_pages = 1;

																			if (json_is_array (job_results_p))
																				{
																					const size_t num_results = json_array_size (job_results_p);
																					size_t i = 0;

																					for (i = 0; i < num_results; ++ i)
																						{
																							const json_t *grassroots_result_p = json_array_get (job_results_p, i);
																							json_t *brapi_result_p = convert_grassroots_to_brapi_fn (grassroots_result_p);

																							if (brapi_result_p)
																								{
																									if (json_array_append_new (brapi_results_p, brapi_result_p) == 0)
																										{

																										}		/* if (json_array_append_new (brapi_results_p, brapi_result_p) == 0) */
																									else
																										{
																											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, brapi_result_p, "Failed to add result");
																											json_decref (brapi_result_p);
																										}
																								}
																							else
																								{
																									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, grassroots_result_p, "Failed to create brapi_result_p");
																								}
																						}

																				}		/* if (json_is_array (job_results_p)) */

																			total_count = json_array_size (brapi_results_p);
																			page_size = json_array_size (brapi_results_p);

																			response_p = CreateResponseJSONForResult (brapi_results_p, current_page, page_size, total_count, total_pages);

																			if (response_p)
																				{
																					const size_t REAL_PRECISION = 8;
																					char *response_s = json_dumps (response_p, JSON_INDENT (2) | JSON_ESCAPE_SLASH | JSON_REAL_PRECISION (REAL_PRECISION));

																					if (response_s)
																						{
																							ap_rputs (response_s, req_p);
																							ap_set_content_type (req_p, "application/json");

																							res = AS_SUCCEEDED;
																							free (response_s);
																						}		/* if (repsonse_s) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed tp get string representation");
																						}


																					json_decref (response_p);
																				}		/* if (response_p) */
																			else
																				{
																					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, job_results_p, "CreateResponseJSONForResult failed");
																				}

																		}		/* if (brapi_results_p) */

																}		/* if (job_results_p) */

														}		/* if ((status == OS_PARTIALLY_SUCCEEDED) || (status == OS_SUCCEEDED)) */

												}		/* if (json_array_size (service_results_p) == 1) */

										}		/* if (json_is_array (service_results_p)) */

								}		/* if (results_p) */


							json_decref (grassroots_response_p);
						}		/* if (grassroots_response_p) */

					FreeConnection (connection_p);
				}		/* if (connection_p) */

			json_decref (grassroots_request_p);
		}		/* if (grassroots_request_p) */

	return res;
}



/* register_hooks: Adds a hook to the httpd process */
static void RegisterHooks (apr_pool_t *pool_p)
{
	ap_hook_handler (BrapiHandler, NULL, NULL, APR_HOOK_MIDDLE);
}


static const char *SetGrassrootsURL (cmd_parms *cmd_p, void *cfg_p, const char *arg_s)
{
	ModBrapiConfig *config_p = (ModBrapiConfig *) cfg_p;

	config_p -> mbc_grassroots_url_s = arg_s;

	return NULL;
}


static void *MergeGrassrootsBrapiDirectoryConfig (apr_pool_t *pool_p, void *base_config_p, void *new_config_p)
{
	ModBrapiConfig *base_brapi_config_p = (ModBrapiConfig *) base_config_p;
	ModBrapiConfig *new_brapi_config_p = (ModBrapiConfig *) new_config_p;

	if (new_brapi_config_p -> mbc_grassroots_url_s)
		{
			base_brapi_config_p -> mbc_grassroots_url_s = new_brapi_config_p -> mbc_grassroots_url_s;
		}

	return base_config_p;
}


static void *MergeGrassrootsBrapiServerConfig (apr_pool_t *pool_p, void *base_config_p, void *vhost_config_p)
{
	/* currently ignore the vhosts config */
	return base_config_p;
}


static void *CreateGrassrootsBrapiServerConfig (apr_pool_t *pool_p, server_rec *server_p)
{
	return ((void *) CreateConfig (pool_p, server_p));
}


static void *CreateGrassrootsBrapiDirectoryConfig (apr_pool_t *pool_p, char *context_s)
{
	return ((void *) CreateConfig (pool_p, NULL));
}


static ModBrapiConfig *CreateConfig (apr_pool_t *pool_p, server_rec *server_p)
{
	ModBrapiConfig *config_p = apr_palloc (pool_p, sizeof (ModBrapiConfig));

	if (config_p)
		{
			config_p -> mbc_grassroots_url_s = NULL;
		}

	return config_p;
}


json_t *CreateResponseJSONForResult (json_t *payload_array_p, const size_t current_page, const size_t page_size, const size_t total_count, const size_t total_pages)
{
	json_t *response_p = json_object ();

	if (response_p)
		{
			json_t *metadata_p = CreateMetadataResponse (current_page, page_size, total_count, total_pages);

			if (metadata_p)
				{
					if (json_object_set_new (response_p, "metadata", metadata_p) == 0)
						{
							json_t *result_p = json_object ();

							if (json_object_set_new (response_p, "result", result_p) == 0)
								{
									if (json_object_set_new (result_p, "data", payload_array_p) == 0)
										{
											return response_p;
										}		/* if (json_object_set (response_p, "data", payload_array_p) == 0) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed to add data");
											json_decref (result_p);
										}

								}		/* if (json_object_set (response_p, "result", result_p) == 0) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed to add result");
									json_decref (result_p);
								}

						}		/* if (json_object_set (response_p, "metadata", metadata_p) == 0) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, response_p, "Failed to add metadata");
							json_decref (metadata_p);
						}

				}		/* if (metadata_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "CreateMetadataResponse failed");
				}

			json_decref (response_p);
		}		/* if (response_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create response");
		}

	return NULL;
}


bool CopyJSONStringValue (const json_t *src_p, const char *src_key_s, json_t *dest_p, const char *dest_key_s)
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
			if (SetJSONNull (dest_p, dest_key_s))
				{
					success_flag = true;
				}
		}

	return success_flag;
}





static json_t *CreateMetadataResponse (const size_t current_page, const size_t page_size, const size_t total_count, const size_t total_pages)
{
	json_t *metadata_p = json_object ();

	if (metadata_p)
		{
			json_t *pagination_p = json_object ();

			if (pagination_p)
				{
					if (json_object_set_new (metadata_p, "pagination", pagination_p) == 0)
						{
							if (SetJSONInteger (pagination_p, "currentPage", current_page))
								{
									if (SetJSONInteger (pagination_p, "pageSize", page_size))
										{
											if (SetJSONInteger (pagination_p, "totalCount", total_count))
												{
													if (SetJSONInteger (pagination_p, "totalPages", total_pages))
														{
															json_t *datafiles_p = json_array ();

															if (datafiles_p)
																{
																	if (json_object_set_new (metadata_p, "datafiles", datafiles_p) == 0)
																		{
																			json_t *status_p = json_array ();

																			if (status_p)
																				{
																					if (json_object_set_new (metadata_p, "status", datafiles_p) == 0)
																						{
																							return metadata_p;
																						}		/* if (json_object_set_new (metadata_p, "status", status_p) == 0) */
																					else
																						{
																							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_p, "Failed to add status");
																							json_decref (status_p);
																						}
																				}		/* if (status_p) */
																			else
																				{
																					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create status_p");
																				}

																		}		/* if (json_object_set_new (metadata_p, "datafiles", datafiles_p) == 0) */
																	else
																		{
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_p, "Failed to add datafiles");
																			json_decref (datafiles_p);
																		}
																}
															else
																{
																	PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create datafiles_p");
																}

														}		/* if (SetJSONInteger (pagination_p, "totalPages", total_pages)) */
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, pagination_p, "Failed to set currentPage to " SIZET_FMT, total_pages);
														}

												}		/* if (SetJSONInteger (pagination_p, "totalCount", total_count)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, pagination_p, "Failed to set totalCount to " SIZET_FMT, total_count);
												}

										}		/* if (SetJSONInteger (pagination_p, "pageSize", page_size)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, pagination_p, "Failed to set pageSize to " SIZET_FMT, page_size);
										}

								}		/* if (SetJSONInteger (pagination_p, "currentPage", current_page)) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, pagination_p, "Failed to set currentPage to " SIZET_FMT, current_page);
								}

						}		/* if (json_object_set (metadata_p, "pagination", pagination_p) == 0) */
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_p, "Failed to add pagination");
							json_decref (pagination_p);
						}

				}		/* if (pagination_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create pagination_p");
				}

			json_decref (metadata_p);
		}		/* if (metadata_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create metadata");
		}

	return NULL;
}


char *GetObjectIdString (const json_t * const grassroots_json_p)
{
	char *id_s = NULL;
	bson_oid_t oid;

	if (GetMongoIdFromJSON (grassroots_json_p, &oid))
		{
			id_s = GetBSONOidAsString (&oid);
		}

	return id_s;
}


const char *GetParameterValue (apr_table_t *params_p, const char * const param_s, apr_pool_t *pool_p)
{
	const char *value_s = NULL;
	const char * const raw_value_s = apr_table_get (params_p, param_s);
	const char *forbid_s = NULL;
	const char *reserved_s = NULL;

	if (raw_value_s)
		{
			value_s = apr_punescape_url (pool_p, raw_value_s, forbid_s, reserved_s, 1);
		}

	return value_s;
}



void GetSortSearchParameters (apr_table_t *params_p, const char **sort_by_ss, const char **sort_order_ss, apr_pool_t *pool_p)
{
	*sort_by_ss = GetParameterValue (params_p, "sortBy", pool_p);
	*sort_order_ss = GetParameterValue (params_p, "sortOrder", pool_p);
}

