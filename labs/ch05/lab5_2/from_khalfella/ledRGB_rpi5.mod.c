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
	{ 0x11e6803e, "misc_deregister" },
	{ 0x34a653d, "gpiod_set_value" },
	{ 0xf9ca2eb4, "kstrtoint_from_user" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x50f5bd59, "devm_kmalloc" },
	{ 0xb7c4d2ac, "devm_gpiod_get" },
	{ 0x507c530b, "of_property_read_string" },
	{ 0xe7ba2c88, "gpiod_direction_output" },
	{ 0x53c34e79, "misc_register" },
	{ 0x122c3a7e, "_printk" },
	{ 0x83b61d90, "platform_driver_unregister" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Carrow,RGBleds");
MODULE_ALIAS("of:N*T*Carrow,RGBledsC*");

MODULE_INFO(srcversion, "3D477D6D26A98B67A9BA7D4");
