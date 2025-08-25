#ifndef __TDX_VMM_DEFS_
#define __TDX_VMM_DEFS_

struct td_params {
	u64 attributes;
	u64 xfam;
	u16 max_vcpus;
	u8 reserved0[6];

	u64 eptp_controls;
	u64 exec_controls;
	u16 tsc_frequency;
	u8  reserved1[38];

	u64 mrconfigid[6];
	u64 mrowner[6];
	u64 mrownerconfig[6];
	u64 reserved2[4];

	union {
		u8 reserved3[768];
	};
} __packed __aligned(1024);

#endif
