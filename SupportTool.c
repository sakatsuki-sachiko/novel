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

    //　文字数、三点リーダーの数、ダッシュの数をカウントする変数
    int char_count = 0;      
    int santen_count = 0;  
    int c;
    int prev1 = 0, prev2 = 0;
    int dash_count = 0;  

    //スペースのエラーをカウントする変数
    int space_error_count = 0;
    int line_byte_count = 0;
    
    //現在の行数をカウントする
    int current_line = 1;


    // ファイルの最後まで1バイトずつ読み込む
    while ((c = fgetc(fp)) != EOF) 
    {
        // 改行が来たら、行のバイト数をリセットする
        if (c == '\n')
        {
            current_line++;
            line_byte_count = 0;
        }

        
        else if (c != '\r') 
        {
            line_byte_count++;
        }
            

        // 【1段目】行の1バイト目を読み込んだ瞬間の判定
        // 全角スペースもカギ括弧も、UTF-8では必ず「0xE3」から始まる。
        // つまり、1バイト目が 0xE3 以外（半角英数字や、他の全角文字）なら、その時点でエラー
        if (line_byte_count == 1 && c != 0xE3) 
        {
            printf("⚠️ [警告] %d行目: 行頭の字下げがありません。（半角文字または不正な行頭）\n", current_line);
            space_error_count++;
        }

        // 【2段目】行の3バイト目が揃った瞬間の判定
        // 1バイト目が 0xE3 だった場合のみ、3バイト目まで待って正確な文字を判定する。
        // 全角スペース、カギ括弧以外の時、検知


        else if (line_byte_count == 3 && prev2 == 0xE3) 
        {
            if (!((prev2 == 0xE3 && prev1 == 0x80 && c == 0x80) || 
                  (prev2 == 0xE3 && prev1 == 0x80 && c == 0x8C)))
            {
                printf("⚠️ [警告] %d行目: 行頭の字下げがありません。\n", current_line);
                space_error_count++;
            }
        }



        //16進数c0でマスク処理。後続バイトでなければ文字数カウントを増やす
        if ((c & 0xC0) != 0x80) 
        {
            char_count++;
        }

        // 「…」は１６進数で三バイト。それが全て正しければ「…」としてカウント
        if (prev2 == 0xE2 && prev1 == 0x80 && c == 0xA6) 
        {
            santen_count++;
        }

        //ダッシュの三バイトであればダッシュカウントを増やす
        if (prev2 == 0xE2 && prev1 == 0x80 && (c == 0x94 || c == 0x95)) 
        {
            dash_count++;
        }

        // 前二バイトを保存しつつループを再開
        prev2 = prev1;
        prev1 = c;
    }
    
    fclose(fp); 

    // 結果発表
    printf("========== 解析結果 ==========\n");
    printf("総文字数（改行等含まない）: %d 文字\n", char_count);
    printf("三点リーダー(…)の数 : %d 個\n", santen_count);
    printf("ダッシュ(—)の数     : %d 個\n", dash_count);
    printf("==============================\n");


    // …の奇数偶数の判定
    if (santen_count % 2 != 0) 
    {
        printf("⚠️警告：三点リーダーが奇数です！「……」のルールから外れています！\n");
    } 
    else 
    {
        printf("✅三点リーダーは偶数です。ルールは完璧に守られています！\n");
    }


    //ダッシュの奇数偶数の判定
    if (dash_count % 2 != 0)
    {
        printf("⚠️警告：ダッシュが奇数です！「——」のルールから外れています！\n");
    }

    else
    {
        printf("✅ダッシュは偶数です。\n");
    }

    if (space_error_count > 0) 
    {
        printf("⚠️警告：行頭の全角スペース（字下げ）忘れが %d 箇所あります！\n", space_error_count);
    } 

    else 
    {
        printf("✅行頭の字下げ（会話文含む）は完璧です。\n");
    }

    return 0;
}