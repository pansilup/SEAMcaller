#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/kconfig.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <uapi/asm/kvm.h>
#include <asm/shared/tdx.h>
#include <linux/kallsyms.h>

#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <asm/io.h>

#include "kdefs.h"
#include "kernel_symbols.h"
#include "seam.h"
#include "tdx_vmm_defs.h"

#include <linux/timex.h> /*to get cycles*/

#define SEAMCALL_LEAF_NAME_BUF_LEN	100
#define KERNEL_SPACE_START		0xffff800000000000UL
#define PAGE_OFST_MASK			0xfffUL

/*To limit the number of seamcall data logs*/
#define TDH_MEM_PAGE_ADD_LOG_COUNT	5
#define TDH_MR_EXTEND_LOG_COUNT		5

static unsigned long seamcall_addr;
static unsigned long seamcall_ret_addr;
static unsigned long seamcall_saved_ret_addr;
static unsigned long tdx_vm_ioctl_addr;
static unsigned long tdx_vcpu_ioctl_addr;

static const char* seamcall_leaf_name[SEAMCALL_LEAF_NAME_BUF_LEN];
static int td_status = TD_STATUS_NONE;

static unsigned long tdh_mem_page_add_log_count = 0;
static unsigned long tdh_mr_extend_log_count = 0;

static unsigned long last_seamcall = 0;

static cycles_t t0 = 0;
static cycles_t t1 = 0;
static unsigned long t00 = 0;
static unsigned long t11 = 0;

void init_seamcall_leaf_names(void);

unsigned long long rdtsc_v(void)
{
    unsigned hi, lo;
    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long) lo | ((unsigned long long) hi << 32));
}

