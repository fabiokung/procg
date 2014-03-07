#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/namei.h>
#include <linux/mutex.h>
#include <linux/err.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabio Kung");

static int meminfo_cgroup_show(struct seq_file *m, void *v)
{
	seq_printf(m, "WIP\n");
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
	printk(KERN_INFO "Raw: %pd\n", raw_proc_fs);
	printk(KERN_INFO "Meminfo: %pd\n", meminfo);
	printk(KERN_INFO "Meminfo inode: %lu\n", meminfo->d_inode->i_ino);

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
