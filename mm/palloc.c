/*
 * kernel/palloc.c
 *
 * Physical driven User Space Allocator info for a set of tasks.
 */

#include <linux/types.h>
#include <linux/cgroup.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/palloc.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/bitmap.h>
#include <linux/module.h>

/*
 * Check if a page is compliant to the policy defined for the given vma
 */
#ifdef CONFIG_CGROUP_PALLOC

#define MAX_LINE_LEN (6*128)
/*
 * Types of files in a palloc group
 * FILE_PALLOC - contain list of palloc bins allowed
*/
typedef enum {
	FILE_PALLOC,
#ifdef CONFIG_DETMEM_PALLOC
	FILE_PALLOC_DM
#endif
} palloc_filetype_t;

/*
 * Top level palloc - mask initialized to zero implying no restriction on
 * physical pages
*/

static struct palloc top_palloc;

/* Retrieve the palloc group corresponding to this cgroup container */
struct palloc *cgroup_ph(struct cgroup *cgrp)
{
	return container_of(cgrp->subsys[palloc_subsys_id],
			    struct palloc, css);
}

struct palloc * ph_from_subsys(struct cgroup_subsys_state * subsys)
{
	return container_of(subsys, struct palloc, css);
}

/*
 * Common write function for files in palloc cgroup
 */
static int update_bitmask(unsigned long *bitmap, const char *buf, int maxbits)
{
	int retval = 0;

	if (!*buf)
		bitmap_clear(bitmap, 0, maxbits);
	else
		retval = bitmap_parselist(buf, bitmap, maxbits);

	return retval;
}


static int palloc_file_write(struct cgroup_subsys_state *css, struct cftype *cft,
			     const char *buf)
{
	int retval = 0;
	struct palloc *ph = container_of(css, struct palloc, css);

	switch (cft->private) {
	case FILE_PALLOC:
		retval = update_bitmask(ph->cmap, buf, palloc_bins());
		printk(KERN_INFO "Bins : %s\n", buf);
		break;
#ifdef CONFIG_DETMEM_PALLOC
	case FILE_PALLOC_DM:
		retval = update_bitmask(ph->dm_cmap, buf, palloc_bins());
		printk(KERN_INFO "DM Bins : %s\n", buf);
		break;
#endif
	default:
		retval = -EINVAL;
		break;

	}

	return retval;
}

static ssize_t palloc_file_read(struct cgroup_subsys_state *css,
				struct cftype *cft,
				struct file *file,
				char __user *buf,
				size_t nbytes, loff_t *ppos)
{
	struct palloc *ph = container_of(css, struct palloc, css);
	char *page;
	ssize_t retval = 0;
	char *s;

	if (!(page = (char *)__get_free_page(GFP_TEMPORARY)))
		return -ENOMEM;

	s = page;

	switch (cft->private) {
	case FILE_PALLOC:
		s += bitmap_scnlistprintf(s, PAGE_SIZE, ph->cmap, palloc_bins());
		printk(KERN_INFO "Bins : %s\n", s);
		break;
#ifdef CONFIG_DETMEM_PALLOC
	case FILE_PALLOC_DM:
		s += bitmap_scnlistprintf(s, PAGE_SIZE, ph->dm_cmap, palloc_bins());
		printk(KERN_INFO "DM Bins : %s\n", s);
		break;
#endif
	default:
		retval = -EINVAL;
		goto out;
	}
	*s++ = '\n';

	retval = simple_read_from_buffer(buf, nbytes, ppos, page, s - page);
out:
	free_page((unsigned long)page);
	return retval;
}


/*
 * struct cftype: handler definitions for cgroup control files
 *
 * for the common functions, 'private' gives the type of the file
 */
static struct cftype files[] = {
	{
		.name = "bins",
		.read = palloc_file_read,
		.write_string = palloc_file_write,
		.max_write_len = MAX_LINE_LEN,
		.private = FILE_PALLOC,
	},
#ifdef CONFIG_DETMEM_PALLOC
	{
		.name = "dm_bins",
		.read = palloc_file_read,
		.write_string = palloc_file_write,
		.max_write_len = MAX_LINE_LEN,
		.private = FILE_PALLOC_DM,
	},
#endif
	{ }	/* terminate */
};

/*
 * palloc_create - create a palloc group
 */
static struct cgroup_subsys_state *palloc_create(struct cgroup_subsys_state *css)
{
	struct palloc * ph_child;
	ph_child = kmalloc(sizeof(struct palloc), GFP_KERNEL);
	if(!ph_child)
		return ERR_PTR(-ENOMEM);

	bitmap_clear(ph_child->cmap, 0, MAX_PALLOC_BINS);
	return &ph_child->css;
}


/*
 * Destroy an existing palloc group
 */
static void palloc_destroy(struct cgroup_subsys_state *css)
{
	struct palloc *ph = container_of(css, struct palloc, css);
	kfree(ph);
}

struct cgroup_subsys palloc_subsys = {
	.name = "palloc",
	.css_alloc = palloc_create,
	.css_free = palloc_destroy,
	.subsys_id = palloc_subsys_id,
	.base_cftypes = files,
};

#endif /* CONFIG_CGROUP_PALLOC */
