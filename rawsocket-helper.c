/*
 * Privileged helper binary.
 *
 * Copyright (c) 2015, Michael Walle <michael@walle.cc>
 * See LICENSE for licensing terms.
 *
 * Quick guide:
 *   make rawsocket_helper
 *   chown root:yourgroup rawsocket_helper
 *   chmod 750 rawsocket_helper
 *   setcap cap_net_raw+ep rawsocket_helper
 */

#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "rawsocket.h"

int send_socket(int fd, int sfd)
{
	int rc;
	struct iovec iov;
	struct msghdr msg = {0};
	struct cmsghdr *cmsg;
	char buf[CMSG_SPACE(sizeof(int))];

	/* dummy write */
	iov.iov_base = "";
	iov.iov_len = 1;

	msg.msg_control = buf;
	msg.msg_controllen = sizeof(buf);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	*(int *)CMSG_DATA(cmsg) = sfd;

	rc = sendmsg(fd, &msg, 0);
	if (rc == -1)
		return -1;

	return 0;
}

int main(int argc, char **argv)
{
	int rc;
	int fd, raw_fd;
	char *endp;
	int family, protocol;
	int ret = EXIT_FAILURE;

	/* argument parsing */
    if (SHOW_VERBOSE) {
        printf("Arguments: %d\n", argc);
    }
	if (argc != 4)
	{
		goto out;
    }
	protocol = strtol(argv[3], &endp, 10);

	family = strtol(argv[2], &endp, 10);
	if (SHOW_VERBOSE) {
        printf("  Arg #0: %s\n", argv[0]);
        printf("  Arg #1: %s\n", argv[1]);
    }
    if (*argv[2] && *endp) {
        goto out;
    }
    if (SHOW_VERBOSE) {
        printf("  Arg #2: %s\n", argv[2]);
    }

	fd = strtol(argv[1], &endp, 10);
	if (*argv[1] && *endp) {
	    printf("there was some error, endp\n");
		goto out;
    }

	/* open raw socket */
	raw_fd = socket(family, SOCK_RAW, protocol);
	if (raw_fd == -1) {
	    printf("there was some error, raw_fd == -1\n");
		goto out;
    }

	rc = send_socket(fd, raw_fd);
	if (rc == -1) {
		close(raw_fd);
		goto out;
	}

	ret = EXIT_SUCCESS;
    if (SHOW_VERBOSE) {
        printf("Operation successfull\n");
    }

out:
	close(fd);
	return ret;
}
