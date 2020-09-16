/*
 * Copyright 2019-2020 NXP
 *
 * NXP Confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef HSM_API_H
#define HSM_API_H

#include <stdint.h>

/**
 *  @defgroup group0 Error codes
 *  @{
 */
/**
 * Error codes returned by HSM functions.
 */
typedef enum {
    HSM_NO_ERROR                        = 0x0,      /**<    Success. */
    HSM_INVALID_MESSAGE                 = 0x1,      /**< 	The received message is invalid or unknown. */
    HSM_INVALID_ADDRESS                 = 0x2,      /**<    The provided address is invalid or doesn’t respect the API requirements. */
    HSM_UNKNOWN_ID                      = 0x3,      /**< 	The provided identifier is not known. */
    HSM_INVALID_PARAM                   = 0x4,      /**< 	One of the parameter provided in the command is invalid. */
    HSM_NVM_ERROR                       = 0x5,      /**< 	NVM generic issue. */
    HSM_OUT_OF_MEMORY                   = 0x6,      /**< 	There is not enough memory to handle the requested operation. */
    HSM_UNKNOWN_HANDLE                  = 0x7,      /**< 	Unknown session/service handle. */
    HSM_UNKNOWN_KEY_STORE               = 0x8,      /**< 	The key store identified by the provided “key store Id” doesn’t exist and the “create” flag is not set. */
    HSM_KEY_STORE_AUTH                  = 0x9,      /**< 	Key store authentication fails. */
    HSM_KEY_STORE_ERROR                 = 0xA,      /**< 	An error occurred in the key store internal processing. */
    HSM_ID_CONFLICT                     = 0xB,      /**< 	An element (key store, key…) with the provided ID already exists. */
    HSM_RNG_NOT_STARTED                 = 0xC,      /**< 	The internal RNG is not started. */
    HSM_CMD_NOT_SUPPORTED               = 0xD,      /**< 	The functionality is not supported for the current session/service/key store configuration. */
    HSM_INVALID_LIFECYCLE               = 0xE,      /**< 	Invalid lifecycle for requested operation. */
    HSM_KEY_STORE_CONFLICT              = 0xF,      /**< 	A key store with the same attributes already exists. */
    HSM_KEY_STORE_COUNTER               = 0x10,     /**<    The current key store reaches the max number of monotonic counter updates, updates are still allowed but monotonic counter will not be blown. */
    HSM_FEATURE_NOT_SUPPORTED           = 0x11,     /**<    The requested feature is not supported by the firwmare. */
    HSM_SELF_TEST_FAILURE               = 0x12,     /**<    Self tests report an issue */
    HSM_NOT_READY_RATING                = 0x13,     /**<    The HSM is not ready to handle the current request */
    HSM_FEATURE_DISABLED                = 0x14,     /**<    The required service/operation is disabled */
    HSM_GENERAL_ERROR                   = 0xFF,     /**<    Error not covered by other codes occured. */
} hsm_err_t;
/** @} end of error code group */

/**
 *  @defgroup group1 Session
 * The API must be initialized by a potential requestor by opening a session.\n
 * Once a session is closed all the associated service flows are closed by the HSM.
 *  @{
 */
typedef uint32_t hsm_hdl_t;
typedef struct {
    uint8_t session_priority;   //!< Priority of the operations performed in this session. */
    uint8_t operating_mode;     //!< Options for the session to be opened (bitfield). */
    uint16_t reserved;
} open_session_args_t;
#define HSM_OPEN_SESSION_PRIORITY_LOW       (0x00U) //!< Low priority. Should be the default setting on platforms that doesn't support sessions priorities.
#define HSM_OPEN_SESSION_PRIORITY_HIGH      (0x01U) //!< High Priority session

#define HSM_OPEN_SESSION_FIPS_MODE_MASK     (1u << 0) //!< Only FIPS certified operations authorized in this session
#define HSM_OPEN_SESSION_EXCLUSIVE_MASK     (1u << 1) //!< No other HSM session will be authorized on the same security enclave.
#define HSM_OPEN_SESSION_LOW_LATENCY_MASK   (1u << 3) //!< Use a low latency HSM implementation
#define HSM_OPEN_SESSION_NO_KEY_STORE_MASK  (1u << 4) //!< No key store will be attached to this session. May provide better performances on some operation depending on the implementation. Usage of the session will be restricted to operations that doesn't involve secret keys (e.g. hash, signature verification, random generation).
#define HSM_OPEN_SESSION_RESERVED_MASK      ((1u << 2) | (1u << 5) | (1u << 6) | (1u << 7)) //!< Bits reserved for future use. Should be set to 0.

/**
 *
 * \param args pointer to the structure containing the function arguments.

 * \param session_hdl pointer to where the session handle must be written.
 *
 * \return error_code error code.
 */
hsm_err_t hsm_open_session(open_session_args_t *args, hsm_hdl_t *session_hdl);

/**
 * Terminate a previously opened session. All the services opened under this session are closed as well \n
 *
 * \param session_hdl pointer to the handle identifying the session to be closed.
 *
 * \return error_code error code.
 */
hsm_err_t hsm_close_session(hsm_hdl_t session_hdl);

/**
 *\addtogroup qxp_specific
 * \ref group1
 *
 * i.MX8QXP HSM is implemented only on SECO core which doesn't offer priority management neither low latencies.
 * - \ref HSM_OPEN_SESSION_FIPS_MODE_MASK not supported and ignored
 * - \ref HSM_OPEN_SESSION_EXCLUSIVE_MASK not supported and ignored
 * - session_priority field of \ref open_session_args_t is ignored.
 * - \ref HSM_OPEN_SESSION_LOW_LATENCY_MASK not supported and ignored.
 *
 */

/**
 *\addtogroup dxl_specific
 * \ref group1
 *
 * i.MX8DXL has 2 separate implementations of HSM on SECO and on V2X cores.
 * - \ref HSM_OPEN_SESSION_FIPS_MODE_MASK not supported and ignored
 * - \ref HSM_OPEN_SESSION_EXCLUSIVE_MASK not supported and ignored
 * - If \ref HSM_OPEN_SESSION_LOW_LATENCY_MASK is unset then SECO implementation will be used.
 * In this case session_priority field of \ref open_session_args_t is ignored.
 * - If \ref HSM_OPEN_SESSION_LOW_LATENCY_MASK is set then V2X implementation is used. session_priority field of \ref open_session_args_t and \ref HSM_OPEN_SESSION_NO_KEY_STORE_MASK are considered.
 *
 */
/** @} end of session group */

/**
 *  @defgroup group2 Key store
 * User must open a key store service flow in order to perform the following operations:
 *  - create a new key store
 *  - perform operations involving keys stored in the key store (ciphering, signature generation...)
 *  - perform a key store reprovisioning using a signed message. A key store re-provisioning results in erasing all the key stores handled by the HSM.
 *
 * To grant access to the key store, the caller is authenticated against the domain ID (DID) and Messaging Unit used at the keystore creation, additionally an authentication nonce can be provided.
 * @{
 */

typedef uint8_t hsm_svc_key_store_flags_t;
typedef struct {
    uint32_t key_store_identifier;      //!< user defined id identifying the key store. Only one key store service can be opened on a given key_store_identifier.
    uint32_t authentication_nonce;      //!< user defined nonce used as authentication proof for accesing the key store.
    uint16_t max_updates_number;        //!< maximum number of updates authorized for the key store. Valid only for create operation.\n This parameter has the goal to limit the occupation of the monotonic counter used as anti-rollback protection.\n If the maximum number of updates is reached, HSM still allows key store updates but without updating the monotonic counter giving the opportunity for rollback attacks.
    hsm_svc_key_store_flags_t flags;    //!< bitmap specifying the services properties.
    uint8_t reserved;                   //!< it must be 0.
    uint8_t *signed_message;            //!< pointer to signed_message to be sent only in case of key store re-provisioning
    uint16_t signed_msg_size;           //!< size of the signed_message to be sent only in case of key store re-provisioning
    uint8_t reserved_1[2];
} open_svc_key_store_args_t;

/**
 * Open a service flow on the specified key store. Only one key store service can be opened on a given key store.
 *
 * \param session_hdl pointer to the handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.

 * \param key_store_hdl pointer to where the key store service flow handle must be written.
 *
 * \return error_code error code.
 */
hsm_err_t hsm_open_key_store_service(hsm_hdl_t session_hdl, open_svc_key_store_args_t *args, hsm_hdl_t *key_store_hdl);
#define HSM_SVC_KEY_STORE_FLAGS_CREATE ((hsm_svc_key_store_flags_t)(1u << 0)) //!< It must be specified to create a new key store. The key store will be stored in the NVM only once a key is generated/imported specyfing the STRICT OPERATION flag.
/**
 * Close a previously opened key store service flow. The key store is deleted from the HSM local memory, any update not written in the NVM is lost \n
 *
 * \param handle identifying the key store service flow to be closed.
 *
 * \return error_code error code.
 */
hsm_err_t hsm_close_key_store_service(hsm_hdl_t key_store_hdl);

/** @} end of key store service flow */


/**
 *  @defgroup group3 Key management
 * @{
 */

typedef uint8_t hsm_svc_key_management_flags_t;
typedef struct {
    hsm_svc_key_management_flags_t flags;   //!< bitmap specifying the services properties.
    uint8_t reserved[3];
} open_svc_key_management_args_t;

/**
 * Open a key management service flow\n
 * User must open this service flow in order to perform operation on the key store keys (generate, update, delete)
 *
 * \param key_store_hdl handle identifying the key store service flow.
 * \param args pointer to the structure containing the function arguments.

 * \param key_management_hdl pointer to where the key management service flow handle must be written.
 *
 * \return error_code error code.
 */
hsm_err_t hsm_open_key_management_service(hsm_hdl_t key_store_hdl, open_svc_key_management_args_t *args, hsm_hdl_t *key_management_hdl);

typedef uint8_t hsm_op_key_gen_flags_t;
typedef uint8_t hsm_key_type_t;
typedef uint16_t hsm_key_info_t;
typedef uint16_t hsm_key_group_t;

