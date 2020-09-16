// SPN线性分析
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace std;

//  K2,K4    (x5,x7,x8)(u6,u8,u14,u16)
//           (x13,x15)(u7,u15)
//备选链      (x9,x11)(u7,u15)
//  K1,K3    (x1,x2,x4)(u1,u5,u9,u13)
//            (x9,x10,x12)(u3,u7,u11,u15)

struct L24cnt  //用于记录 对于一对候选子密钥K2，K4 的 Count 从高到低 的
               // K2,K4，和Count值
{
    int  L2, L4;
    int  cnt = 0;
    bool operator<(const L24cnt& b)
    {
        return cnt > b.cnt;
    }
};

struct L13cnt  //用于记录 对于一对候选子密钥K1，K3 的 Count 从高到低 的
               // K1,K3，和Count值
{
    int  L1, L3;
    int  cnt = 0;
    bool operator<(const L13cnt& b)
    {
        return cnt > b.cnt;
    }
};

// cnt\cnt3:分别记录对应K2,K4两条链的Conunt
// cnt1\cnt2:K1,K3两条链的Count
int cnt[16][16], cnt1[16][16], cnt2[16][16], cnt3[16][16];

//记录8000对 明-密文对
int x_[8000], y_[8000];

//剪枝：用于找到正确密钥时跳出循环
int ppp = 0;

// sbn:S盒的逆
// sb:S盒
int sbn[16] = {0xE, 0x3, 0x4, 0x8, 0x1, 0xC, 0xA, 0xF, 0x7, 0xD, 0x9, 0x6, 0xB, 0x2, 0x0, 0x5};
int sb[16]  = {0xE, 0x4, 0xD, 0x1, 0x2, 0xF, 0xB, 0x8, 0x3, 0xA, 0x6, 0xC, 0x5, 0x9, 0x0, 0x7};

int sb1[65536] = {0};  // S盒打表
int pb1[65536] = {0};  // P盒打表

//函数声明
inline int read(void);         //快速读入
int        sbox(int u);        // S盒置换
int        pbox(int u);        // P盒置换
int        spn(int k, int p);  // spn加密

