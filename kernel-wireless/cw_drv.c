#include <linux/acpi.h>
#include <linux/module.h>
#include <linux/pci.h>

#define DSM_FUNC_ENABLE_6E 3

const guid_t iwl_guid = GUID_INIT(0xF21202BF, 0x8F78, 0x4DC6, 0xA5, 0xB3, 0x1F,
                                  0x73, 0x8E, 0x28, 0x5A, 0xDE);

/* module device id table */
static const struct pci_device_id cw_device_id[] = {
    {PCI_DEVICE(0x8086, 0x272b)},
    {
        0,
    }};
MODULE_DEVICE_TABLE(pci, cw_device_id);

static int cw_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
    struct acpi_device *adev;
    union acpi_object *obj;
    u64 value_u64 = 0;
    u32 value_u32 = 0;
    int ret = -ENODEV;

    printk(KERN_INFO "ACPI: Device ID: 0x%x\n", pdev->device);

    /* check if the device is managed by ACPI */
    adev = ACPI_COMPANION(&pdev->dev);
    if (!adev) {
        printk(KERN_ERR "ACPI: Device is not managed by ACPI\n");
        return -ENODEV;
    }

    /* check if the device has a _DSM method */
    if (!acpi_has_method(adev->handle, "_DSM")) {
        printk(KERN_ERR "ACPI: Device does not have _DSM method\n");
        return -ENODEV;
    }

    /* call the _DSM method */
    obj =
        acpi_evaluate_dsm(adev->handle, &iwl_guid, 0, DSM_FUNC_ENABLE_6E, NULL);
    if (!obj) {
        printk(KERN_ERR "ACPI: DSM method did not return a valid object\n");
        return -ENODEV;
    }

    if (obj->type == ACPI_TYPE_INTEGER) {
        value_u64 = obj->integer.value;
        printk(KERN_INFO "ACPI: DSM method returned integer: 0x%llx\n",
               value_u64);

    } else if (obj->type == ACPI_TYPE_BUFFER) {
        __le64 le_value = 0;
        size_t expected_size = sizeof(u8);

        if (WARN_ON_ONCE(expected_size > sizeof(le_value)))
            return -EINVAL;

        /* if the buffer size doesn't match the expected size */
        if (obj->buffer.length != expected_size)
            printk(KERN_WARNING
                   "ACPI: DSM method returned buffer size, padding or "
                   "truncating (%d)\n",
                   obj->buffer.length);
        else
            printk(KERN_INFO "ACPI: DSM method returned buffer length: %d\n",
                   obj->buffer.length);

        /* assuming LE from Intel BIOS spec */
        memcpy(&le_value, obj->buffer.pointer,
               min_t(size_t, expected_size, (size_t)obj->buffer.length));

        /* cast value u64 to be u32 */
        value_u64 = le64_to_cpu(le_value);
        value_u32 = cpu_to_le32((u32)value_u64);
        printk(KERN_INFO "ACPI: DSM method returned buffer: 0x%x\n", value_u32);

    } else {
        printk(KERN_ERR
               "ACPI: DSM method did not return a valid object, type=%d\n",
               obj->type);
        ret = -ENODEV;
    }

    kfree(obj);
    return ret;
}

static void cw_remove(struct pci_dev *pdev) { /* Cleanup code if needed */ }

static struct pci_driver cw_driver = {
    .name = "cw",
    .id_table = cw_device_id,
    .probe = cw_probe,
    .remove = cw_remove,
};

static int __init cw_init(void) {
    printk(KERN_INFO "ACPI Method Kernel Module Driver\n");
    return pci_register_driver(&cw_driver);
}

static void __exit cw_exit(void) {
    printk(KERN_INFO "ACPI Method Kernel Module Driver removed\n");
    pci_unregister_driver(&cw_driver);
}

module_init(cw_init);
module_exit(cw_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crag Wang");
MODULE_DESCRIPTION("ACPI Method Kernel Module Driver");