typedef struct {
    uint32_t *key_identifier;           //!< pointer to the identifier of the key to be used for the operation.\n In case of create operation the new key identifier will be stored in this location.
    uint16_t out_size;                  //!< length in bytes of the generated key. It must be 0 in case of symmetric keys.
    hsm_op_key_gen_flags_t flags;       //!< bitmap specifying the operation properties.
    hsm_key_type_t key_type;            //!< indicates which type of key must be generated.
    hsm_key_group_t key_group;          //!< Key group of the generated key, relevant only in case of create operation. it must be a value in the range 0-1023. Keys belonging to the same group can be cached in the HSM local memory throug the hsm_manage_key_group API.
    hsm_key_info_t key_info;            //!< bitmap specifying the properties of the key.
    uint8_t *out_key;                   //!< pointer to the output area where the generated public key must be written.
} op_generate_key_args_t;

/**
 * Generate a key or a key pair. Only the confidential keys (symmetric and private keys) are stored in the internal key store, while the non-confidential keys (public key) are exported.\n
 * The generated key can be stored using a new or existing key identifier with the restriction that an existing key can be replaced only by a key of the same type.\n
 * User can call this function only after having opened a key management service flow.
 *
 * \param key_management_hdl handle identifying the key management service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_generate_key(hsm_hdl_t key_management_hdl, op_generate_key_args_t *args);
#define HSM_KEY_TYPE_ECDSA_NIST_P256                        ((hsm_key_type_t)0x02u)
#define HSM_KEY_TYPE_ECDSA_NIST_P384                        ((hsm_key_type_t)0x03u)
#define HSM_KEY_TYPE_ECDSA_NIST_P521                        ((hsm_key_type_t)0x04u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256                 ((hsm_key_type_t)0x13u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_320                 ((hsm_key_type_t)0x14u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_384                 ((hsm_key_type_t)0x15u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_512                 ((hsm_key_type_t)0x16u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_256                 ((hsm_key_type_t)0x23u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_320                 ((hsm_key_type_t)0x24u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_384                 ((hsm_key_type_t)0x25u)
#define HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_512                 ((hsm_key_type_t)0x26u)
#define HSM_KEY_TYPE_AES_128                                ((hsm_key_type_t)0x30u)
#define HSM_KEY_TYPE_AES_192                                ((hsm_key_type_t)0x31u)
#define HSM_KEY_TYPE_AES_256                                ((hsm_key_type_t)0x32u)
#define HSM_KEY_TYPE_DSA_SM2_FP_256                         ((hsm_key_type_t)0x42u)
#define HSM_KEY_TYPE_SM4_128                                ((hsm_key_type_t)0x50u)
#define HSM_KEY_TYPE_HMAC_224                               ((hsm_key_type_t)0x60u)              //!< For use with HMAC algorithm in hsm_mac_one_go
#define HSM_KEY_TYPE_HMAC_256                               ((hsm_key_type_t)0x61u)              //!< For use with HMAC algorithm in hsm_mac_one_go
#define HSM_KEY_TYPE_HMAC_384                               ((hsm_key_type_t)0x62u)              //!< For use with HMAC algorithm in hsm_mac_one_go
#define HSM_KEY_TYPE_HMAC_512                               ((hsm_key_type_t)0x63u)              //!< For use with HMAC algorithm in hsm_mac_one_go
#define HSM_OP_KEY_GENERATION_FLAGS_UPDATE                  ((hsm_op_key_gen_flags_t)(1u << 0))  //!< User can replace an existing key only by generating a key with the same type of the original one.
#define HSM_OP_KEY_GENERATION_FLAGS_CREATE                  ((hsm_op_key_gen_flags_t)(1u << 1))  //!< Create a new key.
#define HSM_OP_KEY_GENERATION_FLAGS_STRICT_OPERATION        ((hsm_op_key_gen_flags_t)(1u << 7))  //!< The request is completed only when the new key has been written in the NVM. This applicable for persistent key only.
#define HSM_KEY_INFO_PERSISTENT                             ((hsm_key_info_t)(0u << 1))          //!< Persistent keys are stored in the external NVM. The entire key group is written in the NVM at the next STRICT operation.
#define HSM_KEY_INFO_PERMANENT                              ((hsm_key_info_t)(1u << 0))          //!< When set, the key is permanent (write locked). Once created, it will not be possible to update or delete the key anymore. Transient keys will be anyway deleted after a PoR or when the corresponding key store service flow is closed. This bit can never be reset.
#define HSM_KEY_INFO_TRANSIENT                              ((hsm_key_info_t)(1u << 1))          //!< Transient keys are deleted when the corresponding key store service flow is closed or after a PoR. Transient keys cannot be in the same key group than persistent keys.
#define HSM_KEY_INFO_MASTER                                 ((hsm_key_info_t)(1u << 2))          //!< When set, the key is considered as a master key. Only master keys can be used as input of key derivation functions (i.e butterfly key expansion).
#define HSM_KEY_INFO_KEK                                    ((hsm_key_info_t)(1u << 3))          //!< When set, the key is considered as a key encryption key. KEK keys can only be used to wrap and import other keys into the key store, all other operation are not allowed. Only keys imported in the key store through the hsm_mange_key API can get this attribute.

typedef uint8_t hsm_op_manage_key_flags_t;
typedef struct {
    uint32_t *key_identifier;           //!< pointer to the identifier of the key to be used for the operation.\n In case of create operation the new key identifier will be stored in this location.
    uint32_t kek_identifier;            //!< identifier of the key to be used to decrypt the key to be imported (Key Encryption Key), only AES-256 key can be uses as KEK. It must be 0 if the HSM_OP_MANAGE_KEY_FLAGS_PART_UNIQUE_ROOT_KEK or HSM_OP_MANAGE_KEY_FLAGS_COMMON_ROOT_KEK flags are set.
    uint16_t input_size;                //!< length in bytes of the input key area. It must be eqaul to the length of the IV (12 bytes) + ciphertext + Tag (16 bytes). It must be 0 in case of delete operation.
    hsm_op_manage_key_flags_t flags;    //!< bitmap specifying the operation properties.
    hsm_key_type_t key_type;            //!< indicates the type of the key to be managed.
    hsm_key_group_t key_group;          //!< key group of the imported key, only relevant in case of create operation (it must be 0 otherwise). It must be a value in the range 0-1023. Keys belonging to the same group can be cached in the HSM local memory throug the hsm_manage_key_group API.
    hsm_key_info_t key_info;            //!< bitmap specifying the properties of the key, in case of update operation it will replace the existing value. It must be 0 in case of delete operation.
    uint8_t *input_data;                //!< pointer to the input buffer. The input buffer is the concatenation of the IV, the encrypted key to be imported and the tag. It must be 0 in case of delete operation.
} op_manage_key_args_t;

/**
 * This command is designed to perform the following operations:
 *  - import a key creating a new key identifier (import and create)
 *  - import a key using an existing key identifier (import and update)
 *  - delete an existing key
 *
 * The key encryption key (KEK) can be previously pre-shared or stored in the key store.
 *
 * The key to be imported must be encrypted by using the KEK as following:
 *  - Algorithm: AES GCM
 *  - Key: root KEK
 *  - AAD = 0
 *  - IV = 12 bytes
 *  - Tag = 16 bytes
 *  - Plaintext: key to be imported
 *
 * User can call this function only after having opened a key management service flow
 *
 * \param key_management_hdl handle identifying the key management service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_manage_key(hsm_hdl_t key_management_hdl, op_manage_key_args_t *args);
#define HSM_OP_MANAGE_KEY_FLAGS_IMPORT_UPDATE                           ((hsm_op_manage_key_flags_t)(1u << 0))   //!< User can replace an existing key only by importing a key with the same type of the original one.
#define HSM_OP_MANAGE_KEY_FLAGS_IMPORT_CREATE                           ((hsm_op_manage_key_flags_t)(1u << 1))   //!< Import a key and create a new identifier.
#define HSM_OP_MANAGE_KEY_FLAGS_DELETE                                  ((hsm_op_manage_key_flags_t)(1u << 2))   //!< Delete an existing key.
#define HSM_OP_MANAGE_KEY_FLAGS_PART_UNIQUE_ROOT_KEK                    ((hsm_op_manage_key_flags_t)(1u << 3))   //!< The key to be imported is encrypted using the part-unique root kek.
#define HSM_OP_MANAGE_KEY_FLAGS_COMMON_ROOT_KEK                         ((hsm_op_manage_key_flags_t)(1u << 4))   //!< The key to be imported is encrypted using the common root kek.
#define HSM_OP_MANAGE_KEY_FLAGS_STRICT_OPERATION                        ((hsm_op_manage_key_flags_t)(1u << 7))   //!< The request is completed only when the new key has been written in the NVM. This is only applicable for persistent key.


typedef uint8_t hsm_op_manage_key_group_flags_t;
typedef struct {
    hsm_key_group_t key_group;                  //!< it must be a value in the range 0-1023. Keys belonging to the same group can be cached in the HSM local memory throug the hsm_manage_key_group API.
    hsm_op_manage_key_group_flags_t flags;      //!< bitmap specifying the operation properties.
    uint8_t reserved;
} op_manage_key_group_args_t;

/**
 * This command is designed to perform the following operations:
 *  - lock/unlock down a key group in the HSM local memory so that the keys are available to the HSM without additional latency
 *  - un-lock a key group. HSM may export the key group into the external NVM to free up local memory as needed
 *  - delete an existing key group
 *
 * User can call this function only after having opened a key management service flow.
 *
 * \param key_management_hdl handle identifying the key management service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_manage_key_group(hsm_hdl_t key_management_hdl, op_manage_key_group_args_t *args);
#define HSM_OP_MANAGE_KEY_GROUP_FLAGS_CACHE_LOCKDOWN          ((hsm_op_manage_key_group_flags_t)(1u << 0))   //!< The entire key group will be cached in the HSM local memory.
#define HSM_OP_MANAGE_KEY_GROUP_FLAGS_CACHE_UNLOCK            ((hsm_op_manage_key_group_flags_t)(1u << 1))   //!< HSM may export the key group in the external NVM to free up the local memory. HSM will copy the key group in the local memory again in case of key group usage/update.
#define HSM_OP_MANAGE_KEY_GROUP_FLAGS_DELETE                  ((hsm_op_manage_key_group_flags_t)(1u << 2))   //!< Delete an existing key group.
#define HSM_OP_MANAGE_KEY_GROUP_FLAGS_STRICT_OPERATION        ((hsm_op_manage_key_group_flags_t)(1u << 7))   //!< The request is completed only when the update has been written in the NVM. Not applicable for cache lockdown/unlock.


typedef uint8_t hsm_op_but_key_exp_flags_t;
typedef struct {
    uint32_t key_identifier;                //!< identifier of the key to be expanded.
    uint8_t *expansion_function_value;      //!< pointer to the expansion function value input
    uint8_t *hash_value;                    //!< pointer to the hash value input.\n In case of explicit certificate, the hash value address must be set to 0.
    uint8_t *pr_reconstruction_value;       //!< pointer to the private reconstruction value input.\n In case of explicit certificate, the pr_reconstruction_value address must be set to 0.
    uint8_t expansion_function_value_size;  //!< length in bytes of the expansion function input
    uint8_t hash_value_size;                //!< length in bytes of the hash value input.\n In case of explicit certificate, the hash_value_size parameter must be set to 0.
    uint8_t pr_reconstruction_value_size;   //!< length in bytes of the private reconstruction value input.\n In case of explicit certificate, the pr_reconstruction_value_size parameter must be set to 0.
    hsm_op_but_key_exp_flags_t flags;       //!< bitmap specifying the operation properties
    uint32_t *dest_key_identifier;          //!< pointer to identifier of the derived key to be used for the operation.\n In case of create operation the new destination key identifier will be stored in this location.
    uint8_t *output;                        //!< pointer to the output area where the public key must be written.
    uint16_t output_size;                   //!< length in bytes of the generated key, if the size is 0, no key is copied in the output.
    hsm_key_type_t key_type;                //!< indicates the type of the key to be derived.
    uint8_t reserved;
    hsm_key_group_t key_group;              //!< it must be a value in the range 0-1023. Keys belonging to the same group can be cached in the HSM local memory throug the hsm_manage_key_group API
    hsm_key_info_t key_info;                //!< bitmap specifying the properties of the derived key.
} op_butt_key_exp_args_t;

/**
 * This command is designed to perform the butterfly key expansion operation on an ECC private key in case of implicit and explicit certificates. Optionally the resulting public key is exported.\n
 * The result of the key expansion function f_k is calculated outside the HSM and passed as input. The expansion function is defined as f_k = f_k_int mod l , where l is the order of the group of points on the curve.\n
 * User can call this function only after having opened a key management service flow. \n\n
 *
 * Explicit certificates:
 *  - f_k = expansion function value
 *
 * out_key = Key  + f_k
 * \n\n
 *
 * Implicit certificates:
 *  - f_k = expansion function value,
 *  - hash = hash value used in the derivation of the pseudonym ECC key,
 *  - pr_v = private reconstruction value
 *
 * out_key = (Key  + f_k)*hash + pr_v
 *
 * \param key_management_hdl handle identifying the key store management service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
*/
hsm_err_t hsm_butterfly_key_expansion(hsm_hdl_t key_management_hdl, op_butt_key_exp_args_t *args);
#define HSM_OP_BUTTERFLY_KEY_FLAGS_UPDATE                ((hsm_op_but_key_exp_flags_t)(1u << 0))   //!< User can replace an existing key only by generating a key with the same type of the original one.
#define HSM_OP_BUTTERFLY_KEY_FLAGS_CREATE                ((hsm_op_but_key_exp_flags_t)(1u << 1))   //!< Create a new key.
#define HSM_OP_BUTTERFLY_KEY_FLAGS_IMPLICIT_CERTIF       ((hsm_op_but_key_exp_flags_t)(0u << 2))   //!< butterfly key expansion using implicit certificate.
#define HSM_OP_BUTTERFLY_KEY_FLAGS_EXPLICIT_CERTIF       ((hsm_op_but_key_exp_flags_t)(1u << 2))   //!< butterfly key expansion using explicit certificate.
#define HSM_OP_BUTTERFLY_KEY_FLAGS_STRICT_OPERATION      ((hsm_op_but_key_exp_flags_t)(1u << 7))   //!< The request is completed only when the new key has been written in the NVM.

