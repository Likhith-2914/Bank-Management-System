typedef struct Account
{
    int cust;
    char acc_no[13];
    char password[9];
    float balance;
    int activity;
} Account;

typedef struct jointAccount
{
    int cust1, cust2;
    char acc_no[13];
    char password[9];
    float balance;
    int activity;
} jointAccount;

typedef struct Customer
{
    int id;
    char name[50];
    char phoneNum[11];
    int accs, jointaccs;
} Customer;

typedef struct Admin
{
    char adminId[6];
    char password[9];
    int customers;
} Admin;

typedef struct loginCred
{
    int type, cust;
    char acc_no[13];
    char password[9];
} loginCred;

typedef struct accDetails
{
    int type, cust1, cust2;
    char acc_no[13];
    char password[9];
    float balance;
} accDetails;

void debug();

int deposit(off_t offset, float money, int type);
int withdraw(off_t offset, float money, int type);
int balanceQuery(off_t offset, float *balance, int type);
int passwordChange(off_t offset, char *newPass, int type);
int viewDetails(off_t offset, accDetails *details, int type);