void init_seamcall_leaf_names(void) {

	/*Initialize the array with "INVALID_SEAMCALL_NAME" by default*/
	for (int i = 0; i < SEAMCALL_LEAF_NAME_BUF_LEN; i++) {
	seamcall_leaf_name[i] = "INVALID_SEAMCALL_NAME";
	}

	/*Now assign valid names to the enum indices*/
	seamcall_leaf_name[TDH_VP_ENTER]               = TDH_VP_ENTER_NAME;
	seamcall_leaf_name[TDH_MNG_ADDCX]              = TDH_MNG_ADDCX_NAME;
	seamcall_leaf_name[TDH_MEM_PAGE_ADD]           = TDH_MEM_PAGE_ADD_NAME;
	seamcall_leaf_name[TDH_MEM_SEPT_ADD]           = TDH_MEM_SEPT_ADD_NAME;
	seamcall_leaf_name[TDH_VP_ADDCX]               = TDH_VP_ADDCX_NAME;
	seamcall_leaf_name[TDH_MEM_PAGE_RELOCATE]      = TDH_MEM_PAGE_RELOCATE_NAME;
	seamcall_leaf_name[TDH_MEM_PAGE_AUG]           = TDH_MEM_PAGE_AUG_NAME;
	seamcall_leaf_name[TDH_MEM_RANGE_BLOCK]        = TDH_MEM_RANGE_BLOCK_NAME;
	seamcall_leaf_name[TDH_MNG_KEY_CONFIG]         = TDH_MNG_KEY_CONFIG_NAME;
	seamcall_leaf_name[TDH_MNG_CREATE]             = TDH_MNG_CREATE_NAME;
	seamcall_leaf_name[TDH_VP_CREATE]              = TDH_VP_CREATE_NAME;
	seamcall_leaf_name[TDH_MNG_RD]                 = TDH_MNG_RD_NAME;
	seamcall_leaf_name[TDH_MEM_RD]                 = TDH_MEM_RD_NAME;
	seamcall_leaf_name[TDH_MNG_WR]                 = TDH_MNG_WR_NAME;
	seamcall_leaf_name[TDH_MEM_WR]                 = TDH_MEM_WR_NAME;
	seamcall_leaf_name[TDH_MEM_PAGE_DEMOTE]        = TDH_MEM_PAGE_DEMOTE_NAME;
	seamcall_leaf_name[TDH_MR_EXTEND]              = TDH_MR_EXTEND_NAME;
	seamcall_leaf_name[TDH_MR_FINALIZE]            = TDH_MR_FINALIZE_NAME;
	seamcall_leaf_name[TDH_VP_FLUSH]               = TDH_VP_FLUSH_NAME;
	seamcall_leaf_name[TDH_MNG_VPFLUSHDONE]        = TDH_MNG_VPFLUSHDONE_NAME;
	seamcall_leaf_name[TDH_MNG_KEY_FREEID]         = TDH_MNG_KEY_FREEID_NAME;
	seamcall_leaf_name[TDH_MNG_INIT]               = TDH_MNG_INIT_NAME;
	seamcall_leaf_name[TDH_VP_INIT]                = TDH_VP_INIT_NAME;
	seamcall_leaf_name[TDH_MEM_PAGE_PROMOTE]       = TDH_MEM_PAGE_PROMOTE_NAME;
	seamcall_leaf_name[TDH_PHYMEM_PAGE_RDMD]       = TDH_PHYMEM_PAGE_RDMD_NAME;
	seamcall_leaf_name[TDH_MEM_SEPT_RD]            = TDH_MEM_SEPT_RD_NAME;
	seamcall_leaf_name[TDH_VP_RD]                  = TDH_VP_RD_NAME;
	seamcall_leaf_name[TDH_MNG_KEY_RECLAIMID]      = TDH_MNG_KEY_RECLAIMID_NAME;
	seamcall_leaf_name[TDH_PHYMEM_PAGE_RECLAIM]    = TDH_PHYMEM_PAGE_RECLAIM_NAME;
	seamcall_leaf_name[TDH_MEM_PAGE_REMOVE]        = TDH_MEM_PAGE_REMOVE_NAME;
	seamcall_leaf_name[TDH_MEM_SEPT_REMOVE]        = TDH_MEM_SEPT_REMOVE_NAME;
	seamcall_leaf_name[TDH_SYS_KEY_CONFIG]         = TDH_SYS_KEY_CONFIG_NAME;
	seamcall_leaf_name[TDH_SYS_INFO]               = TDH_SYS_INFO_NAME;
	seamcall_leaf_name[TDH_SYS_INIT]               = TDH_SYS_INIT_NAME;
	seamcall_leaf_name[TDH_SYS_RD]                 = TDH_SYS_RD_NAME;
	seamcall_leaf_name[TDH_SYS_LP_INIT]            = TDH_SYS_LP_INIT_NAME;
	seamcall_leaf_name[TDH_SYS_TDMR_INIT]          = TDH_SYS_TDMR_INIT_NAME;
	seamcall_leaf_name[TDH_SYS_RDALL]              = TDH_SYS_RDALL_NAME;
	seamcall_leaf_name[TDH_MEM_TRACK]              = TDH_MEM_TRACK_NAME;
	seamcall_leaf_name[TDH_MEM_RANGE_UNBLOCK]      = TDH_MEM_RANGE_UNBLOCK_NAME;
	seamcall_leaf_name[TDH_PHYMEM_CACHE_WB]        = TDH_PHYMEM_CACHE_WB_NAME;
	seamcall_leaf_name[TDH_PHYMEM_PAGE_WBINVD]     = TDH_PHYMEM_PAGE_WBINVD_NAME;
	seamcall_leaf_name[TDH_MEM_SEPT_WR]            = TDH_MEM_SEPT_WR_NAME;
	seamcall_leaf_name[TDH_VP_WR]                  = TDH_VP_WR_NAME;
	seamcall_leaf_name[TDH_SYS_LP_SHUTDOWN]        = TDH_SYS_LP_SHUTDOWN_NAME;
	seamcall_leaf_name[TDH_SYS_CONFIG]             = TDH_SYS_CONFIG_NAME;
	seamcall_leaf_name[TDH_SYS_SHUTDOWN]           = TDH_SYS_SHUTDOWN_NAME;
	seamcall_leaf_name[TDH_SYS_UPDATE]             = TDH_SYS_UPDATE_NAME;
	seamcall_leaf_name[TDH_SERVTD_BIND]            = TDH_SERVTD_BIND_NAME;
	seamcall_leaf_name[TDH_SERVTD_PREBIND]         = TDH_SERVTD_PREBIND_NAME;
	seamcall_leaf_name[TDH_EXPORT_ABORT]           = TDH_EXPORT_ABORT_NAME;
	seamcall_leaf_name[TDH_EXPORT_BLOCKW]          = TDH_EXPORT_BLOCKW_NAME;
	seamcall_leaf_name[TDH_EXPORT_RESTORE]         = TDH_EXPORT_RESTORE_NAME;
	seamcall_leaf_name[TDH_EXPORT_MEM]             = TDH_EXPORT_MEM_NAME;
	seamcall_leaf_name[TDH_EXPORT_PAUSE]           = TDH_EXPORT_PAUSE_NAME;
	seamcall_leaf_name[TDH_EXPORT_TRACK]           = TDH_EXPORT_TRACK_NAME;
	seamcall_leaf_name[TDH_EXPORT_STATE_IMMUTABLE] = TDH_EXPORT_STATE_IMMUTABLE_NAME;
	seamcall_leaf_name[TDH_EXPORT_STATE_TD]        = TDH_EXPORT_STATE_TD_NAME;
	seamcall_leaf_name[TDH_EXPORT_STATE_VP]        = TDH_EXPORT_STATE_VP_NAME;
	seamcall_leaf_name[TDH_EXPORT_UNBLOCKW]        = TDH_EXPORT_UNBLOCKW_NAME;
	seamcall_leaf_name[TDH_IMPORT_ABORT]           = TDH_IMPORT_ABORT_NAME;
	seamcall_leaf_name[TDH_IMPORT_END]             = TDH_IMPORT_END_NAME;
	seamcall_leaf_name[TDH_IMPORT_COMMIT]          = TDH_IMPORT_COMMIT_NAME;
	seamcall_leaf_name[TDH_IMPORT_MEM]             = TDH_IMPORT_MEM_NAME;
	seamcall_leaf_name[TDH_IMPORT_TRACK]           = TDH_IMPORT_TRACK_NAME;
	seamcall_leaf_name[TDH_IMPORT_STATE_IMMUTABLE] = TDH_IMPORT_STATE_IMMUTABLE_NAME;
	seamcall_leaf_name[TDH_IMPORT_STATE_TD]        = TDH_IMPORT_STATE_TD_NAME;
	seamcall_leaf_name[TDH_IMPORT_STATE_VP]        = TDH_IMPORT_STATE_VP_NAME;
	seamcall_leaf_name[TDH_MIG_STREAM_CREATE]      = TDH_MIG_STREAM_CREATE_NAME;
}