/**
 * Terminate a previously opened key management service flow
 *
 * \param key_management_hdl handle identifying the key management service flow.
 *
 * \return error code
 */
hsm_err_t hsm_close_key_management_service(hsm_hdl_t key_management_hdl);

/**
 *\addtogroup qxp_specific
 * \ref group3
 *
 * - \ref HSM_OP_MANAGE_KEY_GROUP_FLAGS_DELETE is not supported.
 *
 * - \ref HSM_KEY_TYPE_ECDSA_NIST_P521 is not supported.
 * - \ref HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_320 is not supported.
 * - \ref HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_512 is not supported.
 * - \ref HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_256 is not supported.
 * - \ref HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_320 is not supported.
 * - \ref HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_384 is not supported.
 * - \ref HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_512 is not supported.
 * - \ref HSM_KEY_TYPE_DSA_SM2_FP_256 is not supported.
 * - \ref HSM_KEY_TYPE_SM4_128 is not supported.
 *
 * - \ref hsm_butterfly_key_expansion: This feature is disabled when part is running in FIPS approved mode. Any call to this API will results in a HSM_FEATURE_DISABLED error.
 * - \ref hsm_key_type_t of op_butt_key_exp_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 are supported.
 */

/**
 *\addtogroup dxl_specific
 * \ref group3
 *
 * - \ref HSM_OP_MANAGE_KEY_GROUP_FLAGS_DELETE is not supported.
 *
 * - \ref hsm_key_type_t of op_butt_key_exp_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256, HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_256 are supported.
 *
 */
/** @} end of key management service flow */

/**
 *  @defgroup group4 Ciphering
 * @{
 */

typedef uint8_t hsm_svc_cipher_flags_t;
typedef struct {
    hsm_svc_cipher_flags_t flags;           //!< bitmap specifying the services properties.
    uint8_t reserved[3];
} open_svc_cipher_args_t;

/**
 * Open a cipher service flow\n
 * User can call this function only after having opened a key store service flow.\n
 * User must open this service in order to perform cipher operation\n
 *
 * \param key_store_hdl handle identifying the key store service flow.
 * \param args pointer to the structure containing the function arguments.
 * \param cipher_hdl pointer to where the cipher service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_cipher_service(hsm_hdl_t key_store_hdl, open_svc_cipher_args_t *args, hsm_hdl_t *cipher_hdl);


typedef uint8_t hsm_op_cipher_one_go_algo_t;
typedef uint8_t hsm_op_cipher_one_go_flags_t;
typedef struct {
    uint32_t key_identifier;                    //!< identifier of the key to be used for the operation
    uint8_t *iv;                                //!< pointer to the initialization vector (nonce in case of AES CCM)
    uint16_t iv_size;                           //!< length in bytes of the initialization vector\n it must be 0 for algorithms not using the initialization vector.\n It must be 12 for AES in CCM mode
    hsm_op_cipher_one_go_algo_t cipher_algo;    //!< algorithm to be used for the operation
    hsm_op_cipher_one_go_flags_t flags;         //!< bitmap specifying the operation attributes
    uint8_t *input;                             //!< pointer to the input area\n plaintext for encryption\n ciphertext for decryption (in case of CCM is the purported ciphertext)
    uint8_t *output;                            //!< pointer to the output area\n ciphertext for encryption (in case of CCM is the output of the generation-encryption process)\n plaintext for decryption
    uint32_t input_size;                        //!< length in bytes of the input. In case of CBC and ECB, the input size should be multiple of a block cipher size (16 bytes).
    uint32_t output_size;                       //!< length in bytes of the output
} op_cipher_one_go_args_t;

/**
 * Perform ciphering operation\n
 * User can call this function only after having opened a cipher service flow
 *
 * \param cipher_hdl handle identifying the cipher service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */

hsm_err_t hsm_cipher_one_go(hsm_hdl_t cipher_hdl, op_cipher_one_go_args_t* args);
#define HSM_CIPHER_ONE_GO_ALGO_AES_ECB              ((hsm_op_cipher_one_go_algo_t)(0x00u))
#define HSM_CIPHER_ONE_GO_ALGO_AES_CBC              ((hsm_op_cipher_one_go_algo_t)(0x01u))
#define HSM_CIPHER_ONE_GO_ALGO_AES_CCM              ((hsm_op_cipher_one_go_algo_t)(0x04u))       //!< Perform AES CCM with following constraints: AES CCM where Adata = 0, Tlen = 16 bytes, nonce size = 12 bytes
#define HSM_CIPHER_ONE_GO_ALGO_SM4_ECB              ((hsm_op_cipher_one_go_algo_t)(0x10u))
#define HSM_CIPHER_ONE_GO_ALGO_SM4_CBC              ((hsm_op_cipher_one_go_algo_t)(0x11u))
#define HSM_CIPHER_ONE_GO_FLAGS_DECRYPT             ((hsm_op_cipher_one_go_flags_t)(0u << 0))
#define HSM_CIPHER_ONE_GO_FLAGS_ENCRYPT             ((hsm_op_cipher_one_go_flags_t)(1u << 0))

typedef uint8_t hsm_op_auth_enc_algo_t;
typedef uint8_t hsm_op_auth_enc_flags_t;
typedef struct {
    uint32_t key_identifier;                    //!< identifier of the key to be used for the operation
    uint8_t *iv;                                //!< pointer to the initialization vector or nonce
    uint16_t iv_size;                           //!< length in bytes of the initialization vector\n It must be 12 bytes.
    uint8_t *aad;                               //!< pointer to the additional authentication data
    uint16_t aad_size;                          //!< length in bytes of the additional authentication data
    hsm_op_auth_enc_algo_t ae_algo;             //!< algorithm to be used for the operation
    hsm_op_auth_enc_flags_t flags;              //!< bitmap specifying the operation attributes
    uint8_t *input;                             //!< pointer to the input area\n plaintext for encryption\n Ciphertext + Tag (16 bytes) for decryption
    uint8_t *output;                            //!< pointer to the output area\n Ciphertext + Tag (16 bytes) for encryption \n plaintext for decryption if the Tag is verified
    uint32_t input_size;                        //!< length in bytes of the input
    uint32_t output_size;                       //!< length in bytes of the output
} op_auth_enc_args_t;

