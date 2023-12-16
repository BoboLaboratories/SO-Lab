#include <string.h>
#include "ipc.h"

void new_ipc_res(struct IpcRes *res) {
    // we set everything to -1 or NULL as each component
    // may keep track of different information
    memset(res, -1, sizeof(struct IpcRes));
    res->main_shmem_addr = NULL;
    res->ctl_addr = NULL;
}