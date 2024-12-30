#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time64.h>
#include <linux/ktime.h>

#define procfs_name "tsulab"
static struct proc_dir_entry* our_proc_file = NULL;
// Обработка полученных данных
static ssize_t procfile_read(struct file* file_pointer, char __user* buffer, size_t buffer_length, loff_t* offset)
{
    struct tm start_time;
    start_time.tm_year = 2008;
    start_time.tm_mon = 9;
    start_time.tm_mday = 10;
    start_time.tm_hour = 12;
    start_time.tm_min = 0;
    start_time.tm_sec = 0;

    time64_t start_time_sec, current_time_sec;
    long long ran_laps;
    struct timespec64 ts;

    // Перевод установленнго времени в секунды
    start_time_sec = mktime64(start_time.tm_year, start_time.tm_mon, start_time.tm_mday, start_time.tm_hour, start_time.tm_min, start_time.tm_sec);
    // Получение текущего времени
    ktime_get_real_ts64(&ts);
    current_time_sec = ts.tv_sec;
    // 2.7 часов - один круг
    ran_laps = (current_time_sec - start_time_sec) / 3600 * 10 / 27;

    char res_str[128];
    ssize_t data_len;
    
    data_len = sprintf(res_str, "Since the launch of the LHC, the runner has run %lld laps.\n", ran_laps);

    pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
    
    if (*offset >= data_len) {
        return 0;
    }
    if (copy_to_user(buffer, res_str, data_len)) {
        return 0;
    }

    *offset += data_len;

    return data_len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
};
#endif

static int __init procfs1_init(void)
{
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    return 0;
}
static void __exit procfs1_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", procfs_name);
}
module_init(procfs1_init);
module_exit(procfs1_exit);
MODULE_LICENSE("GPL");
