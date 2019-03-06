#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xc6c01fa, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xe20c5623, __VMLINUX_SYMBOL_STR(kmem_cache_destroy) },
	{ 0xb210d0e, __VMLINUX_SYMBOL_STR(proc_remove) },
	{ 0x75e82745, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0xc980bc77, __VMLINUX_SYMBOL_STR(kmem_cache_create) },
	{ 0xa13aece1, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0xfd036dc2, __VMLINUX_SYMBOL_STR(proc_mkdir) },
	{ 0x20c55ae0, __VMLINUX_SYMBOL_STR(sscanf) },
	{ 0x85df9b6c, __VMLINUX_SYMBOL_STR(strsep) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x7f02188f, __VMLINUX_SYMBOL_STR(__msecs_to_jiffies) },
	{ 0x16e5c2a, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0xc9993dd0, __VMLINUX_SYMBOL_STR(kmem_cache_free) },
	{ 0x6c09c2a4, __VMLINUX_SYMBOL_STR(del_timer) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x9580deb, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0xa304baeb, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x28b015aa, __VMLINUX_SYMBOL_STR(pid_task) },
	{ 0x9a1604bc, __VMLINUX_SYMBOL_STR(find_vpid) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x5590bfa6, __VMLINUX_SYMBOL_STR(sched_setscheduler) },
	{ 0x6bf1c17f, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0xe259ae9e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0x391afe42, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0x179ea56, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "072DED2CC29CD1B2E458473");
