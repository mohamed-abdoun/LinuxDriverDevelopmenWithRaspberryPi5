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
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x6a6e05bf, "kstrtou8" },
	{ 0x995b0d7e, "__spi_register_driver" },
	{ 0x8b25c8cc, "_dev_info" },
	{ 0xdf23d9a8, "sysfs_remove_group" },
	{ 0x38681060, "input_unregister_device" },
	{ 0xa238d5c4, "spi_write_then_read" },
	{ 0xb341eaaf, "driver_unregister" },
	{ 0xdcb764ad, "memset" },
	{ 0x9b13ba99, "spi_sync" },
	{ 0x9eab8477, "input_event" },
	{ 0x50f5bd59, "devm_kmalloc" },
	{ 0xb33bda2e, "devm_input_allocate_device" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x7569964c, "devm_gpiod_get_index" },
	{ 0x2aaa7b48, "gpiod_to_irq" },
	{ 0x262aaa36, "devm_request_threaded_irq" },
	{ 0x13a0a82f, "_dev_err" },
	{ 0x5a2b22ae, "sysfs_create_group" },
	{ 0xb01bf138, "input_register_device" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:adxl345");
MODULE_ALIAS("of:N*T*Carrow,adxl345");
MODULE_ALIAS("of:N*T*Carrow,adxl345C*");

MODULE_INFO(srcversion, "F6922CC6517A7AFA2FA49C5");