static ulong pa_to_va(ulong pa){

	unsigned pfn;
	struct page *pg;
	ulong va;

	pfn = pa >> 12;
	pg = pfn_to_page(pfn);
	va = (ulong)kmap_atomic(pg);

	return va;
}

static void pa_to_va_free(void *va){
	kunmap_atomic(va);
}

static void print_td_params(struct td_params *td_params){

	int idx;

	LOG(" --- TD_PARAMS data --- \n");
	LOG("td_params attributes: 0x%lx\n", (unsigned long)td_params->attributes);
	LOG("td_params attributes: 0x%lx\n", (unsigned long)td_params->attributes);
	LOG("td_params xfam: %lx\n", (unsigned long)td_params->xfam);
	LOG("td_params max_vcpus: %lu\n", (unsigned long)td_params->max_vcpus);
	LOG("td_params eptp_controls: 0x%lx\n", (unsigned long)td_params->eptp_controls);
	LOG("td_params exec_controls: 0x%lx\n", (unsigned long)td_params->exec_controls);
	LOG("td_params attributes: 0x%lx\n", (unsigned long)td_params->tsc_frequency);
	
	for(idx = 0; idx < 6; idx++)
		LOG("td_params mrconfigid[%d]: 0x%016lx\n", idx, (unsigned long)td_params->mrconfigid[idx]);
	for(idx = 0; idx < 6; idx++)
		LOG("td_params mrowner[%d]: 0x%016lx\n", idx, (unsigned long)td_params->mrowner[idx]);
	for(idx = 0; idx < 6; idx++)
		LOG("td_params mrownerconfig[%d]: 0x%016lx\n", idx, (unsigned long)td_params->mrownerconfig[idx]);

}

