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
	{ 0x703c9b7d, "devm_iio_device_alloc" },
	{ 0x1d124e93, "i2c_transfer_buffer_flags" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xacbcd537, "__devm_iio_device_register" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xe7ad9d12, "i2c_del_driver" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:nunchuck_accel_provi");
MODULE_ALIAS("of:N*T*Cnunchuck_accel_provider");
MODULE_ALIAS("of:N*T*Cnunchuck_accel_providerC*");

MODULE_INFO(srcversion, "CA93F527C92E865E90456D0");
