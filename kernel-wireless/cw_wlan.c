#include <linux/acpi.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "cw_dsm_intel.h"

#define PCI_VENDOR_ID_INTEL 0x8086
#define PCI_DEVICE_ID_INTEL 0x272b

#define PCI_VENDOR_ID_RTK 0x10EC
#define PCI_DEVICE_ID_RTK 0x8852

extern int cw_acpi_evaluate_dsm_intel(struct acpi_device *);
extern void cw_prt_country_codes(void);

static struct acpi_device *cw_get_acpi_device(unsigned int vid,
                                              unsigned int did) {
    struct pci_dev *pdev = NULL;
    printk(KERN_INFO
           "ACPI: Searching for PCI device, Vendor ID: 0x%x, Device ID: 0x%x\n",
           vid, did);
    while ((pdev = pci_get_device(vid, did, pdev)) != NULL) {
        if (pdev)
            return ACPI_COMPANION(&pdev->dev);
    }
    return NULL;
}

static int __init cw_init(void) {
    struct acpi_device *adev = NULL;

    /* Intel */
    adev = cw_get_acpi_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL);
    if (adev && (cw_acpi_evaluate_dsm_intel(adev) == 0))
        printk(KERN_INFO "ACPI: Intel DSM method completed successfully\n");

    /* Realtek */
    adev = cw_get_acpi_device(PCI_VENDOR_ID_RTK, PCI_DEVICE_ID_RTK);
    if (adev)
        printk(KERN_INFO "ACPI: Realtek DSM method completed successfully\n");

    /* show regulatory country code */
    cw_prt_country_codes();

    return 0; // Module loaded successfully
}

static void __exit cw_exit(void) {
    printk(KERN_INFO "ACPI: Exiting module...\n");
}

module_init(cw_init);
module_exit(cw_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crag Wang");
MODULE_DESCRIPTION(
    "Search for PCI device with Vendor ID 0x8086 and Device ID 0x272b and "
    "retrieve its ACPI device information");
