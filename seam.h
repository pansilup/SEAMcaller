#ifndef __SEAM_H_
#define __DEAM_H_

enum TD_STATUS {
	TD_STATUS_NONE,
	TD_STATUS_FINALIZE_REQUESTED,
	TD_STATUS_FINALIZED
};

/**< Enum for SEAMCALL leaves opcodes */
typedef enum seamcall_leaf_opcode_e
{
    TDH_VP_ENTER                 = 0,
    TDH_MNG_ADDCX                = 1,
    TDH_MEM_PAGE_ADD             = 2,
    TDH_MEM_SEPT_ADD             = 3,
    TDH_VP_ADDCX                 = 4,
    TDH_MEM_PAGE_RELOCATE        = 5,
    TDH_MEM_PAGE_AUG             = 6,
    TDH_MEM_RANGE_BLOCK          = 7,
    TDH_MNG_KEY_CONFIG           = 8,
    TDH_MNG_CREATE               = 9,
    TDH_VP_CREATE                = 10,
    TDH_MNG_RD                   = 11,
    TDH_MEM_RD                   = 12,
    TDH_MNG_WR                   = 13,
    TDH_MEM_WR                   = 14,
    TDH_MEM_PAGE_DEMOTE          = 15,
    TDH_MR_EXTEND                = 16,
    TDH_MR_FINALIZE              = 17,
    TDH_VP_FLUSH                 = 18,
    TDH_MNG_VPFLUSHDONE          = 19,
    TDH_MNG_KEY_FREEID           = 20,
    TDH_MNG_INIT                 = 21,
    TDH_VP_INIT                  = 22,
    TDH_MEM_PAGE_PROMOTE         = 23,
    TDH_PHYMEM_PAGE_RDMD         = 24,
    TDH_MEM_SEPT_RD              = 25,
    TDH_VP_RD                    = 26,
    TDH_MNG_KEY_RECLAIMID        = 27,
    TDH_PHYMEM_PAGE_RECLAIM      = 28,
    TDH_MEM_PAGE_REMOVE          = 29,
    TDH_MEM_SEPT_REMOVE          = 30,
    TDH_SYS_KEY_CONFIG           = 31,
    TDH_SYS_INFO                 = 32,
    TDH_SYS_INIT                 = 33,
    TDH_SYS_RD                   = 34,
    TDH_SYS_LP_INIT              = 35,
    TDH_SYS_TDMR_INIT            = 36,
    TDH_SYS_RDALL                = 37,
    TDH_MEM_TRACK                = 38,
    TDH_MEM_RANGE_UNBLOCK        = 39,
    TDH_PHYMEM_CACHE_WB          = 40,
    TDH_PHYMEM_PAGE_WBINVD       = 41,
    TDH_MEM_SEPT_WR              = 42,
    TDH_VP_WR                    = 43,
    TDH_SYS_LP_SHUTDOWN          = 44,
    TDH_SYS_CONFIG               = 45,

    TDH_SYS_SHUTDOWN             = 52,
    TDH_SYS_UPDATE               = 53,
    TDH_SERVTD_BIND              = 48,
    TDH_SERVTD_PREBIND           = 49,
    TDH_EXPORT_ABORT             = 64,
    TDH_EXPORT_BLOCKW            = 65,
    TDH_EXPORT_RESTORE           = 66,
    TDH_EXPORT_MEM               = 68,
    TDH_EXPORT_PAUSE             = 70,
    TDH_EXPORT_TRACK             = 71,
    TDH_EXPORT_STATE_IMMUTABLE   = 72,
    TDH_EXPORT_STATE_TD          = 73,
    TDH_EXPORT_STATE_VP          = 74,
    TDH_EXPORT_UNBLOCKW          = 75,
    TDH_IMPORT_ABORT             = 80,
    TDH_IMPORT_END               = 81,
    TDH_IMPORT_COMMIT            = 82,
    TDH_IMPORT_MEM               = 83,
    TDH_IMPORT_TRACK             = 84,
    TDH_IMPORT_STATE_IMMUTABLE   = 85,
    TDH_IMPORT_STATE_TD          = 86,
    TDH_IMPORT_STATE_VP          = 87,
    TDH_MIG_STREAM_CREATE        = 96
} seamcall_leaf_opcode_t;

