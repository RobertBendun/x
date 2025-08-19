#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/pidfd.h>
#include <unistd.h>
#include <sys/wait.h>

struct pids
{
	pid_t *pids;
	size_t count;
	size_t capacity;
};

/*
 * Return value:
 *   >0 - number of pids that finished
 *   =0 - timeout or no pids were provided
 *   <0 - error (errno is set)
 */
int waitpid_then_eject(struct pids *p, struct pids *ejects, int timeout)
{
	size_t i, fds_count = 0;
	int errno_backup, events, return_ = 0;
	struct pollfd *fds;
	if (p->count == 0) return 0;

	if ((fds = calloc(p->count, sizeof(struct pollfd))) == NULL) {
		errno = ENOMEM;
		return_ = -1;
		goto exit;
	}

	short const mask = POLLIN | POLLRDNORM;

	for (fds_count = 0; fds_count < p->count; ++fds_count) {
		fds[fds_count].fd = pidfd_open(p->pids[fds_count], PIDFD_NONBLOCK);
		fds[fds_count].events = mask;
		if (fds[fds_count].fd < 0) { return_ = -1; goto exit; }
	}

	return_ = poll(fds, fds_count, timeout);

	if (ejects) {
		ejects->pids = p->pids + p->count;
		ejects->count = 0;
		ejects->capacity = 0;
	}

	events = return_;
	for (i = fds_count - 1; i < fds_count && events > 0; --i) {
		if ((fds[i].revents & mask) == mask) {
			--events;

			pid_t tmp = p->pids[i];
			p->pids[i] = p->pids[p->count-1];
			p->pids[--p->count] = tmp;

			if (ejects) {
				--ejects->pids;
				++ejects->count;
			}
		}
	}

exit:
	errno_backup = errno;
	for (i = 0; i < fds_count; ++i) close(fds[i].fd);
	free(fds);
	errno = errno_backup;
	return return_;
}

int cmd(char **argv)
{
	int pid;

	printf("[CMD]");
	for (int i = 0; argv[i]; ++i) printf(" %s", argv[i]);
	printf("\n");

	if ((pid = fork()) == 0) {
		execvp(argv[0], argv);
		perror("execvp");
		exit(1);
	}
	return pid;
}

int main()
{
	struct pids pids;
	pids.count = 0;
	pids.capacity = 4;
	pids.pids = calloc(pids.capacity, sizeof(pid_t));

	for (int i = 0; i < 10; ++i) {
		if (pids.count == pids.capacity) {
			printf("---- wait ----\n");
			waitpid_then_eject(&pids, NULL, -1);
		}
		pids.pids[pids.count++] = cmd((char*[]) { "sleep", (i%2 == 0 ? "2" : "1"), 0 });
	}

	while (wait(NULL) > 0);
}
