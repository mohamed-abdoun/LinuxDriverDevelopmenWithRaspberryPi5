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
	{ 0x15d5957, "i2c_register_driver" },
	{ 0x8b25c8cc, "_dev_info" },
	{ 0x11e6803e, "misc_deregister" },
	{ 0x50f5bd59, "devm_kmalloc" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x53c34e79, "misc_register" },
	{ 0xe7ad9d12, "i2c_del_driver" },
	{ 0xde5769a3, "i2c_smbus_read_byte" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0x122c3a7e, "_printk" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x5c3c7387, "kstrtoull" },
	{ 0xf606d549, "i2c_smbus_write_byte" },
	{ 0x13a0a82f, "_dev_err" },
	{ 0xdcb764ad, "memset" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:ioexp");
MODULE_ALIAS("of:N*T*Carrow,ioexp");
MODULE_ALIAS("of:N*T*Carrow,ioexpC*");

MODULE_INFO(srcversion, "4F130567B84936E2C9CA2E5");
