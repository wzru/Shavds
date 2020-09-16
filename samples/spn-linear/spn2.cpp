#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace std;

typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t  u8;

const int T = 8000, MAXN = 8005, MAXM = 65536;
const int NR = 4, L = 4, M = 4, LM = 16;

const int S[LM]  = {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7};
const int P[LM]  = {1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16};
const int S1[LM] = {14, 3, 4, 8, 1, 12, 10, 15, 7, 13, 9, 6, 11, 2, 0, 5};
const int P1[LM] = {1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16};
u16       K[NR + 1];
u16       TS[MAXM], TP[MAXM];

struct Node
{
    u16  x, y;
    u32  cnt;
    bool operator<(const Node& rhs) const
    {
        return cnt > rhs.cnt;
    }
} L13[256], L24[256];

struct IO
{
#define MAXSIZE (1 << 16)
    char buf[MAXSIZE], *p1, *p2;
    char pbuf[MAXSIZE], *pp;
    IO() : p1(buf), p2(buf), pp(pbuf) {}
    ~IO()
    {
        fwrite(pbuf, 1, pp - pbuf, stdout);
    }
    void flush()
    {
        fwrite(pbuf, 1, pp - pbuf, stdout);
        pp = pbuf;
    }
    inline char gc()
    {
        if (p1 == p2) p2 = (p1 = buf) + fread(buf, 1, MAXSIZE, stdin);
        return (p1 == p2 ? ' ' : *p1++);
    }
    inline bool blank(char ch)
    {
        return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
    }
    template <class T> inline void read_uint(T& x)
    {
        x                = 0;
        register char ch = gc();
        for (; blank(ch); ch = gc())
            ;
        for (; !blank(ch); ch = gc()) x = x * 10 + ch - '0';
    }
    u32 hex2bin_char(char x)
    {
        if (isdigit(x)) return x - '0';
        if (isupper(x)) x = tolower(x);
        return x - 'a' + 10;
    }
    template <class T> inline void read_in_hex(T& x)
    {
        x                = 0;
        register char ch = gc();
        for (; blank(ch); ch = gc())
            ;
        for (; !blank(ch); ch = gc()) x = (x << 4) + hex2bin_char(ch);
    }
    inline void push(const char& c)
    {
        if (pp - pbuf == MAXSIZE) fwrite(pbuf, 1, MAXSIZE, stdout), pp = pbuf;
        *pp++ = c;
    }
    char bin2hex_char(u32 x)
    {
        return x >= 10 ? x - 10 + 'a' : x + '0';
    }
    template <class T> inline void write_in_hex(T x)
    {
        static T sta[30];
        u32      top = 0;
        do {
            sta[top++] = x & 15, x /= 16;
        } while (x);
        while (top != 4) sta[top++] = 0;
        while (top) push(bin2hex_char(sta[--top]));
    }
} io;

inline u16 permutation(u16 x, const int* P)
{
    u16 w = 0;
    for (int j = 0; j < LM; ++j) { w = (w << 1) + (x >> (LM - P[j]) & 1); }
    return w;
}

inline u16 substitution(u16 x, const int* S)
{
    u16 v = 0;
    for (int j = 0; j < M; ++j) { v |= S[(x >> (j * L)) & 15] << (j * L); }
    return v;
}

inline u16 spn_encrypt(u16 plain, u32 key, const int* S, const int* P, u16* K)
{
    u16 u, v, w = plain;
    // for (int i = 0; i <= NR; ++i) { K[i] = key >> (16 - (i << 2)); }
    for (int i = 0; i < NR - 1; ++i) {
        u = w ^ K[i];
        v = substitution(u, S);
        w = permutation(v, P);
    }
    u = w ^ K[NR - 1];
    v = substitution(u, S);
    return v ^ K[NR];
}

inline u16 spn_decrypt(u16 cipher, u32 key, const int* S1, const int* P1, u16* K)
{
    u16 u, v, w = substitution(cipher ^ K[NR], S1) ^ K[NR - 1];
    for (int i = NR - 2; i >= 0; --i) {
        v = permutation(w, P1);
        u = substitution(v, S1);
        w = u ^ K[i];
    }
    return w;
}

