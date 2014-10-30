/*
 * vr_uvhost.c - implements a user-space vhost server that peers with
 * the vhost client inside qemu (version 2.1 and later).
 *
 * Copyright (c) 2014 Juniper Networks, Inc. All rights reserved.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/vhost.h>

#include "vr_uvhost.h"
#include "vr_uvhost_util.h"
#include "vr_uvhost_msg.h"
#include "qemu_uvhost.h"
#include "vr_uvhost_client.h"

/*
 * Prototypes
 */
static void *vr_uvhost_start(void *arg);

/* Global variables */
static vr_uvh_exit_callback_t vr_uvhost_exit_fn;

/*
 * vr_uvhost_init - initializes the user space vhost server and waits
 * for messages from the netlink thread or qemu clients. The netlink thread
 * sends messages on a UNIX pipe in order to specify the names of the UNIX
 * domain socket on which a qemu client will connect. Once this message is 
 * received, the vhost server creates the UNIX domain socket and includes it
 * in the list of fds it waits on. 
 *
 * Returns 0 on success, error otherwise. 
 */
int 
vr_uvhost_init(pthread_t *th, vr_uvh_exit_callback_t exit_fn)
{
    if (pthread_create(th, NULL, vr_uvhost_start, NULL)) {
        return -1;
    }

    vr_uvhost_exit_fn = exit_fn;
    return 0;
}

/*
 * vr_uvhost_exit - exits the user space vhost server thread. Also forces
 * other threads in the process to exit by calling the process exit
 * callback.
 *
 * Returns nothing.
 */
static void
vr_uvhost_exit(void)
{
    vr_uvhost_exit_fn();

    return;
}

/* 
 * vr_uvhost_start - starts the user space vhost server
 *
 * Returns NULL if an error occurs. Otherwise, it runs forever.
 */
static void *
vr_uvhost_start(void *arg)
{
    int s = 0, ret;
    struct sockaddr_un sun;
    fd_set *rfdset, *wfdset;

    vr_uvhost_client_init();

    s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (s < 0) {
        goto error;
    }       

    unlink(VR_UVH_NL_SOCK);
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, VR_UVH_NL_SOCK, sizeof(sun.sun_path));     
   
    ret = bind(s, (struct sockaddr *) &sun, sizeof(sun));
    if (ret < 0) {
        vr_uvhost_log("Error binding vhost server netlink socket\n");
        goto error;
    }

    if (listen(s, 1) < 0) {
        vr_uvhost_log("Error listening on vhost server netlink socket\n");
        goto error;
    }

    vr_uvhost_fdset_init();

    if (vr_uvhost_add_fd(s, UVH_FD_READ, NULL, vr_uvh_nl_listen_handler)) {
        vr_uvhost_log("Error initializing vhost server thread\n");
        goto error;
    }

    while (1) {
        vr_uvh_reset_max_fd();
        rfdset = vr_uvh_rfdset_p();
        wfdset = vr_uvh_wfdset_p();

        if (select(vr_uvh_max_fd()+1, rfdset, wfdset,  NULL, NULL) < 0) {
            vr_uvhost_log("Error in vhoset server select\n");
            goto error;
        }

        if (vr_uvh_call_fd_handlers()) {
            vr_uvhost_log("Error calling socket handler in vhost server\n");
            goto error;
        }
    }
 
error:
    if (s) {
        close(s);
    }

    vr_uvhost_exit();

    return NULL;
}
    