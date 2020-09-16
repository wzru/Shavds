struct L24cnt  //用于记录 对于一对候选子密钥K2，K4 的 Count 从高到低 的
               // K2,K4，和Count值
{
    int  L2, L4;
    int  cnt;
    bool operator<(const L24cnt& b)
    {
        return cnt > b.cnt;
    }
};
int main()
{
    L24cnt tmp;
    tmp.L2  = 1;
    tmp.L4  = 2;
    tmp.cnt = 3;
    return 0;
}