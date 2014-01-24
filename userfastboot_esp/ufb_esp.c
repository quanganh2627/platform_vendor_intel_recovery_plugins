#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <errno.h>

#include <userfastboot_util.h>
#include <userfastboot_plugin.h>
#include <stdlib.h>
#include <unistd.h>
/*from bootable/recovery*/
#include <bootloader.h>

#include "fastboot.h"
#include "userfastboot.h"

#define MAXFILS 16
#define BOOT_PART "bootloader"
#define BOOTLOADER_CAPSULE_TEST_BCB "update-firmware"
#define CAPSULE_TARGET_FOLDER "/mnt/bootloader/fwupdate"

static int cmd_flash_capsule_update(Hashmap *params, int *fd, unsigned sz)
{
	struct fstab_rec *bootloaderVol, *miscVol;
	void *data = NULL;
	int ret;
	static unsigned int fileNum;
	struct bootloader_message msg;
	char fileName[100];

	memset (&msg, 0, sizeof(msg));
	bootloaderVol = volume_for_path("/bootloader");
	if (!bootloaderVol) {
		pr_error("Couldn't find bootloader partition. Is your recovery.fstab valid?\n");
		return -1;
	}
	if (mount_partition(bootloaderVol)) {
		pr_error("Couldn't mount bootloader partition\n");
		return -1;
	}
	if (fileNum == 0) {
		ret = execute_command("rm -rf %s", CAPSULE_TARGET_FOLDER);
		if (ret) {
			pr_error("Failed to delete:%s\n", CAPSULE_TARGET_FOLDER);
			goto err;
		}
	}
	ret = mkdir(CAPSULE_TARGET_FOLDER, 0666);
	if (ret && errno != EEXIST) {
		pr_perror("mkdir");
		goto err;
	}

	data = mmap64(NULL, sz, PROT_READ, MAP_SHARED, *fd, 0);
	if (data == (void*)-1) {
		pr_error("Failed to mmap the file\n");
		goto err;
	}

	snprintf (fileName, sizeof(fileName), "%s/capsule%d.cap", CAPSULE_TARGET_FOLDER, fileNum++);
	if (named_file_write(fileName, data, sz, 0, 0)) {
		pr_error("Couldn't write capsule to bootloader partition.\n");
		munmap(data, sz);
		goto err;
	}
	munmap(data, sz);
	miscVol = volume_for_path("/misc");
	strncpy(msg.command, BOOTLOADER_CAPSULE_TEST_BCB, sizeof(msg.command));

	if (named_file_write(miscVol->blk_device, (const unsigned char*) &msg, sizeof(msg), 0, 0)) {
		pr_error("Failed to write bootloader message\n");
		goto err;
	}
	unmount_partition(bootloaderVol);
	return 0;
err:
	unmount_partition(bootloaderVol);
	return -1;
}

static int rdline(char *ptr)
{
	int cnt = 0;
	char *pos = ptr;
	while (*pos != '\n' && *pos != 0) {
		cnt++;
		pos++;
	}
	*pos = 0;
	return cnt + 1;
}

static int update_droid(Hashmap *params, int *fd, unsigned sz)
{
	int cnt, i, ret = -1;
	char *ptr = NULL, *ptr_orig = NULL;
	const char *bootptn;
	char buf[PATH_MAX];
	struct fstab_rec *vol;

	vol = volume_for_name(BOOT_PART);
	if (!vol) {
		pr_error("Couldn't find bootloader partition. Is your "
			"recovery.fstab valid?\n");
		return -1;
	}
	bootptn = vol->blk_device;

	ptr = ptr_orig = mmap64(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
	if (ptr == (void*)- 1) {
		pr_error("dbupdate: failed to mmap the file\n");
		goto out_fail;
	}

	/* read in file count */
	i = rdline(ptr);
	cnt = strtol(ptr, NULL, 10);
	if (cnt < 1) {
		pr_error("bad blob format");
		goto out_munmap;
	}
	ptr += i;

	if (cnt >= MAXFILS) {
		pr_error("Too many files");
		goto out_munmap;
	}

	if (mount_partition_device(bootptn, vol->fs_type, "/boot")) {
		pr_error("Couldn't mount bootloader partition.\n");
		goto out_munmap;
	}

	for (i = 0; i < cnt; i++) {
		int ret, len;
		char *idx, *name;
		unsigned char *prog;
		ret = rdline(ptr);
		name = ptr;
		ptr += ret;
		prog = (unsigned char*)ptr;

		idx = index(name, ',');
		if (!idx) {
			pr_error("Bad format %s %d %d\n",name, errno, ret);
			goto out_fail;
		}
		*idx = 0;
		len = strtol(idx+1, NULL, 10);
		if (len < 0) {
			pr_error("Bad blob format");
			goto out_fail;
		}
		snprintf(buf, PATH_MAX, "/boot/%s", name);

		pr_info("Writing file '%s'\n", name);
		ret = named_file_write(buf, prog, len, 0, 0);
		if (ret < 0) {
			pr_error("Couldn't write %s %d\n",buf,ret);
			goto out_fail;
		}
		ptr += len;
	}
	ret = 0;

out_munmap:
	munmap(ptr_orig, sz);
out_fail:
	umount(bootptn);

	return ret;
}

void libufb_esp_init(void)
{
	aboot_register_flash_cmd("bootloader", update_droid);
	aboot_register_flash_cmd("capsule", cmd_flash_capsule_update);
}