#define TDH_VP_ENTER_NAME                 "TDH_VP_ENTER"
#define TDH_MNG_ADDCX_NAME                "TDH_MNG_ADDCX"
#define TDH_MEM_PAGE_ADD_NAME             "TDH_MEM_PAGE_ADD"
#define TDH_MEM_SEPT_ADD_NAME             "TDH_MEM_SEPT_ADD"
#define TDH_VP_ADDCX_NAME                 "TDH_VP_ADDCX"
#define TDH_MEM_PAGE_RELOCATE_NAME        "TDH_MEM_PAGE_RELOCATE"
#define TDH_MEM_PAGE_AUG_NAME             "TDH_MEM_PAGE_AUG"
#define TDH_MEM_RANGE_BLOCK_NAME          "TDH_MEM_RANGE_BLOCK"
#define TDH_MNG_KEY_CONFIG_NAME           "TDH_MNG_KEY_CONFIG"
#define TDH_MNG_CREATE_NAME               "TDH_MNG_CREATE"
#define TDH_VP_CREATE_NAME                "TDH_VP_CREATE"
#define TDH_MNG_RD_NAME                   "TDH_MNG_RD"
#define TDH_MEM_RD_NAME                   "TDH_MEM_RD"
#define TDH_MNG_WR_NAME                   "TDH_MNG_WR"
#define TDH_MEM_WR_NAME                   "TDH_MEM_WR"
#define TDH_MEM_PAGE_DEMOTE_NAME          "TDH_MEM_PAGE_DEMOTE"
#define TDH_MR_EXTEND_NAME                "TDH_MR_EXTEND"
#define TDH_MR_FINALIZE_NAME              "TDH_MR_FINALIZE"
#define TDH_VP_FLUSH_NAME                 "TDH_VP_FLUSH"
#define TDH_MNG_VPFLUSHDONE_NAME          "TDH_MNG_VPFLUSHDONE"
#define TDH_MNG_KEY_FREEID_NAME           "TDH_MNG_KEY_FREEID"
#define TDH_MNG_INIT_NAME                 "TDH_MNG_INIT"
#define TDH_VP_INIT_NAME                  "TDH_VP_INIT"
#define TDH_MEM_PAGE_PROMOTE_NAME         "TDH_MEM_PAGE_PROMOTE"
#define TDH_PHYMEM_PAGE_RDMD_NAME         "TDH_PHYMEM_PAGE_RDMD"
#define TDH_MEM_SEPT_RD_NAME              "TDH_MEM_SEPT_RD"
#define TDH_VP_RD_NAME                    "TDH_VP_RD"
#define TDH_MNG_KEY_RECLAIMID_NAME        "TDH_MNG_KEY_RECLAIMID"
#define TDH_PHYMEM_PAGE_RECLAIM_NAME      "TDH_PHYMEM_PAGE_RECLAIM"
#define TDH_MEM_PAGE_REMOVE_NAME          "TDH_MEM_PAGE_REMOVE"
#define TDH_MEM_SEPT_REMOVE_NAME          "TDH_MEM_SEPT_REMOVE"
#define TDH_SYS_KEY_CONFIG_NAME           "TDH_SYS_KEY_CONFIG"
#define TDH_SYS_INFO_NAME                 "TDH_SYS_INFO"
#define TDH_SYS_INIT_NAME                 "TDH_SYS_INIT"
#define TDH_SYS_RD_NAME                   "TDH_SYS_RD"
#define TDH_SYS_LP_INIT_NAME              "TDH_SYS_LP_INIT"
#define TDH_SYS_TDMR_INIT_NAME            "TDH_SYS_TDMR_INIT"
#define TDH_SYS_RDALL_NAME                "TDH_SYS_RDALL"
#define TDH_MEM_TRACK_NAME                "TDH_MEM_TRACK"
#define TDH_MEM_RANGE_UNBLOCK_NAME        "TDH_MEM_RANGE_UNBLOCK"
#define TDH_PHYMEM_CACHE_WB_NAME          "TDH_PHYMEM_CACHE_WB"
#define TDH_PHYMEM_PAGE_WBINVD_NAME       "TDH_PHYMEM_PAGE_WBINVD"
#define TDH_MEM_SEPT_WR_NAME              "TDH_MEM_SEPT_WR"
#define TDH_VP_WR_NAME                    "TDH_VP_WR"
#define TDH_SYS_LP_SHUTDOWN_NAME          "TDH_SYS_LP_SHUTDOWN"
#define TDH_SYS_CONFIG_NAME               "TDH_SYS_CONFIG"
#define TDH_SYS_SHUTDOWN_NAME             "TDH_SYS_SHUTDOWN"
#define TDH_SYS_UPDATE_NAME               "TDH_SYS_UPDATE"
#define TDH_SERVTD_BIND_NAME              "TDH_SERVTD_BIND"
#define TDH_SERVTD_PREBIND_NAME           "TDH_SERVTD_PREBIND"
#define TDH_EXPORT_ABORT_NAME             "TDH_EXPORT_ABORT"
#define TDH_EXPORT_BLOCKW_NAME            "TDH_EXPORT_BLOCKW"
#define TDH_EXPORT_RESTORE_NAME           "TDH_EXPORT_RESTORE"
#define TDH_EXPORT_MEM_NAME               "TDH_EXPORT_MEM"
#define TDH_EXPORT_PAUSE_NAME             "TDH_EXPORT_PAUSE"
#define TDH_EXPORT_TRACK_NAME             "TDH_EXPORT_TRACK"
#define TDH_EXPORT_STATE_IMMUTABLE_NAME   "TDH_EXPORT_STATE_IMMUTABLE"
#define TDH_EXPORT_STATE_TD_NAME          "TDH_EXPORT_STATE_TD"
#define TDH_EXPORT_STATE_VP_NAME          "TDH_EXPORT_STATE_VP"
#define TDH_EXPORT_UNBLOCKW_NAME          "TDH_EXPORT_UNBLOCKW"
#define TDH_IMPORT_ABORT_NAME             "TDH_IMPORT_ABORT"
#define TDH_IMPORT_END_NAME               "TDH_IMPORT_END"
#define TDH_IMPORT_COMMIT_NAME            "TDH_IMPORT_COMMIT"
#define TDH_IMPORT_MEM_NAME               "TDH_IMPORT_MEM"
#define TDH_IMPORT_TRACK_NAME             "TDH_IMPORT_TRACK"
#define TDH_IMPORT_STATE_IMMUTABLE_NAME   "TDH_IMPORT_STATE_IMMUTABLE"
#define TDH_IMPORT_STATE_TD_NAME          "TDH_IMPORT_STATE_TD"
#define TDH_IMPORT_STATE_VP_NAME          "TDH_IMPORT_STATE_VP"
#define TDH_MIG_STREAM_CREATE_NAME        "TDH_MIG_STREAM_CREATE"

#endif