/**
 * Perform authenticated encryption operation\n
 * User can call this function only after having opened a cipher service flow
 *
 * \param cipher_hdl handle identifying the cipher service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_auth_enc(hsm_hdl_t cipher_hdl, op_auth_enc_args_t* args);
#define HSM_AUTH_ENC_ALGO_AES_GCM              ((hsm_op_auth_enc_algo_t)(0x00u))       //!< Perform AES GCM with following constraints: AES GCM where AAD supported, Tag len = 16 bytes, IV len = 12 bytes
#define HSM_AUTH_ENC_FLAGS_DECRYPT             ((hsm_op_auth_enc_flags_t)(0u << 0))
#define HSM_AUTH_ENC_FLAGS_ENCRYPT             ((hsm_op_auth_enc_flags_t)(1u << 0))

typedef uint8_t hsm_op_ecies_dec_flags_t;
typedef struct {
    uint32_t key_identifier;                //!< identifier of the private key to be used for the operation
    uint8_t *input;                         //!< pointer to the VCT input
    uint8_t *p1;                            //!< pointer to the KDF P1 input parameter
    uint8_t *p2;                            //!< pointer to the MAC P2 input parameter should be NULL
    uint8_t *output;                        //!< pointer to the output area where the plaintext must be written
    uint32_t input_size;                    //!< length in bytes of the input VCT should be equal to 96 bytes
    uint32_t output_size;                   //!< length in bytes of the output plaintext should be equal to 16 bytes
    uint16_t p1_size;                       //!< length in bytes of the KDF P1 parameter should be equal to 32 bytes
    uint16_t p2_size;                       //!< length in bytes of the MAC P2 parameter should be zero reserved for generic use cases
    uint16_t mac_size;                      //!< length in bytes of the requested message authentication code should be equal to 16 bytes
    hsm_key_type_t key_type;                //!< indicates the type of the used key
    hsm_op_ecies_dec_flags_t flags;         //!< bitmap specifying the operation attributes.
} op_ecies_dec_args_t;

/**
 * Decrypt data usign ECIES \n
 * User can call this function only after having opened a cipher  store service flow.\n
 * ECIES is supported with the constraints specified in 1609.2-2016.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_ecies_decryption(hsm_hdl_t cipher_hdl, op_ecies_dec_args_t *args);

/**
 * Terminate a previously opened cipher service flow
 *
 * \param cipher_hdl pointer to handle identifying the cipher service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_cipher_service(hsm_hdl_t cipher_hdl);

/**
 *\addtogroup qxp_specific
 * \ref group4
 *
 * - \ref HSM_CIPHER_ONE_GO_ALGO_SM4_ECB is not supported.
 * - \ref HSM_CIPHER_ONE_GO_ALGO_SM4_CBC is not supported.
 *
 * - \ref hsm_ecies_decryption: This feature is disabled when part is running in FIPS approved mode. Any call to this API will results in a HSM_FEATURE_DISABLED error.
 * - \ref hsm_key_type_t of op_ecies_dec_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 are supported.
 *
 */

/**
 *\addtogroup dxl_specific
 * \ref group4
 *
 * - \ref hsm_key_type_t of op_ecies_dec_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256, HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_256 are supported.
 *
 */
/** @} end of cipher service flow */

/**
 *  @defgroup group5 Signature generation
 * @{
 */
typedef uint8_t hsm_svc_signature_generation_flags_t;
typedef struct {
    hsm_svc_signature_generation_flags_t flags;        //!< bitmap specifying the services properties.
    uint8_t reserved[3];
} open_svc_sign_gen_args_t;

/**
 * Open a signature generation service flow\n
 * User can call this function only after having opened a key store service flow.\n
 * User must open this service in order to perform signature generation operations.
 *
 * \param key_store_hdl handle identifying the key store service flow.
 * \param args pointer to the structure containing the function arguments.
 * \param signature_gen_hdl pointer to where the signature generation service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_signature_generation_service(hsm_hdl_t key_store_hdl, open_svc_sign_gen_args_t *args,  hsm_hdl_t *signature_gen_hdl);

/**
 * Terminate a previously opened signature generation service flow
 *
 * \param signature_gen_hdl handle identifying the signature generation service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_signature_generation_service(hsm_hdl_t signature_gen_hdl);


typedef uint8_t hsm_signature_scheme_id_t;
typedef uint8_t hsm_op_generate_sign_flags_t;
typedef struct {
    uint32_t key_identifier;                //!< identifier of the key to be used for the operation
    uint8_t *message;                       //!< pointer to the input (message or message digest) to be signed
    uint8_t *signature;                     //!< pointer to the output area where the signature must be stored. The signature S=(r,s) is stored in format r||s||Ry where Ry is an additional byte containing the lsb of y. Ry has to be considered valid only if the HSM_OP_GENERATE_SIGN_FLAGS_COMPRESSED_POINT is set.
    uint32_t message_size;                  //!< length in bytes of the input
    uint16_t signature_size;                //!< length in bytes of the output
    hsm_signature_scheme_id_t scheme_id;    //!< identifier of the digital signature scheme to be used for the operation
    hsm_op_generate_sign_flags_t flags;     //!< bitmap specifying the operation attributes
} op_generate_sign_args_t;


/**
 * Generate a digital signature according to the signature scheme\n
 * User can call this function only after having opened a signature generation service flow\n
 * The signature S=(r,s) is stored in the format r||s||Ry where Ry is an additional byte containing the lsb of y. Ry has to be considered valid only if the HSM_OP_GENERATE_SIGN_FLAGS_COMPRESSED_POINT is set.
 *
 * In case of HSM_SIGNATURE_SCHEME_DSA_SM2_FP_256_SM3, message of op_generate_sign_args_t should be (as specified in GB/T 32918): \n
 *       - equal to Z||M in case of HSM_OP_GENERATE_SIGN_FLAGS_INPUT_MESSAGE \n
 *       - equal to SM3(Z||M) in case of HSM_OP_GENERATE_SIGN_FLAGS_INPUT_DIGEST
 *
 * \param signature_gen_hdl handle identifying the signature generation service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_generate_signature(hsm_hdl_t signature_gen_hdl, op_generate_sign_args_t *args);
#define HSM_SIGNATURE_SCHEME_ECDSA_NIST_P256_SHA_256            ((hsm_signature_scheme_id_t)0x02u)
#define HSM_SIGNATURE_SCHEME_ECDSA_NIST_P384_SHA_384            ((hsm_signature_scheme_id_t)0x03u)
#define HSM_SIGNATURE_SCHEME_ECDSA_NIST_P521_SHA_512            ((hsm_signature_scheme_id_t)0x04u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_R1_256_SHA_256     ((hsm_signature_scheme_id_t)0x13u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_R1_320_SHA_384     ((hsm_signature_scheme_id_t)0x14u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_R1_384_SHA_384     ((hsm_signature_scheme_id_t)0x15u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_R1_512_SHA_512     ((hsm_signature_scheme_id_t)0x16u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_256_SHA_256     ((hsm_signature_scheme_id_t)0x23u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_320_SHA_384     ((hsm_signature_scheme_id_t)0x24u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_384_SHA_384     ((hsm_signature_scheme_id_t)0x25u)
#define HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_512_SHA_512     ((hsm_signature_scheme_id_t)0x26u)
#define HSM_SIGNATURE_SCHEME_DSA_SM2_FP_256_SM3                 ((hsm_signature_scheme_id_t)0x43u)
#define HSM_OP_GENERATE_SIGN_FLAGS_INPUT_DIGEST                 ((hsm_op_generate_sign_flags_t)(0u << 0))
#define HSM_OP_GENERATE_SIGN_FLAGS_INPUT_MESSAGE                ((hsm_op_generate_sign_flags_t)(1u << 0))
#define HSM_OP_GENERATE_SIGN_FLAGS_COMPRESSED_POINT             ((hsm_op_generate_sign_flags_t)(1u << 1))
#define HSM_OP_GENERATE_SIGN_FLAGS_LOW_LATENCY_SIGNATURE        ((hsm_op_generate_sign_flags_t)(1u << 2))        //! HSM finalizes the signature by using the artifacts of the previously executed hsm_prepare_signature API. The API fails if no artifacts related to the requested scheme id are available.


typedef uint8_t hsm_op_prepare_signature_flags_t;
typedef struct {
    hsm_signature_scheme_id_t scheme_id;        //!< identifier of the digital signature scheme to be used for the operation
    hsm_op_prepare_signature_flags_t flags;     //!< bitmap specifying the operation attributes
    uint16_t reserved;
} op_prepare_sign_args_t;

/**
 * Prepare the creation of a signature by pre-calculating the operations having not dependencies on the input message.\n
 * The pre-calculated value will be stored internally and used once call hsm_generate_signature \n
 * User can call this function only after having opened a signature generation service flow\n
 * The signature S=(r,s) is stored in the format r||s||Ry where Ry is an additional byte containing the lsb of y, Ry has to be considered valid only if the HSM_OP_PREPARE_SIGN_COMPRESSED_POINT is set.
 *
 * \param signature_gen_hdl handle identifying the signature generation service flow
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_prepare_signature(hsm_hdl_t signature_gen_hdl, op_prepare_sign_args_t *args);
#define HSM_OP_PREPARE_SIGN_INPUT_DIGEST           ((hsm_op_prepare_signature_flags_t)(0u << 0))
#define HSM_OP_PREPARE_SIGN_INPUT_MESSAGE          ((hsm_op_prepare_signature_flags_t)(1u << 0))
#define HSM_OP_PREPARE_SIGN_COMPRESSED_POINT       ((hsm_op_prepare_signature_flags_t)(1u << 1))

/**
 *\addtogroup qxp_specific
 * \ref group5
 *
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_NIST_P521_SHA_512 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_R1_320_SHA_384 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_R1_512_SHA_512 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_256_SHA_256 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_320_SHA_384 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_384_SHA_384 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_ECDSA_BRAINPOOL_T1_512_SHA_512 is not supported.
 * - \ref HSM_SIGNATURE_SCHEME_DSA_SM2_FP_256_SM3 is not supported.
 *
 */

/**
 *\addtogroup dxl_specific
 * \ref group5
 *
 * - \ref HSM_OP_GENERATE_SIGN_FLAGS_COMPRESSED_POINT is not supported, in case of HSM_SIGNATURE_SCHEME_DSA_SM2_FP_256_SM3.
 *
 */
/** @} end of signature generation service flow */


/**
 *  @defgroup group6 Signature verification
 * @{
 */
typedef uint8_t hsm_svc_signature_verification_flags_t;
typedef struct {
    hsm_svc_signature_verification_flags_t flags;   //!< bitmap indicating the service flow properties
    uint8_t reserved[3];
} open_svc_sign_ver_args_t;

