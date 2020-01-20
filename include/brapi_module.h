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
 * brapi_module.h
 *
 *  Created on: 9 Nov 2018
 *      Author: billy
 */

#ifndef SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_MODULE_H_
#define SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_MODULE_H_

#include "jansson.h"
#include "httpd.h"

#include "parameter_set.h"



#include "apr_hash.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"

module AP_MODULE_DECLARE_DATA grassroots_brapi_module;


/**
 * @brief The configuration for the Grassroots BrAPI module.
 */
typedef struct
{
	/** The URL to the Grassroots installation */
	const char *mbc_grassroots_url_s;

} ModBrapiConfig;


/**
 * Create the request object to send to the Grassroots
 *
 */
json_t *GetGrassrootsRequest (ParameterSet *params_p);


json_t *CreateResponseJSONForResult (json_t *payload_array_p, const size_t current_page, const size_t page_size, const size_t total_count, const size_t total_pages);


int DoGrassrootsCall (request_rec *req_p, ParameterSet *params_p, json_t * (*convert_grassroots_to_brapi_fn) (const json_t *grassroots_result_p));


char *GetObjectIdString (const json_t * const grassroots_json_p);


const char *GetParameterValue (apr_table_t *params_p, const char * const param_s, apr_pool_t *pool_p);

bool CopyJSONStringValue (const json_t *src_p, const char *src_key_s, json_t *dest_p, const char *dest_key_s);



#endif /* SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_MODULE_H_ */
