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
    int dash_count = 0;  

    // 三点リーダーとダッシュのエラーの数をカウントする変数
    int santen_error_count = 0;
    int dash_error_count = 0;


    // カギ括弧の状態とエラーを管理する変数
    int is_bracket_open = 0;          // ０なら閉じている、１なら開いている
    int bracket_opened_line = 0;      //「 が開かれた行数を記憶する変数
    int bracket_error_count = 0;      //カギ括弧のエラーの数をカウントする変数


    // ファイルから一バイトつづ読み込む変数
    int c;

    // 五バイトまで監視するための変数（なぜかコロンが二つあったので消去）
    int prev1 = 0, prev2 = 0, prev3 = 0, prev4 = 0, prev5 = 0;
    
    //インデントのエラー、連続打ちをカウントする変数
    int space_error_count = 0;
    int typo_error_count = 0;

    //現在の行数をカウントする
    int current_line = 1;
    int line_byte_count = 0;


    // ファイルの最後まで1バイトずつ読み込む
    while ((c = fgetc(fp)) != EOF) 
    {
        // 改行が来たら、行のバイト数をリセットする
        if (c == '\n')
        {
            // その行の中で、三点リーダーが奇数（単独使用など）ならエラー出力
            if (santen_count % 2 != 0) 
            {
                printf("⚠️ [警告] %d行目: 三点リーダーが奇数です（「……」のルール違反）\n", current_line);
                santen_error_count++;
            }
            // その行の中で、ダッシュが奇数ならエラー出力
            if (dash_count % 2 != 0) 
            {
                printf("⚠️ [警告] %d行目: ダッシュが奇数です（「——」のルール違反）\n", current_line);
                dash_error_count++;
            }

            // 次の行の計測のために、記号のカウントをゼロに戻す（リセット）
            santen_count = 0;
            dash_count = 0;

            // 行数を増やし、バイト数をリセット
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


        // 「。。」の検知（E3 80 82 が2連続 = 6バイト）
        if (prev5 == 0xE3 && prev4 == 0x80 && prev3 == 0x82 && 
            prev2 == 0xE3 && prev1 == 0x80 && c == 0x82) 
        {
            printf("⚠️ [警告] %d行目: 「。。」の連続入力ミスを検知しました！\n", current_line);
            typo_error_count++;
        }

        // 「、、」の検知 (E3 80 81 が2連続)
        if (prev5 == 0xE3 && prev4 == 0x80 && prev3 == 0x81 && 
            prev2 == 0xE3 && prev1 == 0x80 && c   == 0x81) 
        {
            printf("⚠️ [警告] %d行目: 「、、」の連続入力ミスを検知しました！\n", current_line);
            typo_error_count++;
        }


        // 「（始めカギ括弧：E3 80 8C）の検知
        if (prev2 == 0xE3 && prev1 == 0x80 && c == 0x8C) 
        {
            if (is_bracket_open == 1)
            {
                // すでに開いているのに、また「が来た。つまり前の「が閉じられていない
                printf("⚠️ [警告] %d行目: 閉じカギ括弧「」」がありません。（%d行目で開かれたカギ括弧が未完了です）\n", current_line, bracket_opened_line);
                bracket_error_count++;
                // 新しい「の行数で記憶を上書きし、開いた状態は継続
                // これにより、複数回「「が続いても、最初の「だけがエラーとしてカウントされる
                bracket_opened_line = current_line;
            }
            else
            {
                // 正常に開いた
                is_bracket_open = 1;
                bracket_opened_line = current_line; // 開いた行数を記憶
            }
        }

        // 」（閉じカギ括弧：E3 80 8D）の検知
        if (prev2 == 0xE3 && prev1 == 0x80 && c == 0x8D) 
        {
            if (is_bracket_open == 0)
            {
                // 開いていないのに「」」が来た（余分な閉じ括弧）
                printf("⚠️ [警告] %d行目: 余分な閉じカギ括弧「」」があります。（対応する「がありません）\n", current_line);
                bracket_error_count++;
            }
            else
            {
                // 正常に閉じた
                is_bracket_open = 0;
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

        // 前五バイトへシフトし、五バイトまで保管する
        prev5 = prev4;
        prev4 = prev3;
        prev3 = prev2;
        prev2 = prev1;
        prev1 = c;
    }
    
        // ファイル末尾（最後の行）が改行されずに終わっていた場合の最終判定
    if (santen_count % 2 != 0) 
    {
        printf("⚠️ [警告] %d行目: 三点リーダーが奇数です（「……」のルール違反）\n", current_line);
        santen_error_count++;
    }
    if (dash_count % 2 != 0) 
    {
        printf("⚠️ [警告] %d行目: ダッシュが奇数です（「——」のルール違反）\n", current_line);
        dash_error_count++;
    }

    // ファイル末尾でカギ括弧が開きっぱなしになっていないかの最終判定
    if (is_bracket_open == 1)
    {
        printf("⚠️ [警告] %d行目: ファイルの最後まで閉じカギ括弧「」」がありません。（%d行目で開かれています）\n", current_line, bracket_opened_line);
        bracket_error_count++;
    }


    fclose(fp);

    // 結果発表
    printf("\n\n========== 解析結果 ==========\n");
    printf("総文字数（改行等含まない）: %d 文字\n", char_count);
    printf("三点リーダーのエラー数: %d\n", santen_error_count);
    printf("ダッシュのエラー数: %d\n", dash_error_count);
    printf("==============================\n\n");


    // …のルールの最終判定
    if (santen_error_count > 0) 
    {
        printf("⚠️警告：三点リーダーのルール違反が %d 箇所あります！\n", santen_error_count);
    } 

    else 
    {
        printf("✅三点リーダーはすべて偶数です。ルールは完璧に守られています。\n");
    }

    // 「。。」や「、、」の連続入力ミスの判定
    if (typo_error_count > 0)
    {
        printf("⚠️警告：「。。」や「、、」の連続入力ミスが %d 箇所あります！\n", typo_error_count);
    }

    else
    {
        printf("✅句読点の連続入力ミス（タイポ）はありません。\n");
    }

    // カギ括弧のエラーチェック
    if (bracket_error_count > 0)
    {
        printf("⚠️警告：カギ括弧の対応エラー（閉じ忘れ、余分な括弧）が %d 箇所あります！\n", bracket_error_count);
    }
    else
    {
        printf("✅カギ括弧の対応（「」のペア）は完璧です。\n");
    }


    // ダッシュのルールの最終判定
   if (dash_error_count > 0)
    {
        printf("⚠️警告：ダッシュのルール違反が %d 箇所あります！\n", dash_error_count);
    }

    else
    {
        printf("✅ダッシュはすべて偶数です。\n");
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