/**
 * User must open this service in order to perform signature verification operations.\n
 * User can call this function only after having opened a session.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 * \param signature_ver_hdl pointer to where the signature verification service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_signature_verification_service(hsm_hdl_t session_hdl, open_svc_sign_ver_args_t *args, hsm_hdl_t *signature_ver_hdl);


typedef uint8_t hsm_op_verify_sign_flags_t;
typedef struct {
    uint8_t *key;                           //!< pointer to the public key to be used for the verification. If the HSM_OP_VERIFY_SIGN_FLAGS_KEY_INTERNAL is set, it must point to the key reference returned by the hsm_import_public_key API.
    uint8_t *message;                       //!< pointer to the input (message or message digest)
    uint8_t *signature;                     //!< pointer to the input signature. The signature S=(r,s) is expected to be in the format r||s||Ry where Ry is an additional byte containing the lsb of y. Ry will be considered as valid only if the HSM_OP_VERIFY_SIGN_FLAGS_COMPRESSED_POINT is set.
    uint16_t key_size;                      //!< length in bytes of the input key
    uint16_t signature_size;                //!< length in bytes of the output - it must contain one additional byte where to store the Ry.
    uint32_t message_size;                  //!< length in bytes of the input message
    hsm_signature_scheme_id_t scheme_id;    //!< identifier of the digital signature scheme to be used for the operation
    hsm_op_verify_sign_flags_t flags;       //!< bitmap specifying the operation attributes
    uint16_t reserved;
} op_verify_sign_args_t;

typedef uint32_t hsm_verification_status_t;
/**
 * Verify a digital signature according to the signature scheme\n
 * User can call this function only after having opened a signature verification service flow\n
 * The signature S=(r,s) is expected to be in format r||s||Ry where Ry is an additional byte containing the lsb of y. Ry will be considered as valid only if the HSM_OP_VERIFY_SIGN_FLAGS_COMPRESSED_POINT is set.\n
 * Only not-compressed keys (x,y) can be used by this command. Compressed keys can be decompressed by using the dedicated API.
 *
 * In case of HSM_SIGNATURE_SCHEME_DSA_SM2_FP_256_SM3, message of op_verify_sign_args_t should be (as specified in GB/T 32918): \n
 *      - equal to Z||M in case of HSM_OP_VERIFY_SIGN_FLAGS_INPUT_MESSAGE \n
 *      - equal to SM3(Z||M) in case of HSM_OP_VERIFY_SIGN_FLAGS_INPUT_DIGEST
 *
 * \param signature_ver_hdl handle identifying the signature verification service flow.
 * \param args pointer to the structure containing the function arguments.
 * \param status pointer to where the verification status must be stored\n if the verification suceed the value HSM_VERIFICATION_STATUS_SUCCESS is returned.
 *
 * \return error code
 */
hsm_err_t hsm_verify_signature(hsm_hdl_t signature_ver_hdl, op_verify_sign_args_t *args, hsm_verification_status_t *status);

#define HSM_OP_VERIFY_SIGN_FLAGS_INPUT_DIGEST               ((hsm_op_verify_sign_flags_t)(0u << 0))
#define HSM_OP_VERIFY_SIGN_FLAGS_INPUT_MESSAGE              ((hsm_op_verify_sign_flags_t)(1u << 0))
#define HSM_OP_VERIFY_SIGN_FLAGS_COMPRESSED_POINT           ((hsm_op_verify_sign_flags_t)(1u << 1))
#define HSM_OP_VERIFY_SIGN_FLAGS_KEY_INTERNAL               ((hsm_op_verify_sign_flags_t)(1u << 2)) //!< when set the value passed by the key argument is considered as the internal reference of a key imported through the hsm_import_pub_key API.
#define HSM_VERIFICATION_STATUS_SUCCESS                     ((hsm_verification_status_t)(0x5A3CC3A5u))


typedef uint8_t hsm_op_import_public_key_flags_t;
typedef struct {
    uint8_t *key;                               //!< pointer to the public key to be imported
    uint16_t key_size;                          //!< length in bytes of the input key
    hsm_key_type_t key_type;                    //!< indicates the type of the key to be imported.
    hsm_op_import_public_key_flags_t flags;     //!< bitmap specifying the operation attributes
} op_import_public_key_args_t;

/**
 * Import a public key to be used for several verification operations, a reference to the imported key is returned. \n
 * User can use the returned reference in the hsm_verify_signature API by setting the HSM_OP_VERIFY_SIGN_FLAGS_KEY_INTERNAL flag \n
 * Only not-compressed keys (x,y) can be imported by this command. Compressed keys can be decompressed by using the dedicated API.
 * User can call this function only after having opened a signature verification service flow.\n
 *
 * \param signature_ver_hdl handle identifying the signature verification service flow.
 * \param args pointer to the structure containing the function arguments.
 * \param key_ref pointer to where the 4 bytes key reference to be used as key in the hsm_verify_signature will be stored\n
 *
 * \return error code
 */
hsm_err_t hsm_import_public_key(hsm_hdl_t signature_ver_hdl, op_import_public_key_args_t *args, uint32_t *key_ref);

/**
 * Terminate a previously opened signature verification service flow
 *
 * \param signature_ver_hdl handle identifying the signature verification service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_signature_verification_service(hsm_hdl_t signature_ver_hdl);

/**
 *\addtogroup qxp_specific
 * \ref group6
 *
 * - \ref HSM_OP_VERIFY_SIGN_FLAGS_KEY_INTERNAL is not supported
 * - \ref hsm_import_public_key: This API is not supported
 *
 */

/**
 *\addtogroup dxl_specific
 * \ref group6
 *
 * - \ref HSM_OP_VERIFY_SIGN_FLAGS_COMPRESSED_POINT is not supported, in case of HSM_SIGNATURE_SCHEME_DSA_SM2_FP_256_SM3.
 * - \ref HSM_OP_VERIFY_SIGN_FLAGS_KEY_INTERNAL is not supported
 * - \ref hsm_import_public_key: This API is a preliminary version
 *
 */
/** @} end of signature verification service flow */

/**
 *  @defgroup group7 Random number generation
 * @{
 */

typedef uint8_t hsm_svc_rng_flags_t;
typedef struct {
    hsm_svc_rng_flags_t flags;                      //!< bitmap indicating the service flow properties
    uint8_t reserved[3];
} open_svc_rng_args_t;

/**
 * Open a random number generation service flow\n
 * User can call this function only after having opened a session.\n
 * User must open this service in order to perform rng operations.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 * \param rng_hdl pointer to where the rng service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_rng_service(hsm_hdl_t session_hdl, open_svc_rng_args_t *args, hsm_hdl_t *rng_hdl);

/**
 * Terminate a previously opened rng service flow
 *
 * \param rng_hdl handle identifying the rng service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_rng_service(hsm_hdl_t rng_hdl);


typedef struct {
    uint8_t *output;                        //!< pointer to the output area where the random number must be written
    uint32_t random_size;                   //!< length in bytes of the random number to be provided.
} op_get_random_args_t;

/**
 * Get a freshly generated random number\n
 * User can call this function only after having opened a rng service flow
 *
 * \param rng_hdl handle identifying the rng service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_get_random(hsm_hdl_t rng_hdl, op_get_random_args_t *args);
/** @} end of rng service flow */

/**
 *  @defgroup group8 Hashing
 * @{
 */
typedef uint8_t hsm_svc_hash_flags_t;
typedef struct {
    hsm_svc_hash_flags_t flags;                      //!< bitmap indicating the service flow properties
    uint8_t reserved[3];
} open_svc_hash_args_t;

/**
 * Open an hash service flow\n
 * User can call this function only after having opened a session.\n
 * User must open this service in order to perform hash operations.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 * \param hash_hdl pointer to where the hash service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_hash_service(hsm_hdl_t session_hdl, open_svc_hash_args_t *args, hsm_hdl_t *hash_hdl);

/**
 * Terminate a previously opened hash service flow
 *
 * \param hash_hdl handle identifying the hash service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_hash_service(hsm_hdl_t hash_hdl);

typedef uint8_t hsm_hash_algo_t;
typedef uint8_t hsm_op_hash_one_go_flags_t;
typedef struct {
    uint8_t *input;                     //!< pointer to the input data to be hashed
    uint8_t *output;                    //!< pointer to the output area where the resulting digest must be written
    uint32_t input_size;                //!< length in bytes of the input
    uint32_t output_size;               //!< length in bytes of the output
    hsm_hash_algo_t algo;               //!< hash algorithm to be used for the operation
    hsm_op_hash_one_go_flags_t flags;   //!< flags bitmap specifying the operation attributes.
    uint16_t reserved;
} op_hash_one_go_args_t;

/**
 * Perform the hash operation on a given input\n
 * User can call this function only after having opened a hash service flow
 *
 * \param hash_hdl handle identifying the hash service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_hash_one_go(hsm_hdl_t hash_hdl, op_hash_one_go_args_t *args);
#define HSM_HASH_ALGO_SHA_224      ((hsm_hash_algo_t)(0x0u))
#define HSM_HASH_ALGO_SHA_256      ((hsm_hash_algo_t)(0x1u))
#define HSM_HASH_ALGO_SHA_384      ((hsm_hash_algo_t)(0x2u))
#define HSM_HASH_ALGO_SHA_512      ((hsm_hash_algo_t)(0x3u))
#define HSM_HASH_ALGO_SM3_256      ((hsm_hash_algo_t)(0x11u))

/**
 *\addtogroup qxp_specific
 * \ref group5
 *
 * - \ref HSM_HASH_ALGO_SM3_256 is not supported.
 *
 */
/** @} end of hash service flow */

/**
 *  @defgroup group9 Public key reconstruction
 * @{
 */
typedef uint8_t hsm_op_pub_key_rec_flags_t;
typedef struct {
    uint8_t *pub_rec;                       //!< pointer to the public reconstruction value extracted from the implicit certificate.
    uint8_t *hash;                          //!< pointer to the input hash value. In the butterfly scheme it corresponds to the hash value calculated over PCA certificate and, concatenated, the implicit certificat.
    uint8_t *ca_key;                        //!< pointer to the CA public key
    uint8_t *out_key;                       //!< pointer to the output area where the reconstructed public key must be written.
    uint16_t pub_rec_size;                  //!< length in bytes of the public reconstruction value
    uint16_t hash_size;                     //!< length in bytes of the input hash
    uint16_t ca_key_size;                   //!< length in bytes of the input CA public key
    uint16_t out_key_size;                  //!< length in bytes of the output key
    hsm_key_type_t key_type;                //!< indicates the type of the managed key.
    hsm_op_pub_key_rec_flags_t flags;       //!< flags bitmap specifying the operation attributes.
    uint16_t reserved;
} op_pub_key_rec_args_t;

