#define _GNU_SOURCE
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "base.h"
#include "scheduler.h"

typedef struct _HookInfo {
    const char* jobs_path;
    int progress_begin;
    int progress_end;
} HookInfo;

//TODO:
//extract /cdrom/casper/filesystem.squashfs
//extract /cdrom/casper/overlay-deepin-${lang_pack}.squashfs
HookInfo before_chroot_info = { HOOKS_DIR"/before_chroot", 5, 70};

//TODO:
//setup accounts
//setup locale
//setup zoneinfo
//fix pxe network (Done)
//update-grub (Done)
//remove-unused-packages (Done)
HookInfo in_chroot_info = { "/host/"HOOKS_DIR"/in_chroot", 70, 85};

//What to do?
HookInfo after_chroot_info = { HOOKS_DIR"/after_chroot", 85, 99};


void run_hooks(HookInfo* info);

gboolean enter_chroot()
{
    gboolean ret = FALSE;

    extern int chroot_fd;
    //Never use "." instead of "/" otherwise if "." equal TARGET then we can't break chroot"
    if ((chroot_fd = open ("/", O_RDONLY)) < 0) {
        g_warning ("chroot:set chroot fd failed\n");
	return ret;
    }

    extern gboolean in_chroot;
    if (chroot ("/target") == 0) {
	chdir("/"); //change to an valid directory
        in_chroot = TRUE;
        ret = TRUE;
    } else {
        g_warning ("chroot:chroot to /target falied:%s\n", strerror (errno));
    }

    return ret;
}

gboolean break_chroot()
{
    extern gboolean in_chroot;
    extern int chroot_fd;

    if (in_chroot) {

	if (fchdir (chroot_fd) != 0) {
	    g_warning ("finish install:reset to chroot fd dir failed\n");
	} else {
	    int i = 0;
	    for (i = 0; i < 1024; i++) {
		chdir ("..");
	    }
	}
	chroot (".");
	in_chroot = FALSE;
    }
}


void run_hooks_before_chroot()
{
    run_hooks(&before_chroot_info);
}

void run_hooks_in_chroot()
{
    enter_chroot();
    run_hooks(&in_chroot_info);
}

void run_hooks_after_chroot()
{
    break_chroot();
    run_hooks(&after_chroot_info);
    //TODO: finish_cleanup
}


void run_one_by_one(GPid pid, gint status, GList* jobs)
{
    if (pid != -1) {
	g_spawn_close_pid(pid);
    }

    if (jobs->data == NULL) {
	g_list_free_full(g_list_first(jobs), g_free);
	enter_next_stage();
	return;
    }

    gint std_out, std_err;
    GPid child_pid;
    GError* error = NULL;

    char* argv[2];
    argv[0] = jobs->data;
    argv[1] = 0;

    g_debug("RUN :%s\n", (char*)jobs->data);
    char* dir = g_path_get_dirname(jobs->data);
    g_spawn_async(dir,
	    argv,
	    NULL,
	    G_SPAWN_DO_NOT_REAP_CHILD,
	    NULL,
	    NULL,
	    &child_pid,
	    &error);
    g_free(dir);
    if (error != NULL) {
	g_error("can't spawn %s: %s\n", argv[0], error->message);
	g_error_free(error);
	return;
    }
    g_child_watch_add(child_pid, (GChildWatchFunc)run_one_by_one, g_list_next(jobs));
}

void run_hooks(HookInfo* info)
{
    const char* path = info->jobs_path;
    GError* error = NULL;
    GList* jobs = NULL;
    GDir* dir = g_dir_open(path, 0, &error);
    if (error != NULL) {
	g_error("can't exec_hoosk %s: %s\n", path, error->message);
	g_error_free(error);
	return;
    }

    const char* job_name = NULL;
    while (job_name = g_dir_read_name(dir)) {
	if (g_str_has_suffix(job_name, ".job")) {
	    jobs = g_list_append(jobs, (gpointer)g_build_filename(path, job_name, NULL));
	}
    }
    g_dir_close(dir);
    chdir(path);

    jobs = g_list_sort(jobs, (GCompareFunc)strcmp);

    //append the end guard element so that run_one_by_one can free the GList
    jobs = g_list_append(jobs, NULL);

    run_one_by_one(-1, 0, jobs);
}
