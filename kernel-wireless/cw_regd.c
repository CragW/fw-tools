#include "cw_regd.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>

/*
 * Iterate through wireless_dev that are registered with cfg80211
 * and print the country code for each device.
 */
void cw_prt_country_codes(void) {
    struct wireless_dev *wdev;
    struct net_device *dev;

    rcu_read_lock();
    for_each_netdev(&init_net, dev) {
        if (!dev->ieee80211_ptr)
            continue;

        wdev = dev->ieee80211_ptr;
        pr_info("Device %s has country code %c%c\n", dev->name,
                wdev->wiphy->regd->alpha2[0], wdev->wiphy->regd->alpha2[1]);
    }
    rcu_read_unlock();
}
EXPORT_SYMBOL(cw_prt_country_codes);