/**
 * Reconstruct an ECC public key provided by an implicit certificate\n
 * User can call this function only after having opened a session\n
 * This API implements the followign formula:\n
 * out_key = (pub_rec * hash) + ca_key
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_pub_key_reconstruction(hsm_hdl_t session_hdl,  op_pub_key_rec_args_t *args);

/**
 *\addtogroup qxp_specific
 * \ref group9
 *
 * - \ref This feature is disabled when part is running in FIPS approved mode. Any call to this API will results in a HSM_FEATURE_DISABLED error.
 * - \ref hsm_key_type_t of op_pub_key_rec_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 are supported.
 *
 */
/**
 *\addtogroup dxl_specific
 * \ref group9
 *
 * - \ref hsm_key_type_t of op_pub_key_rec_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256, HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_256 are supported.
 *
 */
/** @} end of public key reconstruction operation */

/**
 *  @defgroup group10 Public key decompression
 * @{
 */
typedef uint8_t hsm_op_pub_key_dec_flags_t;
typedef struct {
    uint8_t *key;                           //!< pointer to the compressed ECC public key. The expected key format is x||lsb_y where lsb_y is 1 byte having value 1 if the least-significant bit of the original (uncompressed) y coordinate is set, and 0 otherwise.
    uint8_t *out_key;                       //!< pointer to the output area where the decompressed public key must be written.
    uint16_t key_size;                      //!< length in bytes of the input compressed public key
    uint16_t out_key_size;                  //!< length in bytes of the resulting public key
    hsm_key_type_t key_type;                //!< indicates the type of the manged keys.
    hsm_op_pub_key_dec_flags_t flags;       //!< bitmap specifying the operation attributes.
    uint16_t reserved;
} op_pub_key_dec_args_t;

/**
 * Decompress an ECC public key \n
 * The expected key format is x||lsb_y where lsb_y is 1 byte having value 1 if the least-significant bit of the original (uncompressed) y coordinate is set, and 0 otherwise.\n
 * User can call this function only after having opened a session
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_pub_key_decompression(hsm_hdl_t session_hdl,  op_pub_key_dec_args_t *args);

/**
 *\addtogroup qxp_specific
 * \ref group10
 *
 * - \ref This feature is disabled when part is running in FIPS approved mode. Any call to this API will results in a HSM_FEATURE_DISABLED error.
 */
/** @} end of public key decompression operation */

/**
 *  @defgroup group11 ECIES encryption
 * @{
 */
typedef uint8_t hsm_op_ecies_enc_flags_t;
typedef struct {
    uint8_t *input;                         //!< pointer to the input plaintext
    uint8_t *pub_key;                       //!< pointer to the input recipient public key
    uint8_t *p1;                            //!< pointer to the KDF P1 input parameter
    uint8_t *p2;                            //!< pointer to the MAC P2 input parameter should be NULL
    uint8_t *output;                        //!< pointer to the output area where the VCT must be written
    uint32_t input_size;                    //!< length in bytes of the input plaintext should be equal to 16 bytes
    uint16_t p1_size;                       //!< length in bytes of the KDF P1 parameter should be equal to 32 bytes
    uint16_t p2_size;                       //!< length in bytes of the MAC P2 parameter should be zero reserved for generic use cases
    uint16_t pub_key_size;                  //!< length in bytes of the recipient public key should be equal to 64 bytes
    uint16_t mac_size;                      //!< length in bytes of the requested message authentication code should be equal to 16 bytes
    uint32_t out_size;                      //!< length in bytes of the output VCT should be equal to 96 bytes
    hsm_key_type_t key_type;                //!< indicates the type of the recipient public key
    hsm_op_ecies_enc_flags_t flags;         //!< bitmap specifying the operation attributes.
    uint16_t reserved;
} op_ecies_enc_args_t;

/**
 * Encrypt data usign ECIES \n
 * User can call this function only after having opened a session.\n
 * ECIES is supported with the constraints specified in 1609.2-2016.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_ecies_encryption(hsm_hdl_t session_hdl, op_ecies_enc_args_t *args);

/**
 *\addtogroup qxp_specific
 * \ref group11
 *
 * - \ref hsm_ecies_encryption: This feature is disabled when part is running in FIPS approved mode. Any call to this API will results in a HSM_FEATURE_DISABLED error.
 * - \ref hsm_key_type_t of op_ecies_enc_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 are supported.
 *
 */

/**
 *\addtogroup dxl_specific
 * \ref group11
 *
 * - \ref hsm_key_type_t of op_ecies_enc_args_t: Only HSM_KEY_TYPE_ECDSA_NIST_P256, HSM_KEY_TYPE_ECDSA_BRAINPOOL_R1_256 and HSM_KEY_TYPE_ECDSA_BRAINPOOL_T1_256 are supported.
 *
 */
/** @} end of ECIES encryption operation */

/**
 *  @defgroup group12 Public key recovery
 * @{
 */
typedef uint8_t hsm_op_pub_key_recovery_flags_t;
typedef struct {
    uint32_t key_identifier;                //!< pointer to the identifier of the key to be used for the operation
    uint8_t *out_key;                       //!< pointer to the output area where the generated public key must be written
    uint16_t out_key_size;                  //!< length in bytes of the output key
    hsm_key_type_t key_type;                //!< indicates the type of the key to be recovered
    hsm_op_pub_key_recovery_flags_t flags;  //!< bitmap specifying the operation attributes.
} op_pub_key_recovery_args_t;

/**
 * Recover Public key from private key present in key store \n
 * User can call this function only after having opened a key store.\n
 *
 * \param key_store_hdl handle identifying the current key store.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_pub_key_recovery(hsm_hdl_t key_store_hdl, op_pub_key_recovery_args_t *args);
/** @} end of Public key recovery operation */

/**
 *  @defgroup group13 Data storage
 * @{
 */

typedef uint8_t hsm_svc_data_storage_flags_t;
typedef struct {
    hsm_svc_data_storage_flags_t flags;   //!< bitmap specifying the services properties.
    uint8_t reserved[3];
} open_svc_data_storage_args_t;

/**
 * Open a data storage service flow\n
 * User must open this service flow in order to store/retreive generic data in/from the HSM.
 *
 * \param key_store_hdl handle identifying the key store service flow.
 * \param args pointer to the structure containing the function arguments.

 * \param data_storage_hdl pointer to where the data storage service flow handle must be written.
 *
 * \return error_code error code.
 */
hsm_err_t hsm_open_data_storage_service(hsm_hdl_t key_store_hdl, open_svc_data_storage_args_t *args, hsm_hdl_t *data_storage_hdl);

typedef uint8_t hsm_op_data_storage_flags_t;
typedef struct {
    uint8_t *data;                       //!< pointer to the data. In case of store request, it will be the input data to store. In case of retrieve, it will be the the pointer where to load data.
    uint32_t data_size;                  //!< length in bytes of the data
    uint16_t data_id;                    //!< id of the data
    hsm_op_data_storage_flags_t flags;   //!< flags bitmap specifying the operation attributes.
    uint8_t reserved;
} op_data_storage_args_t;

/**
 * Store or retrieve generic data identified by a data_id. \n
 *
 * \param data_storage_hdl handle identifying the data storage service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_data_storage(hsm_hdl_t data_storage_hdl, op_data_storage_args_t *args);
#define HSM_OP_DATA_STORAGE_FLAGS_STORE                  ((hsm_op_data_storage_flags_t)(1u << 0))  //!< Store data.
#define HSM_OP_DATA_STORAGE_FLAGS_RETRIEVE               ((hsm_op_data_storage_flags_t)(0u << 0))  //!< Retrieve data.

/**
 * Terminate a previously opened data storage service flow
 *
 * \param data_storage_hdl handle identifying the data storage service flow.
 *
 * \return error code
 */
hsm_err_t hsm_close_data_storage_service(hsm_hdl_t data_storage_hdl);
/** @} end of data storage service flow */

/**
 *  @defgroup group14 Root KEK export
 * @{
 */
typedef uint8_t hsm_op_export_root_kek_flags_t;
typedef struct {
    uint8_t *signed_message;                   //!< pointer to signed_message authorizing the operation
    uint8_t *out_root_kek;                     //!< pointer to the output area where the derived root kek (key encryption key) must be written
    uint16_t signed_msg_size;                  //!< size of the signed_message authorizing the operation
    uint8_t root_kek_size;                     //!< length in bytes of the root kek. Must be 32 bytes.
    hsm_op_export_root_kek_flags_t flags;      //!< flags bitmap specifying the operation attributes.
    uint8_t reserved[2];
} op_export_root_kek_args_t;

/**
 * Export the root key encryption key. This key is derived on chip. It can be common or chip unique.
 * This key will be used to import key in the key store through the manage key API.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_export_root_key_encryption_key (hsm_hdl_t session_hdl,  op_export_root_kek_args_t *args);
#define HSM_OP_EXPORT_ROOT_KEK_FLAGS_COMMON_KEK   ((hsm_op_export_root_kek_flags_t)(1u << 0))
#define HSM_OP_EXPORT_ROOT_KEK_FLAGS_UNIQUE_KEK   ((hsm_op_export_root_kek_flags_t)(0u << 0))
/** @} end of export root key encryption key operation */

/**
 *  @defgroup group15 Get info
 * @{
 */
typedef struct {
    uint32_t *user_sab_id;              //!< pointer to the output area where the user identifier (32bits) must be written
    uint8_t  *chip_unique_id;           //!< pointer to the output area where the chip unique identifier (64bits) must be written
    uint16_t *chip_monotonic_counter;   //!< pointer to the output are where the chip monotonic counter value (16bits) must be written
    uint16_t *chip_life_cycle;          //!< pointer to the output area where the chip current life cycle bitfield (16bits) must be written
    uint32_t *version;                  //!< pointer to the output area where the module version (32bits) must be written
    uint32_t *version_ext;              //!< pointer to the output area where module extended version (32bits) must be written
    uint8_t  *fips_mode;                //!< pointer to the output area where the FIPS mode bitfield (8bits) must be written. Bitmask definition:\n bit0 - FIPS mode of operation:\n- value 0 - part is running in FIPS non-approved mode.\n- value 1 - part is running in FIPS approved mode.\n bit1 - FIPS certified part:\n- value 0 - part is not FIPS certified.\n- value 1 - part is FIPS certified.\n bit2-7: reserved - 0 value.
} op_get_info_args_t;
/**
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */

