# Handy commands or tools

## Debug a process detached from gdb

Non è possibile debuggare un processo detachato da gdb per
motivi di sicurezza per via dell'hardening del kernel linux.  

```bash
# permette appunto di bypassare l'hardening
sudo bash -c "echo 0 > /proc/sys/kernel/yama/ptrace_scope"
# attacha in debug il processo pid 
gdb path/to/executable <pid>
```