static void print_seamcall(unsigned long addr, unsigned long seamcall_leaf){    

	LOG("kvm %s: %s ------------------------------------ \n", 
		    addr == seamcall_addr ? "__seamcall()" : 
		    	(addr == seamcall_ret_addr ? "__seamcall_ret" : 
		    		 (addr == seamcall_saved_ret_addr ? "__seamcall_saved_ret()" : 
				  	"UNKNOWN KVM FUNCTION")),
		     seamcall_leaf_name[seamcall_leaf]);
}

static void log_tdh_mng_create_args(struct tdx_module_args *args){
	
	LOG("RCX: to add tdr page pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: td hkid: %lu\n", (unsigned long)args->rdx);

	//change args
	//args->rdx = 0x20;
	//LOG("Modified RDX: td hkid: %lu\n", (unsigned long)args->rdx);
}

static void log_tdh_mng_key_config_args(struct tdx_module_args *args){
	
	LOG("RCX: tdr pa: 0x%lx\n", (unsigned long)args->rcx);
}

static void log_tdh_mng_addcx_args(struct tdx_module_args *args){

	LOG("RCX: to add tdcx page pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: tdr pa: 0x%lx\n", (unsigned long)args->rdx);
}

static void log_tdh_mng_init_args(struct tdx_module_args *args){
	
	struct td_params *td_params;
	unsigned long page_va, page_ofst;

	LOG("RCX: tdr pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: TD_PARAMS_STRUCT pa: 0x%lx\n", (unsigned long)args->rdx);
	
	if(args->rdx == 0){
		LOG("invalid pa in RDX\n");
		return;
	}
	
	page_va = (unsigned long)pa_to_va(args->rdx);
	page_ofst = args->rdx & PAGE_OFST_MASK;
	td_params = (struct td_params *)(page_va + page_ofst);
	LOG("TD_PARAMS_STRUCT va: 0x%lx\n", (unsigned long)td_params);	
	print_td_params(td_params);

	pa_to_va_free((void *)td_params);
}

static void log_tdh_vp_create_args(struct tdx_module_args *args){
	
	LOG("RCX: to add tdvpr page pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: tdr pa: 0x%lx\n", (unsigned long)args->rdx);
}

static void log_tdh_vp_addcx_args(struct tdx_module_args *args){
	
	LOG("RCX: to add tdcx page pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: tdvpr pa: 0x%lx\n", (unsigned long)args->rdx);
}

static void log_tdh_vp_init_args(struct tdx_module_args *args){
	
	LOG("RCX: tdvpr pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: td initial vcpu rcx: 0x%lx\n", (unsigned long)args->rdx);
	LOG("R8: x2apic_id, if leaf version > 0: 0x%lx\n", (unsigned long)args->r8);
}

static void log_tdh_vp_wr_args(struct tdx_module_args *args){

	LOG("RCX: tdvpr pa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: field identifier: 0x%lx\n", (unsigned long)args->rdx);
	LOG("R8: field value: 0x%lx\n", (unsigned long)args->r8);
	LOG("R9: 64b write mask: 0x%lx\n", (unsigned long)args->r9);
}

static void log_tdh_mem_sept_add_args(struct tdx_module_args *args){

	LOG("RCX: level: %d gpa: 0x%lx\n",
			(int)(args->rcx & 0b111), (unsigned long)(args->rcx & ~PAGE_OFST_MASK));
	LOG("RDX: allow_existing: %d tdr pa: 0x%lx\n", 
			(int)(args->rdx & 0x1), (unsigned long)(args->rdx & ~PAGE_OFST_MASK));	
	LOG("R8: sept page pa: 0x%lx\n", (unsigned long)args->r8);
	LOG("R9: L2 vm#1 sept page pa: 0x%lx\n", (unsigned long)args->r9);
	LOG("R10: L2 vm#2 sept page pa: 0x%lx\n", (unsigned long)args->r10);
	LOG("R11: L2 vm#3 sept page pa: 0x%lx\n", (unsigned long)args->r11);
}

