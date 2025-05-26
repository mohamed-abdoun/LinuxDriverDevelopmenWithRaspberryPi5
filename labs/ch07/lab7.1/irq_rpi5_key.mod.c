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
	{ 0x851a010f, "__platform_driver_register" },
	{ 0x8b25c8cc, "_dev_info" },
	{ 0x11e6803e, "misc_deregister" },
	{ 0xb7c4d2ac, "devm_gpiod_get" },
	{ 0x2aaa7b48, "gpiod_to_irq" },
	{ 0x21d16599, "platform_get_irq" },
	{ 0x262aaa36, "devm_request_threaded_irq" },
	{ 0x53c34e79, "misc_register" },
	{ 0x13a0a82f, "_dev_err" },
	{ 0x83b61d90, "platform_driver_unregister" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Carrow,intkey");
MODULE_ALIAS("of:N*T*Carrow,intkeyC*");

MODULE_INFO(srcversion, "F4FA1D0FA0F5FF4F2D42DAD");
