/*
 * brapi_program.h
 *
 *  Created on: 2 Jul 2022
 *      Author: billy
 */

#ifndef SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_PROGRAM_H_
#define SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_PROGRAM_H_



#include "httpd.h"
#include "util_script.h"

#include "brapi_module.h"


APIStatus GetAllProgrammes (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p);

APIStatus GetProgrammeByID (request_rec *req_p, const char *api_call_s, apr_table_t *req_params_p, ModBrapiConfig *config_p);

bool AddProgrammeDetails (const json_t *grassroots_data_p, json_t *brapi_response_p);


#endif /* SERVERS_BRAPI_MODULE_INCLUDE_BRAPI_PROGRAM_H_ */
