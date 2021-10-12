#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

asmlinkage long sys_calc_mul (long oprd1, long oprd2, long* mem_block_buf) {
	long res;
	res = oprd1 * oprd2;

	if (copy_to_user(mem_block_buf, &res, sizeof(res))) {
		return -EFAULT;
	}

	return 0;
}

SYSCALL_DEFINE3(calc_mul, long, oprd1, long, oprd2, long*, mem_block_buf) {
	return sys_calc_mul(oprd1, oprd2, mem_block_buf);
}