static void log_tdh_mem_page_add_args(struct tdx_module_args *args){
	
	LOG("RCX: level: %d gpa: 0x%lx\n",
			(int)(args->rcx & 0b111), (unsigned long)(args->rcx & ~PAGE_OFST_MASK));
	LOG("RDX: tdr pa: 0x%lx\n", (unsigned long)args->rdx);
	LOG("R8: to add td page pa: 0x%lx\n", (unsigned long)args->r8);
	LOG("R9: source page pa: 0x%lx\n", (unsigned long)args->r9);
}

static void log_tdh_mr_extend_args(struct tdx_module_args *args){

	LOG("RCX: to be measured td page chunk gpa: 0x%lx\n", (unsigned long)args->rcx);
	LOG("RDX: tdr pa: 0x%lx\n", (unsigned long)args->rdx);
}

static void log_seamcall_args(unsigned long leaf, struct tdx_module_args *args){
	
	switch (leaf) {

		case TDH_MNG_CREATE:
			log_tdh_mng_create_args(args);
			break;
		case TDH_MNG_KEY_CONFIG:
			log_tdh_mng_key_config_args(args);
			break;
		case TDH_MNG_ADDCX:
			log_tdh_mng_addcx_args(args);
			break;
		case TDH_MNG_INIT:
			log_tdh_mng_init_args(args);
			break;
		case TDH_VP_CREATE:
			log_tdh_vp_create_args(args);
			break;
		case TDH_VP_ADDCX:
			log_tdh_vp_addcx_args(args);
			break;
		case TDH_VP_INIT:
			log_tdh_vp_init_args(args);
			break;
		case TDH_VP_WR:
			log_tdh_vp_wr_args(args);
			break;
		case TDH_MEM_SEPT_ADD:
			log_tdh_mem_sept_add_args(args);
			break;
		case TDH_MEM_PAGE_ADD:
			if(tdh_mem_page_add_log_count < TDH_MEM_PAGE_ADD_LOG_COUNT){ 
				log_tdh_mem_page_add_args(args);
				tdh_mem_page_add_log_count++;
			}
			break;
		case TDH_MR_EXTEND:
			if(tdh_mr_extend_log_count < TDH_MR_EXTEND_LOG_COUNT){
				log_tdh_mr_extend_args(args);
				tdh_mr_extend_log_count++;
			}
			break;
		default:
			//LOG("Logging args not implemented for the current seamcall\n");
			break;
	}

}

/*KProbe handler functions*/
static int seamcall_handler(struct kprobe *p, struct pt_regs *regs) {
    
	unsigned long seamcall_leaf = regs->di;
    	last_seamcall = seamcall_leaf;
	if(seamcall_leaf >= SEAMCALL_LEAF_NAME_BUF_LEN)
		seamcall_leaf = SEAMCALL_LEAF_NAME_BUF_LEN - 1;
    
//	if(last_seamcall == 9){
	if(td_status != TD_STATUS_FINALIZED){
		print_seamcall((unsigned long)p->addr, seamcall_leaf);
		log_seamcall_args(seamcall_leaf, (struct tdx_module_args *)regs->si);
	}
    	t00 = rdtsc_v();
	t0 = get_cycles();
	return 0;
}

static void seamcall_ret_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
	t11= rdtsc_v();
	t1 = get_cycles();

	unsigned long t = t1-t0;
	unsigned long tt = t11-t00;
	if(td_status != TD_STATUS_FINALIZED){
	//if(last_seamcall == 9){
		LOG("last seamcall:0x%lx\n", last_seamcall);
		LOG("KProbe: SEAMCALL returned: t0:0x%lx t1:0x%lx cycles:0x%lx\n", t0, t1, t);    
		LOG("KProbe: SEAMCALL returned: t0:0x%lx t1:0x%lx cycles:0x%lx\n", t00, t11, tt);    
		//LOG("KProbe: SEAMCALL returned: return value :0x%lx\n", (unsigned long)regs->ax);    
	}	
	return;
}

