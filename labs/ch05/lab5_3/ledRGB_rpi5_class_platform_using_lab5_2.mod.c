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
	{ 0x122c3a7e, "_printk" },
	{ 0x83b61d90, "platform_driver_unregister" },
	{ 0xca38a148, "platform_get_resource" },
	{ 0x1f89d774, "devm_ioremap" },
	{ 0x62d71962, "of_get_next_child" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xe83bf7ae, "devm_led_classdev_register_ext" },
	{ 0x50f5bd59, "devm_kmalloc" },
	{ 0x507c530b, "of_property_read_string" },
	{ 0x13a0a82f, "_dev_err" },
	{ 0x620d9b38, "of_node_put" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Carrow,RGBclassleds");
MODULE_ALIAS("of:N*T*Carrow,RGBclassledsC*");

MODULE_INFO(srcversion, "4F9B998CBA8C4CFCD22DA25");
