#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


#include "header.h"
int main()
{
    /*Admin Credentials*/
    Admin admin;
    char id[6] = "32567";
    char password[9] = "Admin123";
    strcpy(admin.adminId, id);
    strcpy(admin.password, password);
    admin.customers = 4;

    int fd = open("databases/adminDB", O_CREAT|O_WRONLY, 0744);
    write(fd, &admin, sizeof(admin));
    close(fd);

    /*customer credentials*/
    Customer customers[4];
    char names[5][10] = {"cust1", "cust2", "cust3", "cust4"}; 
    char phones[5][11] = {"9999999999",
                          "8888888888",
                          "7777777777",
                          "6666666666"};
    for(int i = 0; i < 4; i++)
    {
        customers[i].id = i+1;
        strcpy(customers[i].name, names[i]);
        strcpy(customers[i].phoneNum, phones[i]);
        customers[i].accs = 1;
        customers[i].jointaccs = 1;
    }
    customers[0].accs = 2;
    customers[3].accs = 0;

    fd = open("databases/customerDB", O_CREAT|O_WRONLY, 0744);
    write(fd, customers, sizeof(customers));
    close(fd);


    /*acc details*/
    Account accounts[4];
    char acc_nums[4][13] = {"999999999901",
                            "888888888801",
                            "777777777701",
                            "999999999902"};
    char pass[4][9] = {"99999901", "88888801", "77777701", "99999902"};
    int custIds[4] = {1, 2, 3, 1};
    float bals[4] = {50000.00, 50000.00, 75000.00, 1000.00};
    int i = 3;
    for(int i = 0; i <=3; i++)
    {
        accounts[i].cust = custIds[i];
        strcpy(accounts[i].acc_no, acc_nums[i]);
        strcpy(accounts[i].password, pass[i]);
        accounts[i].balance = bals[i];
        accounts[i].activity = 1;
    }

    fd = open("databases/accDB", O_CREAT|O_WRONLY, 0744);
    write(fd, accounts, sizeof(accounts));
    close(fd);

    /*joint acc*/
    jointAccount jaccs[2];
    char jacc_nums[2][13] = {"888888888811", "666666666611"};
    char jpass[2][9] = {"77777711", "99999911"};
    int jcustIds[4] = {2, 3, 4, 1};
    float jbals[2] = {100000.00, 150000.00};
    for(int i = 0; i < 2; i++)
    {
        jaccs[i].cust1 = jcustIds[2*i];
        jaccs[i].cust2 = jcustIds[2*i + 1];
        strcpy(jaccs[i].acc_no, jacc_nums[i]);
        strcpy(jaccs[i].password, jpass[i]);
        jaccs[i].balance = jbals[i];
        jaccs[i].activity = 1;  
    }

    fd = open("databases/jointDB", O_CREAT|O_WRONLY, 0744);
    write(fd, jaccs, sizeof(jaccs));
    close(fd);
}

