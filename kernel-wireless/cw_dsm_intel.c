#include "cw_dsm_intel.h"

#define DSM_FUNC_ENABLE_6E 3

const guid_t guid_intel = GUID_INIT(0xF21202BF, 0x8F78, 0x4DC6, 0xA5, 0xB3,
                                    0x1F, 0x73, 0x8E, 0x28, 0x5A, 0xDE);

int cw_acpi_evaluate_dsm_intel(struct acpi_device *adev) {
    union acpi_object *obj;
    u64 value_u64 = 0;
    u32 value_u32 = 0;

    /* check if the device has a _DSM method */
    if (!acpi_has_method(adev->handle, "_DSM")) {
        printk(KERN_ERR "ACPI: Device does not have _DSM method\n");
        return -ENODEV;
    }

    /* call the _DSM method */
    obj = acpi_evaluate_dsm(adev->handle, &guid_intel, 0, DSM_FUNC_ENABLE_6E,
                            NULL);
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
        if (obj->buffer.length != expected_size) {
            printk(KERN_ERR
                   "ACPI: DSM method returned buffer of unexpected size\n");
            return -EINVAL;
        }

        le_value = *(__le64 *)obj->buffer.pointer;
        value_u64 = le64_to_cpu(le_value);
        value_u32 = cpu_to_le32((u32)value_u64);
        printk(KERN_INFO "ACPI: DSM method returned buffer: 0x%x\n", value_u32);

    } else {
        printk(KERN_ERR "ACPI: DSM method returned unexpected type\n");
        return -EINVAL;
    }

    kfree(obj);
    return 0;
}
EXPORT_SYMBOL(cw_acpi_evaluate_dsm_intel);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crag Wang");
MODULE_DESCRIPTION("ACPI DSM method for Intel devices");
