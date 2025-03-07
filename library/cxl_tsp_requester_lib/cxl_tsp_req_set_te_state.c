/**
 *  Copyright Notice:
 *  Copyright 2021-2022 DMTF. All rights reserved.
 *  License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
 **/

#include "hal/base.h"
#include "hal/library/memlib.h"
#include "library/spdm_requester_lib.h"
#include "library/spdm_transport_pcidoe_lib.h"
#include "library/cxl_tsp_requester_lib.h"

#pragma pack(1)
typedef struct {
    cxl_tsp_header_t header;
    uint8_t te_state;
    uint8_t number_of_memory_ranges;
    uint8_t reserved[0xc];
    cxl_tsp_memory_range_t memory_ranges[32];
} cxl_tsp_set_target_te_state_req_mine_t;
#pragma pack()

/**
 * Send and receive an TSP message
 *
 * @param  spdm_context                 A pointer to the SPDM context.
 * @param  session_id                   Indicates if it is a secured message protected via SPDM session.
 *                                     If session_id is NULL, it is a normal message.
 *                                     If session_id is NOT NULL, it is a secured message.
 *
 * @retval LIBSPDM_STATUS_SUCCESS               The TSP request is sent and response is received.
 * @return ERROR                        The TSP response is not received correctly.
 **/
libspdm_return_t cxl_tsp_set_te_state(
    const void *pci_doe_context,
    void *spdm_context, const uint32_t *session_id,
    uint8_t te_state,
    uint8_t number_of_memory_ranges,
    const cxl_tsp_memory_range_t *memory_ranges)
{
    libspdm_return_t status;
    cxl_tsp_set_target_te_state_req_mine_t request;
    size_t request_size;
    uint8_t res_buf[LIBCXLTSP_ERROR_MESSAGE_MAX_SIZE];
    cxl_tsp_set_target_te_state_rsp_t *response;
    size_t response_size;

    libspdm_zero_mem (&request, sizeof(request));
    request.header.tsp_version = CXL_TSP_MESSAGE_VERSION_10;
    request.header.op_code = CXL_TSP_OPCODE_SET_TARGET_TE_STATE;
    request.te_state = te_state;
    request.number_of_memory_ranges = number_of_memory_ranges;
    if (number_of_memory_ranges > 32) {
        return LIBSPDM_STATUS_INVALID_STATE_LOCAL;
    }
    libspdm_copy_mem (
        request.memory_ranges,
        sizeof(request.memory_ranges),
        memory_ranges,
        sizeof(cxl_tsp_memory_range_t) * number_of_memory_ranges);

    request_size = sizeof(cxl_tsp_set_target_te_state_req_t) +
                   number_of_memory_ranges * sizeof(cxl_tsp_memory_range_t);
    response = (void *)res_buf;
    response_size = sizeof(res_buf);
    status = cxl_tsp_send_receive_data(spdm_context, session_id,
                                       &request, request_size,
                                       response, &response_size);
    if (LIBSPDM_STATUS_IS_ERROR(status)) {
        return status;
    }

    if (response_size != sizeof(cxl_tsp_set_target_te_state_rsp_t)) {
        return LIBSPDM_STATUS_INVALID_MSG_SIZE;
    }
    if (response->header.tsp_version != request.header.tsp_version) {
        return LIBSPDM_STATUS_INVALID_MSG_FIELD;
    }
    if (response->header.op_code != CXL_TSP_OPCODE_SET_TARGET_TE_STATE_RSP) {
        return LIBSPDM_STATUS_INVALID_MSG_FIELD;
    }

    return LIBSPDM_STATUS_SUCCESS;
}
