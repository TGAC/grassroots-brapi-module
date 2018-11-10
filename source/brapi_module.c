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


#include "brapi_module.h"
#include "util_script.h"



/*
 * STATIC FUNCTION DECLARATIONS
 */

static int BrapiHandler (request_rec *req_p);

static void RegisterHooks (apr_pool_t *pool_p);


static void *MergeDirectoryConfig (apr_pool_t *pool_p, void *base_config_p, void *new_config_p);


static void *MergeServerConfig (apr_pool_t *pool_p, void *base_config_p, void *vhost_config_p);


static void *CreateServerConfig (apr_pool_t *pool_p, server_rec *server_p);


static void *CreateDirectoryConfig (apr_pool_t *pool_p, char *context_s);


static ModBrapiConfig *CreateConfig (apr_pool_t *pool_p, server_rec *server_p);


static const char *SetGrassrootsURL (cmd_parms *cmd_p, void *cfg_p, const char *arg_s);




/*
 * STATIC VARIABLES
 */

static const command_rec s_grassroots_brapi_directives [] =
{
	AP_INIT_TAKE1 ("GrassrootsURL", SetGrassrootsURL, NULL, ACCESS_CONF, "The url for the Grassroots controller to call"),
	{ NULL }
};


static const char * const S_BRAPI_API_S = "/brapi/v1";


/*
 * GRASSROOTS BRAPI MODULE
 */


/* Define our module as an entity and assign a function for registering hooks  */
module AP_MODULE_DECLARE_DATA grassroots_brapi_module =
{
    STANDARD20_MODULE_STUFF,
    CreateDirectoryConfig,   	// Per-directory configuration handler
    MergeDirectoryConfig,   	// Merge handler for per-directory configurations
    CreateServerConfig,				// Per-server configuration handler
    MergeServerConfig,				// Merge handler for per-server configurations
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
											if (SetJSONString (service_p, SERVICE_NAME_S, "DFWFieldTrial search service"))
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
  if ((req_p -> handler) && (strcmp (req_p -> handler, "brapi-handler") == 0))
  	{
  		if (req_p -> method_number == M_GET)
  			{
  				char *api_call_s = strstr (req_p -> uri, S_BRAPI_API_S);

  				if (api_call_s)
  					{
  						apr_table_t *params_p = NULL;

  						/* jump to the rest api call string */
  						api_call_s += strlen (S_BRAPI_API_S);

  						ap_args_to_table (req_p, &params_p);


  						/*
  						 * Locations
  						 */

  					}

  			}		/* if (req_p -> method_number == M_GET) */

  	}		/* if ((req_p -> handler) && (strcmp (req_p -> handler, "brapi-handler") == 0)) */

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


static void *MergeDirectoryConfig (apr_pool_t *pool_p, void *base_config_p, void *new_config_p)
{
	/* currently ignore the vhosts config */
	return base_config_p;
}


static void *MergeServerConfig (apr_pool_t *pool_p, void *base_config_p, void *vhost_config_p)
{
	/* currently ignore the vhosts config */
	return base_config_p;
}


static void *CreateServerConfig (apr_pool_t *pool_p, server_rec *server_p)
{
	return ((void *) CreateConfig (pool_p, server_p));
}


static void *CreateDirectoryConfig (apr_pool_t *pool_p, char *context_s)
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


