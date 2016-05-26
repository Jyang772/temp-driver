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
	{ 0xbff2abe1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x97add864, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0xabedead0, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xb61de2d5, __VMLINUX_SYMBOL_STR(usb_free_urb) },
	{ 0xd9516926, __VMLINUX_SYMBOL_STR(usb_register_dev) },
	{ 0x18731d8b, __VMLINUX_SYMBOL_STR(usb_string) },
	{ 0x71d0bb18, __VMLINUX_SYMBOL_STR(usb_alloc_urb) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xdf3deb56, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x74cf6b1f, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x4d33b4de, __VMLINUX_SYMBOL_STR(usb_deregister_dev) },
	{ 0x391093cf, __VMLINUX_SYMBOL_STR(dev_set_drvdata) },
	{ 0xa8c29637, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0xf6403fb9, __VMLINUX_SYMBOL_STR(usb_find_interface) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x2b19cfb2, __VMLINUX_SYMBOL_STR(usb_submit_urb) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "44136BEBE635ADC8FC9D1B6");
