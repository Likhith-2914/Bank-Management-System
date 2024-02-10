#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

void debug()
{
    printf("!\n");
}

void generatePass(Customer customer, char pass[], int type)
{
    strcpy(pass, "");
    for(int i = 2; i <= 8; i++)
    {
        char str[2];
        str[0] = customer.phoneNum[i];
        strcat(pass, str);
    }
    char str[2];
    if(!type) str[0] = customer.accs;
    else str[0] = customer.jointaccs;
}

void generateAccno(Customer customer, char accno[])
{
    strcpy(accno, customer.phoneNum);
    strcat(accno, "0");
    char num[2];
    sprintf(num, "%d", customer.accs);
    strcat(accno, num);
}


int adminVerify(Admin *admin)
{
    int fd = open("databases/adminDB", O_RDONLY);
    Admin temp;
    while(read(fd, &temp, sizeof(Admin)))
    {
        if(!strcmp(temp.adminId, admin->adminId))
        {
            if(!strcmp(temp.password, admin->password))
            {
                admin->customers = temp.customers;
                return 0;
            }
            else return 1;
        }
    }
    return 2;
}

int accVerify(loginCred login, int *blocks)
{
    int type = login.type;
    if(!type)
    {
        int fd = open("databases/accDB", O_RDONLY);
        Account temp;
        int blks = 0;
        while(read(fd, &temp, sizeof(Account)))
        {
            if((temp.cust == login.cust))
            {
                if(!strcmp(temp.password, login.password) && (!strcmp(temp.acc_no, login.acc_no)))
                {
                    if(temp.activity)
                    {
                        *blocks = blks;
                        return 0;
                    }
                    else return 1;
                }
                else return 2;
            }
            blks++;
        }
        return 3;
    }
    else
    {
        int fd = open("databases/jointDB", O_RDONLY);
        jointAccount temp;
        int blks = 0;
        while(read(fd, &temp, sizeof(jointAccount)))
        {
            if((temp.cust1 == login.cust) || (temp.cust2 == login.cust))
            {
                if(!strcmp(temp.password, login.password) && (!strcmp(temp.acc_no, login.acc_no)))
                {
                    if(temp.activity)
                    {
                        *blocks = blks;
                        return 0;
                    }
                    else return 1;
                }
                else return 2;
            }
            blks++;
        }
        return 3;
    }
    
}

void executeAccQuery(int nsd, int task, int blocks)
{
    int type = (task/10)-2;
    off_t offset;
    if(!type) offset = blocks * sizeof(Account);
    else offset = blocks *  sizeof(jointAccount);
    task %= 10;
    if(task == 1)
    {
        float money;
        read(nsd, &money, sizeof(float));
        int r = deposit(offset, money, type);
        write(nsd, &r, sizeof(int));
    }
    else if(task == 2)
    {
        float money;
        read(nsd, &money, sizeof(float));
        int r = withdraw(offset, money, type);
        write(nsd, &r, sizeof(int));
    }
    else if(task == 3)
    {
        float balance;
        balanceQuery(offset, &balance, type);
        write(nsd, &balance, sizeof(balance));
    }
    else if(task == 4)
    {
        char *newpass = (char *)malloc(sizeof(char[9]));
        read(nsd, newpass, sizeof(newpass));
        int r = passwordChange(offset, newpass, type);
        write(nsd, &r, sizeof(int));
    }
    else if(task == 5)
    {
        accDetails details;
        viewDetails(offset, &details, type);
        write(nsd, &details, sizeof(Account));
    }
}

