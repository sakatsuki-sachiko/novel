#include <stdio.h>
#include <stdlib.h>

int main()
 {
    // ファイルを読み込む準備をする
    FILE *fp = fopen("test.txt", "r");
    if (fp == NULL)
    {
        printf("エラー: test.txt が見つからない\n");
        return 1;
    }

    //…は三バイトなのでカウントを三つ用意する
    int char_count = 0;      
    int santen_count = 0;  
    int c;
    int prev1 = 0, prev2 = 0;

    // ファイルの最後まで1バイトずつ読み込む
    while ((c = fgetc(fp)) != EOF) 
    {
        
        // 読み込んだものが先頭一バイト目であれば文字数カウントを増やす
        if ((c & 0xC0) != 0x80) 
        {
            char_count++;
        }

        // 「…」は１６進数で三バイト。それが全て正しければ「…」としてカウント
        if (prev2 == 0xE2 && prev1 == 0x80 && c == 0xA6) 
        {
            santen_count++;
        }

        // 前二バイトを保存しつつループを再開
        prev2 = prev1;
        prev1 = c;
    }
    
    fclose(fp); 

    // 結果発表
    printf("========== 解析結果 ==========\n");
    printf("総文字数（改行等含む）: %d 文字\n", char_count);
    printf("三点リーダー(…)の数 : %d 個\n", santen_count);
    printf("==============================\n");


    // 奇数偶数の判定
    if (santen_count % 2 != 0) 
    {
        printf("⚠️警告：三点リーダーが奇数です！「……」のルールから外れています！\n");
    } 
    else 
    {
        printf("✅三点リーダーは偶数です。ルールは完璧に守られています！\n");
    }

    return 0;
}