static int user_req_handler(struct kprobe *p, struct pt_regs *regs) {

	struct kvm_tdx_cmd tdx_cmd;
	
	LOG("hooked  %s \n", 
		    (unsigned long)p->addr == TDX_VM_IOCTL_ADDR ? "tdx_vm_ioctl()" : 
		    ((unsigned long)p->addr == TDX_VCPU_IOCTL_ADDR ? "tdx_vcpu_ioctl()" : "UNKMOWN FUNCTION"));

	if (copy_from_user(&tdx_cmd, (void *)regs->si, sizeof(struct kvm_tdx_cmd)))
		goto OUT;
	if (tdx_cmd.error)
		goto OUT;


	switch (tdx_cmd.id) {
	case KVM_TDX_CAPABILITIES:
		LOG("ioctl: KVM_TDX_CAPABILITIES\n");
		break;
	case KVM_TDX_INIT_VM:
		LOG("ioctl: KVM_TDX_INIT_VM\n");
		break;
	case KVM_TDX_EXTEND_MEMORY:
		LOG("ioctl: KVM_TDX_EXTEND_MEMORY\n");
		break;
	case KVM_TDX_FINALIZE_VM:
		td_status = TD_STATUS_FINALIZE_REQUESTED;
		LOG("ioctl: KVM_TDX_FINALIZE_VM\n");
		break;
	case KVM_TDX_INIT_VCPU:
		LOG("ioctl: KVM_TDX_INIT_VCPU\n");
		break;	
	default:
		LOG("Unknown or other ioctl\n");
	}
	printk("\n");
    
OUT:
	return 0;
}

static void vmm_agent_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
    
	/*printk(KERN_INFO "KProbe: Function returned from address %px\n", p->addr);*/
	if(td_status == TD_STATUS_FINALIZE_REQUESTED){
		LOG("TD_FINALIZED\n");
		td_status = TD_STATUS_FINALIZED;
	}

    return;
}


/*atic int read_td_params_fn(struct kprobe *p, struct pt_regs *regs) {
	
	struct td_params *td_params = (struct td_params *)regs->si;

	LOG("hooked  %s \n", "__tdx_td_init"); 
	LOG("TD_PARAMS_STRUCT va: 0x%lx\n", (unsigned long)td_params);
	
	print_td_params(td_params);
	
	return 0;
}*/

static struct kprobe kp1 = {
	.pre_handler = user_req_handler,
	.post_handler = vmm_agent_post_handler
};

static struct kprobe kp2 = {
	.pre_handler = user_req_handler,
	.post_handler = vmm_agent_post_handler
};

static struct kprobe kp3 = {
	.pre_handler = seamcall_handler,
	.post_handler = seamcall_ret_handler
};

static struct kprobe kp4 = {
	.pre_handler = seamcall_handler,
	.post_handler = seamcall_ret_handler
};

static struct kprobe kp5 = {
	.pre_handler = seamcall_handler,
	.post_handler = seamcall_ret_handler
};

static int set_seamcall_addrs(void){

	int ret = 0;
		
	/*__seamcall, __seamcall_ret, __seamcall_saved_ret are global symbols, type T.
	hence accessible through __symbol_get()*/
	seamcall_addr = (unsigned long)__symbol_get("__seamcall");
	LOG("__seamcall address: %lx\n", seamcall_addr);

	seamcall_ret_addr = (unsigned long)__symbol_get("__seamcall_ret");
	LOG("__seamcall_ret address: %lx\n", seamcall_ret_addr);

	seamcall_saved_ret_addr = (unsigned long)__symbol_get("__seamcall_saved_ret");
	LOG("__seamcall_saved_ret address: %lx\n", seamcall_saved_ret_addr);

	/*tdx_vm_ioctl, tdx_vcpu_ioctl are not global symbols, they are local, type t.
	hence we initialize  the relavant address variable using predefined constants
	in kernel_symbols.h , generated by a script running as sudo.*/
	tdx_vm_ioctl_addr = TDX_VM_IOCTL_ADDR;
	LOG("tdx_vm_ioctl adress: %lx\n", tdx_vm_ioctl_addr);	
    
	tdx_vcpu_ioctl_addr = TDX_VCPU_IOCTL_ADDR;
	LOG("tdx_vcpu_ioctl adress: %lx\n", tdx_vcpu_ioctl_addr);
    
	if(seamcall_addr == 0 || seamcall_ret_addr == 0 || seamcall_saved_ret_addr == 0 ||
		tdx_vm_ioctl_addr == 0 || tdx_vcpu_ioctl_addr == 0){
		ret = -1;
	}

    return ret;
}

