#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Klim Kornienko");

#define PROC_FILE_NAME      "fortune"
#define PROC_SYMLINK_NAME   "fortune_symlink"
#define PROC_DIR_NAME       "fortune_dir"

static struct proc_dir_entry *dir_dentry;
static struct proc_dir_entry *symlink_dentry;
static struct proc_dir_entry *fortune_dentry;

#define COOKIE_POT_SIZE PAGE_SIZE

static char *cookie_pot = NULL;
char buffer[COOKIE_POT_SIZE];
static int read_i = 0;
static int write_i = 0;

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "md: my_open()");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "md: my_release()");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *u, size_t cnt, loff_t *off)
{
    printk(KERN_INFO "md: my_read()");

    int read_len = 0;

    if ((*off > 0) || (write_i == 0))
    {
        return 0;
    }

    if (read_i >= write_i)
    {
        read_i = 0;
    }

    read_len = snprintf(buffer, COOKIE_POT_SIZE, "%s\n", &cookie_pot[read_i]);

    if (copy_to_user(u, buffer, read_len) != 0)
    {
        printk(KERN_INFO "md: failed copy_to_user!");
        return -1;
    }

    read_i += read_len;
    *off += read_len;

    return read_len;
}

static ssize_t my_write(struct file *f, const char __user *u, size_t cnt, loff_t *off)
{
    printk(KERN_INFO "md: my_write()");

    if (cnt > COOKIE_POT_SIZE - write_i + 1)
    {
        printk(KERN_INFO "md: buffer overflow!");
        return -1;
    }

    if (copy_from_user(&cookie_pot[write_i], u, cnt) != 0)
    {
        printk(KERN_INFO "md: failed copy_from_user!");
        return -1;
    }

    write_i += cnt;
    cookie_pot[write_i - 1] = '\0';
    return cnt;
}

static struct proc_ops ops = {
    .proc_open = my_open,
    .proc_read = my_read,
    .proc_write = my_write,
    .proc_release = my_release,
}; 

static int md_init(void)
{
    if ((cookie_pot = vmalloc(COOKIE_POT_SIZE)) == NULL)
    {
        printk(KERN_INFO "md: failed vmalloc!");
        return -1;
    }

    memset(cookie_pot, 0, COOKIE_POT_SIZE);

    dir_dentry = proc_mkdir(PROC_DIR_NAME, NULL);
    symlink_dentry = proc_symlink(PROC_SYMLINK_NAME, dir_dentry, PROC_FILE_NAME);
    fortune_dentry = proc_create(PROC_FILE_NAME, S_IRUGO | S_IWUGO, dir_dentry, &ops);

    if (fortune_dentry == NULL)
    {
        printk(KERN_ERR "md: fortune_dentry was not created.");
        return -1;
    }

    read_i = 0;
    write_i = 0;

    printk(KERN_INFO "md: module loaded.");
    return 0;
}

static void md_exit(void)
{
    proc_remove(symlink_dentry);
    proc_remove(dir_dentry);
    proc_remove(fortune_dentry);

    vfree(cookie_pot);

    printk(KERN_INFO "md: module unloaded.");
}

module_init(md_init);
module_exit(md_exit);
