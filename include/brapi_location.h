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
 * brapi_location.h
 *
 *  Created on: 12 Nov 2018
 *      Author: billy
 */

#ifndef SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_LOCATION_H_
#define SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_LOCATION_H_

#include "httpd.h"

#include "util_script.h"

#include "brapi_module.h"

#include "typedefs.h"


APIStatus GetAllLocations (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p);

APIStatus GetLocationByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p);


bool GetMinimalLocationData (const json_t *grassroots_json_p, char **name_ss, char **db_id_ss);


#endif /* SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_LOCATION_H_ */
