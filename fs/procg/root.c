#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/res_counter.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/namei.h>
#include <linux/memcontrol.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabio Kung");

static int meminfo_cgroup_show(struct seq_file *m, void *v)
{
	struct mem_cgroup *memcg;
	u64 mem_total, mem_used, mem_free, mem_available;

	memcg = mem_cgroup_from_task(current);
	if (!memcg) {
		return SEQ_SKIP;
	}

	mem_total = mem_cgroup_get_limit(memcg, false);
	mem_used = mem_cgroup_usage(memcg, false);
	mem_free  = mem_total - mem_used;
	mem_available = mem_free;

	seq_printf(m,
		"MemTotal:       %8llu kB\n"
		"MemFree:        %8llu kB\n"
		"MemAvailable:   %8llu kB\n"
		//"Buffers:        %8lu kB\n"
		//"Cached:         %8lu kB\n"
		//"SwapCached:     %8lu kB\n"
		//"Active:         %8lu kB\n"
		//"Inactive:       %8lu kB\n"
		//"Active(anon):   %8lu kB\n"
		//"Inactive(anon): %8lu kB\n"
		//"Active(file):   %8lu kB\n"
		//"Inactive(file): %8lu kB\n"
		//"Unevictable:    %8lu kB\n"
		//"Mlocked:        %8lu kB\n"
		//"SwapTotal:      %8lu kB\n"
		//"SwapFree:       %8lu kB\n"
		//"Dirty:          %8lu kB\n"
		//"Writeback:      %8lu kB\n"
		//"AnonPages:      %8lu kB\n"
		//"Mapped:         %8lu kB\n"
		//"Shmem:          %8lu kB\n"
		//"Slab:           %8lu kB\n"
		//"SReclaimable:   %8lu kB\n"
		//"SUnreclaim:     %8lu kB\n"
		//"KernelStack:    %8lu kB\n"
		//"PageTables:     %8lu kB\n"
		//"CommitLimit:    %8lu kB\n"
		//"Committed_AS:   %8lu kB\n"
		//"VmallocTotal:   %8lu kB\n"
		//"VmallocUsed:    %8lu kB\n"
		//"VmallocChunk:   %8lu kB\n"
		,
		mem_total >> 10,
		mem_free >> 10,
		mem_available >> 10
		);
	return 0;
}

static int meminfo_cgroup_open(struct inode *inode, struct file *file)
{
	return single_open(file, meminfo_cgroup_show, NULL);
}

static const struct file_operations meminfo_cgroup_fop = {
	.open		= meminfo_cgroup_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct dentry *(*original_proc_mount) (struct file_system_type *, int,
		const char *, void *);

static void (*original_proc_kill_sb) (struct super_block *);

static struct file_system_type procg_fs_type = {
	.name = "procg",
	.fs_flags = FS_USERNS_MOUNT
};

static struct dentry *procg_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	struct dentry *raw_proc_fs;
	struct dentry *meminfo;
	struct inode *meminfo_inode;

	raw_proc_fs = original_proc_mount(fs_type, flags, dev_name, data);
	mutex_lock(&raw_proc_fs->d_inode->i_mutex);
	meminfo = lookup_one_len("meminfo", raw_proc_fs, 7);
	meminfo_inode = meminfo->d_inode;
	meminfo_inode->i_fop = &meminfo_cgroup_fop;
	mutex_unlock(&raw_proc_fs->d_inode->i_mutex);

	return raw_proc_fs;
}

static int __init init_procg_fs(void)
{
	struct file_system_type *proc;

	proc = get_fs_type("proc");
	original_proc_mount = proc->mount;
	original_proc_kill_sb = proc->kill_sb;
	procg_fs_type.mount = procg_mount;
	procg_fs_type.kill_sb = original_proc_kill_sb;
	printk(KERN_INFO "loading procg fs\n");
	return register_filesystem(&procg_fs_type);
}

static void __exit exit_procg_fs(void)
{
	printk(KERN_INFO "unloading procg fs\n");
	unregister_filesystem(&procg_fs_type);
}

module_init(init_procg_fs);
module_exit(exit_procg_fs);
