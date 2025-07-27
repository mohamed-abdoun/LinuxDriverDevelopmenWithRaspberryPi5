#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0x851a010f, "__platform_driver_register" },
	{ 0x8b25c8cc, "_dev_info" },
	{ 0x11e6803e, "misc_deregister" },
	{ 0xc1a970b7, "dma_release_channel" },
	{ 0xeced2d0e, "dma_free_attrs" },
	{ 0xa6257a2f, "complete" },
	{ 0x13a0a82f, "_dev_err" },
	{ 0x83b61d90, "platform_driver_unregister" },
	{ 0x50f5bd59, "devm_kmalloc" },
	{ 0x6b10c24d, "dma_alloc_attrs" },
	{ 0xa65c6def, "alt_cb_patch_nops" },
	{ 0x68901eee, "__dma_request_channel" },
	{ 0x53c34e79, "misc_register" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0x25974000, "wait_for_completion" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xdcb764ad, "memset" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Carrow,sdma_m2m");
MODULE_ALIAS("of:N*T*Carrow,sdma_m2mC*");

MODULE_INFO(srcversion, "89D30039FFDF18357875343");