void executeAdminQuery(int nsd, int task, Admin *admin)
{
    int rval;

    //signup a new customer
    if(task == 1)
    {
        Customer customer;
        //read the customer details from client
        read(nsd, &customer, sizeof(Customer));
        customer.id = ++admin->customers;
        //add it to the customer file
        int fd = open("databases/accDB", O_WRONLY);
        int r1 = lseek(fd, 0, SEEK_END);
        int r2 = write(fd, &customer, sizeof(Customer));
        if((r1 == -1) || (r2 == -1)) rval = 0;
        else rval = admin->customers;
        write(nsd, &rval, sizeof(int));
        close(fd);
        fd = open("databases/adminDB", O_WRONLY);
        write(fd, admin, sizeof(Admin));
    }
    
    //inactivate an account
    else if(task == 2)
    {
        int rval = 1;
        char acc_no[13];
        int blocks = 0;
        Account acc;
        read(nsd, acc_no, sizeof(char[13]));
        int fd = open("databases/accDB", O_RDWR);
        while(read(fd, &acc, sizeof(Account)))
        {
            if(!strcmp(acc.acc_no, acc_no))
            {
                acc.activity = 0;
                rval = 0;
                break;
            }
            blocks++;
        }
        if(lseek(fd, blocks*sizeof(Account), SEEK_SET) == -1) rval = 2;
        if(write(fd, &acc, sizeof(Account)) == -1) rval = 3;
        write(nsd, &rval, sizeof(int));
    }
    
    //inactivate a joint account
    else if(task == 3)
    {
        char acc_no[13];
        int rval = 1;
        int blocks = 0;
        jointAccount acc;
        read(nsd, acc_no, sizeof(char[13]));
        int fd = open("databases/jointDB", O_RDWR);
        while(read(fd, &acc, sizeof(jointAccount)))
        {
            if(!strcmp(acc.acc_no, acc_no))
            {
                acc.activity = 0;
                rval = 0;
                break;
            }
            blocks++;
        }
        if(lseek(fd, blocks*sizeof(Account), SEEK_SET) == -1) rval = 2;
        if(write(fd, &acc, sizeof(Account)) == -1) rval = 3;
        write(nsd, &rval, sizeof(int));
    }
    
    //create an account for user
    else if(task == 4)
    {
        int cust_id;
        Account acc;
        read(nsd, &cust_id, sizeof(int));
        if(cust_id > admin->customers)
        {
            acc.cust = 0;
            write(nsd, &acc, sizeof(Account));
        }
        else
        {
            int rval = 0;
            Customer customer;
            int fd = open("databases/customerDB", O_RDWR);
            if(lseek(fd, (cust_id-1)*sizeof(Customer), SEEK_SET) == -1) {rval = 1; return;}
            if(read(fd, &customer, sizeof(Customer)) == -1) {rval = 2; return;}
            close(fd);

            //creating an account number
            char accn[13], pass[9];
            generateAccno(customer, accn);
            strcpy(acc.acc_no, accn);

            //assigning a fixed password
            generatePass(customer, pass, 0);
            strcpy(acc.password, pass);

            //other account fields
            acc.activity = 1;
            acc.cust = cust_id;
            acc.balance = 0.00;

            fd = open("databases/accDB", O_WRONLY);
            lseek(fd, 0, SEEK_END);
            write(fd, &acc, sizeof(Account));
            if(rval) acc.cust = -1;
            write(nsd, &acc, sizeof(Account));
        }
    }

    //creating a joint account
    else if(task == 5)
    {
        int cid[2];
        jointAccount jacc;
        read(nsd, cid, sizeof(cid));
        int cust1 = cid[0], cust2 = cid[1];
        if((cust1 > admin->customers) || (cust2 > admin->customers))
        {
            jacc.cust1 = 0;
            write(nsd, &jacc, sizeof(Account));
        } //error;
        else
        {
            int rval = 0;
            //generating accno on basis of cust1 and password on basis of cust2
            Customer c1, c2;
            int fd = open("databases/customerDB", O_RDONLY);
            if(lseek(fd, (cust1 - 1)*sizeof(Customer), SEEK_SET) == -1) {rval = 1; return;}
            if(read(fd, &c1, sizeof(Customer)) == -1) {rval = 2; return;}
            if(lseek(fd, (cust2-1)*sizeof(Customer), SEEK_SET)) {rval = 3; return;}
            if(read(fd, &c2, sizeof(Customer)) == -1) {rval = 4; return;}
            close(fd);

            //generate accno
            char acc[13], pass[9];
            generateAccno(c1, acc);
            strcpy(jacc.acc_no, acc);

            //generate pass
            generatePass(c2, pass, 1);
            strcpy(jacc.password, pass);

            //add to database
            fd = open("databases/jointDB", O_WRONLY);
            lseek(fd, 0, SEEK_END);
            write(fd, &jacc, sizeof(jointAccount));
            if(rval) jacc.cust1 = -1;
            write(nsd, &jacc, sizeof(jointAccount));
        }

    }

    //acc details
    else if(task == 6)
    {
        int rval = -1;
        char acc_no[13];
        Account acc;
        accDetails ad;
        read(nsd, acc_no, sizeof(char[13]));
        int fd = open("databases/accDB", O_RDWR);
        while(read(fd, &acc, sizeof(Account)))
        {
            if(!strcmp(acc.acc_no, acc_no))
            {
                strcpy(ad.acc_no, acc.acc_no);
                ad.balance = acc.balance;
                ad.cust1 = acc.cust;
                strcpy(ad.password, acc.password);
                rval = 0; break;
            }
        }
        ad.type = rval;
        write(nsd, &ad, sizeof(accDetails));
    }

    //joint acc details
    else if(task == 7)
    {
        int rval = -1;
        char acc_no[13];
        jointAccount acc;
        accDetails ad;
        read(nsd, acc_no, sizeof(char[13]));
        int fd = open("databases/jointDB", O_RDWR);
        while(read(fd, &acc, sizeof(jointAccount)))
        {
            if(!strcmp(acc.acc_no, acc_no))
            {
                strcpy(ad.acc_no, acc.acc_no);
                ad.balance = acc.balance;
                ad.cust1 = acc.cust1;
                ad.cust2 = acc.cust2;
                strcpy(ad.password, acc.password);
                rval = 1; break;
            }
        }
        ad.type = rval;
        write(nsd, &ad, sizeof(accDetails));
    }

}


int main()
{
    struct sockaddr_in server, client;

    int sd = socket(AF_INET, SOCK_STREAM, 0); // TCP Protocol

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5555);

    bind(sd, (void *)&server, sizeof(server));
    listen(sd, 5);

    int sz = sizeof(client);
    int type = 0;
    int success = 1;
    while(1)
    {
        int nsd = accept(sd, (void *)&client, &sz);

        if(!fork())
        {
            close(sd);
            //type of login
            read(nsd, &type, sizeof(type));
            if(type == 1)
            {
                //make admin login
                Admin admin;
                //receive login creds
                read(nsd, &admin, sizeof(Admin));
                //verify login creds
                int st = adminVerify(&admin);
                //send the status
                write(nsd, &st, sizeof(int));
                //receive the task
                if(!st)
                {
                    int task;
                    read(nsd, &task, sizeof(int));
                    if(task) executeAdminQuery(nsd, task, &admin);
                }
            }
            
            else
            {
                loginCred login;
                //make user login
                read(nsd, &login, sizeof(loginCred));
                login.type = type-2;//0-normal, 1-joint
                //verify login
                int blocks;
                int st = accVerify(login, &blocks);
                //send the status
                write(nsd, &st, sizeof(int));
                //receive the task
                if(!st)
                {
                    int task;
                    read(nsd, &task, sizeof(int));
                    //execute  task and send the details required
                    if(task) executeAccQuery(nsd, (type*10) + task, blocks);
                }
            }
            
            exit(0);
        }
        else
        {
            close(nsd);
        }
    }
}
