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

void printAdminMenu()
{
    printf("1. Add a customer\n");
    printf("2. Inactivate a normal account\n");
    printf("3. Inactivate a joint account\n");
    printf("4. Create an account\n");
    printf("5. Create a joint account\n");
    printf("6. View details of a normal account\n");
    printf("7. View details of a joint account\n");
    printf("8. Exit\n");
}

void printClientMenu()
{
    printf("1. Deposit\n");
    printf("2. Withdrawal\n");
    printf("3. Balance Enquiry\n");
    printf("4. change Password\n");
    printf("5. view details\n");
    printf("6. Exit\n");
}

void printAccDetails(accDetails details)
{
    if(!details.type)
    {
        printf("Account Details : \n");
        printf("Customer Id : %d\n", details.cust1);
    }
    else
    {
        printf("Account Details : \n");
        printf("Customer Id 1 : %d\n", details.cust1);
        printf("Customer ID 2 : %d\n", details.cust2);
    }
    printf("Account Details : %s\n", details.acc_no);
    printf("Password : %s\n", details.password);
    printf("Current Balance : %f\n", details.balance);
}

int main()
{
    struct sockaddr_in server;
    
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5555);

    //establish connection
    int i = connect(sd, (void *)&server, sizeof(server));
    if(i == -1)
    {
        printf("Connection error !\n");
        return 1;
    }
    
    printf("Login as : \n");
    printf("1. Admin\n");
    printf("2. Normal User\n");
    printf("3. Joint Accouny User\n");
    int type;
    scanf("%d", &type);

    //communicate the type to server
    write(sd, &type, sizeof(int));

    if(type == 1)
    {
        Admin admin;
        //fill up admin details
        printf("Admin Id : "); scanf("%s", admin.adminId);
        printf("Password : "); scanf("%s", admin.password);
        //send the details
        write(sd, &admin, sizeof(Admin));
        //receive the status
        int st;
        read(sd, &st, sizeof(int));
        if(st)
        {
            if(st == 1) printf("Incorrect Password\n");
            else if(st == 2) printf("Invalid Credentials\n");
            return st;
        }
        else
        {
            printf("Select one of the following : \n");
            printAdminMenu();
            int task; scanf("%d", &task);
            task %= 8;
            write(sd, &task, sizeof(int));
            if(!task)
            {
                printf("Exitted Successfully\n");
                return 0;
            }
            else
            {
                if(task == 1)
                {
                    Customer c;
                    int st;
                    printf("Customer name : "); scanf("%s", c.name);
                    printf("Phone number : "); scanf("%s", c.phoneNum);
                    write(sd, &c, sizeof(Customer));
                    //collect the status
                    read(sd, &st, sizeof(st));
                    if(!st) printf("Some Failure occured. Try Again!\n");
                    else printf("Customer with id = %d got added\n", st);
                }

                else if(task == 2)
                {
                    char accno[13];
                    int st;
                    printf("Account number : "); scanf("%s", accno);
                    write(sd, accno, sizeof(char[13]));
                    read(sd, &st, sizeof(int));
                    if(!st) printf("Inactivated successfully");
                    else printf("Account not found \n");
                }

                else if(task == 3)
                {
                    int st;
                    char accno[13];
                    printf("Account number : "); scanf("%s", accno);
                    write(sd, accno, sizeof(char[13]));
                    read(sd, &st, sizeof(int));
                    if(!st) printf("Inactivated successfully");
                    else printf("Account not found \n");
                }
                else if(task == 4)
                {
                    int c;
                    Account acc;
                    printf("Customer id : "); scanf("%d", &c);
                    write(sd, &c, sizeof(int));
                    read(sd, &acc, sizeof(int));
                    if(acc.cust == 0) printf("Customer not found\n");
                    else if(acc.cust == -1) printf("Some error Occured! Try again\n");
                    else
                    {
                        printf("Account for customer %d is successfully created\n", acc.cust);
                        printf("Account no : %s\n", acc.acc_no);
                        printf("Password : %s\n", acc.password);
                        printf("This is a temperory password. Customer must change it asap for secure transactions\n");
                    }
                }
                else if(task == 5)
                {
                    int c[2], c1, c2;
                    jointAccount jacc;
                    printf("Customer 1 id : "); scanf("%d", &c1);
                    printf("Customer 2 id : "); scanf("%d", &c2);
                    write(sd, c, sizeof(c));
                    read(sd, &jacc, sizeof(jointAccount));
                    if(jacc.cust1 == 0) printf("Customer not found\n");
                    else if(jacc.cust1 == -1) printf("Some error Occured! Try again\n");
                    else
                    {
                        printf("Joint Account for customers %d, %d is successfully created\n", jacc.cust1, jacc.cust2);
                        printf("Account no : %s", jacc.acc_no);
                        printf("Password : %s", jacc.password);
                        printf("This is a temperory password. Customer must change it asap for secure transactions\n");
                    }
                }

                else if((task == 6) || (task == 7))
                {
                    char acc[13];
                    printf("Enter Account number : "); scanf("%s", acc);
                    write(sd, acc, sizeof(char[13]));
                    accDetails ad;
                    read(sd, &ad, sizeof(accDetails));
                    if(ad.type == -1) printf("Account not found\n");
                    else printAccDetails(ad);
                }
            }
        }
    }

    else
    {
        loginCred login;
        //fillup login credentials
        printf("Customer Id : "); scanf("%d", &login.cust);
        printf("Account number : "); scanf("%s", login.acc_no);
        printf("Password : "); scanf("%s", login.password);
        //send to verify
        write(sd, &login, sizeof(loginCred));
        //receive the status
        int st;
        read(sd, &st, sizeof(int));
        if(st)
        {
            if(st == 1) printf("Your account is inactivated\n");
            else if(st == 2) printf("Invalid account number or password\n");
            else if(st == 3) printf("Customer not registered\n");
            return st;
        }
        else
        {
            printf("Select one of the following : \n");
            printClientMenu();
            int task;
            scanf("%d", &task);
            task %= 6;
            //send the task to server
            write(sd, &task, sizeof(int));
            if(!task)
            {
                printf("Exitted successfully !\n");
                return 0;
            }
            else
            {
                if(task == 1)
                {
                    //give the cash
                    float *cash;
                    printf("Cast to deposit : "); scanf("%f", cash);
                    write(sd, cash, sizeof(float));
                    int r;
                    read(sd, &r, sizeof(int));
                    if(!r) printf("Deposited Successfully !\n");
                    else printf("Error while depositing. Try again");
                }
                else if(task == 2)
                {
                    //enter cah to withdraw
                    float *cash;
                    read(sd, cash, sizeof(float));
                    printf("Cast to withdraw : "); scanf("%f", cash);
                    write(sd, cash, sizeof(float));
                    int r;
                    read(sd, &r, sizeof(int));
                    if(r == -1) printf("Insufficient funds \n");
                    else if(r == 0) printf("Successful Withdrawal\n");
                    else printf("Error while withdrawing. Try again\n");
                }
                else if(task == 3)
                {
                    //receive your balance
                    float balance;
                    read(sd, &balance, sizeof(float));
                    if(balance < 0) printf("Error! Try again\n");
                    else printf("Your current balance is : %f\n", balance);
                }
                else if(task == 4)
                {
                    //enter new password
                    char *newpass = (char *)malloc(sizeof(char[9]));
                    printf("New password : "); scanf("%s", newpass);
                    write(sd, newpass, sizeof(char[9]));
                    int r;
                    read(sd, &r, sizeof(int));
                    if(!r) printf("Successfully changed the password\n");
                    else printf("Error ! Try again\n");
                }
                else if(task == 5)
                {
                    accDetails details;
                    read(sd, &details, sizeof(accDetails));
                    printAccDetails(details);
                }
            }
        }
    }

}