hsm_err_t hsm_get_info(hsm_hdl_t session_hdl, op_get_info_args_t *args);

/** @} end of Get info operation */

/**
 *  @defgroup group16 Mac
 * @{
 */

typedef uint8_t hsm_svc_mac_flags_t;
typedef struct {
    hsm_svc_mac_flags_t flags;           //!< bitmap specifying the services properties.
    uint8_t reserved[3];
} open_svc_mac_args_t;

/**
 * Open a mac service flow\n
 * User can call this function only after having opened a key store service flow.\n
 * User must open this service in order to perform mac operation\n
 *
 * \param key_store_hdl handle identifying the key store service flow.
 * \param args pointer to the structure containing the function arguments.
 * \param mac_hdl pointer to where the mac service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_mac_service(hsm_hdl_t key_store_hdl, open_svc_mac_args_t *args, hsm_hdl_t *mac_hdl);


typedef uint8_t hsm_op_mac_one_go_algo_t;
typedef uint8_t hsm_op_mac_one_go_flags_t;

typedef struct {
    uint32_t key_identifier;                    //!< identifier of the key to be used for the operation
    hsm_op_mac_one_go_algo_t algorithm;         //!< algorithm to be used for the operation
    hsm_op_mac_one_go_flags_t flags;            //!< bitmap specifying the operation attributes
    uint8_t *payload;                           //!< pointer to the payload area\n
    uint8_t *mac;                               //!< pointer to the tag area\n
    uint16_t payload_size;                      //!< length in bytes of the payload
    uint16_t mac_size;                          //!< length in bytes of the tag\n the value is in range from 4 to 16 bytes.
} op_mac_one_go_args_t;

typedef uint32_t hsm_mac_verification_status_t;
/**
 * Perform mac operation\n
 * User can call this function only after having opened a mac service flow\n
 * For CMAC algorithm, a key of type HSM_KEY_TYPE_AES_XXX must be used\n
 * For HMAC algorithm, a key of type HSM_KEY_TYPE_HMAC_XXX must be used
 *
 * \param mac_hdl handle identifying the mac service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_mac_one_go(hsm_hdl_t mac_hdl, op_mac_one_go_args_t* args, hsm_mac_verification_status_t *status);

#define HSM_OP_MAC_ONE_GO_FLAGS_MAC_VERIFICATION       ((hsm_op_mac_one_go_flags_t)(0u << 0))
#define HSM_OP_MAC_ONE_GO_FLAGS_MAC_GENERATION         ((hsm_op_mac_one_go_flags_t)(1u << 0))
#define HSM_OP_MAC_ONE_GO_ALGO_AES_CMAC                ((hsm_op_mac_one_go_algo_t)(0x01u))
#define HSM_OP_MAC_ONE_GO_ALGO_HMAC_SHA_224            ((hsm_op_mac_one_go_algo_t)(0x05u))
#define HSM_OP_MAC_ONE_GO_ALGO_HMAC_SHA_256            ((hsm_op_mac_one_go_algo_t)(0x06u))
#define HSM_OP_MAC_ONE_GO_ALGO_HMAC_SHA_384            ((hsm_op_mac_one_go_algo_t)(0x07u))
#define HSM_OP_MAC_ONE_GO_ALGO_HMAC_SHA_512            ((hsm_op_mac_one_go_algo_t)(0x08u))
#define HSM_MAC_VERIFICATION_STATUS_SUCCESS            ((hsm_mac_verification_status_t)(0x6C1AA1C6u))

/**
 * Terminate a previously opened mac service flow
 *
 * \param mac_hdl pointer to handle identifying the mac service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_mac_service(hsm_hdl_t mac_hdl);

/** @} end of mac service flow */

/**
 *  @defgroup group17 SM2 Get Z
 * @{
 */
typedef uint8_t hsm_op_sm2_get_z_flags_t;
typedef struct {
    uint8_t *public_key;                  //!< pointer to the sender public key
    uint8_t *identifier;                  //!< pointer to the sender identifier
    uint8_t *z_value;                     //!< pointer to the output area where the Z value must be written
    uint16_t public_key_size;             //!< length in bytes of the sender public key should be equal to 64 bytes
    uint8_t id_size;                      //!< length in bytes of the identifier
    uint8_t z_size;                       //!< length in bytes of Z should be at least 32 bytes
    hsm_key_type_t key_type;              //!< indicates the type of the sender public key. Only HSM_KEY_TYPE_DSA_SM2_FP_256 is supported.
    hsm_op_sm2_get_z_flags_t flags;       //!< bitmap specifying the operation attributes.
    uint8_t reserved[2];
} op_sm2_get_z_args_t;

/**
 * This command is designed to compute  Z = SM3(Entl || ID || a || b || xG || yG || xpubk || ypubk) \n
 *  - ID, Entl: user distinguishing identifier and length,
 *  - a, b, xG and yG : curve parameters,
 *  - xpubk , ypubk : public key \n\n
 * This value is used for SM2 public key cryptography algorithms, as specified in GB/T 32918.
 * User can call this function only after having opened a session.\n
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_sm2_get_z(hsm_hdl_t session_hdl, op_sm2_get_z_args_t *args);

/**
 *\addtogroup qxp_specific
 * \ref group17
 *
 * - \ref This API is not supported.
 *
 */
/** @} end of SM2 Get Z operation */

/**
 *  @defgroup group18 SM2 ECES decryption
 * @{
 */

typedef uint8_t hsm_svc_sm2_eces_flags_t;
typedef struct {
    hsm_svc_sm2_eces_flags_t flags;           //!< bitmap indicating the service flow properties
    uint8_t reserved[3];
} open_svc_sm2_eces_args_t;

/**
 * Open a SM2 ECES decryption service flow\n
 * User can call this function only after having opened a key store.\n
 * User must open this service in order to perform SM2 decryption.
 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 * \param sm2_eces_hdl pointer to where the sm2 eces service flow handle must be written.
 *
 * \return error code
 */
hsm_err_t hsm_open_sm2_eces_service(hsm_hdl_t key_store_hdl, open_svc_sm2_eces_args_t *args, hsm_hdl_t *sm2_eces_hdl);

/**
 * Terminate a previously opened SM2 ECES service flow
 *
 * \param sm2_eces_hdl handle identifying the SM2 ECES service flow to be closed.
 *
 * \return error code
 */
hsm_err_t hsm_close_sm2_eces_service(hsm_hdl_t sm2_eces_hdl);

typedef uint8_t hsm_op_sm2_eces_dec_flags_t;
typedef struct {
    uint32_t key_identifier;                //!< identifier of the private key to be used for the operation
    uint8_t *input;                         //!< pointer to the input ciphertext
    uint8_t *output;                        //!< pointer to the output area where the plaintext must be written
    uint32_t input_size;                    //!< length in bytes of the input ciphertext.
    uint32_t output_size;                   //!< length in bytes of the output plaintext
    hsm_key_type_t key_type;                //!< indicates the type of the used key. Only HSM_KEY_TYPE_DSA_SM2_FP_256 is supported.
    hsm_op_sm2_eces_dec_flags_t flags;      //!< bitmap specifying the operation attributes.
    uint16_t reserved;
} op_sm2_eces_dec_args_t;

/**
 * Decrypt data usign SM2 ECES \n
 * User can call this function only after having opened a SM2 ECES service flow.\n
 * SM2 ECES is supported with the requirements specified in the GB/T 32918.4.
 *
 * \param sm2_eces_hdl handle identifying the SM2 ECES
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_sm2_eces_decryption(hsm_hdl_t sm2_eces_hdl, op_sm2_eces_dec_args_t *args);

/**
 *\addtogroup qxp_specific
 * \ref group18
 *
 * - \ref All the APIs related the SM2 ECES decryption are not supported.
 *
 */
/**
 *\addtogroup dxl_specific
 * \ref group18
 *
 * - \ref The output_size should be a multiple of 4 bytes.
 *
 */
/** @} end of SM2 ECES decryption flow */

/**
 *  @defgroup group19 SM2 ECES encryption
 * @{
 */

typedef uint8_t hsm_op_sm2_eces_enc_flags_t;
typedef struct {
    uint8_t *input;                         //!< pointer to the input plaintext
    uint8_t *output;                        //!< pointer to the output area where the ciphertext must be written
    uint8_t *pub_key;                       //!< pointer to the input recipient public key
    uint32_t input_size;                    //!< length in bytes of the input plaintext
    uint32_t output_size;                   //!< length in bytes of the output ciphertext. \n It should be at least input_size + 97 bytes (overhead related to C1 and C3 - as specifed below) + size alignment constraints specific to a given implementation (see related chapter).
    uint16_t pub_key_size;                  //!< length in bytes of the recipient public key should be equal to 64 bytes
    hsm_key_type_t key_type;                //!< indicates the type of the recipient public key. Only HSM_KEY_TYPE_DSA_SM2_FP_256 is supported.
    hsm_op_sm2_eces_enc_flags_t flags;      //!< bitmap specifying the operation attributes.
} op_sm2_eces_enc_args_t;

/**
 * Encrypt data usign SM2 ECES \n
 * User can call this function only after having opened a session.\n
 * SM2 ECES is supported with the requirements specified in the GB/T 32918.4. \n
 * The output (i.e. ciphertext) is stored in the format C= C1||C2||C3 : \n
 *      C1 = PC||x1||y1  where PC=04 and (x1,y1) are the coordinates of a an elliptic curve point \n
 *      C2 = M xor t where t=KDF(x2||y2, input_size) and (x2,y2) are the coordinates of a an elliptic curve point \n
 *      C3 = SM3 (x2||M||y2)

 *
 * \param session_hdl handle identifying the current session.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
 */
hsm_err_t hsm_sm2_eces_encryption(hsm_hdl_t session_hdl, op_sm2_eces_enc_args_t *args);

/**
 *\addtogroup qxp_specific
 * \ref group19
 *
 * - \ref This API is not supported.
 *
 */
