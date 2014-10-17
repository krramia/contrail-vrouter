/*
 * vr_uvhost_msg.h - header for handlers for messages received by the user space
 * vhost thread.
 *
 * Copyright (c) 2014 Juniper Networks, Inc. All rights reserved.
 */
#ifndef __VR_UVHOST_MSG_H__
#define __VR_UVHOST_MSG_H__

/*
 * Definitions of messages from the netlink thread that are handled by the 
 * user space vhost thread
 */

#define VR_UVH_VIF_NAME_SIZE 64
#define VR_UVH_VIF_PREFIX "/var/tmp/uvh_vif_"
#define VR_UVH_VIF_PREFIX_SIZE (strlen(VR_UVH_VIF_PREFIX))
#define VHOST_USER_VERSION 1
 
typedef enum vrnu_msg_type {
    VRNU_MSG_VIF_ADD = 1,
    VRNU_MSG_MAX
} vrnu_msg_type_t;

typedef struct vrnu_vif_add {
    char vrnu_vif_name[VR_UVH_VIF_NAME_SIZE];
} vrnu_vif_add_t;

typedef struct vrnu_msg {
    vrnu_msg_type_t vrnum_type;
    union {
        vrnu_vif_add_t vrnum_vif_add;
    };
} vrnu_msg_t;

int vr_uvh_nl_listen_handler(int fd, void *arg);

#endif /* __VR_UVHOST_MSG_H__ */ 