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
	{ 0x38681060, "input_unregister_device" },
	{ 0x8b25c8cc, "_dev_info" },
	{ 0x204e8193, "iio_read_channel_raw" },
	{ 0x9eab8477, "input_event" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x83b61d90, "platform_driver_unregister" },
	{ 0xcd29c3d3, "devm_iio_channel_get" },
	{ 0xf5aebc4f, "iio_get_channel_type" },
	{ 0x13a0a82f, "_dev_err" },
	{ 0x50f5bd59, "devm_kmalloc" },
	{ 0xb33bda2e, "devm_input_allocate_device" },
	{ 0x9d4e84c0, "input_setup_polling" },
	{ 0xc0e70ccc, "input_set_poll_interval" },
	{ 0xa65c6def, "alt_cb_patch_nops" },
	{ 0xf30cb378, "input_set_abs_params" },
	{ 0xb01bf138, "input_register_device" },
	{ 0x6f6ab014, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cnunchuk_consumer");
MODULE_ALIAS("of:N*T*Cnunchuk_consumerC*");

MODULE_INFO(srcversion, "86E5CA3E3EF6B97B7D31452");
