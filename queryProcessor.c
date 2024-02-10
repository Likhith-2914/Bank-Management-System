#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "header.h"

int deposit(off_t offset, float money, int type)
{
    if(!type)
    {
        char *file_name = "databases/accDB";
        Account account;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(Account)) == -1) return 2;
        account.balance += money;
        if(lseek(fd, offset, SEEK_SET) == -1) return 3;
        if(write(fd, &account, sizeof(Account)) == -1) return 4;
        close(fd);
        return 0;
    }
    else
    {
        struct flock lock;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(jointAccount);
        lock.l_pid = getpid();

        char *file_name = "databases/jointDB";
        jointAccount account;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(jointAccount)) == -1) return 2;

        /*critical section*/
        fcntl(fd, F_SETLKW, &lock);
        account.balance += money;
        if(lseek(fd, offset, SEEK_SET) == -1) return 3;
        if(write(fd, &account, sizeof(jointAccount)) == -1) return 4;
        /******************/

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);

        return 0;
    }

}


int withdraw(off_t offset, float money, int type)
{
    if(!type)
    {
        char *file_name = "databases/accDB";
        Account account;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(Account)) == -1) return 2;
        if(account.balance < money) return -1;
        account.balance -= money;
        if(lseek(fd, offset, SEEK_SET) == -1) return 3;
        if(write(fd, &account, sizeof(Account)) == -1) return 4;
        return 0;
    }
    else
    {
        struct flock lock;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(jointAccount);
        lock.l_pid = getpid();

        char *file_name = "databases/jointDB";
        jointAccount account;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(jointAccount)) == -1) return 2;

        /*critical section*/
        fcntl(fd, F_SETLKW, &lock);
        if(account.balance < money) return -1;
        account.balance -= money;
        if(lseek(fd, offset, SEEK_SET) == -1) return 3;
        if(write(fd, &account, sizeof(jointAccount)) == -1) return 4;
        /******************/

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);

        return 0;
    }
}

//read-lock
int balanceQuery(off_t offset, float *balance, int type)
{
    if(!type)
    {
        char *file_name = "databases/accDB";
        Account account;
        int fd = open(file_name, O_RDONLY);
        if(lseek(fd, offset, SEEK_SET) == -1) {*balance = -1; return 1;}
        if(read(fd, &account, sizeof(Account)) == -1) {*balance = -1; return 2;}
        *balance = account.balance;
        return 0;
    }
    else
    {
        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(jointAccount);
        lock.l_pid = getpid();

        char *file_name = "databases/jointDB";
        jointAccount account;
        int fd = open(file_name, O_RDONLY);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(jointAccount)) == -1) return 2;

        /*critical section*/
        fcntl(fd, F_SETLKW, &lock);
        *balance = account.balance;
        /******************/

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);

        return 0;
    }
}

//write-lock
int passwordChange(off_t offset, char *newPass, int type)
{
    if(!type)
    {
        char *file_name = "databases/accDB";
        Account account;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(Account)) == -1) return 2;
        strcpy(account.password, newPass);
        if(lseek(fd, offset, SEEK_SET) == -1) return 3;
        if(write(fd, &account, sizeof(Account)) == -1) return 4;
        return 0;
    }
    else
    {
        struct flock lock;
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(jointAccount);
        lock.l_pid = getpid();

        char *file_name = "databases/jointDB";
        jointAccount account;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &account, sizeof(jointAccount)) == -1) return 2;

        /*critical section*/
        fcntl(fd, F_SETLKW, &lock);
        strcpy(account.password, newPass);
        if(lseek(fd, offset, SEEK_SET) == -1) return 3;
        if(write(fd, &account, sizeof(Account)) == -1) return 4;
        /******************/

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);

        return 0;
    }
}

//read-lock
int viewDetails(off_t offset, accDetails *details, int type)
{
    details->type = type;
    if(!type)
    {
        Account acc;
        char *file_name = "databases/accDB";
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;
        if(read(fd, &acc, sizeof(Account)) == -1) return 2;
        strcpy(details->acc_no, acc.acc_no);
        strcpy(details->password, acc.password);
        details->cust1 = acc.cust;
        details->balance = acc.balance;

        return 0;
    }
    else
    {
        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = offset;
        lock.l_len = sizeof(jointAccount);
        lock.l_pid = getpid();

        char *file_name = "databases/jointDB";
        jointAccount *acc;
        int fd = open(file_name, O_RDWR);
        if(lseek(fd, offset, SEEK_SET) == -1) return 1;

        /*critical section*/
        fcntl(fd, F_SETLKW, &lock);
        if(read(fd, acc, sizeof(Account)) == -1) return 2;
        /******************/

        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);

        strcpy(details->acc_no, acc->acc_no);
        strcpy(details->password, acc->password);
        details->cust1 = acc->cust1;
        details->cust2 = acc->cust2;
        details->balance = acc->balance;

        return 0;
    }
}