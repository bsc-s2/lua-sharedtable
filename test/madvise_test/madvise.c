#define _BSD_SOURCE

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>


#define SHM_PER_REGION_SIZE ((uint32_t)(10*1024*1024))
#define SHM_REGION_CNT      ((uint32_t)(5))


int event_fd[2] = {0};


void
notify(int notify_fd)
{
    uint64_t event = 1;

    write(notify_fd, &event, sizeof(event));
}


void
wait_notify(int wait_fd)
{
    uint64_t event = 0;

    read(wait_fd, &event, sizeof(event));
}


void
notifier_and_wait(int notify_fd, int wait_fd)
{
    notify(notify_fd);
    wait_notify(wait_fd);
}


void
store_self_statm(void)
{
    static int index = 1;

    pid_t pid = getpid();

    char buf[1024];
    char src_path[1024];
    char dst_path[1024];

    memset(buf, 0, 1024);
    memset(src_path, 0, 1024);
    memset(dst_path, 0, 1024);

    snprintf(src_path, 1024, "/proc/%d/statm", pid);
    snprintf(dst_path, 1024, "statm_%d_%d", pid, index++);

    int src_fd = open(src_path, O_RDONLY);
    int dst_fd = open(dst_path, O_WRONLY|O_CREAT, 0766);

    int cnt = read(src_fd, buf, 1024);
    while (0 != cnt) {
        write(dst_fd, buf, cnt);

        cnt = read(src_fd, buf, 1024);
    }

    close(dst_fd);
    close(src_fd);
}


long
get_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}


void
force_load_rss_pages(uint8_t *shm_base_ptr)
{
    uint32_t page_size = (uint32_t)get_page_size();

    int cnt = SHM_PER_REGION_SIZE / page_size * SHM_REGION_CNT;
    printf("lsl-debug: page cnt: %d\n", cnt);

    for (int i = 0; i < cnt; i++) {
        uint8_t *ptr = shm_base_ptr + (i * page_size);
        ptr[0] = 1;
    }
}


void
madvise_remove_regions(uint8_t *shm_base_ptr, int notifier, int waiter)
{
    uint32_t page_size = (uint32_t)get_page_size();

    uint8_t *base = shm_base_ptr - SHM_PER_REGION_SIZE;
    for (int cnt = 0; cnt < SHM_REGION_CNT; cnt++) {
        base += SHM_PER_REGION_SIZE;

        int ret = madvise(base, SHM_PER_REGION_SIZE, MADV_REMOVE);
        if (ret != 0) {
            perror("shit, madvise failed!\n");
        }

        usleep(10);
        store_self_statm();
        notify(notifier);
        wait_notify(waiter);
    }
}


void
force_load_rss_pages_and_report(uint8_t *shm_base_ptr)
{
    force_load_rss_pages(shm_base_ptr);
    store_self_statm();
}


void
fork_two_children(uint8_t *shm_base_ptr, pid_t *children)
{
    event_fd[0] = eventfd(0, 0);
    event_fd[1] = eventfd(0, 0);
    if (event_fd[0] == -1 || event_fd[1] == -1) {
        perror("shit, eventfd failed!\n");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("shit, fork failed!\n");
        exit(1);
    }
    else if (pid == 0) {
        // notifier
      children[0] = getpid();
        printf("notifier child :%d\n", getpid());
        int notifier = event_fd[0];
        int waiter = event_fd[1];

        force_load_rss_pages_and_report(shm_base_ptr);

        notifier_and_wait(notifier, waiter);
        printf("%d: force load\n", getpid());
        madvise_remove_regions(shm_base_ptr, notifier, waiter);

        //force_load_rss_pages_and_report(shm_base_ptr);

        *shm_base_ptr = (uint8_t)10;
        printf("notifier set vaue: %d\n", (uint8_t)*shm_base_ptr);
        notify(notifier);

        exit(0);
    } else {
        pid = fork();
        if (pid < 0) {
            perror("shit, the second child fork failed\n");
            exit(1);
        }
        else if (pid == 0) {
            // wait for notification
            children[1] = getpid();
            printf("waiter child: %d\n", getpid());
            uint64_t event = 0;
            int notifier = event_fd[1];
            int waiter = event_fd[0];
            wait_notify(waiter);

            //store_self_statm();
            force_load_rss_pages_and_report(shm_base_ptr);

            notify(notifier);

            for (int cnt = 0; cnt < SHM_REGION_CNT; cnt++){
                wait_notify(waiter);
                store_self_statm();
                notify(notifier);
            }

            //force_load_rss_pages_and_report(shm_base_ptr);
            wait_notify(waiter);
            uint8_t value = *(shm_base_ptr);
            printf("waiter get vaue: %d\n", value);

            exit(0);
        }
    }
}


int
main(void)
{
    char *shm_path = "/lsl_test";

    int shm_fd = shm_open(shm_path, O_CREAT|O_RDWR|O_TRUNC, 0666);
    if (shm_fd == -1) {
        perror("shit, shm_open failed!\n");
        exit(1);
    }

    int shm_region_size = SHM_PER_REGION_SIZE * SHM_REGION_CNT;
    int ret = ftruncate(shm_fd, shm_region_size);
    if (ret != 0) {
        perror("shit, ftruncate failed!\n");
        goto quit;
    }

    uint8_t *shm_base_ptr = mmap(NULL,
                                 shm_region_size,
                                 PROT_READ | PROT_WRITE,
                                 MAP_SHARED,
                                 shm_fd,
                                 0);
    if (shm_base_ptr == MAP_FAILED) {
        perror("shit, mmap failed!\n");
        goto quit;
    }

    pid_t children[2];
    fork_two_children(shm_base_ptr, children);
    waitpid(children[0], NULL, 0);
    waitpid(children[1], NULL, 0);

quit:
    close(shm_fd);
    shm_unlink(shm_path);

    close(event_fd[0]);
    close(event_fd[1]);
    return 0;
}
