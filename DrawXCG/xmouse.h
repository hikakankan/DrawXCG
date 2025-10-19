// IOCSコールをアセンブラで呼び出す
#include <cstdint>
void msinit()
{
    asm(
        "moveq  #0x70, %%d0\n\t" // _MS_INIT
        "trap   #15\n\t"
        :
        :                        // 入力なし
        : "d0"   // 使用するレジスタ
        );
}

void mscuron()
{
    asm(
        "moveq  #0x71, %%d0\n\t" // _MS_CURON
        "trap   #15\n\t"
        :
        :                        // 入力なし
        : "d0"   // 使用するレジスタ
        );
}

void msstat(int* px, int* py, int* pbl, int* pbr)
{
    uint32_t stat;

    asm(
        "moveq  #0x74, %%d0\n\t" // _MS_GETDT
        "trap   #15\n\t"
        "move.l %%d0, %0\n\t"
        : "=r"(stat)
        :                        // 入力なし
        : "d0"   // 使用するレジスタ
    );

    *px = (stat >> 24) & 0xff;
    *py = (stat >> 16) & 0xff;
    *pbl = (stat >> 8) & 0xff;
    *pbr = (stat >> 0) & 0xff;
}

void mspos(int* px, int* py)
{
    uint32_t pos;

    asm(
        "moveq  #0x75, %%d0\n\t" // _MS_CURGT
        "trap   #15\n\t"
        "move.l %%d0, %0\n\t"
        : "=r"(pos)
        :                        // 入力なし
        : "d0"   // 使用するレジスタ
    );

    *px = (pos >> 16) & 0xffff;
    *py = (pos >> 0) & 0xffff;
}

void keyinp(int* pkey)
{
    uint32_t key;

    asm(
        "moveq  #0, %%d0\n\t" // _B_KEYINP
        "trap   #15\n\t"
        "move.l %%d0, %0\n\t"
        : "=r"(key)
        :                        // 入力なし
        : "d0"   // 使用するレジスタ
    );

    *pkey = key;
}

void keysns(int* pkey)
{
    uint32_t key;

    asm(
        "moveq  #1, %%d0\n\t" // _B_KEYSNS
        "trap   #15\n\t"
        "move.l %%d0, %0\n\t"
        : "=r"(key)
        :                        // 入力なし
        : "d0"   // 使用するレジスタ
    );

    *pkey = key;
}

// ソフトウェアキーボード消去
void skeymod_off()
{
    asm(
        "moveq  #0x7d, %%d0\n\t" // _SKEY_MOD
        "moveq  #0, %%d1\n\t"
        "trap   #15\n\t"
        :
        :                        // 入力なし
        : "d0", "d1"   // 使用するレジスタ
        );
}

void mouse_init()
{
    msinit();
    mscuron();
    skeymod_off();
}
