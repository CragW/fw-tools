#include <linux/acpi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "cw_dsm_intel.h"

struct {
    unsigned int vendor_id;
    unsigned int device_id;
    const char *name;
} intel_pci_ids[] = {{0x8086, 0x272b, "Intel BE200"},
                     {0x8086, 0x7af0, "Intel AX211"}};

struct {
    unsigned int vendor_id;
    unsigned int device_id;
    const char *name;
} rtk_pci_ids[] = {
    {0x10ec, 0x8852, "Realtek RTL8852CE"},
};

extern int cw_acpi_evaluate_dsm_intel(struct acpi_device *);
extern void cw_prt_country_codes(void);

static struct acpi_device *cw_get_acpi_device(unsigned int vid,
                                              unsigned int did) {
    struct pci_dev *pdev = NULL;
    while ((pdev = pci_get_device(vid, did, pdev)) != NULL) {
        if (pdev)
            return ACPI_COMPANION(&pdev->dev);
    }
    return NULL;
}

static int __init cw_init(void) {
    /* Intel */
    for (int i = 0; i < ARRAY_SIZE(intel_pci_ids); i++) {
        struct acpi_device *adev = NULL;
        adev = cw_get_acpi_device(intel_pci_ids[i].vendor_id,
                                  intel_pci_ids[i].device_id);
        if (adev) {
            printk(KERN_INFO "ACPI: found wireless device %s\n",
                   intel_pci_ids[i].name);
            if (cw_acpi_evaluate_dsm_intel(adev))
                printk(KERN_ERR "ACPI: failed to evaluate DSM method for "
                                "regulatory settings\n");
        }
    }

    /* Realtek */
    for (int i = 0; i < ARRAY_SIZE(rtk_pci_ids); i++) {
        // TODO
    }

    /* show regulatory country code */
    cw_prt_country_codes();

    return 0; // Module loaded successfully
}

static void __exit cw_exit(void) {
    printk(KERN_INFO "ACPI: Exiting module...\n");
}

module_init(cw_init);
module_exit(cw_exit);

MODULE_IMPORT_NS(IWLWIFI);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crag Wang");
MODULE_DESCRIPTION(
    "Search for PCI device with Vendor ID 0x8086 and Device ID 0x272b and "
    "retrieve its ACPI device information");
