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
    //ダッシュ用のカウントを準備。同じく三バイト
    int char_count = 0;      
    int santen_count = 0;  
    int c;
    int prev1 = 0, prev2 = 0;
    int dash_count = 0;  

    //スペースのエラーをカウントする変数
    int space_error_count = 0;
    int line_byte_count = 0;
    
    

    // ファイルの最後まで1バイトずつ読み込む
    while ((c = fgetc(fp)) != EOF) 
    {
        // 改行が来たら、行のバイト数をリセットする
        if (c == '\n')
        {
            line_byte_count = 0;
        }

        
        else if (c != '\r') 
        {
            line_byte_count++;
        }
            
            // 行頭の3バイトが揃った瞬間に判定
        if (line_byte_count == 3) 
        {
                // 「全角スペース(E3 80 80)」でも「カギ括弧(E3 80 8C)」でもない場合
            if (!((prev2 == 0xE3 && prev1 == 0x80 && c == 0x80) || 
                      (prev2 == 0xE3 && prev1 == 0x80 && c == 0x8C)))

            {
                space_error_count++;
            }
        
            // もし行頭の1文字目が半角（英語や数字）だった場合はその時点でエラー
            else if (line_byte_count == 1 && c < 0x80)
            {
                space_error_count++;
            }
        }


        //１６進数を足して、先頭ビットが１０でなければカウントを増やす
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
    printf("総文字数（改行等含む）: %d 文字\n", char_count);
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