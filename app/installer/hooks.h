#ifndef __HOOKS_H__
#define __HOOKS_H__

void run_hooks_before_chroot();
void run_hooks_in_chroot();
void run_hooks_after_chroot();

#endif
