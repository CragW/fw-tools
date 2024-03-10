#include <linux/acpi.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>

static int __init acpi_amac_init(void) {
    acpi_status status;
    struct acpi_buffer buffer = {ACPI_ALLOCATE_BUFFER, NULL};
    acpi_object_type mac_obj_type = ACPI_TYPE_BUFFER;
    union acpi_object *obj;
    char *mac_obj_name = "\\_SB.AMAC";
    int mac_strlen = 0x17;
    int ret = -EINVAL;
    unsigned char buf[6];

    // Evaluate the ACPI object "\\SB.AMAC"
    status = acpi_evaluate_object(NULL, mac_obj_name, NULL, &buffer);
    if (ACPI_FAILURE(status)) {
        pr_err("Failed to evaluate ACPI object %s: %s\n", mac_obj_name,
               acpi_format_exception(status));
        kfree(buffer.pointer);
        return -ENODEV;
    }

    obj = (union acpi_object *)buffer.pointer;
    if (obj->type != mac_obj_type || obj->string.length != mac_strlen) {
        pr_err("Invalid buffer for passthru MAC addr: (%d, %d)\n", obj->type,
               obj->string.length);
        goto amacout;
    }

    if (strncmp(obj->string.pointer, "_AUXMAC_#", 9) != 0 ||
        strncmp(obj->string.pointer + 0x15, "#", 1) != 0) {
        pr_err("Invalid header when reading passthru MAC addr\n");
        goto amacout;
    }

    ret = hex2bin(buf, obj->string.pointer + 9, 6);
    if (!(ret == 0 && is_valid_ether_addr(buf))) {
        pr_err("Invalid MAC for passthru MAC addr: %d, %pM\n", ret, buf);
        ret = -EINVAL;
        goto amacout;
    }

    /* Copy to MAC address: PROD
            struct sockaddr *sa;
            memcpy(sa->sa_data, buf, sizeof(buf));
    */
    pr_info("Using passthru MAC addr %pM\n", buf);

    // Free the allocated buffer
amacout:
    kfree(buffer.pointer);
    return ret;
}

static void __exit acpi_amac_exit(void) {
    pr_info("Exiting ACPI AMAC module\n");
}

module_init(acpi_amac_init);
module_exit(acpi_amac_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Crag Wang");
MODULE_DESCRIPTION("ACPI AMAC Module");
