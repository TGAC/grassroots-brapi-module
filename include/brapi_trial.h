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
 * brapi_trial.h
 *
 *  Created on: 8 Jan 2019
 *      Author: billy
 */

#ifndef SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_TRIAL_H_
#define SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_TRIAL_H_

#include "httpd.h"
#include "util_script.h"

#include "brapi_module.h"


APIStatus GetAllTrials (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p);

APIStatus GetTrialByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p);




#endif /* SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_TRIAL_H_ */