int main()
{
    int n;               //数据总组数
    int L1, L2, L3, L4;  // 4bits的候选子密钥
#ifndef ONLINE_JUDGE
    freopen("T2.in", "r", stdin);
#endif

    for (int i = 0; i < 65536; i++) sb1[i] = sbox(i);  // S盒打表
    for (int i = 0; i < 65536; i++) pb1[i] = pbox(i);  // P盒打表

    scanf("%d", &n);

    L13cnt L13[256];
    L24cnt L24[256];

    while (n--) {
        memset(cnt, 0, sizeof(cnt));
        memset(cnt3, 0, sizeof(cnt3));
        //求K2,K4
        int t = 0;  //计数器
        for (int i = 0; i < 8000; i++) {
            x_[i] = read(), y_[i] = read();
            int x = x_[i], y = y_[i];
            int y1 = y >> 12, y2 = (y & 0x0f00) >> 8,
                y3 = (y & 0x00f0) >> 4,  //密文处理
                y4 = (y & 0x000f);
            int x1 = (x & 0x8000) >> 15, x2 = (x & 0x4000) >> 14, x4 = (x & 0x1000) >> 12, x5 = (x & 0x0800) >> 11,
                x7 = (x & 0x0200) >> 9, x8 = (x & 0x0100) >> 8, x13 = (x & 0x0008) >> 3,
                x15 = (x & 0x0002) >> 1;  //明文处理
            int v_1 = 0, v_2 = 0, v_3 = 0, v_4 = 0, u_1 = 0, u_2 = 0, u_3 = 0, u_4 = 0;
            int u1 = 0, u5 = 0, u6 = 0, u7 = 0, u8 = 0, u9 = 0, u13 = 0, u14 = 0, u15 = 0, u16 = 0;
            int z = 0;

            for (L2 = 0; L2 < 16; L2++)
                for (L4 = 0; L4 < 16; L4++) {  //枚举密钥对

                    v_1 = L2 ^ y2;
                    v_2 = L4 ^ y4;
                    u_1 = sbn[v_1];
                    u_2 = sbn[v_2];
                    u6  = (u_1 & 4) >> 2;
                    u8  = u_1 & 1;
                    u14 = (u_2 & 4) >> 2;
                    u16 = u_2 & 1;
                    u7  = (u_1 & 2) >> 1;
                    u15 = (u_2 & 2) >> 1;

                    z = x5 ^ x7 ^ x8 ^ u6 ^ u8 ^ u14 ^ u16;
                    if (!z) cnt[L2][L4]++;
                    z = x13 ^ x15 ^ u7 ^ u15;
                    if (!z) cnt3[L2][L4]++;
                }
        }
        //对Count绝对值之和排序
        for (int i = 0; i < 16; i++)
            for (int j = 0; j < 16; j++) {
                cnt[i][j]  = abs(cnt[i][j] - 4000);
                cnt3[i][j] = abs(cnt3[i][j] - 4000);
            }
        t = 0;
        for (int i = 0; i < 16; i++)
            for (int j = 0; j < 16; j++) {
                L24[t].L2    = i;
                L24[t].L4    = j;
                L24[t++].cnt = cnt3[i][j] + cnt[i][j];
            }
        sort(L24, L24 + 256);

        //根据剩余两条链对K1,K3进行处理，
        ppp = 0;
        for (int N = 0; N < 256; N++) {  //根据Count从高到底枚举，假定K2,K4已知
            if (ppp) break;
            memset(cnt1, 0, sizeof(cnt1));
            memset(cnt2, 0, sizeof(cnt2));
            //求K1，K3的Count
            for (int i = 0; i < 8000; i++) {
                int x = x_[i], y = y_[i];
                int y1 = y >> 12, y2 = (y & 0x0f00) >> 8,
                    y3 = (y & 0x00f0) >> 4,  //密文处理
                    y4 = (y & 0x000f);
                int x1 = (x & 0x8000) >> 15, x2 = (x & 0x4000) >> 14, x4 = (x & 0x1000) >> 12, x5 = (x & 0x0800) >> 11,
                    x7 = (x & 0x0200) >> 9, x8 = (x & 0x0100) >> 8, x9 = (x & 0x0080) >> 7, x10 = (x & 0x0040) >> 6,
                    x12 = (x & 0x0010) >> 4, x13 = (x & 0x0008) >> 3,
                    x15 = (x & 0x0002) >> 1;  //明文处理
                int v_1 = 0, v_2 = 0, v_3 = 0, v_4 = 0, u_1 = 0, u_2 = 0, u_3 = 0, u_4 = 0;
                int u1 = 0, u3 = 0, u11 = 0, u5 = 0, u6 = 0, u7 = 0, u8 = 0, u9 = 0, u13 = 0, u14 = 0, u15 = 0, u16 = 0;
                int z = 0;

                v_2 = L24[N].L2 ^ y2;
                v_4 = L24[N].L4 ^ y4;
                u_2 = sbn[v_2];
                u_4 = sbn[v_4];
                u5  = u_2 >> 3;
                u13 = u_4 >> 3;
                u7  = (u_2 & 2) >> 1;
                u15 = (u_4 & 2) >> 1;

                for (L1 = 0; L1 < 16; L1++)
                    for (L3 = 0; L3 < 16; L3++) {
                        v_1 = L1 ^ y1;
                        v_3 = L3 ^ y3;
                        u_1 = sbn[v_1];
                        u_3 = sbn[v_3];

                        u1  = u_1 >> 3;
                        u9  = u_3 >> 3;
                        u3  = (u_1 & 2) >> 1;
                        u11 = (u_3 & 2) >> 1;

                        z = x1 ^ x2 ^ x4 ^ u1 ^ u5 ^ u9 ^ u13;
                        if (!z) cnt1[L1][L3]++;
                        z = x9 ^ x10 ^ x12 ^ u3 ^ u7 ^ u11 ^ u15;
                        if (!z) cnt2[L1][L3]++;
                    }
            }
            //对K1,K3 Count排序
            for (int i = 0; i < 16; i++)
                for (int j = 0; j < 16; j++) {
                    cnt1[i][j] = abs(cnt1[i][j] - 4000);
                    cnt2[i][j] = abs(cnt2[i][j] - 4000);
                }
            t = 0;
            for (int i = 0; i < 16; i++)
                for (int j = 0; j < 16; j++) {
                    L13[t].L1    = i;
                    L13[t].L3    = j;
                    L13[t++].cnt = cnt1[i][j] + cnt2[i][j];
                }
            sort(L13, L13 + 256);
            //验证过程，K1，K3从高到低枚举
            for (int j = 0; j < 2; j++) {
                if (ppp) break;
                int k = 0;
                k |= L13[j].L1;
                k <<= 4;
                k |= L24[N].L2;
                k <<= 4;
                k |= L13[j].L3;
                k <<= 4;
                k |= L24[N].L4;
                for (int l = 0; l < 65536; l++) {  //枚举剩余16位密钥
                    int l1 = l << 16;
                    int kk = k | l1;
                    if (spn(kk, x_[1]) == y_[1] && spn(kk, x_[2]) == y_[2]) {
                        printf("%08x\n", kk);
                        ppp = 1;
                        break;
                    }
                }
            }
        }
    }
    fclose(stdin);
    return 0;
}

inline int read()
{
    int  x  = 0;
    char ch = getchar();
    while (ch == ' ' || ch == '\n') ch = getchar();
    while (ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'f') {
        if (ch >= '0' && ch <= '9')
            x = (x << 4) + (ch - '0');
        else if (ch >= 'a' && ch <= 'f')
            x = (x << 4) + 10 + (ch - 'a');
        ch = getchar();
    }
    return x;
}

// S盒
int sbox(int u)
{
    int sb[16] = {0xE, 0x4, 0xD, 0x1, 0x2, 0xF, 0xB, 0x8, 0x3, 0xA, 0x6, 0xC, 0x5, 0x9, 0x0, 0x7};
    int ut = 0, u1 = 0;
    for (int i = 1; i <= 4; i++) {
        if (i == 1)
            u1 = u & 0xF000;
        else if (i == 2)
            u1 = u & 0x0F00;
        else if (i == 3)
            u1 = u & 0x00F0;
        else
            u1 = u & 0x000F;
        int ui = u1 >> ((4 - i) << 2);
        ui     = sb[ui];
        ut <<= 4;
        ut |= ui;
    }
    return ut;
}

// P盒
int pbox(int v)
{
    int vt = 0, vr = 0, v1 = 0;
    for (int i = 15; i >= 12; i--) {
        for (int j = 0; j < 4; j++) {
            v1 = v << (15 - i + (j << 2));
            vr = v1 >> 15;
            vt <<= 1;
            vr &= 0x1;
            vt |= vr;
        }
    }
    return vt;
}

int spn(int k, int p)
{
    int N = 4;
    int n;
    int w = 0, v = 0, kr = 0, u = 0, c = 0;
    w = p;
    for (int i = 0; i < 3; i++) {
        kr = (k >> (16 - (i << 2))) & 0xFFFF;
        u  = w ^ kr;
        v  = sb1[u];
        w  = pb1[v];
    }
    kr = (k >> 4) & 0xFFFF;
    u  = w ^ kr;
    v  = sb1[u];
    kr = k & 0xFFFF;
    u  = v ^ kr;
    return u;
}