inline u8 spn_linear_analysis(u16* plains, u16* ciphers, int T, const int* S1)
{
    int cnt1[LM][LM] = {0}, cnt2[LM][LM] = {0}, cnt3[LM][LM] = {0}, cnt4[LM][LM] = {0};
    for (int i = 0; i < T; ++i) {
        u16 x = plains[i], y = ciphers[i];
        int y1 = y >> 12, y2 = (y & 0x0f00) >> 8,
            y3 = (y & 0x00f0) >> 4,  //密文处理
            y4 = (y & 0x000f);
        int x1 = (x & 0x8000) >> 15, x2 = (x & 0x4000) >> 14, x4 = (x & 0x1000) >> 12, x5 = (x & 0x0800) >> 11,
            x7 = (x & 0x0200) >> 9, x8 = (x & 0x0100) >> 8, x13 = (x & 0x0008) >> 3,
            x15 = (x & 0x0002) >> 1;  //明文处理
        int v_1 = 0, v_2 = 0, v_3 = 0, v_4 = 0, u_1 = 0, u_2 = 0, u_3 = 0, u_4 = 0;
        int u1 = 0, u5 = 0, u6 = 0, u7 = 0, u8 = 0, u9 = 0, u13 = 0, u14 = 0, u15 = 0, u16 = 0;
        int z = 0;
        for (int L2 = 0; L2 < 16; L2++)
            for (int L4 = 0; L4 < 16; L4++) {  //枚举密钥对
                v_1 = L2 ^ y2;
                v_2 = L4 ^ y4;
                u_1 = S1[v_1];
                u_2 = S1[v_2];
                u6  = (u_1 & 4) >> 2;
                u8  = u_1 & 1;
                u14 = (u_2 & 4) >> 2;
                u16 = u_2 & 1;
                u7  = (u_1 & 2) >> 1;
                u15 = (u_2 & 2) >> 1;
                z   = x5 ^ x7 ^ x8 ^ u6 ^ u8 ^ u14 ^ u16;
                if (!z) cnt2[L2][L4]++;
                z = x13 ^ x15 ^ u7 ^ u15;
                if (!z) cnt4[L2][L4]++;
            }
    }
    //对Count绝对值之和排序
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) {
            cnt2[i][j] = abs(cnt2[i][j] - T / 2);
            cnt4[i][j] = abs(cnt4[i][j] - T / 2);
        }
    int t = 0;
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) {
            L24[t].x     = i;
            L24[t].y     = j;
            L24[t++].cnt = cnt4[i][j] + cnt2[i][j];
        }
    sort(L24, L24 + 256);

    //根据剩余两条链对K1,K3进行处理，
    bool flag = 0;
    for (int N = 0; N < 256; N++) {  //根据Count从高到底枚举，假定K2,K4已知
        if (flag) break;
        memset(cnt1, 0, sizeof(cnt1));
        memset(cnt3, 0, sizeof(cnt3));
        //求K1，K3的Count
        for (int i = 0; i < T; i++) {
            int x = plains[i], y = ciphers[i];
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
            v_2   = L24[N].x ^ y2;
            v_4   = L24[N].y ^ y4;
            u_2   = S1[v_2];
            u_4   = S1[v_4];
            u5    = u_2 >> 3;
            u13   = u_4 >> 3;
            u7    = (u_2 & 2) >> 1;
            u15   = (u_4 & 2) >> 1;
            for (int L1 = 0; L1 < 16; L1++)
                for (int L3 = 0; L3 < 16; L3++) {
                    v_1 = L1 ^ y1;
                    v_3 = L3 ^ y3;
                    u_1 = S1[v_1];
                    u_3 = S1[v_3];

                    u1  = u_1 >> 3;
                    u9  = u_3 >> 3;
                    u3  = (u_1 & 2) >> 1;
                    u11 = (u_3 & 2) >> 1;

                    z = x1 ^ x2 ^ x4 ^ u1 ^ u5 ^ u9 ^ u13;
                    if (!z) ++cnt1[L1][L3];
                    z = x9 ^ x10 ^ x12 ^ u3 ^ u7 ^ u11 ^ u15;
                    if (!z) ++cnt3[L1][L3];
                }
        }
        //对K1,K3 Count排序
        for (int i = 0; i < 16; i++)
            for (int j = 0; j < 16; j++) {
                cnt1[i][j] = abs(cnt1[i][j] - T / 2);
                cnt3[i][j] = abs(cnt3[i][j] - T / 2);
            }
        int t = 0;
        for (int i = 0; i < 16; i++)
            for (int j = 0; j < 16; j++) {
                L13[t].x     = i;
                L13[t].y     = j;
                L13[t++].cnt = cnt1[i][j] + cnt3[i][j];
            }
        sort(L13, L13 + 256);
        //验证过程，K1，K3从高到低枚举
        for (int j = 0; j < 2; j++) {
            if (flag) break;
            int k = 0;
            k |= L13[j].x;
            k <<= 4;
            k |= L24[N].x;
            k <<= 4;
            k |= L13[j].y;
            k <<= 4;
            k |= L24[N].y;
            for (int l = 0; l < 65536; l++) {  //枚举剩余16位密钥
                int l1 = l << 16;
                int kk = k | l1;
                if (spn_encrypt(plains[1], kk, S, P, K) == ciphers[1] &&
                    spn_encrypt(plains[2], kk, S, P, K) == ciphers[2]) {
                    return kk;
                    flag = 1;
                    break;
                }
            }
        }
    }
}

inline u32 spn_brute_force(u16* plains, u16* ciphers, int T, u32 K5_68)
{
    for (u32 K5_7 = 0; K5_7 < LM; ++K5_7)
        for (u32 K5_12345 = 0; K5_12345 < LM * LM * LM * LM * LM; ++K5_12345) {
            u32 key = K5_68 + (K5_7 << 4) + (K5_12345 << 12);
            for (int i = 0; i <= NR; ++i) { K[i] = key >> (16 - (i << 2)); }
            bool flag = true;
            for (int i = 0; i < T; ++i) {
                if (spn_encrypt(plains[i], key, S, P, K) != ciphers[i]) {
                    flag = false;
                    break;
                }
            }
            if (flag) return key;
        }
    return 0;
}

int main()
{
#ifndef ONLINE_JUDGE
    freopen("T2.in", "r", stdin);
#endif
    u32 n;
    u16 plains[T], ciphers[T];
    for (int i = 0; i < MAXM; ++i) TS[i] = substitution(i, S), TP[i] = permutation(i, P);
    io.read_uint(n);
    while (n--) {
        for (int i = 0; i < T; ++i) io.read_in_hex(plains[i]), io.read_in_hex(ciphers[i]);
        printf("%08x\n", spn_linear_analysis(plains, ciphers, T, S1));
    }
    return 0;
}