static int register_kprobes_now(void){

	int ret, count;
    
	kp1.addr = (void *)tdx_vm_ioctl_addr;
	kp2.addr = (void *)tdx_vcpu_ioctl_addr;
	kp3.addr = (void *)seamcall_addr;
	kp4.addr = (void *)seamcall_ret_addr;
	kp5.addr = (void *)seamcall_saved_ret_addr;

	count = 0;
	ret = register_kprobe(&kp1);
	if (ret < 0)
		goto ERR;

	count++;
	ret = register_kprobe(&kp2);
	if (ret < 0)
		goto ERR;

	count++;
	ret = register_kprobe(&kp3);
	if (ret < 0)
		goto ERR;

	count++;
	ret = register_kprobe(&kp4);
	if (ret < 0)
		goto ERR;

	count++;
	ret = register_kprobe(&kp5);
	if (ret < 0)
		goto ERR;
	
	LOG("KProbe: registration successful");
	return ret;
	
ERR:
	LOG("KProbe: %d registration failed: err %d\n", count, ret);
	return ret;
}

static int __init agent_init(void){

	int ret;
	int five_level_paging, tdx_global_keyid_val, pgdir_shift_val;
	LOG("BAKBEE\n");
	if(__PGTABLE_L5_ENABLED_ADDR == 0){
		LOG("invalid __PGTABLE_L5_ENABLED_ADDR: 0x%lx\n", __PGTABLE_L5_ENABLED_ADDR);
		return -1;
	}
	five_level_paging = *(int *)__PGTABLE_L5_ENABLED_ADDR;
	LOG("__pgtable_l5_enabled val: %d\n", five_level_paging); 
	
	if(PGDIR_SHIFT_ADDR == 0){
		LOG("invalid PGDIR_SHIFT_ADDR: 0x%lx\n", PGDIR_SHIFT_ADDR);
		return -1;
	}
	pgdir_shift_val = *(int *)PGDIR_SHIFT_ADDR;
	LOG("pgdir_shift val: %d\n", pgdir_shift_val); 
	
	if(TDX_GLOBAL_KEYID_ADDR == 0){
		LOG("invalid TDX_GLOBAL_KEYID_VAL: 0x%lx\n", TDX_GLOBAL_KEYID_ADDR);
		return -1;
	}
	tdx_global_keyid_val = *(int *)TDX_GLOBAL_KEYID_ADDR;
	LOG("tdx_global_hkid: %d\n", tdx_global_keyid_val);
     
	ret = set_seamcall_addrs();
	if(ret < 0)
		return ret;
	ret = register_kprobes_now(); 
	if( ret < 0)
		return ret;
    
	init_seamcall_leaf_names();
	
	LOG("vmm_agent loaded\n");
	return 0;
}

static void __exit agent_exit(void){

	unregister_kprobe(&kp1);
	unregister_kprobe(&kp2);
	unregister_kprobe(&kp3);
	unregister_kprobe(&kp4);
	unregister_kprobe(&kp5);

	LOG("KProbe: unregistration successful\n");

	LOG("vmm_agent unloaded\n");
}

module_init(agent_init);
module_exit(agent_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pansilu pitigalaarachchi");
MODULE_DESCRIPTION(
	"This module hooks in to KVM TDX functions in vmm at runtime."
);