/**
 *\addtogroup dxl_specific
 * \ref group19
 *
 * - \ref The output_size should be a multiple of 4 bytes.
 *
 */
/** @} end of SM2 ECES encryption operation */

/**
 *  @defgroup group20 Key exchange
 * @{
 */
typedef uint8_t hsm_kdf_algo_id_t;
typedef uint8_t hsm_key_exchange_scheme_id_t;
typedef uint8_t hsm_op_key_exchange_flags_t;
typedef struct {
    uint32_t key_identifier;                            //!< identifier of the key used for derivation. It must be zero, if HSM_OP_KEY_EXCHANGE_FLAGS_USE_EPHEMERAL is set.
    uint8_t *shared_key_identifier_array;               //!< pointer to the identifiers of the derived keys. In case of create operation the new destination key identifiers will be stored in this location.\n In case of HSM_KDF_HMAC_SHA_256_TLS_0_16_4, HSM_KDF_HMAC_SHA_384_TLS_0_32_4 or HSM_KDF_HMAC_SHA_256_TLS_0_32_4 KDF it contains the concatenation of client_write_key id (4 bytes) and the server_write_key id (4 bytes). In case of HSM_KDF_HMAC_SHA_256_TLS_32_16_4 or HSM_KDF_HMAC_SHA_384_TLS_48_32_4 KDF it contains the concatenation of client_write_MAC_key id (4 bytes), server_write_MAC_key id (4 bytes), client_write_key id (4 bytes) and the server_write_key id (4 bytes).
    uint8_t *ke_input;                                  //!< pointer to the initiator input data related to the key exchange function.
    uint8_t *ke_output;                                 //!< pointer to the output area where the data related to the key exchange function must be written. It corresponds to the receiver public data.\n
    uint8_t *kdf_input;                                 //!< pointer to the input data of the KDF.\n In case of HSM_KDF_HMAC_SHA_256_TLS_0_16_4, HSM_KDF_HMAC_SHA_384_TLS_0_32_4 KDF, HSM_KDF_HMAC_SHA_256_TLS_0_32_4, HSM_KDF_HMAC_SHA_256_TLS_32_16_4 or HSM_KDF_HMAC_SHA_384_TLS_48_32_4 it must contain to the concatenarion of clientHello_random (32 bytes), serverHello_random (32 bytes), server_random (32 bytes) and client_random (32 bytes), it must be 0 otherwise
    uint8_t *kdf_output;                                //!< pointer to the output area where the non sensitive output data related to the KDF are written. In case of HSM_KDF_HMAC_SHA_256_TLS_0_16_4, HSM_KDF_HMAC_SHA_384_TLS_0_32_4 KDF, HSM_KDF_HMAC_SHA_256_TLS_0_32_4, HSM_KDF_HMAC_SHA_256_TLS_32_16_4 or HSM_KDF_HMAC_SHA_384_TLS_48_32_4 KDF the concatenation of client_write_iv (4 bytes) and server_write_iv (4 bytes) will be stored at this address, it must be 0 otherwise
    hsm_key_group_t shared_key_group;                   //!< It specifies the group where the derived keys will be stored.\n It must be a value in the range 0-1023. Keys belonging to the same group can be cached in the HSM local memory throug the hsm_manage_key_group API
    hsm_key_info_t shared_key_info;                     //!< bitmap specifying the properties of the derived keys, it will be applied to all the derived keys.
    hsm_key_type_t shared_key_type;                     //!< indicates the type of the derived key. Not relevant in case of HSM_KDF_HMAC_SHA_256_TLS_0_16_4, HSM_KDF_HMAC_SHA_384_TLS_0_32_4 KDF, HSM_KDF_HMAC_SHA_256_TLS_0_32_4, HSM_KDF_HMAC_SHA_256_TLS_32_16_4 or HSM_KDF_HMAC_SHA_384_TLS_48_32_4 KDF
    hsm_key_type_t initiator_public_data_type;          //!< indicates the public data type specified by the initiator, e.g. public key type.\n For CMAC KDF, this must be HSM_KEY_TYPE_ECDSA_NIST_P256.\n For HMAC KDF, this can be HSM_KEY_TYPE_ECDSA_NIST_P256 or HSM_KEY_TYPE_ECDSA_NIST_P384.
    hsm_key_exchange_scheme_id_t key_exchange_scheme;   //!< indicates the key exchange scheme
    hsm_kdf_algo_id_t kdf_algorithm;                    //!< indicates the KDF algorithm
    uint16_t ke_input_size;                             //!< length in bytes of the input data of the key exchange function
    uint16_t ke_output_size;                            //!< length in bytes of the output data of the key exchange function
    uint8_t shared_key_identifier_array_size;           //!< length in byte of the area containing the shared key identifiers
    uint8_t kdf_input_size;                             //!< length in bytes of the input data of the KDF. It must be 128 bytes in case of HSM_KDF_HMAC_SHA_256_TLS_0_16_4, HSM_KDF_HMAC_SHA_384_TLS_0_32_4 KDF, HSM_KDF_HMAC_SHA_256_TLS_0_32_4 KDF, HSM_KDF_HMAC_SHA_256_TLS_32_16_4 or HSM_KDF_HMAC_SHA_384_TLS_48_32_4, 0 otherwise
    uint8_t kdf_output_size;                            //!< length in bytes of the non sensitive output data related to the KDF. It must be 8 bytes in case of HSM_KDF_HMAC_SHA_256_TLS_0_16_4, HSM_KDF_HMAC_SHA_384_TLS_0_32_4, HSM_KDF_HMAC_SHA_256_TLS_0_32_4 KDF, HSM_KDF_HMAC_SHA_256_TLS_32_16_4 or HSM_KDF_HMAC_SHA_384_TLS_48_32_4 KDF
    hsm_op_key_exchange_flags_t flags;                  //!< bitmap specifying the operation properties
} op_key_exchange_args_t;

/**
 * This command is designed to to derive a secret key that will be stored in the key store as a new key or as an update of an existing key.\n
 * A freshly generated key or an existing key can be used as input for the shared secret calculation.\n
 * User can call this function only after having opened a key management service flow.\n
 * When using CMAC KDF, only Key Encryption Keys (KEKs) can be generated. The input data to the CMAC uses the format described in NIST SP 800-108, with Context set to 'NXP HSM KEY ENCRYPTION KEY' and Label set to 'NXP HSM USER_KEK'.
 *
 * \param key_management_hdl handle identifying the key store management service flow.
 * \param args pointer to the structure containing the function arguments.
 *
 * \return error code
*/
hsm_err_t hsm_key_exchange(hsm_hdl_t key_management_hdl, op_key_exchange_args_t *args);
#define HSM_KDF_ALG_AES_CMAC_256_COUNTER                ((hsm_kdf_algo_id_t)0x00u)  //!< CMAC KDF can only be used to generate KEKs (key encryption keys)
#define HSM_KDF_ALG_FOR_SM2                             ((hsm_kdf_algo_id_t)0x10u)
#define HSM_KDF_HMAC_SHA_256_TLS_0_16_4                 ((hsm_kdf_algo_id_t)0x20u)  //!< TLS PRF based on HMAC with SHA-256, the resulting mac_key_length is 0 bytes, enc_key_length is 16 bytes and fixed_iv_length is 4 bytes.
#define HSM_KDF_HMAC_SHA_384_TLS_0_32_4                 ((hsm_kdf_algo_id_t)0x21u)  //!< TLS PRF based on HMAC with SHA-384, the resulting mac_key_length is 0 bytes, enc_key_length is 32 bytes and fixed_iv_length is 4 bytes.
#define HSM_KDF_HMAC_SHA_256_TLS_0_32_4                 ((hsm_kdf_algo_id_t)0x22u)  //!< TLS PRF based on HMAC with SHA-256, the resulting mac_key_length is 0 bytes, enc_key_length is 32 bytes and fixed_iv_length is 4 bytes.
#define HSM_KDF_HMAC_SHA_256_TLS_32_16_4                ((hsm_kdf_algo_id_t)0x23u)  //!< TLS PRF based on HMAC with SHA-256, the resulting mac_key_length is 32 bytes, enc_key_length is 16 bytes and fixed_iv_length is 4 bytes.
#define HSM_KDF_HMAC_SHA_384_TLS_48_32_4                ((hsm_kdf_algo_id_t)0x24u)  //!< TLS PRF based on HMAC with SHA-384, the resulting mac_key_length is 48 bytes, enc_key_length is 32 bytes and fixed_iv_length is 4 bytes.
#define HSM_KE_SCHEME_ECDH_NIST_P256                    ((hsm_key_exchange_scheme_id_t)0x02u)
#define HSM_KE_SCHEME_ECDH_NIST_P384                    ((hsm_key_exchange_scheme_id_t)0x03u)
#define HSM_KE_SCHEME_SM2_FP_256                        ((hsm_key_exchange_scheme_id_t)0x42u)
#define HSM_OP_KEY_EXCHANGE_FLAGS_UPDATE                ((hsm_op_key_exchange_flags_t)(1u << 0))  //!< User can replace an existing key only by the derived key which should have the same type of the original one.
#define HSM_OP_KEY_EXCHANGE_FLAGS_CREATE                ((hsm_op_key_exchange_flags_t)(1u << 1))  //!< Create a new key
#define HSM_OP_KEY_EXCHANGE_FLAGS_USE_EPHEMERAL         ((hsm_op_key_exchange_flags_t)(1u << 2))  //!< Use an ephemeral key (freshly generated key)
#define HSM_OP_KEY_EXCHANGE_FLAGS_STRICT_OPERATION      ((hsm_op_key_exchange_flags_t)(1u << 7))  //!< The request is completed only when the new key has been written in the NVM. This applicable for persistent key only.


/**
 *\addtogroup qxp_specific
 * \ref group20
 *
 * - \ref HSM_KE_SCHEME_SM2_FP_256 is not supported.
 * - \ref HSM_KDF_ALG_FOR_SM2 is not supported.
 *
 */
/**
 *\addtogroup dxl_specific
 * \ref group20
 *
 * - \ref This API should be considered as a preliminary version.
 * - \ref HSM_KDF_HMAC_SHA_256_TLS_0_16_4 is not supported.
 * - \ref HSM_KDF_HMAC_SHA_384_TLS_0_32_4 is not supported.
 *
 */
/** @} end of key exchange operation */

/** \}*/
#endif
