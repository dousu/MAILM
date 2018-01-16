
/*
 * KnowledgeBase.cpp
 *
 *  Created on: 2016/11/16
 *      Author: Hiroki Sudo
 */

#include "KnowledgeBase.h"

bool KnowledgeBase::LOGGING_FLAG = false;
int KnowledgeBase::ABSENT_LIMIT = 1;
uint32_t KnowledgeBase::CONTROLS = 0x00L;
int KnowledgeBase::buzz_length = 3;
int KnowledgeBase::RECURSIVE_LIMIT = 3;
// int KnowledgeBase::MEASURE_NO = 10000;
int KnowledgeBase::VARIABLE_NO = -10000;

KnowledgeBase::KnowledgeBase() {
    // DIC_BLD = false;
    cat_indexer.index_counter = 1;
    var_indexer.index_counter = 1;
    ind_indexer.index_counter = 1;
}

KnowledgeBase::~KnowledgeBase() {
}

KnowledgeBase&
    KnowledgeBase::operator=(
        const KnowledgeBase& dst
    ) {
    cat_indexer = dst.cat_indexer;
    var_indexer = dst.var_indexer;
    ind_indexer = dst.ind_indexer;

    box_buffer = dst.box_buffer;
    input_box = dst.input_box;
    sentenceDB = dst.sentenceDB;
    wordDB = dst.wordDB;

    DB_dic = dst.DB_dic;
    // DIC_BLD = dst.DIC_BLD;
    LOGGING_FLAG = dst.LOGGING_FLAG;
    ABSENT_LIMIT = dst.ABSENT_LIMIT;
    CONTROLS = dst.CONTROLS;
    buzz_length = dst.buzz_length;

    intention = dst.intention;

    return *this;
}

void
KnowledgeBase::clear(void) {
    cat_indexer.index_counter = 1;
    var_indexer.index_counter = 1;
    ind_indexer.index_counter = 1;

    box_buffer.clear();
    input_box.clear();
    sentenceDB.clear();
    wordDB.clear();

    DB_dic.clear();
    intention.clear();
}

void
KnowledgeBase::send_box(Rule& mail) {
    input_box.push_back(mail);
}

void
KnowledgeBase::send_box(
    std::vector<Rule>& mails
) {
    std::vector<Rule>::iterator it;
    it = mails.begin();
    while (it != mails.end()) {
        send_box(*it);
        // dic_add(DB_dic,*it);
        it = mails.erase(it);
    }
}

void
KnowledgeBase::send_db(Rule& mail) {
    if (mail.type == RULE_TYPE::NOUN || mail.type == RULE_TYPE::MEASURE) {
        wordDB.push_back(mail);
    } else if (
        mail.type == RULE_TYPE::SENTENCE
    ) {
        sentenceDB.push_back(mail);
    }else{
        std::cerr <<
            "DB ASSIGNMENT ERROR" <<
            std::endl;
        throw "FUNC SEND_DB ERROR";
    }
}

void
KnowledgeBase::send_db(
    std::vector<Rule>& mails
    ) {
    std::vector<Rule>::iterator it;
    it = mails.begin();
    while (it != mails.end()) {
        send_db(*it);
        it = mails.erase(it);
        // it++;
    }
}

void
KnowledgeBase::unique(RuleDBType& DB) {
    RuleDBType::iterator it, it2;
    std::map<int, bool> unified_cat;
    it = DB.begin();
    while (it != DB.end()) {
        if(
            (*it).external.size() == 1 &&
            (*it).external.front().is_cat() &&
            (*it).cat ==
                (*it).external.front().cat
        ){ //ループルールを削るA->A
            int tocat = (*it).cat;
            Rule base = (*it);
            std::vector<Element> e_v;
            Element int_el = (*it).internal.front();

            {
                for(auto& r : wordDB){
                    if(r!=base&&r.cat == tocat){
                        e_v.push_back(r.internal.front());
                    }
                }
                for(auto& r : sentenceDB){
                    if(r!=base&&r.cat == tocat){
                        e_v.push_back(r.internal.front());
                    }
                }
                for(auto& r : input_box){
                    if(r!=base&&r.cat == tocat){
                        e_v.push_back(r.internal.front());
                    }
                }
                for(auto& r : box_buffer){
                    if(r!=base&&r.cat == tocat){
                        e_v.push_back(r.internal.front());
                    }
                }
            }

            intention.unique_unify(int_el,e_v);
            it = DB.erase(it);
            continue;
        }
        {//A===B
            it2 = it + 1;
            while (it2 != DB.end()) {
                if (//(*it).type == (*it2).type &&
                    (*it).cat == (*it2).cat &&
                    (*it).internal.front()
                        ==
                    (*it2).internal.front() &&
                    (*it).external
                        ==
                    (*it2).external
                ) {
                    if(
                        (*it).type != (*it2).type &&
                        (
                            (*it).type == RULE_TYPE::MEASURE ||
                            (*it2).type == RULE_TYPE::MEASURE
                        )
                    ){
                        if((*it2).type == RULE_TYPE::MEASURE){
                            (*it).type = RULE_TYPE::MEASURE;
                        }
                        it2 = DB.erase(it2);
                    }else if(
                        (*it).type != (*it2).type &&
                        !(
                            (*it).type == RULE_TYPE::MEASURE ||
                            (*it2).type == RULE_TYPE::MEASURE
                        )
                    ){
                        continue;
                    }else{
                        it2 = DB.erase(it2);
                    }
                } else {
                    it2++;
                }
            }
        }
        it++;
    }
}

//基本的にinput_boxのunifyを想定（次の周回に入る前にループを削除）
//mergeやreplaceによってループが発生するが，その結果はinput_boxに集まる
//input_boxとbox_bufferを組み合わせてループが発生する場合はない．（chunkでループを発生するルールが作られないから）
void
KnowledgeBase::unify(RuleDBType& DB) {
    // std::cout << to_s() << std::endl;
//同じカテゴリは新規に生まれないため，
    // 代名詞的なルールは削る
    // ループを作成するルールが表れるとすべて統一する．
    RuleDBType::iterator it, it2;
    int target_cat, to_cat, now_cat, type;

    // int count=1;

    bool flag, modified;;
    it = DB.begin();
    while (it != DB.end()) {

        // std::cout << (*it).to_s() << " " << count << std::endl;

        if((*it).external.size() == 1){
            //代名ルールを削る

            // std::cout << (*it).to_s() << " " << count << std::endl;
            // count++;

            if((*it).external.front().is_cat() &&
                (*it).cat !=
                    (*it).external.front().cat &&
                ((*it).internal.front().sent_type == (*it).external.front().sent_type)
            ){

                // std::cout << (*it).to_s() << std::endl;

                flag = true;
                it2 = DB.begin();
                while(it2 != DB.end()){
                    if(
                        (*it).cat == (*it2).cat &&
                        (*it) != (*it2)
                    ){
                        flag = false;
                        break;
                    }
                    it2++;
                }
                if(flag){//このルールを消して下のルールを上のルールにする
                    bool it_sent = ((*it).type == RULE_TYPE::SENTENCE);
                    std::vector<Element> e_v;
                    Element int_el = (*it).internal.front();
                    
                    to_cat = (*it).cat;
                    target_cat = (*it).external.front().cat;

                    // std::cout << "TARGET_CAT: " << target_cat << std::endl << "TO_CAT: "  << to_cat << std::endl;

                    it = DB.erase(it);

                    // std::map<int, bool> target_list;

                    it2 = wordDB.begin();
                    while(
                        it2 != wordDB.end()
                    ){
                        if((*it2).cat == target_cat){
                            (*it2).cat = to_cat;
                            if(it_sent){
                                (*it2).type = RULE_TYPE::SENTENCE;
                            }
                            e_v.push_back((*it2).internal.front());
                        }
                        modified = false;
                        for(
                            int i = 1;
                            i< (*it2).internal.size();
                            i++
                        ){
                            if(
                                (*it2).internal[i].cat ==
                                    target_cat
                            ){
                                modified |= true;
                                (*it2).internal[i].cat =
                                    to_cat;
                                if(it_sent){
                                    (*it2).internal[i].sent_type = true;
                                }
                            }
                        }
                        if(modified){
                            for(int j = 0;
                                j < (*it2).external.size();
                                j++
                            ){
                                if(
                                    (*it2).external[j].is_cat() &&
                                    (*it2).external[j].cat
                                        ==
                                    target_cat
                                ){
                                    (*it2).external[j].cat =
                                        to_cat;
                                    if(it_sent){
                                        (*it2).external[j].sent_type = true;
                                    }
                                }
                            }
                        }
                        it2++;
                    }

                    it2 = sentenceDB.begin();
                    while(it2 != sentenceDB.end()){
                        if((*it2).cat == target_cat){
                            (*it2).cat = to_cat;
                            e_v.push_back((*it2).internal.front());
                        }
                        modified = false;
                        for(int i = 1;
                            i< (*it2).internal.size();
                            i++
                        ){
                            if(
                                (*it2).internal[i].cat
                                    ==
                                target_cat
                            ){
                                modified = true;
                                (*it2).internal[i].cat =
                                    to_cat;
                                if(it_sent){
                                    (*it2).internal[i].sent_type = true;
                                }
                            }
                        }
                        if(modified){
                            for(int j = 0;
                                j < (*it2).external.size();
                                j++
                            ){
                                if(
                                    (*it2).external[j].
                                        is_cat() &&
                                    (*it2).external[j].cat
                                        ==
                                    target_cat
                                ){
                                    (*it2).external[j].cat
                                        = to_cat;
                                    if(it_sent){
                                        (*it2).external[j].sent_type = true;
                                    }
                                }
                            }
                        }
                        it2++;
                    }
                    it2 = input_box.begin();
                    while(it2 != input_box.end()){
                        if((*it2).cat == target_cat){
                            (*it2).cat = to_cat;
                            if(it_sent &&
                                (
                                    (*it2).type == RULE_TYPE::NOUN ||
                                    (*it2).type == RULE_TYPE::MEASURE
                                )
                            ){
                                (*it2).type = RULE_TYPE::SENTENCE;
                            }
                            e_v.push_back((*it2).internal.front());
                        }
                        modified = false;
                        for(int i = 1;
                            i< (*it2).internal.size();
                            i++
                        ){
                            if(
                                (*it2).internal[i].cat
                                    ==
                                target_cat
                            ){
                                modified = true;
                                (*it2).internal[i].cat =
                                    to_cat;
                                if(it_sent){
                                    (*it2).internal[i].sent_type = true;
                                }
                            }
                        }
                        if(modified){
                            for(int j = 0;
                                j < (*it2).external.size();
                                j++
                            ){
                                if(
                                    (*it2).external[j].
                                        is_cat() &&
                                    (*it2).external[j].cat
                                        ==
                                    target_cat
                                ){
                                    (*it2).external[j].cat
                                        = to_cat;
                                    if(it_sent){
                                        (*it2).external[j].sent_type = true;
                                    }
                                }
                            }
                        }
                        it2++;
                    }
                    intention.unique_unify(int_el,e_v);
                    continue;
                }else{//ループ検知したら同値と判定

                    // std::cout << "TARGET_CAT: " << (*it).cat << std::endl << "NOW_CAT:"  << (*it).external.front().cat << std::endl;

                    std::vector<int> list;
                    list.push_back((*it).external.front().cat);
                    if((*it).external.front().sent_type)
                        flag = unify_loop_sub((*it).external.front().cat,(*it).cat,RULE_TYPE::SENTENCE,DB,list);
                    else
                        flag = unify_loop_sub((*it).external.front().cat,(*it).cat,RULE_TYPE::NOUN,DB,list);

                    // std::cout << "FIN. " << flag << std::endl;

                    if(flag){//A->B->A loop... merge A and B.

                        to_cat = (*it).cat;
                        it = DB.erase(it);
                        std::vector<int>::iterator cat_it;
                        for(cat_it= list.begin();cat_it!=list.end();cat_it++){
                            target_cat = *cat_it;

                            // std::cout << "[loop]TARGET_CAT: " << target_cat << std::endl << "TO_CAT: "  << to_cat << std::endl;

                            it2 = wordDB.begin();
                            while(
                                it2 != wordDB.end()
                            ){
                                if((*it2).cat == target_cat){
                                    (*it2).cat = to_cat;
                                    if(to_cat == 0){
                                        (*it2).type = RULE_TYPE::SENTENCE;
                                    }
                                }
                                modified = false;
                                for(
                                    int i = 1;
                                    i< (*it2).internal.size();
                                    i++
                                ){
                                    if(
                                        (*it2).internal[i].cat
                                            ==
                                        target_cat
                                    ){
                                        modified = true;
                                        (*it2).internal[i].cat
                                            =
                                            to_cat;
                                        if(to_cat == 0){
                                            (*it2).internal[i].sent_type = true;
                                        }
                                    }
                                }
                                if(modified){
                                    for(int j = 0;
                                        j < (*it2).external.size();
                                        j++
                                    ){
                                        if(
                                            (*it2).external[j].is_cat() &&
                                            (*it2).external[j].cat
                                                ==
                                            target_cat
                                        ){
                                            (*it2).external[j].cat =
                                                to_cat;
                                            if(to_cat == 0){
                                                (*it2).external[j].sent_type = true;
                                            }
                                        }
                                    }
                                }
                                it2++;
                            }

                            it2 = sentenceDB.begin();
                            while(it2 != sentenceDB.end()){
                                if((*it2).cat == target_cat){
                                    (*it2).cat = to_cat;
                                }
                                modified = false;
                                for(int i = 1;
                                    i< (*it2).internal.size();
                                    i++
                                ){
                                    if(
                                        (*it2).internal[i].cat
                                            ==
                                        target_cat
                                    ){
                                        modified = true;
                                        (*it2).internal[i].cat =
                                            to_cat;
                                        if(to_cat == 0){
                                            (*it2).internal[i].sent_type = true;
                                        }
                                    }
                                }
                                if(modified){
                                    for(int j = 0;
                                        j < (*it2).external.size();
                                        j++
                                    ){
                                        if(
                                            (*it2).external[j].
                                                is_cat() &&
                                            (*it2).external[j].cat
                                                ==
                                            target_cat
                                        ){
                                            (*it2).external[j].cat
                                                = to_cat;
                                            if(to_cat == 0){
                                                (*it2).external[j].sent_type = true;
                                            }
                                        }
                                    }
                                }
                                it2++;
                            }

                            it2 = input_box.begin();
                            while(
                                it2 != input_box.end()
                            ){
                                if((*it2).cat == target_cat){
                                    (*it2).cat = to_cat;
                                    if(to_cat == 0 &&
                                        (
                                            (*it2).type == RULE_TYPE::NOUN ||
                                            (*it2).type == RULE_TYPE::MEASURE
                                        )
                                    ){
                                        (*it2).type = RULE_TYPE::SENTENCE;
                                    }
                                }
                                modified = false;
                                for(
                                    int i = 1;
                                    i< (*it2).internal.size();
                                    i++
                                ){
                                    if(
                                        (*it2).internal[i].cat
                                            ==
                                        target_cat
                                    ){
                                        modified = true;
                                        (*it2).internal[i].cat
                                            =
                                            to_cat;
                                        if(to_cat == 0){
                                            (*it2).internal[i].sent_type = true;
                                        }
                                    }
                                }
                                if(modified){
                                    for(int j = 0;
                                        j < (*it2).external.size();
                                        j++
                                    ){
                                        if(
                                            (*it2).external[j].is_cat() &&
                                            (*it2).external[j].cat
                                                ==
                                            target_cat
                                        ){
                                            (*it2).external[j].cat =
                                                to_cat;
                                            if(to_cat == 0){
                                                (*it2).external[j].sent_type = true;
                                            }
                                        }
                                    }
                                }
                                it2++;
                            }
                        }
                        continue;
                    }

                }
            }
        }
        it++;
    }//while
    unique(wordDB);
    unique(sentenceDB);
    unique(input_box);
}

//input_boxかbox_bufferを既存知識に加えた時のループを検知
bool
KnowledgeBase::unify_loop_sub(int now_cat,int target_cat,int type,RuleDBType& DB,std::vector<int>& list){

    // std::cout << "LOOP" << std::endl;
    // std::cout << "TARGET_CAT: " << target_cat << std::endl << "NOW_CAT:"  << now_cat << std::endl;

    bool flag = false;
    RuleDBType::iterator it;
    it=DB.begin();
    while(it != DB.end()){
        if(
            (*it).external.size() == 1 &&
            (*it).external.front().is_cat() &&
            now_cat == (*it).cat &&
            std::find(list.begin(),list.end(),(*it).external.front().cat) == list.end()
        ){
            if((*it).external.front().cat == target_cat){
                flag = true;
                break;
            }
            std::vector<int> tmp;
            tmp = list;
            tmp.push_back((*it).external.front().cat);
            flag = true;
            if((*it).external.front().sent_type)
                flag &= unify_loop_sub((*it).external.front().cat,target_cat,RULE_TYPE::SENTENCE,DB,tmp);
            else
                flag &= unify_loop_sub((*it).external.front().cat,target_cat,RULE_TYPE::NOUN,DB,tmp);
            if(flag){
                list = tmp;
                break;
            }
        }
        it++;
    }
    if(flag)
        return flag;
    else{
        RuleDBType tmp_DB;
        if(type == RULE_TYPE::SENTENCE)
            tmp_DB = sentenceDB;
        else if(type == RULE_TYPE::NOUN)
            tmp_DB = wordDB;
        it=tmp_DB.begin();
        while(it != tmp_DB.end()){
            if(
                (*it).external.size() == 1 &&
                (*it).external.front().is_cat() &&
                now_cat == (*it).cat &&
                std::find(list.begin(),list.end(),(*it).external.front().cat) == list.end()
            ){
                if((*it).external.front().cat == target_cat){
                    flag = true;
                    break;
                }
                std::vector<int> tmp;
                tmp = list;
                tmp.push_back((*it).external.front().cat);
                flag = true;
                flag &= unify_loop_sub((*it).external.front().cat,target_cat,type,DB,tmp);
                if(flag){
                    list = tmp;
                    break;
                }
            }
            it++;
        }
        return flag;
    }
}

/*
 * ALGORITHM
 * Chunk
 * ・Chunkの結果はChunkを発生させる可能性がある
 * -新しい文規則が発生
 * ・Chunkの結果はMergeを発生させる可能性がある
 * --新しい単語規則が発生する
 * ・Chunkの結果はReplaceを発生させる可能性がある
 * --新しい単語規則が発生する
 * ・Sentenceカテゴリがwordに入る可能性はない
 *
 * Merge
 * ・Mergeの結果はChunkを発生させる可能性がある
 * --カテゴリが書き換えられた文規則が発生するため
 * ・Mergeの結果がMergeを発生させる可能性は無い
 * --単語や文規則の内部と外部が変わる事がないため
 * ・Mergeの結果がReplaceを発生させる可能性は無い
 * --単語や文の外部言語列が変化しないため
 * ・Sentenceカテゴリがwordに入る可能性はある
 *
 * Replace
 * ・Replaceの結果はChunkを発生させる可能性がある
 * --外部言語、内部言語が書き換えられた文や単語規則が発生する
 * ・Replaceの結果がMergeを発生させる可能性がある
 * --文規則の外部言語列が変化するため
 * ・Replaceの結果がReplaceを発生させる可能性がある
 * --文規則の外部言語列が変化するため
 * ・Sentenceカテゴリがwordに入る可能性はある
 */

bool
KnowledgeBase::consolidate(void) {
    bool flag = true, flag1= true, flag2= true, flag3= true, flag4;
    if(LOGGING_FLAG)
        LogBox::push_log("\n\n!!CONSOLIDATE!!");
    unify(input_box);
    while (flag) {
        flag = false;
        flag1 = flag2 = flag3 = true;

        // std::cout << to_s() << std::endl;

        while(flag1){
            // std::cout << "\n\n!!CHUNK STEP!!" << std::endl;
            if(LOGGING_FLAG)
                LogBox::push_log("\n\n!!CHUNK STEP!!");
            flag1 = chunk();
            flag |= flag1;
        }
        // std::cout << "\n****************test check7" << std::endl;

        //必要？
        // unique(box_buffer);

        // std::cout << "\n****************test check7 end" << std::endl;

        // std::cout << "\n****************test check8" << std::endl;

        while(flag2){
            // std::cout << "\n\n!!MERGE STEP!!" << std::endl;
            if(LOGGING_FLAG)
                LogBox::push_log("\n\n!!MERGE STEP!!");
            flag2 = merge();
            flag |= flag2;
        }

        // std::cout << "\n****************test check8 end" << std::endl;


        while(flag3){
            // std::cout << "\n\n!!REPLACE STEP!!" << std::endl;
            if(LOGGING_FLAG)
                LogBox::push_log("\n\n!!REPLACE STEP!!");
            flag3 = replace();
            flag |= flag3;
        }

        if(flag){
            send_box(box_buffer);
            unify(input_box);

            // std::cout << "FINISHED UNIFORMIZATION" << std::endl;

        }

        if(LOGGING_FLAG)
            LogBox::refresh_log();
    }

    std::cout << "\n\n!!CONSOLIDATE FIN!!" << std::endl;

    if (input_box.size() != 0) {
        std::cerr << "UNCHECKED SENTENCE REMAINNED" << std::endl;
        for (int i = 0; i < input_box.size(); i++)
            std::cerr << input_box[i].to_s() << std::endl;
        if (flag1)
            std::cerr << "F1" << std::endl;
        if (flag2)
            std::cerr << "F2" << std::endl;
        if (flag3)
            std::cerr << "F3" << std::endl;

        throw;
    }

    send_db(box_buffer);
    //box_bufferはuniqueしなくてよいのはここでまとめてuniqueしているから
    unique(sentenceDB);
    unique(wordDB);

    if(box_buffer.size() != 0){
        std::cerr << "Still remaining data(box_buffer): " << box_buffer.size() << std::endl;
        box_buffer.clear();
    }
    if(input_box.size() != 0){
        std::cerr << "Still remaining data(input_box): " << input_box.size() << std::endl;
        input_box.clear();
    }
    // DIC_BLD = false;
    build_word_index();

    return true;
}

bool
KnowledgeBase::chunk(void) {

    // std::cout << to_s() << std::endl;
    // std::cout << "\n****************test check3" << std::endl;

    RuleDBType::iterator it;
    Rule unchecked_sent;
    bool chunked;
    bool total_chunked = false;

    //BOXがからになるまで
    while (input_box.size() != 0) {
        //chunk ベースルール取り出し,取り出した要素を削除
        unchecked_sent = input_box.front();
        input_box.erase(input_box.begin());

        // if(unchecked_sent.type == RULE_TYPE::SENTENCE){
        //     //全てのチェック済み文規則とChunkテスト
        //     chunked = chunking_loop(unchecked_sent, sentenceDB);
        //     total_chunked |= chunked;
        //     if (chunked)
        //         break;
        // }else if(unchecked_sent.type == RULE_TYPE::NOUN || unchecked_sent.type == RULE_TYPE::MEASURE){
        //     //全てのチェック済み単語規則とChunkテスト
        //     chunked = chunking_loop(unchecked_sent, wordDB);
        //     total_chunked |= chunked;
        //     if (chunked)
        //         break;
        // }else{
        //     std::cerr << "TYPE ERROR @ CHUNK" << std::endl;
        //     throw "FUNC CHUNK ERROR";
        // }

        //全てのチェック済み文規則とChunkテスト
        chunked = chunking_loop(unchecked_sent, sentenceDB);
        total_chunked |= chunked;
        if (chunked)
            break;

        //全てのチェック済み単語規則とChunkテスト
        chunked = chunking_loop(unchecked_sent, wordDB);
        total_chunked |= chunked;
        if (chunked)
            break;

        //全ての未チェック規則とChunkテスト
        chunked = chunking_loop(unchecked_sent, box_buffer);
        total_chunked |= chunked;
        if (chunked)
            break;

        //もしChunkが発生してなかったなら
        box_buffer.push_back(unchecked_sent);
    } //未チェック規則が無くなれば終了

    // std::cout << "\n****************test check3 end" << std::endl;

    return total_chunked;
}

bool
KnowledgeBase::chunking_loop(
    Rule& unchecked_sent,
    RuleDBType& checked_rules
) {

    // std::cout << "\n****************test check4" << std::endl;

    RuleDBType buffer;
    RuleDBType::iterator it;
    Rule logging_rule, temp;
    bool chunked;

    chunked = false;
    it = checked_rules.begin();
    while (!chunked && it != checked_rules.end()) {
        //CHUNKが未発生 且つ 最後に行くまで
        //Chunk
        temp = *it;
        // if(unchecked_sent.type != temp.type){
        //     it++;
        //     continue;
        // }
        buffer.clear();
        buffer = chunking(unchecked_sent,temp);

        //結果解析
        if (buffer.size() == 0) { //未発生
            it++;
        } else { //発生

            //log取得
            if (LOGGING_FLAG) {
                logging_rule = *it;
                LogBox::push_log("\n-->>CHUNK:");
                LogBox::push_log("**FROM");
                LogBox::push_log(unchecked_sent.to_s());
                LogBox::push_log(logging_rule.to_s());
                LogBox::push_log("**TO");

                std::vector<Rule>::iterator it_log;
                it_log = buffer.begin();
                for (; it_log != buffer.end(); it_log++) {
                    LogBox::push_log((*it_log).to_s());
                }
                LogBox::push_log("<<--CHUNK");
            }
            send_box(buffer);

            //Chunkされた既存文規則を削除
            it = checked_rules.erase(it);

            chunked = true;
        }
    } //Chunkが発生したか、全既存規則とテスト終了

    // std::cout << "\n****************test check4 end" << std::endl;

    return chunked;
}

/*
 # chunk!
 # Algorithm
 # //1．内部言語の差異の個数が1
 # //1.1 内部言語の差異は、(非変数, 変数)、
 または(非変数,非変数)の組み合わせのいずれか
 # 2．外部言語の前方一致部分、後方一致部分が
 いずれか空でない
 # 3．外部言語で一致部分を取り除いた時、
 差異部分がともに空でない
 # 4．外部言語の差異部分は、両方ともすべて文字か、
 または片方が変数1文字でもう片方がすべて文字
 #
 # 1．上記条件が満たされるとき以下
 # //1．内部言語の差異部分が変数であるとき
 # 1．外部言語列の差異部分のどちらか一方が
 変数であるとき
 # 2．変数でない方の規則について、
 外部言語の差異部分を使い単語規則を生成する。
 この単語規則のカテゴリは、
 差異部分の変数のカテゴリを使う．
 #    新奇な意味を作り出し，カテゴリと新奇な意味，
 外部言語列の差異で新しい規則を作る．
 #    さらにそのカテゴリから新奇な意味変数から
 元の意味変数の規則を導出するルールを作る．
 # 3. 新たな文規則を生成する．内部言語の対応する
 意味変数と外部言語列の差異部分の変数を新奇な
 意味に書き換えたものとする．
 #
 # 1．外部言語列の差異部分がともに非変数であるとき
 # 2．それぞれを表現する新奇な意味、
外部言語の差異部分を用いて
 #    単語規則を二つ生成する。
 この単語規則のカテゴリは新奇なカテゴリを
 #    生成したものを使用し、
 また二つの単語規則ともに同じものを使用する。
 # 3．元の文規則に対して、
 差異部分を変数で置き換える。
 この変数のカテゴリは2で使用したカテゴリを用いる．
 さらに，全体の意味を新奇なものにして
 #    単語規則で使った意味を追加する
 #
 # Chunkが発生した場合その瞬間に該当する規則は削除される
 #
 */
std::vector<Rule>
KnowledgeBase::chunking(Rule& src, Rule& dst) {
    //0: unchunkable
    //1: chunk type 1
    //2: chunk type 2
    //検討要素type3

    //各チャンクタイプの動作
    //TYPE1: カテゴリの創出・意味変数の創出
    //TYPE2: カテゴリの引継・意味変数の引継
    //TYPE3: カテゴリの統一・意味変数の統一

    // std::cout << "\n****************test check5" << std::endl;
    // std::cout << "\n" << src.to_s() << "\n" << dst.to_s() << std::endl;

    enum CHUNK_TYPE {
        UNABLE, TYPE1, TYPE2
    };
    RuleDBType buf;
    buf.clear();
    bool m_check = false;

    //chunk のための分析
    //ルールカテゴリの一致検査
    if(src.cat != dst.cat){
        // if(src.type == RULE_TYPE::MEASURE && dst.type == RULE_TYPE::MEASURE){
        if(intention.chunk_equal(
            src.internal.front(),
            dst.internal.front()
            )
        ){
            m_check = true;
        }else{
            return buf;
        }
    }

    //外部言語検査
    //前方一致長の取得
    ExType::iterator src_it, dst_it;
    src_it = src.external.begin();
    dst_it = dst.external.begin();
    int fmatch_length = 0;
    while (
        src_it != src.external.end() &&
        dst_it != dst.external.end() &&
        *src_it == *dst_it
    ) {
        fmatch_length++;
        src_it++;
        dst_it++;
    }

    //後方一致長の取得
    ExType::reverse_iterator src_rit, dst_rit;
    src_rit = src.external.rbegin();
    dst_rit = dst.external.rbegin();
    int rmatch_length = 0;
    while (
        src_rit != src.external.rend() &&
        dst_rit != dst.external.rend() &&
        *src_rit == *dst_rit
    ) {
        rmatch_length++;
        src_rit++;
        dst_rit++;
    }

    //前・後方一致長が0でない
    if (fmatch_length + rmatch_length == 0)
        return buf;

    //一致長より外部言語列は長い（差異要素が必ず存在する）
    if (fmatch_length + rmatch_length >= src.external.size())
        return buf;
    if (fmatch_length + rmatch_length >= dst.external.size())
        return buf;

    //チャンクタイプ検査
    bool type3 = false;
    CHUNK_TYPE chunk_type = UNABLE;
    Rule base, targ;
    ExType noun1_ex, noun2_ex;
    std::copy(src.external.begin() + fmatch_length,
                src.external.end() - rmatch_length,
                std::back_inserter(noun1_ex));
    std::copy(dst.external.begin() + fmatch_length,
                dst.external.end() - rmatch_length,
                std::back_inserter(noun2_ex));
    if(
        noun1_ex.size() == 1 &&
        noun1_ex.front().is_cat()
    ){
        if(
            noun2_ex.size() == 1 &&
            noun2_ex.front().is_cat()
        ){
            type3 = true;//確実にtype3
            base = src;
            targ = dst;
            chunk_type = TYPE1;

            return buf;//type3はloopを作るので要考察

        } else {
            //基本はbase=src,targ=dst
            base = src;//TYPE2の可能性あり
            targ = dst;
            chunk_type = TYPE2;
        }
    } else {
        if(
            noun2_ex.size() == 1 &&
            (*noun2_ex.begin()).is_cat()
        ){
            //TYPE2のみbaseにdstが入る可能性があるのでnoun1_ex,noun2_exは使わない（base,targとのマッピングが作れない）
            base = dst;//TYPE2かも
            targ = src;
            chunk_type = TYPE2;
        } else {
            base = src;//TYPE1かも
            targ = dst;
            chunk_type = TYPE1;
        }
    }

    //typeごとに対象シンボル列に条件があればここでチェック
    //今回はなし
    if(chunk_type != TYPE1 && chunk_type != TYPE2){
        std::cerr << "UNKNOWN CHUNK TYPE ERROR" << std::endl;
        throw "UNKNOWN CHUNK TYPE ERROR";
    }
    // switch (chunk_type) {
    //     TYPE1:
    //     TYPE2:
    //     default:
    // }

    // std::cout << "\n****************test check5 end" << std::endl;


    //for translation in intention
    int in_pos,d_size,e_size;
    in_pos=d_size=e_size=0;
    //chunk条件チェック終了
    //CHUNK処理
    Rule sent, sent2;//生成される可能性のあるルール
    switch (chunk_type) {
        case TYPE1://type3も含む
        {
            // std::cout << "\n****************test check6 type1" << std::endl;
            // std::cout << "\n" << src.to_s() << "\n" << dst.to_s() << std::endl;
            Rule noun1, noun2;
            int new_cat_id;//汎化用category
            int new_var_id;//sentのvariable
            int new_sent_ind_id;//sent,sent2のid
            int new_ind_id_base;//noun1のid
            int new_ind_id_targ;//noun2のid

            //generate
            new_cat_id =
                cat_indexer.generate();
            new_var_id =
                var_indexer.generate();
            new_sent_ind_id =
                ind_indexer.generate();
            new_ind_id_base =
                ind_indexer.generate();
            new_ind_id_targ =
                ind_indexer.generate();

            //declaration for sentence
            Element new_cat, new_var, new_sent_ind;
            std::map<int,int> erase_list1;//noun1に紐づいているbaseから消すvarのobjリスト
            std::map<int,int>::iterator er_it;

            //noun
            Element ind1,ind2;
            if(!type3){//type1
                std::vector<Element> var_vector;
                InType new_noun_internal1, new_noun_internal2;
                new_noun_internal1.clear();
                new_noun_internal2.clear();
                ind1.set_ind(new_ind_id_base);
                new_noun_internal1.push_back(
                    ind1
                );
                ind2.set_ind(new_ind_id_targ);
                new_noun_internal2.push_back(
                    ind2
                );

                //src, base
                //汎化対象文字列の中のvariableを抜き出す
                var_vector.clear();
                for(int i = 0;i < noun1_ex.size();i++){
                    Element tmp_el, tmp_var;
                    tmp_el = noun1_ex[i];
                    if(tmp_el.is_cat()){
                        //sentence internalのerase list
                        er_it = erase_list1.find(tmp_el.obj);
                        if(er_it != erase_list1.end()){
                            (*er_it).second += 1;
                        }else{
                            erase_list1.insert(
                                std::map<int,
                                    int>::value_type(
                                    tmp_el.obj,
                                    1
                                )
                            );
                        }
                        //noun internalの中身作成
                        tmp_var.set_var(
                            tmp_el.obj,
                            tmp_el.cat
                        );
                        if(tmp_el.sent_type){
                            tmp_var.sent_type = true;
                        }
                        var_vector.push_back(tmp_var);

                        d_size++;
                    }
                }

                new_noun_internal1.insert(
                    new_noun_internal1.end(),
                    var_vector.begin(),
                    var_vector.end()
                );

                //dst, targ
                var_vector.clear();
                for(
                    int i = 0;
                    i < noun2_ex.size();
                    i++
                ){
                    Element tmp_el, tmp_var;
                    tmp_el = noun2_ex[i];
                    if(
                        tmp_el.is_cat()
                    ){
                        tmp_var.set_var(
                            tmp_el.obj,
                            tmp_el.cat
                        );
                        if(
                            tmp_el.sent_type
                        ){
                            tmp_var.sent_type = true;
                        }
                        var_vector.push_back(
                            tmp_var
                        );
                    }
                }
                new_noun_internal2.insert(
                    new_noun_internal2.end(),
                    var_vector.begin(),
                    var_vector.end()
                );

                //internal,external完成=>new_cat_idをいれてnoun1,2完成
                noun1.set_noun(
                    new_cat_id,
                    new_noun_internal1,
                    noun1_ex
                );
                noun2.set_noun(
                    new_cat_id,
                    new_noun_internal2,
                    noun2_ex
                );
                //nounの完成とerase listの完成
            }else{
                //type3であれば作られるnounはひとつしかvariableを持たない
                Element var1, var2;

                //index生成
                ind1.set_ind(new_ind_id_base);
                ind2.set_ind(new_ind_id_targ);

                //variable生成
                var1.set_var(
                    noun1_ex.front().obj,
                    noun1_ex.front().cat
                );//src, base
                var2.set_var(
                    noun2_ex.front().obj,
                    noun2_ex.front().cat
                );//dst, targ
                if(noun1_ex.front().sent_type){
                    var1.sent_type = true;
                }
                if(noun2_ex.front().sent_type){
                    var2.sent_type = true;
                }

                InType new_noun_internal1,
                    new_noun_internal2;
                new_noun_internal1.clear();
                new_noun_internal2.clear();
                //src, base
                new_noun_internal1.push_back(ind1);
                new_noun_internal1.push_back(var1);
                noun1.set_noun(
                    new_cat_id,
                    new_noun_internal1,
                    noun1_ex
                );
                //dst, targ
                new_noun_internal2.push_back(ind2);
                new_noun_internal2.push_back(var2);
                noun2.set_noun(
                    new_cat_id,
                    new_noun_internal2,
                    noun2_ex
                );

                d_size = 1;
            }

            //sentence
            //新規要素の生成
            new_cat.set_cat(
                new_var_id,
                new_cat_id
            );
            new_var.set_var(
                new_var_id,
                new_cat_id
            );
            new_sent_ind.set_ind(
                new_sent_ind_id
            );

            sent = base;
            sent.internal.front() = new_sent_ind;
            e_size = targ.internal.size() - (base.internal.size() - d_size);
            if(!type3){
                InType::iterator it_in;
                it_in = sent.internal.begin() + 1;
                while(it_in != sent.internal.end()){
                    er_it = erase_list1.find(
                        (*it_in).obj
                    );
                    if(er_it != erase_list1.end()){
                        it_in = sent.internal.erase(
                            it_in
                        );
                        if((*er_it).second == 1){
                            erase_list1.erase(er_it);
                        }else{
                            (*er_it).second -= 1;
                        }
                    }else{
                        it_in++;
                    }
                }
                // sent.internal.push_back(
                //     new_var
                // );
                if(erase_list1.size() > 0){
                    std::cerr << "Chunk Error(sent internal) size: " << erase_list1.size() << std::endl;
                    throw "CHUNK ERROR";
                }
            }else{
                InType::iterator var_search_it =
                    sent.internal.begin() + 1;
                for(
                    ;
                    var_search_it != sent.internal.end();
                    var_search_it++
                ){
                    if(
                        (*var_search_it).is_var() &&
                        (*var_search_it).obj == noun1_ex.front().obj
                    ){ //baseはsrcでnoun1_exのvarで変更対象のvar探す．
                        sent.internal.erase(
                            var_search_it
                        );
                        break;
                    }
                }
                // sent.internal.push_back(
                //     new_var
                // );
            }
            //sentence internal内の変数削除完了

            //sentence external前方作成
            //同時にinsertポジションも調べる
            sent.external.clear();
            ExType::iterator it;
            it = base.external.begin();
            while (
                it != base.external.begin() + fmatch_length
            ) {
                sent.external.push_back(*it);
                
                //indexの挿入ポイント=in_pos+1//この1はルールのindex分
                if((*it).is_cat()){
                    in_pos++;
                }

                it++;
            }
            //indexの挿入ポイント=in_pos+1//この1はルールのindex分
            in_pos++;//ここで最終調整

            //間に新しいcategoryをいれる
            sent.external.push_back(new_cat);

            //external後方作成
            //後ろから巡っていくので最前に追加していく
            ExType tmp_ex;
            tmp_ex.clear();
            std::vector<Element>::reverse_iterator rit;
            rit = base.external.rbegin();
            while (
                rit !=
                    base.external.rbegin() +
                        rmatch_length
            ) {
                tmp_ex.insert(
                    tmp_ex.begin(),
                    *rit
                );
                rit++;
            }
            sent.external.insert(
                sent.external.end(),
                tmp_ex.begin(),
                tmp_ex.end()
            );

            //insertポジションがわかったのでvariable挿入
            sent.internal.insert(
                sent.internal.begin()+in_pos,
                new_var
            );

            // if(
            //     !m_check &&
            //     sent.type != RULE_TYPE::MEASURE &&
            //     (src.type == RULE_TYPE::MEASURE || dst.type == RULE_TYPE::MEASURE)
            // ){
            //     sent.type = RULE_TYPE::MEASURE;
            // }
            if(m_check){
                sent2 = sent;
                sent2.cat = targ.cat;
                sent2.type = targ.type;
            }
            //DB revision
            // dic_erase(DB_dic,base);
            // dic_erase(DB_dic,targ);
            // dic_add(DB_dic,sent);
            // dic_add(DB_dic,noun1);
            // dic_add(DB_dic,noun2);
            // if(m_check){
            //     dic_add(DB_dic,sent2);
            // }
            //
            buf.push_back(sent);
            if(m_check){
                buf.push_back(sent2);
            }
            buf.push_back(noun1);
            buf.push_back(noun2);
            //
            
            intention.chunk(
                base.internal.front(),
                targ.internal.front(),
                sent.internal.front(),
                noun1.internal.front(),
                noun2.internal.front(),
                in_pos,
                d_size,
                e_size,
                chunk_type);
            break;
        }

        case TYPE2:
        {
            // std::cout << "\n****************test check6" << std::endl;
            // std::cout << "\n" << base.to_s() << "\n" << targ.to_s() << std::endl;

            int new_ind_id_targ;//新nounのため
            int new_sent_ind_id;//意味に変更が起きるかもしれないため

            //generate
            new_sent_ind_id =
                ind_indexer.generate();
            new_ind_id_targ =
                ind_indexer.generate();


            Rule noun;
            ExType noun_ex;//noun1_ex,noun2_exは使わない
            Element ind,emp,new_sent_ind;
            std::vector<Element>
                var_vector;
            //nounのindex完成
            ind.set_ind(
                new_ind_id_targ);
            //sentのindex完成
            new_sent_ind.set_ind(
                new_sent_ind_id);

            //var_vectorとnounのexternal完成
            for (
                int i = fmatch_length;
                i < targ.external.size() - rmatch_length;
                i++
            ) {
                noun_ex.push_back(
                    targ.external[i]
                );
                if(
                    targ.external[i].is_cat()
                ){
                    Element tmp_var;
                    tmp_var.set_var(
                        targ.external[i].obj,
                        targ.external[i].cat
                    );
                    if(targ.external[i].sent_type){
                        tmp_var.sent_type =true;
                    }
                    var_vector.push_back(
                        tmp_var
                    );
                    //nounの含む変数の数計算
                    d_size++;
                }
            }

            //noun internal完成
            InType noun_in;
            noun_in.push_back(ind);
            noun_in.insert(
                noun_in.end(),
                var_vector.begin(),
                var_vector.end()
            );

            //noun完成
            if(base.external[fmatch_length].sent_type){
                noun.set_sentence(
                    noun_in,
                    noun_ex
                );
            }else{
                noun.set_noun(
                    base.external[fmatch_length].cat,
                    noun_in,
                    noun_ex
                );
            }

            // if(
            //     !m_check &&
            //     base.type != RULE_TYPE::MEASURE &&
            //     (base.type == RULE_TYPE::MEASURE || targ.type == RULE_TYPE::MEASURE)
            // ){
            //     base.type = RULE_TYPE::MEASURE;
            // }

            sent = base;
            sent.internal.front() = new_sent_ind;
            if(m_check){
                sent2 = base;
                sent2.cat = targ.cat;
                sent2.internal.front() = new_sent_ind;
                sent2.type = targ.type;
            }
            buf.push_back(sent);
            if(m_check){
                buf.push_back(sent2);
            }
            buf.push_back(noun);
            //DB revision
            // dic_erase(DB_dic,base);
            // dic_erase(DB_dic,targ);
            // dic_add(DB_dic,sent);
            // dic_add(DB_dic,noun);
            // if(m_check){
            //     dic_add(DB_dic,sent2);
            // }

            //insertポジション計算
            for(int i =0;
                i< fmatch_length;
                i++
            ) {
                if(targ.external[i].is_cat()){
                    //variableの挿入位置の計算
                    in_pos++;
                }
            }
            //Rule自体のindex分
            in_pos++;

            // std::cout << "\n****************test check6 end " << m_check << std::endl;

            if(m_check){
                intention.chunk(
                    targ.internal.front(),
                    base.internal.front(),
                    sent.internal.front(),
                    noun.internal.front(),
                    emp,
                    in_pos,
                    d_size,
                    e_size,
                    chunk_type);
            }else{
                intention.chunk(
                    targ.internal.front(),
                    base.internal.front(),
                    sent.internal.front(),
                    noun.internal.front(),
                    emp,
                    in_pos,
                    d_size,
                    e_size,
                    chunk_type);    
            }
            // std::cout << "\n****************test check6 end end" << std::endl;
            break;
        }
        default:
            std::cerr << "CHUNK PROC ERROR" << std::endl;
            throw "CHUNK PROC ERROR";
    }

    return buf;
}

bool
KnowledgeBase::merge(void) {
    RuleDBType::iterator it;
    bool occurrence = false;
    bool merged;

    it = box_buffer.begin();
    while (it != box_buffer.end()) {

        // std::cout << "->MERGE CHECK" << std::endl;
        // std::cout << (*it).to_s() << std::endl;
        // std::cout << "<-MERGE CHECK" << std::endl;

        merged = merging(*it);
        occurrence |= merged;
        if (merged) {
            //box_bufferの中身が変更される可能性あり
            break;
        } else {
            it++;
        }
    }

    return occurrence;
}

bool
KnowledgeBase::merging(Rule& src) {

    // std::cout << "\n****************test check9" << std::endl;
    // std::cout << "\n" << src.to_s() << std::endl;

    //word_boxを変更しなければイテレータは使える
    RuleDBType::iterator it;
    RuleDBType buf, sub_buf;

    // Rule src_tmp;
    // src_tmp = src;

    //統一先のcategoryとindex
    int new_cat_id,new_ind_id;
    new_cat_id =
        cat_indexer.generate();
    new_ind_id =
        ind_indexer.generate();

    //word 適合するか検索
    //Merge対象として適合したら、直接カテゴリを書き換えて、
    //書き換えられたカテゴリを収拾
    //※mergeで単語を書き換えた結果はReplaceに影響しない
    std::map<int, bool>
        unified_sent_cat,
        unified_word_cat,
        unified_ind;

    //既存単語規則の被変更カテゴリの収拾
    collect_merge_cat(
        src,
        sentenceDB,
        unified_sent_cat,
        unified_word_cat,
        unified_ind
    );

    //既存単語規則の被変更カテゴリの収拾
    collect_merge_cat(
        src,
        wordDB,
        unified_sent_cat,
        unified_word_cat,
        unified_ind
    );

    //未検証単語規則の被変更カテゴリの収拾
    collect_merge_cat(
        src,
        box_buffer,
        unified_sent_cat,
        unified_word_cat,
        unified_ind
    );

    //被変更カテゴリの数が0ならMergeは起こらない
    if (
        unified_sent_cat.size() == 0 &&
        unified_word_cat.size() == 0 &&
        unified_ind.size() == 0
    ){

        // std::cout << "FALSE in MERGING" << std::endl;

        return false;
    }

    // std::cout << "->LOGGING" << std::endl;

    if (LOGGING_FLAG) {
        LogBox::push_log("\n-->>MERGE:");
        LogBox::push_log(src.to_s());
        LogBox::push_log("**ABOUT");

        // std::cout << "\n-->>MERGE:" << std::endl;
        // std::cout << src.to_s() << std::endl;
        // std::cout << "**ABOUT" << std::endl;

        std::map<int, bool>::iterator it;
        std::vector<std::string> cat_vec, ind_vec;
        cat_vec.push_back("SENTENCE->");
        it = unified_sent_cat.begin();
        for (
            ;
            it != unified_sent_cat.end();
            it++
        ) {
            cat_vec.push_back(
                    Prefices::CAT +
                    Prefices::CLN +
                    boost::lexical_cast<std::string>(
                        (*it).first)
                    );
        }
        cat_vec.push_back("NOUN->");
        it = unified_word_cat.begin();
        for (
            ;
            it != unified_word_cat.end();
            it++
        ) {
            cat_vec.push_back(
                Prefices::CAT +
                Prefices::CLN +
                boost::lexical_cast<std::string>(
                    (*it).first
                )
            );
        }
        std::string ss =
            boost::algorithm::join(
                cat_vec,
                ","
            )
        ;
        LogBox::push_log(ss);

        // std::cout << ss << std::endl;

        it = unified_ind.begin();
        for (
            ;
            it != unified_ind.end();
            it++
        ) {
            ind_vec.push_back(
                Prefices::IND +
                Prefices::CLN +
                boost::lexical_cast<std::string>(
                    (*it).first
                )
            );
        }
        ss =
            boost::algorithm::join(
                ind_vec,
                ","
                )
        ;
        LogBox::push_log(ss);

        // std::cout << ss << std::endl;
        // std::cout << "**TARGET" << std::endl;

        LogBox::push_log("**TARGET");
    }

    // std::cout << "\n****************test check9 end" << std::endl;

    // std::cout << "\n****************test check10" << std::endl;

    //新しいindex
    Element base_el;
    base_el.set_ind(new_ind_id);
    // base_el = src.internal.front();

    //srcのindexも新しくする
    unified_ind.insert(std::make_pair(src.internal.front().obj,true));

    //indの統一これだけならchunkはおきないのでinput_boxに送らないt直接書き換え
    if(LOGGING_FLAG)
        LogBox::push_log("IND sentenceDB " + src.to_s() );
    sub_buf = merge_ind_proc(
        base_el,
        sentenceDB,
        unified_ind
    );
    // buf.insert(
    //     buf.end(),
    //     sub_buf.begin(),
    //     sub_buf.end()
    // );

    if(LOGGING_FLAG)
        LogBox::push_log("IND wordDB " + src.to_s() );
    sub_buf = merge_ind_proc(
        base_el,
        wordDB,
        unified_ind
    );
    // buf.insert(
    //     buf.end(),
    //     sub_buf.begin(),
    //     sub_buf.end()
    // );

    if(LOGGING_FLAG)
        LogBox::push_log("IND input box " + src.to_s() );
    sub_buf = merge_ind_proc(
        base_el,
        input_box,
        unified_ind
    );
    // buf.insert(
    //     buf.end(),
    //     sub_buf.begin(),
    //     sub_buf.end()
    // );

    if(LOGGING_FLAG)
        LogBox::push_log("IND buffer " + src.to_s() );
    sub_buf = merge_ind_proc(
        base_el,
        box_buffer,
        unified_ind
    );
    // buf.insert(
    //     buf.end(),
    //     sub_buf.begin(),
    //     sub_buf.end()
    // );


    // box_buffer.insert(
    //     box_buffer.end(),
    //     buf.begin(),
    //     buf.end()
    // );

    // std::cout << "IND PROC. FIN." << std::endl;
    
    std::map<int, bool>::iterator int_it = unified_ind.begin();
    for(;int_it != unified_ind.end();int_it++){
        Element tmp_e;
        tmp_e.set_ind((*int_it).first);
        //DB revision
        // std::cout << "->DIC CHANGE" << std::endl;

        // dic_change_ind(DB_dic,tmp_e.obj,base_el.obj);
        
        // std::cout << "<-DIC CHANGE" << std::endl;
        //
        intention.merge(base_el,tmp_e,base_el);
    }

    // std::cout << "\n****************test check10 end" << std::endl;

    // std::cout << "\n****************test check11" << std::endl;

    buf.clear();

    //indの統一も考えて組みなおし
    //ELEMENT::sent_type == RULE_TYPE::SENTENCEへの変換
    int base_cat = new_cat_id;
    if(unified_sent_cat.size() != 0){
        //ひとつでもSentenceへの変換が混じっていたらすべてSentenceへの変換とする

        // std::cout << "SENTENCE VAR." << std::endl;

        unified_sent_cat.insert(
            unified_word_cat.begin(),
            unified_word_cat.end()
        );
        unified_sent_cat.insert(std::make_pair(src.cat,true));

        // std::map<int, bool>::iterator int_it = unified_sent_cat.begin();
        // for(;int_it != unified_sent_cat.end();int_it++){
        //     //DB revision
        //     // dic_change_cat(DB_dic,(*int_it).first,base_cat);
        //     //
        // }

        if(LOGGING_FLAG)
            LogBox::push_log("SENT sentenceDB " + src.to_s() );
        sub_buf = merge_sent_proc(
            base_cat,
            sentenceDB,
            unified_sent_cat
        );
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );

        if(LOGGING_FLAG)
            LogBox::push_log("SENT wordDB " + src.to_s() );
        sub_buf = merge_sent_proc(
            base_cat,
            wordDB,
            unified_sent_cat
        );
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );

        if(LOGGING_FLAG)
            LogBox::push_log("SENT input box " + src.to_s() );
        sub_buf = merge_sent_proc(
            base_cat,
            input_box,
            unified_sent_cat
        );
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );

        if(LOGGING_FLAG)
            LogBox::push_log("SENT buffer " + src.to_s() );
        sub_buf = merge_sent_proc(
            base_cat,
            box_buffer,
            unified_sent_cat
        );
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );

        input_box.insert(
            input_box.end(),
            buf.begin(),
            buf.end()
        );
    }else if(unified_word_cat.size() != 0){

        // std::cout << "WORD VER." << std::endl;

        // std::cout << "CAT sentenceDB " << std::endl;

        unified_word_cat.insert(std::make_pair(src.cat, true));

        // std::cout << "\n****************test check15" << std::endl;

        // std::map<int, bool>::iterator int_it = unified_word_cat.begin();
        // for(;int_it != unified_word_cat.end();int_it++){
        //     //DB revision
        //     dic_change_cat(DB_dic,(*int_it).first,base_cat);
        //     //
        // }

        // std::cout << "\n****************test check15 end" << std::endl;

        if(LOGGING_FLAG)
            LogBox::push_log("CAT sentenceDB " + src.to_s() );
        sub_buf = merge_word_proc(
            base_cat,
            sentenceDB,
            unified_word_cat
        );
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );


        // std::cout << "CAT wordDB " << std::endl;

        if(LOGGING_FLAG)
            LogBox::push_log("CAT wordDB " + src.to_s() );
        sub_buf = merge_word_proc(
            base_cat,
            wordDB,
            unified_word_cat
        );
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );

        // std::cout << "CAT input box " << std::endl;
        // std::cout << "CAT input box " + src.to_s() << std::endl;

        if(LOGGING_FLAG)
            LogBox::push_log("CAT input box " + src.to_s() );
        sub_buf = merge_word_proc(
            base_cat,
            input_box,
            unified_word_cat
        );
        // std::cout << "CAT input box FIN." << std::endl;
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );

        // std::cout << "CAT buffer " << std::endl;
        // std::cout << "CAT buffer " + src.to_s() << std::endl;


        //box_bufferへの変更は必ず最後にする．
        //box_bufferないでループしているため
        if(LOGGING_FLAG)
            LogBox::push_log("CAT buffer " + src.to_s() );
        // std::cout << "CAT buffer " + src.to_s() << " ";
        // std::cout << src.to_s() << std::endl;
        sub_buf = merge_word_proc(
            base_cat,
            box_buffer,
            unified_word_cat
        );
        // std::cout << "CAT_NUM: " << src.cat << std::endl;
        // std::cout << "CAT buffer " + src.to_s() << std::endl;
        buf.insert(
            buf.end(),
            sub_buf.begin(),
            sub_buf.end()
        );
        // std::cout << "CAT buffer " + src.to_s() << std::endl;

        input_box.insert(
            input_box.end(),
            buf.begin(),
            buf.end()
        );

        // std::cout << "WORD VER. FIN." << std::endl;

    }

    if (LOGGING_FLAG) {
        LogBox::push_log("<<--MERGE");
    }

    // std::cout << "MERGED" << std::endl;

    // std::cout << "\n****************test check11 end" << std::endl;

    //単語規則でカテゴリの書き換えが発生していることから
    //Mergeが発生している
    return true;
}

void
KnowledgeBase::collect_merge_cat(
    Rule& src,
    std::vector<Rule>& rule_db,
    std::map<int, bool>& unified_sent_cat,
    std::map<int, bool>& unified_word_cat,
    std::map<int, bool>& unified_ind
) {
    std::vector<Rule>::iterator it;
    it = rule_db.begin();
    if(
        src.type == RULE_TYPE::SENTENCE
    ){
        while (
            it != rule_db.end()
        ) {
            if (
                src.external == (*it).external && intention.merge_equal(src.internal.front(),(*it).internal.front())
            ){
                if(
                    src.cat != (*it).cat
                ){
                    unified_sent_cat.insert(
                        std::map<int, bool>::value_type(
                            (*it).cat,
                            true
                        )
                    );
                }
                if(
                    src.internal.front() !=
                        (*it).internal.front()
                ){
                    unified_ind.insert(
                        std::map<int, bool>::value_type(
                            (*it).internal.front().obj,
                            true
                        )
                    );
                }
            }
            it++;
        }
    }else{ //RULE_TYPE::NOUN
        while (it != rule_db.end()) {
            if (
                src.external == (*it).external && intention.merge_equal(src.internal.front(),(*it).internal.front())
            ){
                if( src.cat != (*it).cat ){
                    if(
                        (*it).type ==
                            RULE_TYPE::SENTENCE
                    ){
                        unified_sent_cat.insert(
                            std::map<int, bool>::value_type(
                                src.cat,
                                true
                            )
                        );
                        // //元ルールを直接書き換え
                        // src.cat = 0;
                        // src.type = RULE_TYPE::SENTENCE;
                    }else{ //RULE_TYPE::NOUN
                        unified_word_cat.insert(
                            std::map<int, bool>::value_type(
                                (*it).cat,
                                true
                            )
                        );
                    }
                }
                if(
                    src.internal.front() !=
                        (*it).internal.front()
                ){
                    unified_ind.insert(
                        std::map<int, bool>::value_type(
                            (*it).internal.front().obj,
                            true
                        )
                    );
                }
            }
            it++;
        }
    }
}

KnowledgeBase::RuleDBType
KnowledgeBase::merge_sent_proc(
    int base_word_cat,
    RuleDBType& DB,
    std::map<int, bool>& unified_cat
) {
    RuleDBType buf; //BOXへ送られる規則のバッファ
    RuleDBType::iterator it;
    Rule temp;
    bool modified, log_out;

    it = DB.begin();
    while (it != DB.end()) {
        modified = false;
        log_out = false;
        temp = *it;

        //Rule自体のcategoryチェック
        if (
            unified_cat.find(
                temp.cat
            ) !=
                unified_cat.end()
        ) {
            if (LOGGING_FLAG) {
                LogBox::push_log(
                    "MERGE-> " + temp.to_s()
                );
            }
            temp.cat = base_word_cat;
            temp.type =
                RULE_TYPE::SENTENCE;
            log_out = true;
        }

        //内部に被変更カテゴリの変数があるか調べ、
        // 合ったら書き換える
        for (
            int i = 0;
            i < temp.internal.size();
            i++
        ) {
            
            if (
                temp.internal[i].is_var() &&
                unified_cat.find(
                    temp.internal[i].cat
                ) !=
                    unified_cat.end()
            ) {
                if (
                    LOGGING_FLAG &&
                    !log_out
                ) {
                    LogBox::push_log(
                        "MERGE-> " + (*it).to_s()
                    );
                }
                //sent_procだからすべてをSentenceへ
                temp.internal[i].sent_type = true;
                temp.internal[i].cat = base_word_cat;
                modified = true;
                log_out=true;
            }

        }

        //内部に変更があった場合
        if (modified) {
            //外部のカテゴリ変数も書き換える
            for (
                int j = 0;
                j < temp.external.size();
                j++
            ) {
                if (//find unified cat
                    temp.external[j].is_cat() &&
                    unified_cat.find(
                        temp.external[j].cat
                    ) !=
                        unified_cat.end()
                ) {
                    temp.external[j].cat = base_word_cat;
                    temp.external[j].sent_type = true;
                }
            }
        }
        if (LOGGING_FLAG && log_out) {
            LogBox::push_log("MERGE<- " + temp.to_s());
        }

        if (log_out || modified) {
        //カテゴリの書き換えが発生している場合
            //バッファに書き換えられた規則を追加
            buf.push_back(temp);

            //書き換えられた元の規則をメインDBから削除
            it = DB.erase(it);
        } else { //カテゴリの書き換えが発生していない場合
            //検査対象の文規則イテレータを次に進める
            it++;
        }
    }

    return buf;
}

KnowledgeBase::RuleDBType
KnowledgeBase::merge_word_proc(
    int base_word_cat,
    RuleDBType& DB,
    std::map<int, bool>& unified_cat
) {

    // std::cout << "\n****************test check14" << std::endl;

    RuleDBType buf; //BOXへ送られる規則のバッファ
    RuleDBType::iterator it;
    Rule temp;
    bool modified, log_out;

    // std::cout << "\n****************test check14 end" << std::endl;

    // std::cout << "CAT_NUM: " << base_word_cat << " " << base_cat << std::endl;

    it = DB.begin();
    while (it != DB.end()) {
        modified = false;
        log_out = false;
        temp = *it;

        if (
            unified_cat.find(
                temp.cat
            ) !=
                unified_cat.end()
        ) {
            if (LOGGING_FLAG) {

                // std::cout << "A" << std::endl;

                LogBox::push_log(
                    "MERGE-> " + temp.to_s()
                );
            }
            temp.cat = base_word_cat;
            log_out = true;
        }

        //内部に被変更カテゴリの変数があるか調べ、
        // 合ったら書き換える
        for (
            int i = 0;
            i < temp.internal.size();
            i++
        ) {

            // std::cout << "\n****************test check12" << std::endl;
            
            if (
                temp.internal[i].is_var() &&
                unified_cat.find(
                    temp.internal[i].cat
                ) !=
                    unified_cat.end()
            ) {
                if (LOGGING_FLAG && !log_out) {

                // std::cout << "B" << std::endl;

                    LogBox::push_log(
                        "MERGE-> " + temp.to_s()
                    );
                }
                temp.internal[i].cat =
                    base_word_cat;
                modified = true;
                log_out = true;
            }

            // std::cout << "\n****************test check12 end" << std::endl;

        }

        //内部に変更があった場合
        if (modified) {
            //外部のカテゴリ変数も書き換える
            for (
                int j = 0;
                j < temp.external.size();
                j++
            ) {

                // std::cout << "\n****************test check13" << std::endl;
                
                if (//find unified cat
                    temp.external[j].is_cat() &&
                    unified_cat.find(
                        temp.external[j].cat
                    ) !=
                        unified_cat.end()
                ) {
                    temp.external[j].cat = base_word_cat;
                }

                // std::cout << "\n****************test check13 end" << std::endl;

            }
        }
        if (LOGGING_FLAG && log_out) {

            // std::cout << "C" << std::endl;

            LogBox::push_log(
                "MERGE<- " + temp.to_s()
            );
        }

        if (log_out || modified) { //カテゴリの書き換えが発生している場合
            //バッファに書き換えられた規則を追加
            buf.push_back(temp);

            //書き換えられた元の規則をメインDBから削除
            it = DB.erase(it);
        } else { //カテゴリの書き換えが発生していない場合
            //検査対象の文規則イテレータを次に進める
            it++;
        }
    }

    // std::cout << "CAT_NUM: " << base_word_cat << " " << base_cat << std::endl;

    return buf;
}

//ind_procは直接書き換えに変更
KnowledgeBase::RuleDBType
KnowledgeBase::merge_ind_proc(
    Element& base_word_el,
    RuleDBType& DB,
    std::map<int, bool>& unified_ind
) {
    RuleDBType buf; //BOXへ送られる規則のバッファ
    RuleDBType::iterator it;
    Rule temp;

    it = DB.begin();
    while (it != DB.end()) {
        temp = *it;
        Element base_el;
        base_el = base_word_el;

        if (
            unified_ind.find(
                temp.internal.front().obj
            ) !=
                unified_ind.end()
        ) {
            if (LOGGING_FLAG) {
                LogBox::push_log(
                    "MERGE-> " + temp.to_s()
                );
            }
            (*it).internal.front() =
                base_el;
            if (LOGGING_FLAG) {
                LogBox::push_log(
                    "MERGE<- " + (*it).to_s()
                );
            }
            // //バッファに書き換えられた規則を追加
            // buf.push_back(temp);

            //書き換えられた元の規則をメインDBから削除
            // it = DB.erase(it);
        } else { //カテゴリの書き換えが発生していない場合
            //検査対象の文規則イテレータを次に進める
            it++;
        }
    }//while

    return buf;
}

bool
KnowledgeBase::replace(void) {
    RuleDBType::iterator it;
    bool occurrence = false;
    bool flag1, flag2, flag3, flag4;
    Rule log_rule;

    it = box_buffer.begin();
    while (it != box_buffer.end()) {
        flag1 = flag2 = flag3 = flag4 = false;
        log_rule = (*it);
        if (LOGGING_FLAG) {
            LogBox::push_log("\n-->>REPLACE");
        }
        //iteratorと切り離してlog_ruleに入れてるから順番は関係ない
        //しかし，box_bufferに変更があった場合はbreak
        flag1 = replacing(log_rule, box_buffer);
        flag2 = replacing(log_rule, sentenceDB);
        flag3 = replacing(log_rule, wordDB);
        flag4 = replacing(log_rule, input_box);

        occurrence = occurrence |
            flag1 |
            flag2 |
            flag3 |
            flag4;
        if (
            flag1 ||
            flag2 ||
            flag3 ||
            flag4
        ) {
            if (LOGGING_FLAG) {
                LogBox::push_log("USED WORD:");
                LogBox::push_log(log_rule.to_s());
                LogBox::push_log("<<--REPLACE");
            }
            if(flag1){break;}//box_bufferに変更あり
        } else {
            if (LOGGING_FLAG)
                LogBox::pop_log();
        }
        it++;
    }

    it = sentenceDB.begin();
    while (it != sentenceDB.end()) {
        flag1 = flag2 = false;
        log_rule = (*it);

        if (LOGGING_FLAG) {
            LogBox::push_log("\n-->>REPLACE");
        }

        //変化があったものとだけ検査する
        flag1 = replacing(log_rule, box_buffer);
        flag2 = replacing(log_rule, input_box);

        occurrence = occurrence | flag1 | flag2;
        if (flag1 || flag2) {
            if (LOGGING_FLAG) {
                LogBox::push_log("USED WORD:");
                LogBox::push_log(log_rule.to_s());
                LogBox::push_log("<<--REPLACE");
            }
        } else {
            if (LOGGING_FLAG)
                LogBox::pop_log();
        }
        it++;
    }

    it = wordDB.begin();
    while (it != wordDB.end()) {
        flag1 = flag2 = false;
        log_rule = (*it);

        if (LOGGING_FLAG) {
            LogBox::push_log("\n-->>REPLACE");
        }

        //変化があったものとだけ検査する
        flag1 = replacing(log_rule, box_buffer);
        flag2 = replacing(log_rule, input_box);

        occurrence = occurrence | flag1 | flag2;
        if (flag1 || flag2) {
            if (LOGGING_FLAG) {
                LogBox::push_log("USED WORD:");
                LogBox::push_log(log_rule.to_s());
                LogBox::push_log("<<--REPLACE");
            }
        } else {
            if (LOGGING_FLAG)
                LogBox::pop_log();
        }
        it++;
    }

    return occurrence;
}

bool
KnowledgeBase::replacing(
    Rule word,
    RuleDBType& checking_sents
) {

    // std::cout << "\n****************test check22" << std::endl;

    RuleDBType::iterator it;
    RuleDBType buffer;

    int ematchb;
    bool ematched;
    bool replaced;
    bool total_match = false;
    Rule temp;

    //文規則をWordでReplaceできるか調べる
    it = checking_sents.begin();
    while (
        it != checking_sents.end()
    ) {
        // std::cout << "\n****************test check23" << std::endl;
        replaced = false;
        temp = *it;

        //replace不可条件
        if (
            (
                temp.type == word.type &&
                temp.cat == word.cat &&
                temp.external == word.external &&
                temp.internal.front() == word.internal.front()
            ) ||
            temp.external.size() <= word.external.size() //|| intention.replace_equal(temp.internal.front(),word.internal.front())
        ) {
            it++;
            continue; //while
        }else if(//要検討
            word.external.size() == 1 &&
            word.external.front().is_cat() &&
            temp.cat == word.external.front().cat
        ){
            it++;
            continue; //while
        }

        // std::cout << "\n****************test check23 end" << std::endl;

        // std::cout << "\n****************test check24" << std::endl;

        //external check
        int ex_limit;
        ex_limit = temp.external.size() - word.external.size();
        ematched = false;
        ematchb = 0;
        //左から検索
        for (int i = 0; !ematched && i <= ex_limit; i++) {
            if (std::equal(temp.external.begin() + i,
                    temp.external.begin() + i + word.external.size(),
                    word.external.begin())) {
                ematched = true;
                ematchb = i;
                break;
            }
        }
        //後処理
        if (!ematched) { //外部言語列に一致部分無し
            it++;
            continue; // while
        }

        // std::cout << "\n****************test check24 end" << std::endl;

        //文規則内部言語に一致箇所がある場合
        //置き換えるcategory，入れ替えるvariable，新ルールのindex
        Element catvar, var, new_ind;
        ExType::iterator eit;

        int new_var_id, new_ind_id;
        int b_pos, b_size;
        b_pos = b_size = 0;

        new_var_id = var_indexer.generate();
        new_ind_id = ind_indexer.generate();
        
        //index生成
        new_ind.set_ind(new_ind_id);

        //category, variable生成sentenceかどうかで分岐
        if(word.type == RULE_TYPE::SENTENCE){
            catvar.set_cat(new_var_id, word.cat);
            catvar.sent_type = true;
            var.set_var(new_var_id, word.cat);
            var.sent_type = true;
        }else{
            catvar.set_cat(new_var_id, word.cat);
            var.set_var(new_var_id, word.cat);
        }

        if (LOGGING_FLAG) {
            LogBox::push_log("REPLACE-> " + temp.to_s());
        }

        // std::cout << "\n****************test check25" << std::endl;

        //内部書き換えのリストを作成
        //word.externalで書き換えられる数を参照，
        // 書き換えられる部分のvar_idをカウントする
        std::map<int,int> rewrite_ls;
        std::map<int, int>::iterator r_it;
        for(int i = 0;i < word.external.size();i++){
            Element tmp_el;
            tmp_el = temp.external[ematchb + i];
            if(tmp_el.is_cat()){
                r_it = rewrite_ls.find(tmp_el.obj);
                if(r_it != rewrite_ls.end()){
                    (*r_it).second += 1;
                }else{
                    rewrite_ls.insert(
                        std::map<int, int>::value_type(
                            tmp_el.obj,
                            1
                        )
                    );
                }
                b_size++;
            }
        }

        // std::cout << "\n****************test check25 end" << std::endl;

        // std::cout << "\n****************test check26" << std::endl;

        //書き換え
        //内部
        InType::iterator it_in;
        it_in = temp.internal.begin() + 1;
        while(it_in != temp.internal.end()){
            r_it = rewrite_ls.find((*it_in).obj);
            if(r_it != rewrite_ls.end()){
                it_in = temp.internal.erase(it_in);
                if((*r_it).second == 1){
                    rewrite_ls.erase(r_it);
                }else{
                    (*r_it).second -= 1;
                }
            }else{
                it_in++;
            }
        }
        // temp.internal.push_back(var);//
        temp.internal.front() = new_ind;

        // std::cout << "\n****************test check26 end" << std::endl;

        //外部
        eit = temp.external.begin();
        eit = temp.external.erase(eit + ematchb,
                eit + ematchb + word.external.size());
                //消した部分のイテレータもらう

        eit=temp.external.insert(eit, catvar);
        
        // std::cout << "\n****************test check27" << std::endl;

        for(auto itt = temp.external.begin();itt != eit;itt++){
            if((*itt).is_cat()){
                b_pos++;
            }
        }
        b_pos++;

        // std::cout << "\n****************test check27 end" << std::endl;

        temp.internal.insert(temp.internal.begin()+b_pos,var);

        buffer.push_back(temp);

        // std::cout << "\n****************test check28" << std::endl;

        //DB revision
        // dic_erase(DB_dic,*it);
        // dic_add(DB_dic,temp);

        // std::cout << "\n****************test check28 end" << std::endl;

        // std::cout << "\n****************test check29" << std::endl;

        //
        intention.replace(
            (*it).internal.front(),
            word.internal.front(),
            temp.internal.front(),
            b_pos,
            b_size
        );

        // std::cout << "\n****************test check29 end" << std::endl;

        if (LOGGING_FLAG) {
            LogBox::push_log("REPLACE<- " + temp.to_s());
        }
        replaced = true;
        total_match |= replaced;

        //文DBイテレータ
        if (replaced) {
            it = checking_sents.erase(it);
        } else { //次の文規則へ
            it++;
        }
    } //while

    send_box(buffer);

    // std::cout << "\n****************test check22 end" << std::endl;

    return total_match;
}

//wordDBの検索を高速化するためwordDBの全情報をDB_dicへ．
void
KnowledgeBase::build_word_index(void) {

    typedef std::multimap<int, Rule> ItemType;
    RuleDBType::iterator it;
    RuleDBType allDB = sentenceDB;
    int cat_num;

    DB_dic.clear();

    allDB.insert(allDB.end(),wordDB.begin(),wordDB.end());

    it = allDB.begin();
    while (it != allDB.end()) {
        if((*it).type == RULE_TYPE::SENTENCE){
            cat_num = 0;// 0はsentenceの格納場所//C0は存在してはならない
        }else{
            cat_num = (*it).cat;
        }
        if (DB_dic.find(cat_num) != DB_dic.end()) {
            DB_dic[cat_num].insert(
                ItemType::value_type(
                    (*it).internal.front().obj,
                    *it
                )
            );
        } else {
            ItemType temp;
            temp.insert(
                ItemType::value_type(
                    (*it).internal.front().obj,
                    *it
                )
            );
            DB_dic.insert(
                std::map<int, ItemType>::value_type(
                    cat_num,
                    temp
                )
            );
        }

        it++;
    }
}

//internalにはsent_type入っていない
Rule
KnowledgeBase::fabricate(Rule& src1) {

    std::vector<PatternType> groundable_patterns;
    std::map<
        PATTERN_TYPE,
        std::vector<PatternType>
    > all_patterns;
    Rule src;
    int rand_index;
    src = src1;//一応コピー
    PatternType target_pattern;

    //pattern作成
    all_patterns =
        construct_grounding_patterns(src);

    if (LOGGING_FLAG) {
        LogBox::push_log(
            "\n-->>FABRICATE1:"
        );
    }

    if (
        all_patterns[COMPLETE].size() != 0
    ) {
        rand_index =
            MT19937::irand() %
                all_patterns[COMPLETE].size();

        target_pattern =
            (
                all_patterns[COMPLETE]
            )[rand_index];

        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;

            LogBox::push_log(
                "**CONSTRUCT"
            );
            LogBox::push_log(
                "***->>USED_RULES"
            );
            deb_it =
                (
                    all_patterns[COMPLETE]
                )[rand_index].begin();
            while (
                deb_it !=
                    (
                        all_patterns[COMPLETE]
                    )[rand_index].end()
            ) {
                LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            LogBox::push_log("***<<-USED_RULES");
        }

        ground_with_pattern(
            src,
            target_pattern
        );
    } else if (all_patterns[ABSOLUTE].size() != 0) {
        if (LOGGING_FLAG) {
            LogBox::push_log("**ABSOLUTE");
        }

        rand_index =
            MT19937::irand() %
            all_patterns[ABSOLUTE].size();

        src =
            (
                all_patterns[ABSOLUTE]
            )[rand_index].front();
    } else if (
        all_patterns[SEMICOMPLETE].size() != 0
    ) {
        rand_index =
            MT19937::irand() %
                all_patterns[SEMICOMPLETE].size();

        if (LOGGING_FLAG) {
            std::vector<Rule>::iterator deb_it;

            LogBox::push_log("**SEMI CONSTRUCT");
            LogBox::push_log("***->>USED_RULES");
            deb_it =
                (
                    all_patterns[SEMICOMPLETE]
                )[rand_index].begin();
            while (
                deb_it !=
                    (
                        all_patterns[SEMICOMPLETE]
                    )[rand_index].end()
            ) {
                LogBox::push_log((*deb_it).to_s());
                deb_it++;
            }
            LogBox::push_log("***<<-USED_RULES");
        }

        //completion
        PatternType use_pattern;
        PatternType::iterator it;

        use_pattern =
            (
                all_patterns[SEMICOMPLETE]
            )[rand_index];
        for (
            it = use_pattern.begin();
            it != use_pattern.end();
            it++
        ) {
            if (
                (*it).is_noun() &&
                (*it).external.size() == 0
            ) {
                ExType buzz;
                buzz = construct_buzz_word();
                (*it).external = buzz;

                if (
                    CONTROLS &
                    USE_ADDITION_OF_RANDOM_WORD
                ) {
                    Rule keep_word;
                    keep_word = *it;
                    send_db(keep_word);
                    dic_add(DB_dic, keep_word);
                    if (LOGGING_FLAG) {
                        LogBox::push_log(
                            "***->>KEPT THE COMP_RULE"
                        );
                        LogBox::push_log(
                            keep_word.to_s()
                        );
                        LogBox::push_log(
                            "***<<-KEPT THE COMP_RULE"
                        );
                    }
                    // DIC_BLD = false;
                    // DB_dic.clear();
                    // build_word_index();
                }
                if (LOGGING_FLAG) {
                    LogBox::push_log("***->>COMP_RULE");
                    LogBox::push_log((*it).to_s());
                    LogBox::push_log("***<<-COMP_RULE");
                }
            }
        }
        target_pattern = use_pattern;

        ground_with_pattern(
            src,
            target_pattern
        );
    } else{
        if (LOGGING_FLAG) {
            LogBox::push_log("**RANDOM");
        }

        ExType ex;
        ex = construct_buzz_word();
        src.external.swap(ex);

        if (
            CONTROLS &
            USE_ADDITION_OF_RANDOM_WORD
        ) {
            send_db(src);
            if (LOGGING_FLAG) {
                LogBox::push_log(
                    "**KEPT THE RULE"
                );
            }
        }
    }

    if (LOGGING_FLAG) {
        LogBox::push_log(
            "**OUTPUT"
        );
        LogBox::push_log(
            src.to_s()
        );
        LogBox::push_log(
            "<<--FABRICATE"
        );
    }

    return src;
}

KnowledgeBase::ExType
KnowledgeBase::construct_buzz_word(void) {
    //ランダム生成
    int length;
    int sym_id;
    std::vector<Element> ex;
    std::vector<Element> sym_buf;

    length =
        (
            MT19937::irand() %
                buzz_length
        ) + 1;
    for (int i = 0; i < length; i++) {
        Element sym_buf;
        sym_id =
            MT19937::irand() %
                Dictionary::symbol.size();
        sym_buf.set_sym(sym_id);
        ex.push_back(sym_buf);
    }

    if (ex.size() == 0) {
        std::cerr <<
            "Failed making random" <<
            std::endl;
        throw "make random external error";
    }
    return ex;
}


//from 3049 to 5310

void
KnowledgeBase::ground_with_pattern(
    Rule& src,
    PatternType& pattern
) {
    ExType::iterator sent_ex_it;
    PatternType::iterator pattern_it;
    Rule base_rule;
    PatternType tmp_ptn;

    // std::cout << "GWP CHECK" << std::endl;
    // std::cout << "given rules" << std::endl;
    // for(auto& r : pattern){
    //     std::cout << r.to_s() << std::endl;
    // }

    tmp_ptn = pattern;
    base_rule = tmp_ptn.front();
    tmp_ptn.erase(tmp_ptn.begin());
    sent_ex_it =
        base_rule.external.begin();
    src.external.clear();

    while (
        sent_ex_it != base_rule.external.end()
    ) {
        if (
            (*sent_ex_it).is_sym()
        ) {
            src.external.push_back(
                *sent_ex_it
            );
        } else if (
            (*sent_ex_it).is_cat()
        ) {
            // pattern_it =
            //     tmp_ptn.begin();
            // while (
            //     pattern_it != tmp_ptn.end()
            // ) {
            //     //objとcatが一致することで
            //     // 置き換え可能
            //     // (categoryのobjはあらかじめ
            //     // 対応するvariableのobjで
            //     // 書き換えられている)
            //     if (
            //         (*pattern_it).cat == (*sent_ex_it).cat &&
            //         (*pattern_it).internal.front().obj ==
            //             (*sent_ex_it).obj
            //     ) {

                    //pattern_itのinternal.front()のchに
                    // その文ルールが使う意味の数を書いとく
                    //categoryのchは受け入れる意味の数を書いとく

                    Rule tmp_rule;
                    ground_with_pattern_sub(
                        (*sent_ex_it).obj,
                        tmp_ptn,
                        tmp_rule
                    );

                    // std::cout << "GWP sub CHECK" << std::endl;
                    // tmp_rule.type = RULE_TYPE::NOUN;
                    // std::cout << tmp_rule.to_s() << std::endl;
                    
                    src.external.insert(
                        src.external.end(),
                        tmp_rule.external.begin(),
                        tmp_rule.external.end()
                    );
                //     break;
                // }
                // pattern_it++;
            // }
        } else {
            std::cerr <<
                "fabricate error" <<
                std::endl;
            throw "fabricate error";
        }
        sent_ex_it++;
    }
}

void
KnowledgeBase::ground_with_pattern_sub(
    int base_front,
    PatternType& pattern,
    Rule& src
) {
    ExType::iterator sent_ex_it;
    PatternType::iterator pattern_it;
    Rule base_rule;

    pattern_it = pattern.begin();
    for(
        ;
        pattern_it != pattern.end();
        pattern_it++
    ){
        if(
            (*pattern_it).internal.front().obj ==
                base_front
        ){//objをsrcの意味の順番にする
            base_rule = *pattern_it;
            pattern.erase(pattern_it);
            break;
        }
    }
    sent_ex_it = base_rule.external.begin();

    while (sent_ex_it != base_rule.external.end()) {
        if ((*sent_ex_it).is_sym()) {
            src.external.push_back(*sent_ex_it);
        } else if ((*sent_ex_it).is_cat()) {
            // pattern_it = pattern.begin();
            // while (pattern_it != pattern.end()) {
            //     //objとcatが一致することで置き換え
            //     // 可能(categoryのobjはあらかじめ
            //     // 対応するvariableのobjで書き換え
            //     // られている)
            //     if (
            //         (*pattern_it).cat == (*sent_ex_it).cat &&
            //         (*pattern_it).internal.front().obj ==
            //             (*sent_ex_it).obj
            //     ) {//事前の書き換えあり

            //         //pattern_itのinternal.front()のchに
            //         // その文ルールが使う意味の数を書いとく
            //         //categoryのchは受け入れる
            //         // 意味の数を書いとく
                    Rule tmp_rule;
                    ground_with_pattern_sub(
                        (*sent_ex_it).obj,
                        pattern,
                        tmp_rule
                    );
                    src.external.insert(
                        src.external.end(),
                        tmp_rule.external.begin(),
                        tmp_rule.external.end()
                    );
            //         break;
            //     }
            //     pattern_it++;
            // }
        } else {
            std::cerr << "fabricate error" << std::endl;
            throw "fabricate error";
        }
        sent_ex_it++;
    }
}

bool
KnowledgeBase::lemma_construct_grounding_patterns(
    std::vector<Element>::iterator it,
    std::vector<Element>::iterator it_end,
    int location,
    int category,
    int& ungrounded_variable_num,
    std::vector<KnowledgeBase::PatternType>& patterns,
    bool& is_applied,
    bool& is_absolute,
    bool& is_complete,
    bool& is_semicomplete,
    bool& sent_search
){
    typedef std::pair<
        std::multimap<int, Rule>::iterator,
        std::multimap<int, Rule>::iterator
    > DictionaryRange;
    RuleDBType::iterator
        sent_it,
        end_it;
    bool succeed = false, var_fl;

    //SentenceDBシーケンス用
    ExType::iterator sent_ex_it;
    bool succeed_sub;
    //グラウンドパターンの格納庫とそのイテレータ
    std::vector<PatternType>
        patternList,
        patterns_sub;
    std::vector<PatternType>::iterator
        patternList_it;
    //初期パターン
    PatternType pattern;
    // RuleDBType search_box;
    RuleDBType allDB=sentenceDB;
    allDB.insert(allDB.end(),wordDB.begin(),wordDB.end());

    if(it + 1 != it_end)
        {var_fl =true;}
    else
        {var_fl = false;}

    if(sent_search){
        sent_it = allDB.begin();
        end_it = allDB.end();
    }else{
        sent_it = wordDB.begin();
        end_it = wordDB.end();
    }
    while (sent_it != end_it) {
        Rule tmp_rule;

        if (
            (*sent_it).internal.front() != (*it) ||
            (var_fl && (*sent_it).internal.size() == 1) ||
            (!var_fl && (*sent_it).internal.size() != 1)
        ) {
            sent_it++;
            continue;
        }

        //組み上げようの書き換え
        int
            count=1,
            index;
        tmp_rule = (*sent_it);
        for (
            index = 1;
            (
                (it + count != it_end) &&
                index < tmp_rule.internal.size()
            );
            index++
        ) {
            if (
                tmp_rule.internal[index].is_var()
            ){
                sent_ex_it =
                    tmp_rule.external.begin();
                for(
                    ;
                    sent_ex_it != tmp_rule.external.end();
                    sent_ex_it++
                ){
                    if(
                        (*sent_ex_it).is_cat() &&
                        tmp_rule.internal[index].obj ==
                            (*sent_ex_it).obj
                    ){
                        (*sent_ex_it).obj =
                            location + count + VARIABLE_NO;
                        tmp_rule.internal[index].obj =
                            location + count + VARIABLE_NO;
                    }
                }
            }
            count += (*(it + count)).ch.front();
        }
        if(
            index != tmp_rule.internal.size() ||
            (it + count) != it_end
        ){
            sent_it++;
            continue;
        }

        tmp_rule.internal.front().obj = location+VARIABLE_NO;

        //グラウンドパターンの格納庫とそのイテレータ
        patternList.clear();

        //初期パターン
        pattern.clear();

        std::cout << "lemma_construct_grounding_patterns " << tmp_rule.to_s() << std::endl;

        //始めに検索対象文規則をパターン格納庫に入れる
        pattern.push_back(tmp_rule);
        //書き換えてからパターンに詰める
        patternList.push_back(pattern);

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;
        int in_idx, grd_idx;

        is_applied &= true;
        for (
            in_idx = grd_idx = 1;
            (
                is_applied &&
                (it + in_idx) != it_end
            ); //sent_itとsrcの0番目(front)は必ず一致
                in_idx += (*(it+in_idx)).ch.front()
        ) {
            grnd_elm =
                (*sent_it).internal[grd_idx];
                //検査するインターナル要素
            mean_elm =
                *(it + in_idx);
                //基準のインターナル要素
            if (grnd_elm == mean_elm) {
                //単語がそのまま一致する場合
                is_absolute &= true;
                continue;
            } else if(
                mean_elm.ch.front() > 1 &&
                grnd_elm.is_var()
            ){
                patterns_sub.clear();
                succeed_sub =
                    lemma_construct_grounding_patterns(
                        it+in_idx,
                        it+in_idx+(*(it+in_idx)).ch.front(),
                        location + in_idx,
                        grnd_elm.cat,
                        ungrounded_variable_num,
                        patterns_sub,
                        is_applied,
                        is_absolute,
                        is_complete,
                        is_semicomplete,
                        grnd_elm.sent_type
                    )
                ;
                if(succeed_sub){
                    std::vector<PatternType>
                        patternList_buffer;
                    std::vector<PatternType>::iterator
                        sub_patterns_it;
                    //patternsは複数のpattern
                        // なので直積を作る必要がある．
                    patternList_it =
                        patternList.begin();
                    while (
                        patternList_it !=
                            patternList.end()
                    ) {
                        sub_patterns_it =
                            patterns_sub.begin();
                        while (
                            sub_patterns_it !=
                                patterns_sub.end()
                        ){
                            PatternType
                                sub_pattern;
                            //すでに作られてる
                                // 単語規則の組をコピー
                            sub_pattern =
                                *patternList_it;

                            //そこへ新しく
                                // 単語規則を追加
                            sub_pattern.insert(
                                sub_pattern.end(),
                                (*sub_patterns_it).begin(),
                                (*sub_patterns_it).end()
                            );

                            //新しく単語規則が
                            // 追加された単語規則組を新しく保存
                            patternList_buffer.push_back(
                                sub_pattern
                            );

                            sub_patterns_it++;
                        }
                        patternList_it++;
                    }
                    //変更された単語規則組の列で元の単語規則組の列を置き換える
                    patternList.swap(
                        patternList_buffer
                    );

                    is_complete &= true;
                    is_absolute &= false;
                }else{
                    is_applied &= false;
                }
            }else if(
                grnd_elm.is_var()
            ){
                std::multimap<int, Rule> app_sent_map;
                if(grnd_elm.sent_type){
                    app_sent_map.insert(DB_dic[0].begin(),DB_dic[0].end());//sentenceはすべてここにいれてある
                }
                
                if(//変数の場合で、グラウンド可能な場合//Sentenceの検索が抜けてる
                    (DB_dic.find(grnd_elm.cat) != DB_dic.end() && //変数のカテゴリが辞書に有り
                        DB_dic[grnd_elm.cat].find(mean_elm.obj) != DB_dic[grnd_elm.cat].end() ) //辞書の指定カテゴリに単語がある
                    ||(grnd_elm.sent_type && app_sent_map.find(mean_elm.obj) != app_sent_map.end() ) //sentenceの方に適用可能ルールがある
                ){
                    DictionaryRange item_range;
                    std::vector<PatternType>
                        patternList_buffer;

                    //変数に適用可能単語規則集合取得
                    if(!grnd_elm.sent_type){
                        item_range =
                            DB_dic[grnd_elm.cat].equal_range(mean_elm.obj);
                    }else{
                        app_sent_map.insert(DB_dic[grnd_elm.cat].begin(),DB_dic[grnd_elm.cat].end());
                        item_range =
                            app_sent_map.equal_range(mean_elm.obj);
                    }


                    while(
                        item_range.first !=
                            item_range.second
                    ){
                        Rule word;
                        word =
                            (*(item_range.first)).second;
                        (word.internal.front()).obj =
                            location + in_idx+VARIABLE_NO;
                        //patternsは複数のpattern
                            // なので直積を作る必要がある．
                        patternList_it =
                            patternList.begin();
                        while (
                            patternList_it !=
                                patternList.end()
                        ) {
                            PatternType
                                sub_pattern;
                            //すでに作られてる
                                // 単語規則の組をコピー
                            sub_pattern =
                                *patternList_it;

                            //そこへ新しく単語規則を追加
                            sub_pattern.push_back(word);

                            //新しく単語規則が
                            // 追加されたルールリストを
                            // 新しく保存
                            patternList_buffer.push_back(
                                sub_pattern
                            );

                            patternList_it++;
                        }

                        item_range.first++;
                    }

                    //変更された単語規則組の列で
                    // 元の単語規則組の列を置き換える
                    patternList.swap(patternList_buffer);

                    is_complete &= true;
                    is_absolute &= false;

                }else{
                    ungrounded_variable_num++;

                    if (ungrounded_variable_num > ABSENT_LIMIT ||
                        mean_elm.ch.front() != 1
                    ) {
                        is_applied &= false;
                    } else {
                        if (
                            grnd_elm.is_var()
                        ) {
                            std::vector<PatternType>
                                patternList_buffer;

                            //すでに作られてる単語組に
                                // 対し組み合わせの直積の生成
                            patternList_it =
                                patternList.begin();
                            while (
                                patternList_it !=
                                    patternList.end()
                            ) {

                                //検索した適用可能な単語規則列
                                PatternType sub_pattern;
                                Rule empty_word;
                                ExType empty_ex;

                                //空の単語規則を作る
                                empty_word.set_noun(
                                    grnd_elm.cat,
                                    mean_elm,
                                    empty_ex
                                );
                                // empty_word.internal.front().obj = in_idx+VARIABLE_NO;

                                //すでに作られてる
                                // 単語規則の組をコピー
                                sub_pattern =
                                    *patternList_it;

                                //そこへ新しく
                                    // 単語規則を追加
                                sub_pattern.push_back(
                                    empty_word
                                );

                                //新しく単語規則が
                                // 追加された単語規則組を
                                // 新しく保存
                                patternList_buffer.push_back(
                                    sub_pattern
                                );

                                //単語規則組が
                                // 無くなるまで繰り返す
                                patternList_it++;
                            }
                            //変更された単語規則組の列で
                            // 元の単語規則組の列を置き換える
                            patternList.swap(
                                patternList_buffer
                            );

                            is_absolute &= false;
                            is_complete &= false;
                            is_semicomplete &= true;
                        } else {
                            std::cerr <<
                                "pattern error" <<
                                std::endl;
                            throw;
                        }
                    }
                }
            }else{
                is_applied &= false;
            }
            grd_idx++;
        }
        //ある文規則に対して取得できたグラウンドパターンを保存
        if(is_applied){
            patterns.insert(
                patterns.end(),
                patternList.begin(),
                patternList.end()
            );
            succeed = true;
        }

        //次の文規則を検査
        sent_it++;
    } //文規則のループ*/

    return succeed;
}

std::map<KnowledgeBase::PATTERN_TYPE,
    std::vector<KnowledgeBase::PatternType>
>
KnowledgeBase::construct_grounding_patterns(
    Rule& src
) {
    typedef std::pair<
        std::multimap<int, Rule>::iterator,
        std::multimap<int, Rule>::iterator
    > DictionaryRange;

    /*
     * Srcに対して、それぞれの文規則がグラウンド可能か検査する
     * グラウンディング可能な場合、そのグラウンディングに使用する
     * 単語規則とその文規則の組の全パターンを集める
     */
    // build_word_index();

    //SentenceDBシーケンス用
    RuleDBType::iterator sent_it;
    ExType::iterator sent_ex_it;
    int ungrounded_variable_num;
    bool succeed, var_fl;
    bool
        is_applied,
        is_absolute,
        is_complete,
        is_semicomplete
    ;
    std::map<
        PATTERN_TYPE,
        std::vector<KnowledgeBase::PatternType>
    > ret;
    //グラウンドパターンの格納庫とそのイテレータ
    std::vector<
        PatternType
    > patternDB, patterns;
    std::vector<
        PatternType
    >::iterator patternDB_it;
    //初期パターン
    PatternType pattern;

    if(src.internal.size() > 2)
        {var_fl = true;}
    else
        {var_fl = false;}

    sent_it = sentenceDB.begin();
    while (sent_it != sentenceDB.end()) {
        Rule tmp_rule;
        ungrounded_variable_num = 0;

        if (
            (*sent_it).internal.front() !=
                src.internal.front() ||
            (
                var_fl &&
                (*sent_it).internal.size() == 1
            ) ||
            (
                !var_fl &&
                (*sent_it).internal.size() != 1
            )
        ) {
            sent_it++;
            continue;
        }

        //組み上げようの書き換え
        int count=1, index;
        tmp_rule = (*sent_it);
        for (
            index = 1;
            (
                count < src.internal.size() &&
                index < tmp_rule.internal.size()
            );
            index++
        ) {
            if (
                tmp_rule.internal[
                    index
                ].is_var()
            ){
                sent_ex_it =
                    tmp_rule.external.begin();
                for(
                    ;
                    sent_ex_it != tmp_rule.external.end();
                    sent_ex_it++
                ){
                    if(
                        (*sent_ex_it).is_cat() &&
                        tmp_rule.internal[index].obj ==
                            (*sent_ex_it).obj
                    ){
                        (*sent_ex_it).obj = count + VARIABLE_NO;
                        tmp_rule.internal[index].obj = count + VARIABLE_NO;
                    }
                }
            }
            count +=
                src.internal[count].ch.front();
        }
        //indexとcountが同期していることを確認
        if(
            index != tmp_rule.internal.size() ||
            count != src.internal.size()
        ){
            sent_it++;
            continue;
        }

        tmp_rule.internal.front().obj = 0;

        //グラウンドパターンの格納庫とそのイテレータ
        patternDB.clear();

        //初期パターン
        pattern.clear();

        std::cout << "construct_grounding_patterns " << tmp_rule.to_s() << std::endl;

        //始めに検索対象文規則をパターン格納庫に入れる
        pattern.push_back(tmp_rule);//書き換えてからパターンに詰める
        patternDB.push_back(pattern);

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;
        int in_idx, grd_idx;

        is_applied =
        is_absolute =
        is_complete =
        is_semicomplete =
        true;
        for (
            in_idx = grd_idx = 1;
            (
                is_applied &&
                in_idx < src.internal.size()
            );
            in_idx += (*(src.internal.begin()+in_idx)).ch.front()
        ) {
            grnd_elm = (*sent_it).internal[grd_idx];
            //検査するインターナル要素
            mean_elm = src.internal[in_idx];
            //基準のインターナル要素
            if (grnd_elm == mean_elm) {
            //単語がそのまま一致する場合
                is_absolute &= true;
                continue;
            } else if(
                mean_elm.ch.front() > 1 &&
                grnd_elm.is_var()
            ){
                patterns.clear();
                succeed = lemma_construct_grounding_patterns(
                    src.internal.begin()+in_idx,
                    src.internal.begin()+in_idx+(*(src.internal.begin()+in_idx)).ch.front(),
                    in_idx,
                    grnd_elm.cat,
                    ungrounded_variable_num,
                    patterns,
                    is_applied,
                    is_absolute,
                    is_complete,
                    is_semicomplete,
                    grnd_elm.sent_type
                );
                if(succeed){
                    std::vector<PatternType>
                        patternDB_buffer;
                    std::vector<PatternType>::iterator
                        patterns_it;
                    //patternsは複数のpatternなので
                        // 直積を作る必要がある．
                    patternDB_it = patternDB.begin();
                    while (
                        patternDB_it != patternDB.end()
                    ) {
                        patterns_it = patterns.begin();
                        while (
                            patterns_it != patterns.end()
                        ){
                            PatternType sub_pattern;
                            //すでに作られてる
                        // 単語規則の組をコピー
                            sub_pattern = *patternDB_it;

                            //そこへ新しく
                            // 単語規則を追加
                            sub_pattern.insert(
                                sub_pattern.end(),
                                (*patterns_it).begin(),
                                (*patterns_it).end()
                            );

                            //新しく単語規則が
                            // 追加された単語規則組を
                            // 新しく保存
                            patternDB_buffer.push_back(
                                sub_pattern
                            );

                            patterns_it++;
                        }
                        patternDB_it++;
                    }
                    //変更された単語規則組の列で元の単語規則組の列を置き換える
                    patternDB.swap(
                        patternDB_buffer
                    );

                    is_complete &= true;
                    is_absolute &= false;
                }else{
                    is_applied &= false;
                }
            }else if(grnd_elm.is_var()){
                std::multimap<int, Rule> app_sent_map;
                if(grnd_elm.sent_type){
                    app_sent_map.insert(DB_dic[0].begin(),DB_dic[0].end());//sentenceはすべてここにいれてある
                }
                
                if(//変数の場合で、グラウンド可能な場合//Sentenceの検索が抜けてる
                    (DB_dic.find(grnd_elm.cat) != DB_dic.end() && //変数のカテゴリが辞書に有り
                        DB_dic[grnd_elm.cat].find(mean_elm.obj) != DB_dic[grnd_elm.cat].end() ) //辞書の指定カテゴリに単語がある
                    ||(grnd_elm.sent_type && app_sent_map.find(mean_elm.obj) != app_sent_map.end() ) //sentenceの砲に適用可能ルールがある
                ){
                    DictionaryRange item_range;
                    std::vector<PatternType>
                        patternDB_buffer;

                    //変数に適用可能単語規則集合取得
                    if(!grnd_elm.sent_type){
                        item_range =
                            DB_dic[grnd_elm.cat].equal_range(mean_elm.obj);
                    }else{
                        app_sent_map.insert(DB_dic[grnd_elm.cat].begin(),DB_dic[grnd_elm.cat].end());
                        item_range =
                            app_sent_map.equal_range(mean_elm.obj);
                    }


                    while(
                        item_range.first !=
                            item_range.second
                    ){
                        Rule word;
                        word = (
                            *(item_range.first)
                        ).second;
                        (word.internal.front()).obj = in_idx+VARIABLE_NO;
                        //patternsは複数のpatternなので
                        // 直積を作る必要がある．
                        patternDB_it =
                            patternDB.begin();
                        while (
                            patternDB_it !=
                                patternDB.end()
                        ) {
                            PatternType sub_pattern;
                            //すでに作られてる
                        // 単語規則の組をコピー
                            sub_pattern =
                                *patternDB_it;

                            //そこへ新しく
                                // 単語規則を追加
                            sub_pattern.push_back(
                                word
                            );

                            //新しく単語規則が追加された
                            // ルールリストを新しく保存
                            patternDB_buffer.push_back(
                                sub_pattern
                            );

                            patternDB_it++;
                        }

                        item_range.first++;
                    }

                    //変更された単語規則組の列で
                    // 元の単語規則組の列を置き換える
                    patternDB.swap(patternDB_buffer);

                    is_complete &= true;
                    is_absolute &= false;

                }else{
                    ungrounded_variable_num++;

                    if (
                        ungrounded_variable_num >
                            ABSENT_LIMIT ||
                        mean_elm.ch.front() !=
                            1
                    ) {
                        is_applied &= false;
                    } else {
                        if (grnd_elm.is_var()) {
                            std::vector<PatternType>
                                patternDB_buffer;

                            //すでに作られてる
                                // 単語組に対し
                                // 組み合わせの直積の生成
                            patternDB_it =
                                patternDB.begin();
                            while (
                                patternDB_it !=
                                    patternDB.end()
                            ) {

                                //検索した適用可能な
                                // 単語規則列
                                PatternType
                                    sub_pattern;
                                Rule
                                    empty_word;
                                ExType
                                    empty_ex;

                                //空の単語規則を作る
                                empty_word.set_noun(
                                    grnd_elm.cat,
                                    mean_elm,
                                    empty_ex
                                );
                                // empty_word.internal.front().obj = in_idx+VARIABLE_NO;

                                //すでに作られてる
                                // 単語規則の組をコピー
                                sub_pattern =
                                    *patternDB_it;

                                //そこへ新しく
                                    // 単語規則を追加
                                sub_pattern.push_back(
                                    empty_word
                                );

                                //新しく単語規則が
                                // 追加された単語規則組を
                                // 新しく保存
                                patternDB_buffer.push_back(
                                    sub_pattern
                                );

                                //単語規則組が無くなるまで繰り返す
                                patternDB_it++;
                            }
                            //変更された単語規則組の列で
                            // 元の単語規則組の列を置き換える
                            patternDB.swap(
                                patternDB_buffer
                            );
                            is_absolute &= false;
                            is_complete &= false;
                            is_semicomplete &= true;
                        } else {
                            std::cerr <<
                                "pattern error" <<
                                std::endl;
                            throw;
                        }
                    }
                }
            }else{
                is_applied = false;
            }
            grd_idx++;
        }
            // //ある文規則に対して取得できたグラウンドパターンを保存
        if (is_applied) {
            if (is_absolute) {
                ret[ABSOLUTE].insert(
                    ret[ABSOLUTE].end(),
                    patternDB.begin(),
                    patternDB.end()
                );
            } else if (is_complete) {
                ret[COMPLETE].insert(
                    ret[COMPLETE].end(),
                    patternDB.begin(),
                    patternDB.end()
                );
            } else if (is_semicomplete) {
                ret[SEMICOMPLETE].insert(
                    ret[SEMICOMPLETE].end(),
                    patternDB.begin(),
                    patternDB.end()
                );
            }
        }
        //次の文規則を検査
        sent_it++;
    } //文規則のループ*/

    return ret;
}

std::vector<Rule>
KnowledgeBase::grounded_rules(Rule src) {
    RuleDBType grounded_rules;
    std::map<
        PATTERN_TYPE,
        std::vector<PatternType>
    > patterns;

    patterns =
        construct_grounding_patterns(src);

    if (
        patterns[ABSOLUTE].size() == 0 &&
        patterns[COMPLETE].size() == 0
    )
        return grounded_rules;

    if (
        patterns[ABSOLUTE].size() != 0
    ) {
        std::vector<PatternType>::iterator it;
        it = patterns[ABSOLUTE].begin();
        while (
            it != patterns[ABSOLUTE].end()
        ) {
            grounded_rules.push_back((*it).front());
            it++;
        }
    }

    if (
        patterns[COMPLETE].size() != 0
    ) {
        std::vector<PatternType>::iterator pat_it;
        pat_it = patterns[COMPLETE].begin();

        while (
            pat_it != patterns[COMPLETE].end()
        ) {
            Rule grounded_rule;
            grounded_rule = src;
            ground_with_pattern(
                grounded_rule,
                (*pat_it)
            );
            grounded_rules.push_back(
                grounded_rule
            );
            pat_it++;
        }
    }

    return grounded_rules;
}

//perse: src.external.end()まで
//parse_step: 指定されたend以内で先頭から部分的に埋ればよい
//perse,perse_step: 区間列については完全に埋まらないといけない

bool//srcのexternal::iteratorを送る
KnowledgeBase::parse(
    Rule& src,
    std::vector<std::vector<Rule> >& r_list,
    std::vector<std::string>& t_list
){

    std::cout << "PARSE TARGET: " << src.to_s() << std::endl;

    /*
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    >
    上記は
    std::map<
        std::vector<
            std::vector<Rule>
        >,
        ExType::iterator
    >
    の方が効率がいい
    push_~~~となっている変数はたぶん余計．
    */
    // build_word_index();
    r_list.clear();
    RuleDBType::iterator sent_it =
        sentenceDB.begin();
    bool p_fl = false; //全体
    bool flag; //汎用
    std::vector< std::vector<ExType> >
        ex_patterns; //分割用
    // Rule rbase; //初期ルール
    // std::vector<Rule> base_list; //初期ルールだけのリスト
    std::vector<std::vector<ExType> >::iterator ex_pattern_it; //分割したシンボル列のパターンを順にチェックするためのイテレータ
    std::vector<ExType>::iterator ex_list_it; //分割したシンボル列を順にチェックするためのイテレータ
    // ExType::iterator ex_it; //対象のシンボル列内を順にチェックするイテレータ
    ExType::iterator base_ex_it; //ベースルールのシンボル列をチェックするイテレータ
    // ExType::iterator first_it, end_it; //ベースルールの中からカテゴリ列を指定する
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    > pattern_pairs; //ルールリストとその終了位置の記録．各パターンごとに作成．最後にendになっているリストだけr_listに追加する
    // std::pair<
    //     std::vector<Rule>,
    //     ExType::iterator
    // > base_pair; //rbaseと対象のシンボル列の開始位置を記録する
    // std::vector<
    //     std::pair<
    //         std::vector<Rule>,
    //         ExType::iterator
    //     >
    // > sub_pairs; //分割したシンボル列を埋めるルールリストとその終了位置の記録．各分割シンボル列ごとに作成．最後に埋めるべきシンボル列がすべて埋まっていたらpattern_pairsに追加（pattern_stepではこの限りでない）
    // std::vector<
    //     std::pair<
    //         std::vector<Rule>,
    //         ExType::iterator
    //     >
    // > step_pairs; //parse_stepの受け取り用
    //交換用
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    >::iterator
        main_it, trg_it; //直積のベースとターゲットのイテレータ
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    > buf; //直積の一時蓄積用，または，入れ替え時の一時蓄積用
    std::pair<
        std::vector<Rule>,
        ExType::iterator
    > tmp_pair; //pairの仮置き
    // std::vector<Rule> tmp_rules; //ルールリストの仮置き

    while(
        sent_it!=sentenceDB.end()
    ){
        Rule rbase = (*sent_it);
        std::vector<Rule> base_list; //初期ルールだけのリスト
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        > base_pair; //rbaseと対象のシンボル列の開始位置を記録する
        //表現する対称のシンボル列を
        // 各カテゴリ列に割り振る候補の計算
        ex_patterns.clear();
        flag = parse_option_symbols
            (
                rbase,
                src.external,
                ex_patterns
            )
        ;
        if(
            !flag
        ){
            sent_it++;
            continue;
        }
        //初期ルールリストbase_listにsentenceだけ追加
        base_list.clear();
        base_list.push_back(rbase);
        base_pair.first = base_list;

        //割り振り候補が0の場合は
        // カテゴリ数も0で完全一致しているから次へ
        if(ex_patterns.size() == 0){

            // std::cout << "FORMAT FAULT: " << (*sent_it).to_s() << std::endl;

            p_fl |= true;
            r_list.push_back(base_list);
            sent_it++;
            continue;
        }

        // std::cout << "SENTENCE: " << (*sent_it).to_s() << std::endl;
        // std::cout << "Ex size: " << ex_patterns.size() << std::endl;

        //全ルールリスト候補: r_list
        //一文単位の候補: buf
        //symbol列の割り当て単位の候補: pairs

        //各ex_lilstを完全に埋めれたものだけ
        // r_listに追加する
        ex_pattern_it
            = ex_patterns.begin();
        while(ex_pattern_it!=ex_patterns.end()){
            std::vector<
                std::pair<
                    std::vector<Rule>,
                    ExType::iterator
                >
            > pattern_pairs; //ルールリストとその終了位置の記録．各パターンごとに作成．最後にendになっているリストだけr_listに追加する


            int counter = 0;

            //各カテゴリ列の埋め方でbufのなか分岐
            //初期ルールリストから分岐開始
            pattern_pairs.clear();
            pattern_pairs.push_back(base_pair);

            //カテゴリ列特定用
            base_ex_it = rbase.external.begin();

            // std::cout << "ExList_It3" << std::endl;

            ex_list_it = (*ex_pattern_it).begin();
            while(
                ex_list_it != (*ex_pattern_it).end()
            ){
                std::vector<
                    std::pair<
                        std::vector<Rule>,
                        ExType::iterator
                    >
                > sub_pairs; //分割したシンボル列を埋めるルールリストとその終了位置の記録．各分割シンボル列ごとに作成．最後に埋めるべきシンボル列がすべて埋まっていたらpattern_pairsに追加（pattern_stepではこの限りでない）

                std::pair<ExType::iterator, ExType::iterator>
                    cat_seq =
                        find_next_cat_seq(
                            base_ex_it,
                            rbase.external.end()
                        )
                ;
                if(cat_seq.first == rbase.external.end()){
                    std::cerr << "Can not find category sequence" << std::endl;
                    throw "Error";
                }

                //各カテゴリ列の各カテゴリを埋める
                flag = true;
                sub_pairs.clear();
                std::vector<Rule> emp_rules;
                std::pair<
                    std::vector<Rule>,
                    ExType::iterator
                > push_pair_b (emp_rules,(*ex_list_it).begin());
                sub_pairs.push_back(push_pair_b);
                while(
                    cat_seq.first != cat_seq.second
                ){
                    std::map<int,int>
                        loop_det;
                    loop_det.insert(
                        std::map<int,int>::value_type(0,1)
                    );

                    // std::cout << "[m]check CAT: " << (*cat_seq.first).to_s() << std::endl;

                    //pairsでルールリストと
                    // そのルールリストと消費した位置を管理
                    //(*ex_it)（ExType）を使い切ら
                    // なければならない
                    buf.clear();
                    int category =
                        (*cat_seq.first).cat;

                    bool
                        grounded_fl = false; //少なくとも一つ候補があったか
                    main_it = sub_pairs.begin();
                    for(
                        ;
                        main_it != sub_pairs.end();
                        main_it++
                    ){
                        std::vector<
                            std::pair<
                                std::vector<Rule>,
                                ExType::iterator
                            >
                        > step_pairs; //parse_stepの受け取り用
                        step_pairs.clear();

                        std::cout << "[m]FILLED PR: 0 - " << (*main_it).second - (*ex_list_it).begin() << std::endl;

                        grounded_fl = parse_step
                            (
                                (*main_it).second,
                                (*ex_list_it).end(),
                                category,
                                step_pairs,
                                loop_det
                            )
                        ;
                        if(grounded_fl){
                            trg_it = step_pairs.begin();
                            while(trg_it != step_pairs.end()){
                                std::vector<Rule> tmp_rules; //ルールリストの仮置き
                                tmp_rules = (*main_it).first;
                                tmp_rules.insert(tmp_rules.end(),(*trg_it).first.begin(),(*trg_it).first.end());
                                std::pair<
                                    std::vector<Rule>,
                                    ExType::iterator
                                > push_pair (tmp_rules,(*trg_it).second);
                                buf.push_back(push_pair);
                                trg_it++;
                            }

                            // std::cout << "[m]BUF SIZE: " << buf.size() << std::endl;

                        }
                    }
                    // std::vector<
                    //     std::pair<
                    //         std::vector<Rule>,
                    //         ExType::iterator
                    //     >
                    // >().swap(sub_pairs);
                    sub_pairs.swap(buf);
                    if(sub_pairs.size() == 0){
                        flag = false;
                        break;
                    }
                    cat_seq.first++;
                }

                // std::cout << "ExList_It6" << std::endl;
                // std::cout << "[m]PARSE STEP RESULT: " << flag << std::endl;
                // std::cout << "[m]PARSE STEP SIZE: " << sub_pairs.size() << std::endl;

                //while抜けた時点でカテゴリを使い切っているので
                // pairのsecondが(*ex_it).end()になっている
                //endになっているものでbufと直積生成
                if(flag){
                    buf.clear();
                    trg_it = sub_pairs.begin();
                    while(trg_it != sub_pairs.end()){
                        main_it = pattern_pairs.begin();
                        while(main_it != pattern_pairs.end()){
                            if((*trg_it).second == (*ex_list_it).end()){
                                std::pair<
                                    std::vector<Rule>,
                                    ExType::iterator
                                > push_pair ((*main_it).first,src.external.end());
                                push_pair.first.insert(push_pair.first.end(),(*trg_it).first.begin(),(*trg_it).first.end());

                                // std::cout << std::endl << "[m pattern_pairs]Additional rules" << std::endl;
                                // std::vector<Rule>::iterator out_it;
                                // for(out_it = push_pair.first.begin();out_it != push_pair.first.end();out_it++){
                                //     std::cout << (*out_it).to_s() << std::endl;
                                // }
                                // std::cout << "[END]" << std::endl << std::endl;

                                buf.push_back(push_pair);
                            }
                            main_it++;
                        }
                        trg_it++;
                    }
                }else{
                    //埋められなかったら
                    // この割り振りでは埋めきれないためbufをクリア
                    //pairsのサイズゼロ
                    buf.clear();
                    pattern_pairs.swap(buf);
                    break;
                }
                pattern_pairs.swap(buf);
                ex_list_it++;
                base_ex_it = cat_seq.second;
            }

            // std::cout << "ExList_It4" << std::endl;
            // std::cout << "[m]PATTERNS SIZE: " << pattern_pairs.size() << std::endl;

            //埋め方全パターンbufをr_listへ追加
            if(pattern_pairs.size() > 0){
                p_fl |= true;
                trg_it = pattern_pairs.begin();
                while(trg_it != pattern_pairs.end()){
                    std::vector<Rule> push_rules;
                    push_rules = (*trg_it).first;

                    // std::cout << std::endl << "[m r_list]Additional rules" << std::endl;
                    // std::vector<Rule>::iterator out_it;
                    // for(out_it = push_rules.begin();out_it != push_rules.end();out_it++){
                    //     std::cout << (*out_it).to_s() << std::endl;
                    // }
                    // std::cout << "[END]" << std::endl << std::endl;

                    r_list.push_back(
                        push_rules
                    );
                    trg_it++;
                }
            }

            // std::cout << "ExList_It2" << std::endl;

            ex_pattern_it++;
        }

        sent_it++;
    }

    //for graphviz
    std::string begin_str("digraph sample{\n"), end_str("}");
    t_list.clear();
    std::vector<Rule>::iterator ls_it;
    std::vector<std::vector<Rule> >::iterator r_it;
    r_it=r_list.begin();
    std::map<int, int> memo;
    std::map<std::string, int> memo2;
    std::vector<std::string> stack;
    int val=0;
    std::multimap<int, std::string> r_int_to_str;
    std::map<std::string, int> r_str_to_int;
    // std::vector<std::vector<std::string> > opt;
    for(;r_it!=r_list.end();r_it++){
        std::vector<std::string> smp;
        memo.clear();
        memo2.clear();
        stack.clear();
        r_int_to_str.clear();
        r_str_to_int.clear();
        stack.push_back(Prefices::SEN/*+std::to_string((*r_it).front().cat)*/);
        r_int_to_str.insert(std::multimap<int,std::string>::value_type(0,Prefices::SEN/*+std::to_string((*r_it).front().cat)*/));
        r_str_to_int.insert(std::map<std::string,int>::value_type(Prefices::SEN/*+std::to_string((*r_it).front().cat)*/,0));
        ls_it=(*r_it).begin();
        for(;ls_it!=(*r_it).end();ls_it++){
            std::string frm = stack.front();
            std::vector<std::string> mini_stack;
            ExType::iterator g_ex_it = (*ls_it).external.begin();
            for(;g_ex_it!=(*ls_it).external.end();g_ex_it++){
                if((*g_ex_it).is_cat()){

                    if(memo.find((*g_ex_it).cat) == memo.end()){
                        memo.insert(std::map<int,int>::value_type((*g_ex_it).cat,1));
                    }else{
                        memo[(*g_ex_it).cat] += 1;
                    }

                    mini_stack.push_back(Prefices::CAT + std::to_string((*g_ex_it).cat) + Prefices::UNO + std::to_string(memo[(*g_ex_it).cat]));

                    r_int_to_str.insert(std::multimap<int,std::string>::value_type(r_str_to_int[frm] + 1,mini_stack.back()));
                    r_str_to_int.insert(std::map<std::string,int>::value_type(mini_stack.back(),r_str_to_int[frm] + 1));

                    smp.push_back(frm + std::string(" -> ") + mini_stack.back());

                }else{
                    if(memo2.find((*g_ex_it).to_s()) == memo2.end()){
                        memo2.insert(std::map<std::string,int>::value_type((*g_ex_it).to_s(),1));
                    }else{
                        memo2[(*g_ex_it).to_s()] += 1;
                    }

                    smp.push_back(frm + std::string(" -> \"") + std::to_string(memo2[(*g_ex_it).to_s()]) + std::string(" ") + (*g_ex_it).to_s() + std::string("\""));
                }
            }
            stack.erase(stack.begin());
            stack.insert(stack.begin(),mini_stack.begin(),mini_stack.end());
        }
        //rankルール生成
        int upper_bound = 0;
        std::multimap<int, std::string>::iterator itr=r_int_to_str.begin();
        upper_bound = (*itr).first;
        while((itr = r_int_to_str.upper_bound((*itr).first) )!=r_int_to_str.end()){
            upper_bound = (*itr).first;
        }
        //terminal symbolのrank生成(仮としてmemo2)
        std::string t_rank;
        std::map<std::string,int>::iterator str_to_int_it;
        t_rank = std::string("{rank = max; \"");
        t_rank += (*memo2.begin()).first + std::string("\";");
        str_to_int_it = memo2.begin();
        str_to_int_it++;
        for(;str_to_int_it!=memo2.end();str_to_int_it++){
            for(int i = 1; i <= (*str_to_int_it).second; i++){
                t_rank += (std::string(" \"") + std::to_string(i) + std::string(" ") + (*str_to_int_it).first + std::string("\";"));
            }
        }
        t_rank += std::string("}");

        std::vector<std::string> nt_ranks;
        nt_ranks.push_back(std::string("{rank = min; S;}"));
        for(int i=1;i<=upper_bound;i++){
            auto ra = r_int_to_str.equal_range(i);
            std::string nt_rank("{rank = same; ");
            nt_rank += (*ra.first).second + std::string(";");
            ra.first++;
            while(ra.first != ra.second){
                nt_rank += (std::string(" ") + (*ra.first).second + std::string(";"));
                ra.first++;
            }
            nt_rank += std::string("}");
            nt_ranks.push_back(nt_rank);
        }


        //smp処理
        std::string tmp = begin_str;
        tmp += boost::algorithm::join(smp, ";\n");
        tmp += std::string(";\n");
        tmp += t_rank;
        tmp += std::string("\n");
        tmp += boost::algorithm::join(nt_ranks, "\n");
        tmp += std::string("\n");
        tmp += end_str;
        t_list.push_back(tmp);
    }

    return p_fl;
}

bool
KnowledgeBase::lemma_construct_grounding_patterns_keep_id(
    std::vector<Element>::iterator it,
    std::vector<Element>::iterator it_end,
    int location,
    int category,
    int& ungrounded_variable_num,
    std::vector<KnowledgeBase::PatternType>& patterns,
    bool& is_applied,
    bool& is_absolute,
    bool& is_complete,
    bool& is_semicomplete,
    bool& sent_search
){
    typedef std::pair<
        std::multimap<int, Rule>::iterator,
        std::multimap<int, Rule>::iterator
    > DictionaryRange;
    RuleDBType::iterator
        sent_it,
        end_it;
    bool succeed = false, var_fl;

    //SentenceDBシーケンス用
    ExType::iterator sent_ex_it;
    bool succeed_sub;
    //グラウンドパターンの格納庫とそのイテレータ
    std::vector<PatternType>
        patternList,
        patterns_sub;
    std::vector<PatternType>::iterator
        patternList_it;
    //初期パターン
    PatternType pattern;
    // RuleDBType search_box;
    RuleDBType allDB=sentenceDB;
    allDB.insert(allDB.end(),wordDB.begin(),wordDB.end());

    if(it + 1 != it_end)
        {var_fl =true;}
    else
        {var_fl = false;}

    if(sent_search){
        sent_it = allDB.begin();
        end_it = allDB.end();
    }else{
        sent_it = wordDB.begin();
        end_it = wordDB.end();
    }
    while (sent_it != end_it) {
        Rule tmp_rule;

        if (
            (*sent_it).internal.front() != (*it) ||
            (var_fl && (*sent_it).internal.size() == 1) ||
            (!var_fl && (*sent_it).internal.size() != 1)
        ) {
            sent_it++;
            continue;
        }

        //組み上げようの書き換え
        int
            count=1,
            index;
        tmp_rule = (*sent_it);
        for (
            index = 1;
            (
                (it + count != it_end) &&
                index < tmp_rule.internal.size()
            );
            index++
        ) {
            if (
                tmp_rule.internal[index].is_var()
            ){
                sent_ex_it =
                    tmp_rule.external.begin();
                for(
                    ;
                    sent_ex_it != tmp_rule.external.end();
                    sent_ex_it++
                ){
                    if(
                        (*sent_ex_it).is_cat() &&
                        tmp_rule.internal[index].obj ==
                            (*sent_ex_it).obj
                    ){
                        // (*sent_ex_it).obj =
                        //     location + count + VARIABLE_NO;
                        // tmp_rule.internal[index].obj =
                        //     location + count + VARIABLE_NO;
                    }
                }
            }
            count += (*(it + count)).ch.front();
        }
        if(
            index != tmp_rule.internal.size() ||
            (it + count) != it_end
        ){
            sent_it++;
            continue;
        }

        // tmp_rule.internal.front().obj = location+VARIABLE_NO;

        //グラウンドパターンの格納庫とそのイテレータ
        patternList.clear();

        //初期パターン
        pattern.clear();

        // std::cout << "lemma_construct_grounding_patterns " << tmp_rule.to_s() << std::endl;

        //始めに検索対象文規則をパターン格納庫に入れる
        pattern.push_back(tmp_rule);
        //書き換えてからパターンに詰める
        patternList.push_back(pattern);

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;
        int in_idx, grd_idx;

        is_applied &= true;
        for (
            in_idx = grd_idx = 1;
            (
                is_applied &&
                (it + in_idx) != it_end
            ); //sent_itとsrcの0番目(front)は必ず一致
                in_idx += (*(it+in_idx)).ch.front()
        ) {
            grnd_elm =
                (*sent_it).internal[grd_idx];
                //検査するインターナル要素
            mean_elm =
                *(it + in_idx);
                //基準のインターナル要素
            if (grnd_elm == mean_elm) {
                //単語がそのまま一致する場合
                is_absolute &= true;
                continue;
            } else if(
                mean_elm.ch.front() > 1 &&
                grnd_elm.is_var()
            ){
                patterns_sub.clear();
                succeed_sub =
                    lemma_construct_grounding_patterns_keep_id(
                        it+in_idx,
                        it+in_idx+(*(it+in_idx)).ch.front(),
                        location + in_idx,
                        grnd_elm.cat,
                        ungrounded_variable_num,
                        patterns_sub,
                        is_applied,
                        is_absolute,
                        is_complete,
                        is_semicomplete,
                        grnd_elm.sent_type
                    )
                ;
                if(succeed_sub){
                    std::vector<PatternType>
                        patternList_buffer;
                    std::vector<PatternType>::iterator
                        sub_patterns_it;
                    //patternsは複数のpattern
                        // なので直積を作る必要がある．
                    patternList_it =
                        patternList.begin();
                    while (
                        patternList_it !=
                            patternList.end()
                    ) {
                        sub_patterns_it =
                            patterns_sub.begin();
                        while (
                            sub_patterns_it !=
                                patterns_sub.end()
                        ){
                            PatternType
                                sub_pattern;
                            //すでに作られてる
                                // 単語規則の組をコピー
                            sub_pattern =
                                *patternList_it;

                            //そこへ新しく
                                // 単語規則を追加
                            sub_pattern.insert(
                                sub_pattern.end(),
                                (*sub_patterns_it).begin(),
                                (*sub_patterns_it).end()
                            );

                            //新しく単語規則が
                            // 追加された単語規則組を新しく保存
                            patternList_buffer.push_back(
                                sub_pattern
                            );

                            sub_patterns_it++;
                        }
                        patternList_it++;
                    }
                    //変更された単語規則組の列で元の単語規則組の列を置き換える
                    patternList.swap(
                        patternList_buffer
                    );

                    is_complete &= true;
                    is_absolute &= false;
                }else{
                    is_applied &= false;
                }
            }else if(
                grnd_elm.is_var()
            ){
                std::multimap<int, Rule> app_sent_map;
                if(grnd_elm.sent_type){
                    app_sent_map.insert(DB_dic[0].begin(),DB_dic[0].end());//sentenceはすべてここにいれてある
                }
                
                if(//変数の場合で、グラウンド可能な場合//Sentenceの検索が抜けてる
                    (DB_dic.find(grnd_elm.cat) != DB_dic.end() && //変数のカテゴリが辞書に有り
                        DB_dic[grnd_elm.cat].find(mean_elm.obj) != DB_dic[grnd_elm.cat].end() ) //辞書の指定カテゴリに単語がある
                    ||(grnd_elm.sent_type && app_sent_map.find(mean_elm.obj) != app_sent_map.end() ) //sentenceの砲に適用可能ルールがある
                ){
                    DictionaryRange item_range;
                    std::vector<PatternType>
                        patternList_buffer;

                    //変数に適用可能単語規則集合取得
                    if(!grnd_elm.sent_type){
                        item_range =
                            DB_dic[grnd_elm.cat].equal_range(mean_elm.obj);
                    }else{
                        app_sent_map.insert(DB_dic[grnd_elm.cat].begin(),DB_dic[grnd_elm.cat].end());
                        item_range =
                            app_sent_map.equal_range(mean_elm.obj);
                    }
                    

                    while(
                        item_range.first !=
                            item_range.second
                    ){
                        Rule word;
                        word =
                            (*(item_range.first)).second;
                        // (word.internal.front()).obj =
                        //     location + in_idx+VARIABLE_NO;
                        //patternsは複数のpattern
                            // なので直積を作る必要がある．
                        patternList_it =
                            patternList.begin();
                        while (
                            patternList_it !=
                                patternList.end()
                        ) {
                            PatternType
                                sub_pattern;
                            //すでに作られてる
                                // 単語規則の組をコピー
                            sub_pattern =
                                *patternList_it;

                            //そこへ新しく単語規則を追加
                            sub_pattern.push_back(word);

                            //新しく単語規則が
                            // 追加されたルールリストを
                            // 新しく保存
                            patternList_buffer.push_back(
                                sub_pattern
                            );

                            patternList_it++;
                        }

                        item_range.first++;
                    }
                    //srの中を順番に処理

                    //変更された単語規則組の列で
                    // 元の単語規則組の列を置き換える
                    patternList.swap(patternList_buffer);

                    is_complete &= true;
                    is_absolute &= false;

                }else{
                    ungrounded_variable_num++;

                    if (ungrounded_variable_num > ABSENT_LIMIT ||
                        mean_elm.ch.front() != 1
                    ) {
                        is_applied &= false;
                    } else {
                        if (
                            grnd_elm.is_var()
                        ) {
                            std::vector<PatternType>
                                patternList_buffer;

                            //すでに作られてる単語組に
                                // 対し組み合わせの直積の生成
                            patternList_it =
                                patternList.begin();
                            while (
                                patternList_it !=
                                    patternList.end()
                            ) {

                                //検索した適用可能な単語規則列
                                PatternType sub_pattern;
                                Rule empty_word;
                                ExType empty_ex;

                                //空の単語規則を作る
                                empty_word.set_noun(
                                    grnd_elm.cat,
                                    mean_elm,
                                    empty_ex
                                );
                                // empty_word.internal.front().obj = in_idx+VARIABLE_NO;

                                //すでに作られてる
                                // 単語規則の組をコピー
                                sub_pattern =
                                    *patternList_it;

                                //そこへ新しく
                                    // 単語規則を追加
                                sub_pattern.push_back(
                                    empty_word
                                );

                                //新しく単語規則が
                                // 追加された単語規則組を
                                // 新しく保存
                                patternList_buffer.push_back(
                                    sub_pattern
                                );

                                //単語規則組が
                                // 無くなるまで繰り返す
                                patternList_it++;
                            }
                            //変更された単語規則組の列で
                            // 元の単語規則組の列を置き換える
                            patternList.swap(
                                patternList_buffer
                            );

                            is_absolute &= false;
                            is_complete &= false;
                            is_semicomplete &= true;
                        } else {
                            std::cerr <<
                                "pattern error" <<
                                std::endl;
                            throw;
                        }
                    }
                }
            }else{
                is_applied &= false;
            }
            grd_idx++;
        }
        //ある文規則に対して取得できたグラウンドパターンを保存
        if(is_applied){
            patterns.insert(
                patterns.end(),
                patternList.begin(),
                patternList.end()
            );
            succeed = true;
        }

        //次の文規則を検査
        sent_it++;
    } //文規則のループ*/

    return succeed;
}

std::map<KnowledgeBase::PATTERN_TYPE,
    std::vector<KnowledgeBase::PatternType>
>
KnowledgeBase::construct_grounding_patterns_keep_id(
    Rule& src
) {
    typedef std::pair<
        std::multimap<int, Rule>::iterator,
        std::multimap<int, Rule>::iterator
    > DictionaryRange;

    /*
     * Srcに対して、それぞれの文規則がグラウンド可能か検査する
     * グラウンディング可能な場合、そのグラウンディングに使用する
     * 単語規則とその文規則の組の全パターンを集める
     */
    // build_word_index();

    //SentenceDBシーケンス用
    RuleDBType::iterator sent_it;
    ExType::iterator sent_ex_it;
    int ungrounded_variable_num;
    bool succeed, var_fl;
    bool
        is_applied,
        is_absolute,
        is_complete,
        is_semicomplete
    ;
    std::map<
        PATTERN_TYPE,
        std::vector<KnowledgeBase::PatternType>
    > ret;
    //グラウンドパターンの格納庫とそのイテレータ
    std::vector<
        PatternType
    > patternDB, patterns;
    std::vector<
        PatternType
    >::iterator patternDB_it;
    //初期パターン
    PatternType pattern;

    if(src.internal.size() > 2)
        {var_fl = true;}
    else
        {var_fl = false;}

    sent_it = sentenceDB.begin();
    while (sent_it != sentenceDB.end()) {
        Rule tmp_rule;
        ungrounded_variable_num = 0;

        if (
            (*sent_it).internal.front() !=
                src.internal.front() ||
            (
                var_fl &&
                (*sent_it).internal.size() == 1
            ) ||
            (
                !var_fl &&
                (*sent_it).internal.size() != 1
            )
        ) {
            sent_it++;
            continue;
        }

        //組み上げようの書き換え
        int count=1, index;
        tmp_rule = (*sent_it);
        for (
            index = 1;
            (
                count < src.internal.size() &&
                index < tmp_rule.internal.size()
            );
            index++
        ) {
            if (
                tmp_rule.internal[
                    index
                ].is_var()
            ){
                sent_ex_it =
                    tmp_rule.external.begin();
                for(
                    ;
                    sent_ex_it != tmp_rule.external.end();
                    sent_ex_it++
                ){
                    if(
                        (*sent_ex_it).is_cat() &&
                        tmp_rule.internal[index].obj ==
                            (*sent_ex_it).obj
                    ){
                        // (*sent_ex_it).obj = count + VARIABLE_NO;
                        // tmp_rule.internal[index].obj = count + VARIABLE_NO;
                    }
                }
            }
            count +=
                src.internal[count].ch.front();
        }
        //indexとcountが同期していることを確認
        if(
            index != tmp_rule.internal.size() ||
            count != src.internal.size()
        ){
            sent_it++;
            continue;
        }

        // tmp_rule.internal.front().obj = 0;

        //グラウンドパターンの格納庫とそのイテレータ
        patternDB.clear();

        //初期パターン
        pattern.clear();

        // std::cout << "construct_grounding_patterns " << tmp_rule.to_s() << std::endl;

        //始めに検索対象文規則をパターン格納庫に入れる
        pattern.push_back(tmp_rule);//書き換えてからパターンに詰める
        patternDB.push_back(pattern);

        //ある単語規則に対するグラウンドパターン検索
        Element grnd_elm, mean_elm;
        int in_idx, grd_idx;

        is_applied =
        is_absolute =
        is_complete =
        is_semicomplete =
        true;
        for (
            in_idx = grd_idx = 1;
            (
                is_applied &&
                in_idx < src.internal.size()
            );
            in_idx += (*(src.internal.begin()+in_idx)).ch.front()
        ) {
            grnd_elm = (*sent_it).internal[grd_idx];
            //検査するインターナル要素
            mean_elm = src.internal[in_idx];
            //基準のインターナル要素
            if (grnd_elm == mean_elm) {
            //単語がそのまま一致する場合
                is_absolute &= true;
                continue;
            } else if(
                mean_elm.ch.front() > 1 &&
                grnd_elm.is_var()
            ){
                patterns.clear();
                succeed = lemma_construct_grounding_patterns_keep_id(
                    src.internal.begin()+in_idx,
                    src.internal.begin()+in_idx+(*(src.internal.begin()+in_idx)).ch.front(),
                    in_idx,
                    grnd_elm.cat,
                    ungrounded_variable_num,
                    patterns,
                    is_applied,
                    is_absolute,
                    is_complete,
                    is_semicomplete,
                    grnd_elm.sent_type
                );
                if(succeed){
                    std::vector<PatternType>
                        patternDB_buffer;
                    std::vector<PatternType>::iterator
                        patterns_it;
                    //patternsは複数のpatternなので
                        // 直積を作る必要がある．
                    patternDB_it = patternDB.begin();
                    while (
                        patternDB_it != patternDB.end()
                    ) {
                        patterns_it = patterns.begin();
                        while (
                            patterns_it != patterns.end()
                        ){
                            PatternType sub_pattern;
                            //すでに作られてる
                        // 単語規則の組をコピー
                            sub_pattern = *patternDB_it;

                            //そこへ新しく
                            // 単語規則を追加
                            sub_pattern.insert(
                                sub_pattern.end(),
                                (*patterns_it).begin(),
                                (*patterns_it).end()
                            );

                            //新しく単語規則が
                            // 追加された単語規則組を
                            // 新しく保存
                            patternDB_buffer.push_back(
                                sub_pattern
                            );

                            patterns_it++;
                        }
                        patternDB_it++;
                    }
                    //変更された単語規則組の列で元の単語規則組の列を置き換える
                    patternDB.swap(
                        patternDB_buffer
                    );

                    is_complete &= true;
                    is_absolute &= false;
                }else{
                    is_applied &= false;
                }
            }else if(grnd_elm.is_var()){
                std::multimap<int, Rule> app_sent_map;
                if(grnd_elm.sent_type){
                    app_sent_map.insert(DB_dic[0].begin(),DB_dic[0].end());//sentenceはすべてここにいれてある
                }
                
                if(//変数の場合で、グラウンド可能な場合//Sentenceの検索が抜けてる
                    (DB_dic.find(grnd_elm.cat) != DB_dic.end() && //変数のカテゴリが辞書に有り
                        DB_dic[grnd_elm.cat].find(mean_elm.obj) != DB_dic[grnd_elm.cat].end() ) //辞書の指定カテゴリに単語がある
                    ||(grnd_elm.sent_type && app_sent_map.find(mean_elm.obj) != app_sent_map.end() ) //sentenceの砲に適用可能ルールがある
                ){
                    DictionaryRange item_range;
                    std::vector<PatternType>
                        patternDB_buffer;

                    //変数に適用可能単語規則集合取得
                    if(!grnd_elm.sent_type){
                        item_range =
                            DB_dic[grnd_elm.cat].equal_range(mean_elm.obj);
                    }else{
                        app_sent_map.insert(DB_dic[grnd_elm.cat].begin(),DB_dic[grnd_elm.cat].end());
                        item_range =
                            app_sent_map.equal_range(mean_elm.obj);
                    }

                    while(
                        item_range.first !=
                            item_range.second
                    ){
                        Rule word;
                        word = (
                            *(item_range.first)
                        ).second;
                        // (word.internal.front()).obj = in_idx+VARIABLE_NO;
                        //patternsは複数のpatternなので
                        // 直積を作る必要がある．
                        patternDB_it =
                            patternDB.begin();
                        while (
                            patternDB_it !=
                                patternDB.end()
                        ) {
                            PatternType sub_pattern;
                            //すでに作られてる
                        // 単語規則の組をコピー
                            sub_pattern =
                                *patternDB_it;

                            //そこへ新しく
                                // 単語規則を追加
                            sub_pattern.push_back(
                                word
                            );

                            //新しく単語規則が追加された
                            // ルールリストを新しく保存
                            patternDB_buffer.push_back(
                                sub_pattern
                            );

                            patternDB_it++;
                        }

                        item_range.first++;
                    }

                    //変更された単語規則組の列で
                    // 元の単語規則組の列を置き換える
                    patternDB.swap(patternDB_buffer);

                    is_complete &= true;
                    is_absolute &= false;

                }else{
                    ungrounded_variable_num++;

                    if (
                        ungrounded_variable_num >
                            ABSENT_LIMIT ||
                        mean_elm.ch.front() !=
                            1
                    ) {
                        is_applied &= false;
                    } else {
                        if (grnd_elm.is_var()) {
                            std::vector<PatternType>
                                patternDB_buffer;

                            //すでに作られてる
                                // 単語組に対し
                                // 組み合わせの直積の生成
                            patternDB_it =
                                patternDB.begin();
                            while (
                                patternDB_it !=
                                    patternDB.end()
                            ) {

                                //検索した適用可能な
                                // 単語規則列
                                PatternType
                                    sub_pattern;
                                Rule
                                    empty_word;
                                ExType
                                    empty_ex;

                                //空の単語規則を作る
                                empty_word.set_noun(
                                    grnd_elm.cat,
                                    mean_elm,
                                    empty_ex
                                );
                                // empty_word.internal.front().obj = in_idx+VARIABLE_NO;

                                //すでに作られてる
                                // 単語規則の組をコピー
                                sub_pattern =
                                    *patternDB_it;

                                //そこへ新しく
                                    // 単語規則を追加
                                sub_pattern.push_back(
                                    empty_word
                                );

                                //新しく単語規則が
                                // 追加された単語規則組を
                                // 新しく保存
                                patternDB_buffer.push_back(
                                    sub_pattern
                                );

                                //単語規則組が無くなるまで繰り返す
                                patternDB_it++;
                            }
                            //変更された単語規則組の列で
                            // 元の単語規則組の列を置き換える
                            patternDB.swap(
                                patternDB_buffer
                            );
                            is_absolute &= false;
                            is_complete &= false;
                            is_semicomplete &= true;
                        } else {
                            std::cerr <<
                                "pattern error" <<
                                std::endl;
                            throw;
                        }
                    }
                }
            }else{
                is_applied = false;
            }
            grd_idx++;
        }
            // //ある文規則に対して取得できたグラウンドパターンを保存
        if (is_applied) {
            if (is_absolute) {
                ret[ABSOLUTE].insert(
                    ret[ABSOLUTE].end(),
                    patternDB.begin(),
                    patternDB.end()
                );
            } else if (is_complete) {
                ret[COMPLETE].insert(
                    ret[COMPLETE].end(),
                    patternDB.begin(),
                    patternDB.end()
                );
            } else if (is_semicomplete) {
                ret[SEMICOMPLETE].insert(
                    ret[SEMICOMPLETE].end(),
                    patternDB.begin(),
                    patternDB.end()
                );
            }
        }
        //次の文規則を検査
        sent_it++;
    } //文規則のループ*/

    return ret;
}



/* Algorithm
1. 文ルールを一文ずつ回る．
文ルール単位で分岐．文ルールをルールリストへ．
2. symbol列の分配の候補列挙
（各ルールは一文字以上表現する）と同時に文ルールの
symbol部分が一致するか．また，分配の候補で分岐
（文ルールとシンボル列をとって
std::vector< std::vector<Element> >を返す関数を使う）．
分配候補がない場合はパース不可→次の文ルールへ．
3. 各カテゴリ列単位で検査
4. 各カテゴリで与えられたシンボル列を
消費しながらルール集め．
消費の割り振りで分岐（カテゴリとシンボル列を
とって1つ以上のルールとシンボル列のイテレータの
std::pairを返す関数を使う）．
同じ文字の割り振り間で直積を生成．
5. カテゴリ列単位での検査終了．このとき，
シンボル列イテレータがendを指していないリストは
パース不可．symbol列の分配の候補ごとに直積を生成．
6. すべてのカテゴリ列検査終了．
symbol列の分配の候補ごとのルールリストを
ルールリストのリストへ．
7. 生成したルールリストのリストを回って，
各parse_tree生成（ルールリストをとってtreeを示す
std::string生成する関数を使う）．
*/


bool
KnowledgeBase::parse_step(
    ExType::iterator ex_it1,
    ExType::iterator ex_it2,
    int category,
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    >& r_list,
    std::map<int,int>& loop_det
){
    //src.externalがex_it1とex_it2で表現されている

    r_list.clear();
    if(ex_it1 != ex_it2){

        // std::cout << "TARGET PR: " << (*ex_it1).to_s() << " - " << (*(ex_it2-1)).to_s()  << std::endl;

    }
    else{

        // std::cout << "reached to end" << std::endl;
        // std::cout << "FALSE0" << std::endl;

        return false;
    }

    //categoryを適用
    RuleDBType search_db;
    RuleDBType::iterator begin_it, end_it;
    if(category==0){
        if(loop_det.find(0) == loop_det.end()){
            loop_det.insert(
                std::map<int,bool>::value_type(0,1)
            );
        }else{
            loop_det[category] += 1;
        }
        begin_it = sentenceDB.begin();
        end_it   = sentenceDB.end();
    }else{
        if(DB_dic.find(category) == DB_dic.end() || DB_dic.count(category) == 0){
            std::cerr << "[ERROR] DIC not found" << std::endl << "CATEGORY: " << category << std::endl;
            return false;
        }
        if(loop_det.find(category) == loop_det.end()){
            loop_det.insert(
                std::map<int,bool>::value_type(category,1)
            );
        }else{
            loop_det[category] += 1;
        }

        // std::cout << "SEARCH CAT: " << category << std::endl;
        // std::cout << "CAT SIZE: " << DB_dic[category].size() << std::endl;

        std::multimap<int, Rule>::iterator
            dic_it = DB_dic[category].begin();
        while(
            dic_it != DB_dic[category].end()
        ){
            search_db.push_back(
                (*dic_it).second
            );
            dic_it++;
        }
        // std::cout << "DIC_SIZE: " << tmp_db.size() << std::endl;
        begin_it = search_db.begin();
        end_it   = search_db.end();
    }

    bool p_fl = false; //全体
    bool flag; //汎用
    std::vector<
        std::pair<
            std::vector<ExType>,
            ExType::iterator
        >
    > ex_patterns; //分割用
    // Rule rbase; //初期ルール
    // std::vector<Rule> base_list; //初期ルールだけのリスト
    std::vector<
        std::pair<
            std::vector<ExType>,
            ExType::iterator
        >
    >::iterator ex_pattern_it; //分割したシンボル列のパターンを順にチェックするためのイテレータ
    std::vector<ExType>::iterator ex_list_it; //分割したシンボル列を順にチェックするためのイテレータ
    // ExType::iterator ex_it; //対象のシンボル列内を順にチェックするイテレータ
    ExType::iterator base_ex_it; //ベースルールのシンボル列をチェックするイテレータ
    // ExType::iterator first_it, end_it; //ベースルールの中からカテゴリ列を指定する
    // std::vector<
    //     std::pair<
    //         std::vector<Rule>,
    //         ExType::iterator
    //     >
    // > pattern_pairs; //ルールリストとその終了位置の記録．各パターンごとに作成．最後にendになっているリストだけr_listに追加する
    // std::pair<
    //     std::vector<Rule>,
    //     ExType::iterator
    // > base_pair; //rbaseと対象のシンボル列の開始位置を記録する
    // std::vector<
    //     std::pair<
    //         std::vector<Rule>,
    //         ExType::iterator
    //     >
    // > sub_pairs; //分割したシンボル列を埋めるルールリストとその終了位置の記録．各分割シンボル列ごとに作成．最後に埋めるべきシンボル列がすべて埋まっていたらpattern_pairsに追加（pattern_stepではこの限りでない）
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    >::iterator sub_pairs_it; //分割したシンボル列を埋める候補はそれぞれの埋め方をするので，次のカテゴリを試行する際は全通りする必要がある．
    // std::vector<
    //     std::pair<
    //         std::vector<Rule>,
    //         ExType::iterator
    //     >
    // > step_pairs; //parse_stepの受け取り用
    //交換用
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    >::iterator
        main_it, trg_it; //直積のベースとターゲットのイテレータ
    std::vector<
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        >
    > buf; //直積の一時蓄積用，または，入れ替え時の一時蓄積用
    std::pair<
        std::vector<Rule>,
        ExType::iterator
    > tmp_pair; //pairの仮置き
    // std::vector<Rule> tmp_rules; //ルールリストの仮置き
    std::pair<
        std::vector<Rule>,
        ExType::iterator
    > push_pair;
    std::pair<
        std::vector<Rule>,
        ExType::iterator
    > push_pair_b;

    // std::cout << "ExList_It7" << std::endl;

    while(begin_it!=end_it){
        Rule rbase; //初期ルール
        std::vector<Rule> base_list; //初期ルールだけのリスト
        std::pair<
            std::vector<Rule>,
            ExType::iterator
        > base_pair; //rbaseと対象のシンボル列の開始位置を記録する

        // std::cout << "DIC RULE: " << (*begin_it).to_s() << std::endl;

        //表現する対称のシンボル列を
        // 各カテゴリ列に割り振る候補の計算
        rbase=(*begin_it);
        ex_patterns.clear();

        // std::cout << "ExList_ItA" << std::endl;

        flag = parse_option_symbols_no_end(
            rbase,
            ex_it1,
            ex_it2,
            ex_patterns
        );

        // std::cout << "PATTERN SIZE match: " << ex_patterns.size() << std::endl;

        if(!flag){
            begin_it++;

            // std::cout << "FALSE1" << std::endl;

            continue;
        }

        //初期ルールリストtmpにsentenceだけ追加
        base_list.push_back(rbase);
        base_pair.first = base_list;

        //割り振り候補が0の場合はカテゴリ数も0で全シンボルと完全一致しているから次へ
        if(ex_patterns.size() == 0){
            p_fl |= true;
            base_pair.second = ex_it2;
            r_list.push_back(base_pair);
            begin_it++;

            // std::cout << "MATCH2 " << rbase.to_s() << std::endl;

            continue;
        }

        //先頭からの部分一致は完全一致と似たような扱い，1パターンしかない
        if(
            ex_patterns.size() == 1 &&
            ex_patterns.front().first.size() == 1 &&
            (
                ex_patterns.front().first.front().size() == 0
            )
        ){
            p_fl |= true;

            base_pair.second = ex_it1 + rbase.external.size();
            r_list.push_back(base_pair);
            begin_it++;

            // std::cout << "MATCH3 next: " << tmp_pair.second - ex_it1 << std::endl;
            // std::cout << "MATCH3 " << rbase.to_s() << std::endl;

            continue;
        }

        //全ルールリストと位置候補: pairs
        //一文単位の候補: buf
        //symbol列の割り当て単位の候補: options
        //ex_list_it -> ex_pair_it

        //各ex_lilstを
        // 完全に埋めれたものだけr_listに追加する
        ex_pattern_it = ex_patterns.begin();

        // std::cout << "ExList_It9" << std::endl;

        while(
            ex_pattern_it!=ex_patterns.end()
        ){
            std::vector<
                std::pair<
                    std::vector<Rule>,
                    ExType::iterator
                >
            > pattern_pairs; //ルールリストとその終了位置の記録．各パターンごとに作成．最後にendになっているリストだけr_listに追加する

            // std::cout << "THE PATTERN LIST SIZE: " << (*ex_pattern_it).first.size() << std::endl;

            //各カテゴリ列の埋め方でbufのなか分岐
            //初期ルールリストから分岐開始

            base_pair.second = (*ex_pattern_it).first.front().begin();

            ex_list_it = (*ex_pattern_it).first.begin();

            //カテゴリ列特定用
            base_ex_it = rbase.external.begin();

            pattern_pairs.clear();
            pattern_pairs.push_back(base_pair);

            while(ex_list_it != (*ex_pattern_it).first.end()){
                std::vector<
                    std::pair<
                        std::vector<Rule>,
                        ExType::iterator
                    >
                > sub_pairs; //分割したシンボル列を埋めるルールリストとその終了位置の記録．各分割シンボル列ごとに作成．最後に埋めるべきシンボル列がすべて埋まっていたらpattern_pairsに追加（pattern_stepではこの限りでない）
                // std::cout << "PATTERN LOOP: " << rbase.to_s() << std::endl;
                // std::cout << "PATTERN SIZE: " << ex_patterns.size() << std::endl;

                //カテゴリ列の特定
                std::pair<ExType::iterator, ExType::iterator>
                    cat_seq =
                        find_next_cat_seq(
                            base_ex_it,
                            rbase.external.end()
                        )
                ;
                if(cat_seq.first == rbase.external.end()){
                    std::cerr << "Can not find category sequence" << std::endl;
                    throw "Error";
                }

                //各カテゴリ列の各カテゴリを埋める
                flag = true;
                sub_pairs.clear();
                std::vector<Rule> emp_rules;
                std::pair<
                    std::vector<Rule>,
                    ExType::iterator
                > push_pair_a (emp_rules,(*ex_list_it).begin());
                sub_pairs.push_back(push_pair_a);

                // std::cout << "TARGET ch: " << (*push_pair_b.second).to_s() << " - " << (*ex_list_it).back().to_s()  << std::endl;

                while(cat_seq.first != cat_seq.second){

                    // std::cout << "SUB_PAIRS size: " << sub_pairs.size() << std::endl;
                    // std::cout << "CAT LOCATION: 1: " << cat_seq.first - rbase.external.begin() << " 2: " << cat_seq.second - rbase.external.begin()  << std::endl;
                    // std::cout << "check CAT2: " << (*cat_seq.first).to_s() << std::endl;
                    // std::cout << "[sample] TARGET ch2: " << sub_pairs.front().second - (*ex_list_it).begin() << " in " << (*ex_list_it).end() - (*ex_list_it).begin()  << std::endl;

                    //pairsでルールリストと
                    // そのルールリストと消費した位置を管理
                    //(*ex_it)（ExType）を使い切らなければならない
                    buf.clear();
                    int category = (*cat_seq.first).cat;
                    if(
                        loop_det.find(category) == loop_det.end() ||
                        loop_det[category] <= RECURSIVE_LIMIT
                    ){
                        // std::cout << "category: " << category;
                        // std::cout << " Loop: " << loop_det[category];
                        // std::cout << std::endl;

                        std::map<int,int> copy;
                        bool
                            grounded_fl = false;
                        copy = loop_det;
                        main_it=sub_pairs.begin();
                        for(
                            ;
                            main_it != sub_pairs.end();
                            main_it++
                        ){
                            std::vector<
                                std::pair<
                                    std::vector<Rule>,
                                    ExType::iterator
                                >
                            > step_pairs; //parse_stepの受け取り用

                            // std::cout << "covered TARGET: 0 - " << (*main_it).second - (*ex_list_it).begin() << std::endl;
                            // std::cout << "EX length: " << (*ex_list_it).size() << " Iterator differene: " << (*ex_list_it).end() - ((*main_it).second) << std::endl;
                            // std::cout << "TARGET: " << (*ex_list_it).front().to_s() << " - " << (*ex_list_it).back().to_s()  << std::endl;
                            // std::cout << "TARGET: " << (*(*main_it).second).to_s() << " - " << (*ex_list_it).end() - (*main_it).second  << std::endl;

                            step_pairs.clear();
                            grounded_fl =
                                parse_step(
                                    (*main_it).second,
                                    (*ex_list_it).end(),
                                    category,
                                    step_pairs,
                                    copy
                                )
                            ;

                            if(step_pairs.size() > 0){

                                // std::cout << "[sample] NOW: " << step_pairs.front().second - (*ex_list_it).begin() << std::endl << "RESULT: " << grounded_fl << std::endl;

                            }
                            else{

                                // std::cout << "STEP_PAIR FALSE" << std::endl;

                            }

                            if(grounded_fl){
                                trg_it = step_pairs.begin();
                                while(trg_it != step_pairs.end()){
                                    std::vector<Rule> tmp_rules; //ルールリストの仮置き
                                    // std::cout << "LOOP COMBINATION" << std::endl;
                                    // std::cout << "NOW: " << (*trg_it).second - (*ex_list_it).begin() << std::endl;

                                    tmp_rules = (*main_it).first;
                                    tmp_rules.insert(tmp_rules.end(),(*trg_it).first.begin(),(*trg_it).first.end());
                                    std::pair<
                                        std::vector<Rule>,
                                        ExType::iterator
                                    > push_pair (tmp_rules,(*trg_it).second);
                                    buf.push_back(push_pair);
                                    trg_it++;
                                }
                            }
                        }
                        sub_pairs.swap(buf);
                        if(sub_pairs.size() == 0){
                            flag = false;
                            break;
                        }
                    }else{
                        flag=false;
                        sub_pairs.swap(buf);

                        // std::cout << "RECURSIVE_LIMIT" << std::endl;
                        // std::cout << "RECURSIVE_LIMIT" << std::endl;

                        break;
                    }

                    cat_seq.first++;
                }

                // std::cout << "ExList_It14" << std::endl;
                // std::cout << "PARSE STEP RESULT: " << flag << std::endl;
                // std::cout << "PARSE STEP SIZE: " << sub_pairs.size() << std::endl;

                //while抜けた時点でカテゴリを使い切っているので
                // pairのsecondが(*ex_it).end()になっている
                //endになっているものでbufと直積生成
                if(flag){
                    buf.clear();
                    trg_it = sub_pairs.begin();
                    while(trg_it != sub_pairs.end()){ //基本的に埋めきったら追加
                        main_it = pattern_pairs.begin();
                        while(main_it != pattern_pairs.end()){
                            //最後の場合と途中の場合にわける．
                            //最後の場合はパターンのsecondをpointerとして使えばいい (埋めきっていない場合はpointerをずらす)
                            //途中の場合はsecondを無視
                            if(
                                (*trg_it).second == (*ex_list_it).end() ||
                                ex_list_it == (*ex_pattern_it).first.end() - 1
                            ){
                                if(ex_list_it == (*ex_pattern_it).first.end() - 1){
                                    if(cat_seq.second == rbase.external.end() && rbase.external.back().is_cat()){//カテゴリで終わる
                                        std::pair<
                                            std::vector<Rule>,
                                            ExType::iterator
                                        > push_pair_c ((*main_it).first, (*ex_pattern_it).second - ((*ex_list_it).end() - (*trg_it).second));//
                                        push_pair_c.first.insert(push_pair_c.first.end(),(*trg_it).first.begin(),(*trg_it).first.end());

                                        // std::cout << "FILLED: " << (*ex_pattern_it).second - ((*ex_list_it).end() - (*trg_it).second) - ex_it1 << std::endl;

                                        // std::cout << std::endl << "[s pattern_pairs3]Additional rules" << std::endl;
                                        // std::vector<Rule>::iterator out_it;
                                        // for(out_it = push_pair_c.first.begin();out_it != push_pair_c.first.end();out_it++){
                                        //     std::cout << (*out_it).to_s() << std::endl;
                                        // }
                                        // std::cout << "[END]" << std::endl << std::endl;

                                        buf.push_back(push_pair_c);
                                    }else{//終端記号で終わる
                                        std::pair<
                                            std::vector<Rule>,
                                            ExType::iterator
                                        > push_pair_c ((*main_it).first, (*ex_pattern_it).second );//
                                        push_pair_c.first.insert(push_pair_c.first.end(),(*trg_it).first.begin(),(*trg_it).first.end());

                                        // std::cout << "FILLED: " << (*ex_pattern_it).second - ex_it1 << std::endl;

                                        // std::cout << std::endl << "[s pattern_pairs1]Additional rules" << std::endl;
                                        // std::vector<Rule>::iterator out_it;
                                        // for(out_it = push_pair_c.first.begin();out_it != push_pair_c.first.end();out_it++){
                                        //     std::cout << (*out_it).to_s() << std::endl;
                                        // }
                                        // std::cout << "[END]" << std::endl << std::endl;

                                        buf.push_back(push_pair_c);
                                    }
                                }else{//途中経過
                                    std::pair<
                                        std::vector<Rule>,
                                        ExType::iterator
                                    > push_pair_c ((*main_it).first, (*(ex_list_it+1)).begin() );//
                                    push_pair_c.first.insert(push_pair_c.first.end(),(*trg_it).first.begin(),(*trg_it).first.end());

                                    buf.push_back(push_pair_c);
                                }
                            }
                            main_it++;
                        }
                        trg_it++;
                    }
                }else{

                    //埋められなかったら
                    // この割り振りでは埋めきれないためbufをクリア
                    //pairsのサイズゼロ

                    // std::cout << "FALSE4" << std::endl;
                    buf.clear();
                    pattern_pairs.swap(buf);
                    break;
                }
                pattern_pairs.swap(buf);
                ex_list_it++;

                // std::cout << "sent_ex_it loop end" << std::endl;

                base_ex_it = cat_seq.second;
            }

            // std::cout << "ExList_It12" << std::endl;

            //埋め方全パターンbufをr_listへ追加

            // std::cout << "PATTERN LIST SIZE: " << pattern_pairs.size() << std::endl;

            if(pattern_pairs.size() > 0){

                // std::cout << "[sample] PATTERNS SIZE: " << pattern_pairs.front().first.size() << std::endl;

            }

            if(pattern_pairs.size() > 0){
                p_fl |= true;

                //secondがex_it2であればr_listに追加
                for(trg_it = pattern_pairs.begin();trg_it != pattern_pairs.end();trg_it++){
                    if( true ){
                        std::vector<Rule> push_rules;
                        push_rules = (*trg_it).first;

                        // std::cout << std::endl << "[s r_list]Additional rules: " << rbase.to_s() << std::endl;
                        // std::vector<Rule>::iterator out_it;
                        // for(out_it = push_rules.begin();out_it != push_rules.end();out_it++){
                        //     std::cout << (*out_it).to_s() << std::endl;
                        // }
                        // std::cout << "[END]" << std::endl << std::endl;

                        std::pair<
                            std::vector<Rule>,
                            ExType::iterator
                        > push_pair (push_rules, (*trg_it).second);
                        r_list.push_back(
                            push_pair
                        );
                    }
                }

                // std::cout << "MATCH5" << std::endl;

            }

            ex_pattern_it++;
        }
        // std::cout << "ExList_It10" << std::endl;

        begin_it++;
    }
    // std::cout << "ExList_It8" << std::endl;

    // std::cout << "PARSE STEP END" << std::endl;

    return p_fl;
}

bool
KnowledgeBase::parse_option_symbols(
    Rule& base,
    ExType& ex,
    std::vector<
        std::vector<ExType>
    >& ex_lists
){
    bool flag = false;
    ex_lists.clear();

    // std::cout << "OPTION TARGET: " << base.to_s() << std::endl;
    // Rule out_ex;out_ex.type = RULE_TYPE::NOUN;
    // out_ex.external = ex;
    // std::cout << "OPTION EX: " << out_ex.to_s() << std::endl;

    //マッチすべきシンボル列の抽出
    ExType::iterator base_it =
        base.external.begin();//*sent_it
    std::vector<ExType>
        match_symbols;//一致すべきシンボル列
    {
        ExType tmp;
        while(base_it != base.external.end()){
            if((*base_it).is_sym()){
                tmp.push_back((*base_it));
            }else if(tmp.size()>0){
                ExType tmp2 = tmp;
                match_symbols.push_back(tmp2);
                tmp.clear();
            }
            base_it++;
        }
        if(tmp.size()>0){
            match_symbols.push_back(tmp);
        }
    }

    //完全一致した場合は割り当て候補は存在しない
    if(
        base.external == ex &&
        match_symbols.size() == 1 &&
        (*ex.begin()).is_sym()
    ){
        // std::cout << "NO OPTION" << std::endl;
        return true;
    }



    // std::cout << "M_SYM: " << match_symbols.size() << std::endl;
    // std::vector<std::vector<Extype> > symbols_list;
    //マッチすべきシンボルをどこにするかで
    // カテゴリ列に割り振られるシンボル列のパターン作成
    std::vector<
        std::pair<
            std::vector<ExType>,
            ExType::iterator
        >
    > options;
    if(match_symbols.size() > 0){
        std::vector<ExType>::iterator
            match_symbols_it = match_symbols.begin();
        std::pair<
            std::vector<ExType>,
            ExType::iterator
        >
            option;
        std::vector<
            std::pair<
                std::vector<ExType>,
                ExType::iterator
            >
        > buf;
        ExType::iterator targ_it;

        //最初のマッチすべき文字列があるかも検出
        targ_it = std::search(
            ex.begin(),
            ex.end(),
            (*match_symbols_it).begin(),
            (*match_symbols_it).end()
        );

        //最初にマッチする場所検出して
        // *sent_itの最初の文字が
        // シンボルの場合文頭で
        // ちゃんと一致しているか検出
        if(
            targ_it == ex.end()||
            (
                base.external.front().is_sym() &&
                targ_it != ex.begin()
            )
        ){
            // std::cout << "FALSE" << std::endl;
            return false;
        }

        //割り当てのシンボル列の数は，
        // 最初がシンボルであれば
        // 最初がカテゴリより一つ少ない
        //最後がシンボルであれば
        // 最後がカテゴリより一つ少ない
        //最初がシンボル，最後がカテゴリ，
        // または，この逆の組み合わせでマッチすべき
        // シンボル列と割り当てのシンボル列の
        // 要素数は等しくなる

        //分岐する最初の情報optionを構築
        //最初の文字がシンボルの場合，
        // マッチすべきシンボル列の数より
        // 割り当てのシンボル列の数は
        // 少なくとも一つ少ないからそのためのずらす処理
        if(targ_it == ex.begin() && base.external.front().is_sym()){

            targ_it += (*match_symbols_it).size();
            // targ_it ++;

            match_symbols_it++;
            ExType::iterator sub_tmp;
            sub_tmp = targ_it;
            option.second = sub_tmp;
        }else{
            //最初の文字がカテゴリの場合，
            ExType::iterator sub_tmp;
            sub_tmp = ex.begin();
            option.second = sub_tmp;
        }

        //最初の情報を追加
        options.push_back(option);

        //各マッチすべきシンボル列ごとに
        // ルールの組み合わせを検証
        //各マッチすべきシンボル列ごとに分岐
        while(
            match_symbols_it != match_symbols.end()
        ){
            //作った分岐によってoptionsを
            // 汚染しないようにbufにためる
            buf.clear();
            std::vector<
                std::pair<
                    std::vector<ExType>,
                    ExType::iterator
                >
            >::iterator op_it;
            //targ_itは表現したい
            // シンボル列の位置を示す
            op_it = options.begin();
            while(op_it != options.end()){
                //現在位置をコピー．
                // 分岐で汚染されないようにコピーしておく．
                targ_it = (*op_it).second;

                //まだマッチすべきシンボル列が残っているのに
                // endになっている場合は
                // このoptionは不可だったと判断する
                if(targ_it == ex.end()){
                    op_it++;
                    continue;
                }

                //検索してその結果が
                // endでなければtmp_optionに
                // シンボル列と次の検索開始位置を記録
                while(
                    (
                        targ_it = std::search(
                            targ_it,
                            ex.end(),
                            (*match_symbols_it).begin(),
                            (*match_symbols_it).end()
                        )
                    ) != ex.end()
                ){
                    if(targ_it == (*op_it).second){
                        targ_it++;
                        continue;
                    }
                    //tmp_option作成
                    std::pair<std::vector<ExType>,ExType::iterator> tmp_option;
                    tmp_option = (*op_it);
                    tmp_option.first.push_back(
                        ExType(
                            (*op_it).second,
                            targ_it
                        )
                    );
                    ExType::iterator sub_tmp1;
                    sub_tmp1 =
                        targ_it + (*match_symbols_it).size();
                    tmp_option.second = sub_tmp1;

                    //このループはbaseがシンボルで終わることを
                        // 想定しているのでカテゴリで終わる
                        // 場合は最後までのシンボル列を追加する
                    //最後がカテゴリの時の追加の条件は，
                        // マッチすべきシンボル列のループが
                        // 最後であり，baseがカテゴリで終わっていて，
                        // 検索語のtarg_itの位置がendではないときとなる
                    //検索したシンボル列分だけ位置を動かす

                    // targ_it += (*match_symbols_it).size();
                    targ_it++;

                    if(
                        match_symbols_it == (match_symbols.end()-1) &&
                        (*base.external.rbegin()).is_cat() &&
                        targ_it != ex.end()
                    ){
                        tmp_option.first.push_back(
                            ExType(targ_it,ex.end())
                        );
                        ExType::iterator sub_tmp2;
                        sub_tmp2 = ex.end();
                        tmp_option.second = sub_tmp2;
                    }

                    //bufにtmp_optionをため込む
                    buf.push_back(tmp_option);
                }

                //次のoptionで(*match_symbols_it)をチェック
                op_it++;
            }
            //できたbufでoptionを置き換える
            options.swap(buf);

            //次のマッチすべきシンボル列へ
            match_symbols_it++;
        }
    }else{
        ExType tmp;
        tmp = ex;
        std::vector<ExType> ttmp;
        ttmp.push_back(tmp);
        ex_lists.push_back(ttmp);
        // std::cout << "OPTION: " << out_ex.to_s() << std::endl;
        return true;
    }
    {
        std::vector<
            std::pair<
                std::vector<ExType>,
                ExType::iterator
            >
        >::iterator it =
            options.begin();
        while(
            it != options.end()
        ){
            if(
                (*it).second == ex.end()
            ){
                flag |= true;
                // std::cout << "OPTION No. " << it - options.begin() << std::endl;
                // std::vector<ExType>::iterator it_it = (*it).first.begin();
                // for(;it_it != (*it).first.end();it_it++){
                //     out_ex.external = (*it_it);
                //     std::cout << "OPTION: " << out_ex.to_s() << std::endl;
                // }
                ex_lists.push_back(
                    (*it).first
                );
            }
        }
    }

    return flag;
}

bool
KnowledgeBase::parse_option_symbols_no_end(Rule& base, ExType::iterator ex_it1, ExType::iterator ex_it2, std::vector<std::pair<std::vector<ExType>,ExType::iterator > >& options){
    bool flag = false;
    options.clear();

    //src.external : ex_it1~ex_it2
    // std::cout << "OPTION TARGET: " << base.to_s() << std::endl;
    // Rule out_ex;out_ex.type = RULE_TYPE::NOUN;
    // out_ex.external = ExType(ex_it1,ex_it2);
    // std::cout << "OPTION EX: " << out_ex.to_s() << std::endl;

    //マッチすべきシンボル列の抽出
    ExType::iterator base_it = base.external.begin();//*sent_it
    std::vector<ExType> match_symbols;//一致すべきシンボル列
    {
        ExType tmp;
        while(base_it != base.external.end()){
            if((*base_it).is_sym()){
                tmp.push_back((*base_it));
            }else if(tmp.size()>0){
                ExType tmp2 = tmp;
                match_symbols.push_back(tmp2);
                tmp.clear();
            }
            base_it++;
        }
        if(tmp.size()>0){
            match_symbols.push_back(tmp);
        }
    }

    // std::cout << "OPTION TARGET: " << base.to_s() << std::endl;
    // Rule out;out.type =RULE_TYPE::NOUN;
    // out.external = ExType(ex_it1,ex_it2);
    // std::cout << "OPTION Ex: " << out.to_s() << std::endl;
    // std::cout << "M_SYM: " << match_symbols.size() << std::endl;

    //完全一致した場合は割り当て候補は存在しない
    if(base.external == ExType(ex_it1,ex_it2) && match_symbols.size() == 1 && (*ex_it1).is_sym()){
        // std::cout << "NO OPTION" << std::endl;
        return true;
    }

    // std::vector<std::vector<Extype> > symbols_list; //分配リスト保持
    //マッチすべきシンボルをどこにするかでカテゴリ列に割り振られるシンボル列のパターン作成
    if(match_symbols.size()>0){
        std::vector<ExType>::iterator match_symbols_it = match_symbols.begin();
        std::pair<std::vector<ExType>,ExType::iterator> option, tmp_option;
        std::vector<std::pair<std::vector<ExType>,ExType::iterator> > buf;
        ExType::iterator targ_it, old_it;

        // std::cout << "M_SYM: " << match_symbols.size() << std::endl;


        //最初のマッチすべき文字列があるかも検出
        targ_it = std::search(ex_it1, ex_it2, (*match_symbols_it).begin(),(*match_symbols_it).end());

        // std::cout << "First M_NUM: " << targ_it - ex_it1 << std::endl;

        //最初にマッチする場所検出して*sent_itの最初の文字がシンボルの場合文頭でちゃんと一致しているか検出
        if( targ_it == ex_it2 ||
            (
                base.external.front().is_sym() && targ_it != ex_it1
            )/* ||
            (
                match_symbols.size() == 1 &&
                !base.external.back().is_cat() &&
                (*match_symbols_it).size() != base.external.size() &&
                base.external.front().is_sym() &&
                targ_it != ex_it1
            )*/
        ){
            // std::cout << "FALSE" << std::endl;
            return false;
        }

        //割り当てのシンボル列の数は，最初がシンボルであれば最初がカテゴリより一つ少ない
        //最後がシンボルであれば最後がカテゴリより一つ少ない
        //最初がシンボル，最後がカテゴリ，または，この逆の組み合わせでマッチすべきシンボル列と割り当てのシンボル列の要素数は等しくなる

        //分岐する最初の情報optionを構築
        //最初の文字がシンボルの場合，マッチすべきシンボル列の数より割り当てのシンボル列の数は少なくとも一つ少ないからそのためのずらす処理
        if( targ_it == ex_it1 && base.external.front().is_sym() ){

            targ_it += (*match_symbols_it).size();
            // targ_it ++;

            // option.second = targ_it;
            if( (*match_symbols_it).size() == base.external.size() ){//全部シンボルの前方一致
                ExType empty_els;
                option.first.push_back(empty_els);
                option.second = targ_it;
                match_symbols_it++;
            }else if(  match_symbols.size() == 1 && base.external.back().is_cat()  ){//
                ExType empty_els(ex_it1 + (*match_symbols_it).size(), ex_it2);
                // out_ex.external = empty_els;
                // std::cout << "cat last" << out_ex.to_s() << std::endl;
                option.first.push_back(empty_els);
                option.second = ex_it2;
                match_symbols_it++;
            }else{
                option.second = targ_it;
                match_symbols_it++;
            }
        }else{
            //最初の文字がカテゴリの場合，
            option.second = ex_it1;
        }

        //最初の情報を追加
        options.push_back(option);

        //各マッチすべきシンボル列ごとにルールの組み合わせを検証
        //各マッチすべきシンボル列ごとに分岐
        while(match_symbols_it != match_symbols.end()){
            //作った分岐によってoptionsを汚染しないようにbufにためる
            buf.clear();
            std::vector<std::pair<std::vector<ExType>,ExType::iterator> >::iterator op_it;
            //targ_itは表現したいシンボル列の位置を示す
            op_it = options.begin();
            while( op_it != options.end() ){
                //現在位置をコピー．分岐で汚染されないようにコピーしておく．
                targ_it = (*op_it).second;
                old_it = (*op_it).second;

                //まだマッチすべきシンボル列が残っているのにendになっている場合はこのoptionは不可だったと判断する
                if(targ_it == ex_it2){
                    op_it++;
                    continue;
                }

                //検索してその結果がendでなければtmp_optionにシンボル列と次の検索開始位置を記録
                while(  ( targ_it = std::search(targ_it, ex_it2, (*match_symbols_it).begin(),(*match_symbols_it).end()) ) != ex_it2 ){
                    if(targ_it == (*op_it).second){
                        targ_it++;
                        continue;
                    }
                    //tmp_option作成
                    std::pair<std::vector<ExType>,ExType::iterator> tmp_option;
                    tmp_option = (*op_it);
                    tmp_option.first.push_back(ExType((*op_it).second,targ_it));
                    tmp_option.second = targ_it + (*match_symbols_it).size();

                    //このループはbaseがシンボルで終わることを想定しているのでカテゴリで終わる場合は最後までのシンボル列を追加する
                    //最後がカテゴリの時の追加の条件は，マッチすべきシンボル列のループが最後であり，baseがカテゴリで終わっていて，検索語のtarg_itの位置がendではないときとなる
                    //検索したシンボル列分だけ位置を動かす

                    // targ_it += (*match_symbols_it).size();
                    targ_it ++;

                    if(match_symbols_it == (match_symbols.end()-1) && base.external.back().is_cat() && targ_it != ex_it2){
                        tmp_option.first.push_back(ExType(targ_it,ex_it2));
                        tmp_option.second = ex_it2;
                    }

                    //bufにtmp_optionをため込む
                    buf.push_back(tmp_option);
                }

                //次のoptionで(*match_symbols_it)をチェック
                op_it++;
            }
            //できたbufでoptionを置き換える
            options.swap(buf);

            //次のマッチすべきシンボル列へ
            match_symbols_it++;
        }
    }else{
        ExType tmp(ex_it1,ex_it2);
        std::pair<std::vector<ExType>,ExType::iterator > ttmp;
        ttmp.first.push_back(tmp);
        ttmp.second = ex_it2;
        options.push_back(ttmp);
        // out_ex.external = tmp;
        // std::cout << "OPTION: " << out_ex.to_s() << std::endl;
        return true;
    }
    if(options.size() > 0){
        // std::vector<std::pair<std::vector<ExType>,ExType::iterator > >::iterator out_op_it;
        // out_op_it = options.begin();
        // for(;out_op_it != options.end(); out_op_it++){
        //     std::cout << "OPTION No. " << out_op_it - options.begin() + 1 << std::endl;
        //     std::cout << "OPTION location " << (*out_op_it).second - ex_it1 + 1 << std::endl;
        //     std::vector<ExType>::iterator out_ex_it;
        //     out_ex_it = (*out_op_it).first.begin();
        //     for(;out_ex_it !=(*out_op_it).first.end();out_ex_it++){
        //         out_ex.external = *out_ex_it;
        //         std::cout << "OPTION: " << out_ex.to_s() << std::endl;
        //     }
        // }
        return true;
    }

    return flag;
}

int
KnowledgeBase::next_category(
    ExType::iterator it1,
    ExType::iterator it2
){
    int count=0;
    while((it1+count) != it2){
        if((*(it1+count)).is_cat()){
            return count;
        }
        count++;
    }
    return -1;
}

int
KnowledgeBase::next_symbol(
    ExType::iterator it1,
    ExType::iterator it2
){
    int count=0;
    while((it1+count) != it2){
        if((*(it1+count)).is_sym()){
            return count;
        }
        count++;
    }
    return -1;
}

void
KnowledgeBase::knowledge_tree(std::string& tree_str){

}

std::string
KnowledgeBase::to_s(void) {
    std::vector<Rule> rule_buf;
    std::vector<std::string> buf;
    std::vector<Rule>::iterator it;
    std::string sbuf;
    int count;

    rule_buf.clear();
    sbuf = std::string("\nBOX\n");
    buf.push_back(sbuf);
    rule_buf = box_buffer;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    count=0;
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        count ++;
        buf.push_back(std::to_string(count)+std::string(": ")+(*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nInput BOX\n");
    buf.push_back(sbuf);
    rule_buf = input_box;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    count=0;
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        count++;
        buf.push_back(std::to_string(count)+std::string(": ")+(*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nSent DB\n");
    buf.push_back(sbuf);
    rule_buf = sentenceDB;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    count=0;
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        count++;
        buf.push_back(std::to_string(count)+std::string(": ")+(*it).to_s());
        it++;
    }

    rule_buf.clear();
    sbuf = std::string("\nWord DB\n");
    buf.push_back(sbuf);
    rule_buf = wordDB;
    //	std::sort(rule_buf.begin(), rule_buf.end(), RuleSort());
    count=0;
    it = rule_buf.begin();
    while (it != rule_buf.end()) {
        count++;
        buf.push_back(std::to_string(count)+std::string(": ")+(*it).to_s());
        it++;
    }

    return boost::algorithm::join(buf, "\n");
}

void
KnowledgeBase::logging_on(void) {
    LOGGING_FLAG = true;
}

void
KnowledgeBase::logging_off(void) {
    LOGGING_FLAG = false;
}

void
KnowledgeBase::set_control(uint32_t FLAGS) {
    CONTROLS |= FLAGS;
}

std::vector<Rule>
KnowledgeBase::rules(void) {
    std::vector<Rule> kb_all;
    std::vector<Rule>::iterator sentenceDB_it, wordDB_it;
    sentenceDB_it = sentenceDB.begin();
    for (; sentenceDB_it != sentenceDB.end(); sentenceDB_it++) {
        kb_all.push_back(*sentenceDB_it);
    }
    wordDB_it = wordDB.begin();
    for (; wordDB_it != wordDB.end(); wordDB_it++) {
        kb_all.push_back(*wordDB_it);
    }

    return kb_all;
}

std::pair<KnowledgeBase::ExType::iterator, KnowledgeBase::ExType::iterator>
KnowledgeBase::find_next_cat_seq(ExType::iterator begin_it, ExType::iterator end_it){
    std::pair<ExType::iterator, ExType::iterator> it_pair;
    int tmp =
        next_category(
            begin_it,
            end_it
        )
    ;
    if(tmp == -1){
        it_pair.first = end_it;
        it_pair.second = end_it;
        return it_pair;
    }//これになるとparse_option_symbols関数が間違っている
    it_pair.first =
        begin_it + tmp;
    int tmp2 =
        next_symbol(
            begin_it + tmp,
            end_it
        )
    ;
    if(tmp2 == -1){
        it_pair.second = end_it;
    }else{
        it_pair.second = begin_it + tmp + tmp2;
    }
    return it_pair;
}

bool
KnowledgeBase::explain(InType ref, std::vector<Rule>& res){
    std::vector<PatternType> groundable_patterns;
    std::map<
        PATTERN_TYPE,
        std::vector<PatternType>
    > all_patterns;
    Rule src;
    int rand_index;
    src.internal = ref;
    PatternType target_pattern;

    //pattern作成
    all_patterns =
        construct_grounding_patterns_keep_id(src);
        // construct_grounding_patterns(src);
    if (
        all_patterns[COMPLETE].size() != 0
    ) {
        rand_index =
            MT19937::irand() %
                all_patterns[COMPLETE].size();

        res =
            (
                all_patterns[COMPLETE]
            )[rand_index];
        return true;
    } else{
        return false;
    }
}

void
KnowledgeBase::define(Element a, Conception m){
    intention.store(a,m);
    // std::cout << "Define size: "<< intention.mapping.size() << std::endl;
}

void
KnowledgeBase::init_semantics_rules(TransRules& obj){
    intention.init_rules(obj);
}

std::vector<Element>
KnowledgeBase::meaning_no(int obj){
    Element e;
    e.set_ind(obj);
    return intention.trans(e);
}

//typedef std::map<int, std::multimap<int, Rule> > DicDBType
void
KnowledgeBase::dic_erase(DicDBType& dic, Rule r){
    int cat = r.cat;
    int ind = r.internal.front().obj;
    if(dic.count(cat) == 0 || dic[cat].count(ind) == 0){
        std::cerr << "no entry" << r.to_s() << std::endl;
        return;
    }
    auto dic_r = dic[cat].equal_range(ind);
    for(auto& it = dic_r.first; it != dic_r.second; it++){
        if((*it).second == r){
            dic[cat].erase(it);
            break;
        }
    }
}

void
KnowledgeBase::dic_add(DicDBType& dic, std::vector<Rule>& vec_r){
    for(auto& r : vec_r){
        dic_add(dic,r);
    }
}

void
KnowledgeBase::dic_add(DicDBType& dic, Rule r){
    
    // std::cout << "DIC: " << r.to_s() << std::endl;
    
    int cat = r.cat;
    int ind = r.internal.front().obj;
    typedef std::multimap<int, Rule> ItemType;
    if(dic.count(cat) == 0){
        ItemType tmp;
        dic.insert(std::make_pair(cat,tmp));
    }
    std::pair<const int, Rule> pp = std::make_pair(ind,r);
    if(std::find(dic[cat].begin(),dic[cat].end(),pp)==dic[cat].end()){
        dic[cat].insert(pp);
    }
    // std::cout << (*dic[cat].equal_range(ind).first).second.to_s() << std::endl;
}

void
KnowledgeBase::dic_change_cat(DicDBType& dic, int cat, int tocat){

    // std::cout << "\n****************test check16" << std::endl;

    if(dic.count(cat) == 0){
        std::cerr << "no entry [change cat]" << std::endl;
        throw "no entry [change cat]";
    }
    // dic[tocat].insert(dic[cat].begin(),dic[cat].end());
    // dic.erase(cat);
    Rule ttmp;
    std::vector<Rule> tmp;
    //cat抽出
    for(auto it = dic[cat].begin();it != dic[cat].end();it++){
        ttmp = (*it).second;
        tmp.push_back(ttmp);
    }
    //cat削除
    dic.erase(cat);
    //Rule自体のcategoryをtocatへ
    for(auto r : tmp){
        r.cat = tocat;
        dic_add(dic,r);
    }

    // std::cout << "\n****************test check17 " << cat << " " << tocat << std::endl;

    //発話にcatが入っていればtocatに書き換え
    tmp.clear();
    std::vector<Rule> tmp2;
    for(auto& mm : dic){//pair(cat,multimap(ind,Rule))
        for(auto& r : mm.second){//pair(ind,Rule)
            Rule r2 = r.second;
            bool flag = false;
            //search対象
            Element el;
            el.set_cat(0,cat,false);
            //初期search
            auto it = std::find(r2.external.begin(),r2.external.end(),el);
            //見つかったらinternalも書き換える
            while(it != r2.external.end()){

                // std::cout << "\n****************test check18 " << r2.to_s() << std::endl;

                flag = true;
                //external埋め込み用
                Element el2;
                el2.set_cat((*it).obj,tocat,(*it).sent_type);
                *it = el2;

                // std::cout << "\n****************test check18 end" << std::endl;

                // std::cout << "\n****************test check19" << std::endl;

                //internalサーチ用，internal埋め込み用
                Element el3, el4;
                el3.set_var((*it).obj,cat);
                el4.set_var((*it).obj,tocat);

                // std::cout << "\n****************test check19 end" << std::endl;

                auto in_it = std::find(r2.internal.begin(),r2.internal.end(),el3);

                // std::cout << "\n****************test check20 " << (in_it == r2.internal.end()) << std::endl;

                if((*in_it).sent_type){
                    *in_it = el4;
                    (*in_it).sent_type = true;
                }else{
                    *in_it = el4;
                }

                // std::cout << "\n****************test check20 end" << std::endl;


                // std::cout << "\n****************test check21" << std::endl;

                //次の値をサーチ
                it = std::find(it+1,r2.external.end(),el);

                // std::cout << "\n****************test check21 end" << std::endl;

            }
            
            if(flag){
                //erase list
                tmp.push_back(r.second);
                //add list
                tmp2.push_back(r2);
            }
        }
    }

    // std::cout << "\n****************test check17 end" << std::endl;

    if(tmp.size() != tmp2.size()){
        std::cerr << "ALERT" << std::endl;
    }

    // std::cout << "\n****************test check16 end" << std::endl;

    for(int i=0; i < tmp.size();i++){
        dic_erase(dic,tmp[i]);
        dic_add(dic,tmp2[i]);
    }


}

void
KnowledgeBase::dic_change_ind(DicDBType& dic, int ind, int toind){
    // std::cout << "From : " << ind << std::endl << "To : " << toind << std::endl;
    std::vector<Rule> tmp;
    Rule ttmp;
    for(auto& mm : dic){
        tmp.clear();
        auto rr = mm.second.equal_range(ind);
        for(auto it = rr.first;it != rr.second;it++){
            // mm.second.insert(std::make_pair(toind, (*it).second));
            ttmp = (*it).second;
            tmp.push_back(ttmp);
            // std::cout << (*it).second.to_s() << std::endl;
        }
        mm.second.erase(rr.first,rr.second);
        for(auto r : tmp){
            // mm.second.insert(std::make_pair(toind, r));
            r.internal.front().obj = toind;
            dic_add(dic,r);
        }
    }
}

// void
// KnowledgeBase::dic_unify(DicDBType& dic){
//     // std::cout << "From : " << ind << std::endl << "To : " << toind << std::endl;
//     for(auto& mm : dic){
//         for(auto& r : mm.second){

//         }
//     }
// }

//3つの流れ（invent with conditions, remap meaning for music score, make concepts for transfer）
std::vector<Rule>
KnowledgeBase::generate_score(int beat_num, std::map<int, std::vector<std::string> >& core_meaning){
    std::vector<Rule> res;
    //invention
    //1.sをランダムに選ぶ. stateをbeat_numにしておく.
    //2.要素を順番にチェック(create_measures(res,cat,beat_num))．symbolががでてくるかひとつでもfalseなら1へ戻る．symbolが出たら1へ戻る．
    //3.最初にできたものをgenerateしたものとする．できなかった場合は，std::vector<Rule>()を返す.
    {
      int rand_index,i;
        bool creatable = false;
        RuleDBType temp = sentenceDB;
        std::vector<Rule> work_list;
        for(i = 0; i < sentenceDB.size(); i++){
            bool suc=true;
            work_list.clear();
            rand_index = MT19937::irand() % temp.size();
            Rule base_r = temp[rand_index];
            work_list.push_back(base_r);
            for(auto& ex_el : base_r.external){
                if(!create_measures(work_list, ex_el, beat_num)){
                    work_list.clear();
                    suc=false;
                    break;
                }
            }
            if(suc){
                break;
            }

            temp.erase(temp.begin() + rand_index);
        }
        if(i != sentenceDB.size()){
            res = work_list;
            creatable = true;
        }

    }
    //remaping and making concepts
    {

    }


    return res;
}

//measureがひとつ以上でるように好きに組み立てる
bool
KnowledgeBase::create_measures(std::vector<Rule>& res, Element& cat_num, int beat_num){
    //1.cat_numに基づいてランダムにルールを選択
    //2.measureであればそのルールのexternalをチェック（create_beats(res,external,beat_num)）．falseであれば1へ戻る.
    //2.measureでなければ各要素をcreate_measures(res,cat,beat_num)でチェック．symbolがでてくるか一つでもfalseであれば1へ戻る. 
    //3.すべての候補を試す前にここまでくればtrueを返す．
    //(
    //    DB_dic.find(grnd_elm.cat) != DB_dic.end() && //変数のカテゴリが辞書に有り
    //    DB_dic[grnd_elm.cat].find(mean_elm.obj) != DB_dic[grnd_elm.cat].end() //辞書の指定カテゴリに単語がある
    //) ||
    //(grnd_elm.sent_type && app_sent_map.find(mean_elm.obj) != app_sent_map.end() ) //sentenceの方に適用可能ルールがある
    // DictionaryRange item_range =DB_dic[grnd_elm.cat].equal_range(mean_elm.obj);
}

//beat_numの制約を満たすように好きに組み立てる
bool
KnowledgeBase::create_beats(std::vector<Rule>& res, std::vector<Rule> external, int beat_num){
    //1.state==beat_numであればfalseを返す．
    //2.cat_numに基づいてランダムにルールを選択.
    //3.measureであるルールなら2に戻る．要素数がbeat_numより大きければ2に戻る．
    //4.各要素をチェック．
    //5.1.symbolの分だけstateをインクリメント
    //5.2.1categoryの数がbeat_num-stateより大きければ2に戻る.
    //5.2.2各categoryへのbeat_num-stateの整数分配パターンを列挙する.
    //5.2.3分配パターンからランダムに選択する．
    //5.2.4各categoryでランダムにルールを選択．
    //5.2.5そのexternalをcreate_beats(res,sub_external,pattern[i])でチェック．falseであれば5.2.4に戻る．
    //5.2.6すべての候補でfalseであれば5.2.3に戻る．
    //5.2.7すべての分配パターンを試す前にここまでくればtrueを返す．
    //6.すべての2の候補が終わってしまったらfalseを返す．
}

std::string
KnowledgeBase::meaning_no_to_s(int obj){
    return "["+intention[obj].to_s()+"]";
}

#ifdef DEBUG_KB

void
load_input_data(std::vector<std::string>& buf, std::string& file_path){
    //存在しない場合，中身がない場合はすぐreturn
    const boost::filesystem::path path(file_path.c_str());
    boost::system::error_code error;
    const bool result = boost::filesystem::exists(path,error);
    if(!result || error){
        return;
    }else{
        std::string line;
        std::ifstream ifs(file_path.c_str());
        while(std::getline(ifs, line)){
            boost::algorithm::trim_if(line, boost::algorithm::is_any_of("\r\n "));
            // std::cout << line << std::endl;
            buf.push_back(line);
        }
    }
}

int main(int arg, char **argv) {

    Dictionary dic;
    XMLreader reader;
    std::string xml_file = "./test.xml";
    std::string dic_file = "./data.dic";
    // std::string input_file = "./input.txt";
    // std::string alias_file = "./alias.data";
    // std::string dic_xml_file = "./dic_xml.data";
    // std::string xml_dir = "../XML";
    std::map<int, std::multimap<int, Rule> >::iterator dit;
    std::multimap<int, Rule>::iterator item_it;
    Conception empty_m;

    LogBox::set_filepath("./test_log.txt");

    // std::vector<std::string> inputs;
    // load_input_data(inputs, input_file);

    // if(inputs.size() == 0){
    //     std::vector<std::string> file_list;
    //     const boost::filesystem::path path(xml_dir.c_str());
    //     BOOST_FOREACH(const boost::filesystem::path& p, std::make_pair(boost::filesystem::directory_iterator(path),
    //                                                 boost::filesystem::directory_iterator())) {
    //         if(!boost::filesystem::is_directory(p)){
    //             file_list.push_back(xml_dir + "/" + p.filename().generic_string());
    //             std::cout << xml_dir + "/" + p.filename().generic_string() << std::endl;
    //         }
    //     }

    //     reader.make_data(file_list,input_file,dic_file,alias_file,dic_xml_file);
    //     load_input_data(inputs, input_file);
    // }else{
    //     reader.alias_load(alias_file);
    //     reader.dic_xml_load(dic_xml_file);
    // }
    dic.load(dic_file);
    Rule buf;
    KnowledgeBase kb;
    std::vector<Rule> vec;

    KnowledgeBase::logging_on();
    LogBox::set_filepath("./test_log.txt");

    //Dictionary test
    std::cout << "\n****************Dictionary test\nSize: " << dic.symbol.size() << std::endl;

    //chunk1 test
    std::cout << "\n****************chunk1 test" << std::endl;
    vec.push_back(Rule(std::string("S/I:1->a b c d")));
    vec.push_back(Rule(std::string("S/I:2->a d c d")));

    // std::cout << "****************Input" << std::endl;
    kb.dic_add(kb.DB_dic,vec);
    kb.send_box(vec);
    for(auto r : vec){
        kb.define(r.internal.front(),empty_m);
    }
    kb.build_word_index();

    std::cout << "dic size : " << kb.DB_dic.size() << std::endl;
    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;

    kb.chunk();
    kb.build_word_index();

    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    LogBox::refresh_log();

    //chunk2 test
    std::cout << "\n****************chunk2 test" << std::endl;
    vec.clear();
    vec.push_back(Rule(std::string("S/I:3->a g c d")));
    kb.dic_add(kb.DB_dic,vec);
    kb.send_box(vec);
    for(auto r : vec){
        kb.define(r.internal.front(),empty_m);
    }
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    
    kb.chunk();
    kb.build_word_index();

    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;
    LogBox::refresh_log();

    //chunk3 test
    std::cout << "\n****************chunk3 test" << std::endl;
    vec.clear();
    vec.push_back(Rule(std::string("S/I:4 C:1/V:1->a C:1/V:1 c d")));
    vec.push_back(Rule(std::string("C:1/I:5->d")));

    kb.dic_add(kb.DB_dic,vec);
    kb.send_box(vec);
    for(auto r : vec){
        kb.define(r.internal.front(),empty_m);
    }
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;

    kb.chunk();
    kb.build_word_index();
    
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }
    LogBox::refresh_log();

    //merge1 test
    vec.clear();
    std::cout << "\n****************merge1 test" << std::endl;
    std::cout << "\n%%% previous" << std::endl;
    vec.push_back(Rule(std::string("C:2/I:6->d")));
    kb.dic_add(kb.DB_dic,vec);
    kb.send_box(vec);
    for(auto r : vec){
        kb.define(r.internal.front(),empty_m);
    }
    kb.chunk();
    kb.build_word_index();
    std::cout << kb.to_s() << std::endl;

    kb.merge();
    kb.build_word_index();
    std::cout << "\n%%% after1" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.chunk();
    kb.merge();
    kb.build_word_index();
    std::cout << "\n%%% after2" << std::endl;
    std::cout << kb.to_s() << std::endl;

    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }
    LogBox::refresh_log();

    //merge4 test
    vec.clear();
    std::cout << "\n****************merge4 test" << std::endl;
    Rule buf1,buf2;
    buf1 = Rule(std::string("S/I:7 C:10002/V:10->a C:10002/V:10"));
    buf2 = Rule(std::string("C:10002/I:7 C:10001/V:100->C:10001/V:100"));
    kb.dic_add(kb.DB_dic,buf1);
    kb.dic_add(kb.DB_dic,buf2);
    kb.define(buf1.internal.front(),empty_m);
    kb.define(buf2.internal.front(),empty_m);
    
    kb.box_buffer.push_back(buf1);
    kb.box_buffer.push_back(buf2);
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;

    kb.merge();
    kb.build_word_index();
    std::cout << "\n%%% after1" << std::endl;
    std::cout << kb.to_s() << std::endl;
    // std::cout << "CHUNK2: " << kb.chunk() << std::endl;
    // std::cout << "MERGE2: " << kb.merge() << std::endl;
    kb.chunk();
    kb.merge();
    kb.build_word_index();
    std::cout << "\n%%% after2" << std::endl;
    std::cout << kb.to_s() << std::endl;

    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }
    LogBox::refresh_log();

    //replace test
    std::cout << "\n****************replace test" << std::endl;
    buf = Rule(std::string("C:3/I:9->a"));
    kb.dic_add(kb.DB_dic,buf);
    kb.send_box(buf);
    kb.define(buf.internal.front(),empty_m);
    kb.chunk();
    kb.build_word_index();
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.replace();
    kb.build_word_index();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }

    LogBox::refresh_log();

    //consolidate test
    std::cout << "\n****************consolidate test" << std::endl;
    vec.push_back(Rule(std::string("S/I:10->a b c")));
    vec.push_back(Rule(std::string("S/I:11->a d c")));
    vec.push_back(Rule(std::string("C:2/I:12->d")));
    vec.push_back(Rule(std::string("C:4/I:13->c")));
    kb.dic_add(kb.DB_dic,vec);
    kb.send_box(vec);
    for(auto r : vec){
        kb.define(r.internal.front(),empty_m);
    }
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.build_word_index();
    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }
    kb.consolidate();
    kb.build_word_index();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    //build index test
    std::cout << "\n****************build index test" << std::endl;
    // kb.build_word_index();

    dit = kb.DB_dic.begin();
    while (dit != kb.DB_dic.end()) {
        std::cout << "\nNOW... C:" << (*dit).first << std::endl;
        item_it = (*dit).second.begin();
        while (item_it != (*dit).second.end()) {
            std::cout << "ind: " << (*item_it).first << std::endl;
            std::cout << "rule: " << (*item_it).second.to_s() << std::endl;
            item_it++;
        }
        dit++;
    }

    kb.clear();
    std::cout << "\n****************learning test" << std::endl;
    vec.clear();
    // vec.push_back(Rule(std::string("S/I:1->t h i s i s a p e n")));
    // vec.push_back(Rule(std::string("S/I:2->t h i s i s a n a p p l e")));
    // vec.push_back(Rule(std::string("S/I:3->a p p l e p e n")));
    // vec.push_back(Rule(std::string("S/I:4->t h i s i s a p e n")));
    // vec.push_back(Rule(std::string("S/I:5->t h i s i s a p i n e a p p l e")));
    // vec.push_back(Rule(std::string("S/I:6->p i n e a p p l e p e n")));
    // vec.push_back(Rule(std::string("S/I:7->p e n p i n e a p p l e a p p l e p e n")));
    // vec.push_back(Rule(std::string("S/I:1->a n a p p l e i s f r u i t")));
    // vec.push_back(Rule(std::string("S/I:2->a b a n a n a i s f r u i t")));
    // vec.push_back(Rule(std::string("S/I:3->b r e a d i s f o o d")));
    // vec.push_back(Rule(std::string("S/I:4->a p p l e i s f o o d")));
    // vec.push_back(Rule(std::string("S/I:5->a b a n a n a i s f o o d")));

    //組成にわけて学習
    vec.push_back(Rule(std::string("S/I:1 C:1/x:1 C:2/x:2 C:3/x:3 C:4/x:4->C:1/x:1 C:2/x:2 C:3/x:3 C:4/x:4")));
    vec.push_back(Rule(std::string("C:1/I:2->a n")));
    vec.push_back(Rule(std::string("C:2/I:3->a p p l e")));
    vec.push_back(Rule(std::string("C:3/I:4->i s")));
    vec.push_back(Rule(std::string("C:4/I:5->f r u i t")));
    vec.push_back(Rule(std::string("S/I:6 C:5/x:1 C:6/x:2 C:7/x:3 C:8/x:4->C:5/x:1 C:6/x:2 C:7/x:3 C:8/x:4")));
    vec.push_back(Rule(std::string("C:5/I:7->a")));
    vec.push_back(Rule(std::string("C:6/I:8->b a n a n a")));
    vec.push_back(Rule(std::string("C:7/I:9->i s")));
    vec.push_back(Rule(std::string("C:8/I:10->f r u i t")));
    vec.push_back(Rule(std::string("S/I:11 C:9/x:1 C:10/x:2 C:11/x:3->C:9/x:1 C:10/x:2 C:11/x:3")));
    vec.push_back(Rule(std::string("C:9/I:12->b r e a d")));
    vec.push_back(Rule(std::string("C:10/I:13->i s")));
    vec.push_back(Rule(std::string("C:11/I:14->f o o d")));
    vec.push_back(Rule(std::string("S/I:15 C:12/x:1 C:13/x:2 C:14/x:3 C:15/x:4->C:12/x:1 C:13/x:2 C:14/x:3 C:15/x:4")));
    vec.push_back(Rule(std::string("C:12/I:16->a n")));
    vec.push_back(Rule(std::string("C:13/I:17->a p p l e")));
    vec.push_back(Rule(std::string("C:14/I:18->i s")));
    vec.push_back(Rule(std::string("C:15/I:19->f o o d")));
    vec.push_back(Rule(std::string("S/I:20 C:16/x:1 C:17/x:2 C:18/x:3 C:19/x:4->C:16/x:1 C:17/x:2 C:18/x:3 C:19/x:4")));
    vec.push_back(Rule(std::string("C:16/I:21->a")));
    vec.push_back(Rule(std::string("C:17/I:22->b a n a n a")));
    vec.push_back(Rule(std::string("C:18/I:23->i s")));
    vec.push_back(Rule(std::string("C:19/I:24->f o o d")));
    std::reverse(vec.begin(),vec.end());
    kb.dic_add(kb.DB_dic,vec);
    kb.send_box(vec);
    for(auto r : vec){
        kb.define(r.internal.front(),empty_m);
    }

    Element e1,e2,e3,e4,e5;
    std::map<int,std::vector<Element> > i_rules;

    e1.set_ind(1);e1.set_ch(5);
    e2.set_ind(2);
    e3.set_ind(3);
    e4.set_ind(4);
    e5.set_ind(5);
    i_rules[1] = std::vector<Element>({e1,e2,e3,e4,e5});
    e1.obj=6;
    e2.obj=7;
    e3.obj=8;
    e4.obj=9;
    e5.obj=10;
    i_rules[2] = std::vector<Element>({e1,e2,e3,e4,e5});
    e1.obj=11;e1.ch.front()=4;
    e2.obj=12;
    e3.obj=13;
    e4.obj=14;
    i_rules[3] = std::vector<Element>({e1,e2,e3,e4});
    e1.obj=15;e1.ch.front()=5;
    e2.obj=16;
    e3.obj=17;
    e4.obj=18;
    e5.obj=19;
    i_rules[4] = std::vector<Element>({e1,e2,e3,e4,e5});
    e1.obj=20;
    e2.obj=21;
    e3.obj=22;
    e4.obj=23;
    e5.obj=24;
    i_rules[5] = std::vector<Element>({e1,e2,e3,e4,e5});
    // i_rules[6] = std::vector<int>({7,8,9,10});
    // i_rules[11] = std::vector<int>({12,13,14});
    // i_rules[15] = std::vector<int>({16,17,18,19});
    // i_rules[20] = std::vector<int>({21,22,23,24});
    kb.init_semantics_rules(i_rules);

    std::cout << "initialized rules" << std::endl << kb.intention.rules_to_s() << std::endl;

    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    kb.consolidate();
    kb.build_word_index();
    std::cout << "\n%%% after" << std::endl;
    std::cout << kb.to_s() << std::endl;

    // std::cout << "\n****************sample test" << std::endl;
    // kb.clear();
    // vec.clear();
    // std::vector<std::string>::reverse_iterator input_it = inputs.rbegin();
    // for(;input_it != inputs.rend();input_it++){
    //     Rule s_tmp(*input_it);
    //     std::cout << s_tmp.to_s() << std::endl;
    //     vec.push_back(s_tmp);
    // }
    // std::cout << "completion to parse" << std::endl;
    // kb.send_box(vec);

    // std::cout << "\n%%% previous" << std::endl;
    // std::cout << kb.to_s() << std::endl;
    // kb.consolidate();
    // std::cout << "\n%%% after" << std::endl;
    // std::cout << kb.to_s() << std::endl;

    // Rule test("S/I:1 I:2 I:3 I:4 I:5->");
    // std::cout << "\nInType Test View" << std::endl;
    // KnowledgeBase::InType::iterator in_it = test.internal.begin();
    // for(;in_it != test.internal.end();in_it++){
    //     std::cout << (*in_it).to_s() << " ch=";
    //     std::vector<int>::iterator ch_it = (*in_it).ch.begin();
    //     std::cout << "[";
    //     for(;ch_it != (*in_it).ch.end();ch_it++){
    //         if(ch_it+1 == (*in_it).ch.end()){
    //             std::cout << (*ch_it);
    //         }else{
    //             std::cout << (*ch_it) << ", ";
    //         }
    //     }
    //     std::cout << "]" << std::endl;
    // }
    // std::cout << "TEST RULE CHECK " << std::endl << test.to_s() << std::endl << std::endl;

    std::cout << "rules based on learning" << std::endl << kb.intention.rules_to_s() << std::endl;
    std::cout << "merge_list based on learning" << std::endl << kb.intention.merge_list_to_s() << std::endl;
    // KnowledgeBase::InType inter = kb.explain(test.internal);
    Element trans_e;
    trans_e.set_ind(1);
    KnowledgeBase::InType inter = kb.intention.trans(trans_e);

    // std::cout << "explain fin." << std::endl;

    // std::cout << "\nInType Test View" << std::endl;
    // KnowledgeBase::InType::iterator in_it = inter.begin();
    // for(;in_it != inter.end();in_it++){
    //     std::cout << (*in_it).to_s() << " ch:" << (*in_it).ch << std::endl;
    // }

    Rule r_sample;
    r_sample.internal = inter;
    r_sample.type = RULE_TYPE::SENTENCE;
    r_sample.cat = 0;
    std::cout << "TRANS TEST \"an apple is fruit\"" << std::endl << r_sample.to_s() << std::endl;
    std::cout << "Fabricate Test" << std::endl;
    std::cout << "kb.fabricate(Rule(" << r_sample.to_s() << ")) =>\n" << (kb.fabricate(r_sample)).to_s() << std::endl;
    /*
    //parse test
    // vec.clear();
    std::cout << std::endl << std::endl << "\n****************parse test" << std::endl;
    std::cout << "\n%%% previous" << std::endl;
    std::cout << kb.to_s() << std::endl;
    std::vector<std::vector<Rule> > r_list;
    std::vector<std::string> t_list;

    bool parse;
    Rule targ;
    std::vector<std::vector<Rule> >::iterator r_l_it;


    targ = Rule(std::string("S/I:1->a n a p p l e i s f o o d"));

    std::cout << "PARSE TARGET: " << targ.to_s() << std::endl;

    parse = kb.parse(targ,r_list,t_list);

    std::cout << std::endl << std::endl << std::endl << "RESULT: " << parse << std::endl;
    r_l_it = r_list.begin();
    for(;r_l_it != r_list.end();r_l_it++){
        std::cout << std::endl << "Option No. " << r_l_it - r_list.begin()+1 << std::endl;
        std::vector<Rule>::iterator r_it = (*r_l_it).begin();
        for(;r_it != (*r_l_it).end();r_it++){
            std::cout << (*r_it).to_s() << std::endl;
        }
    }//*/

    /*
    targ = Rule(std::string("S/I:1->a b a n a n a i s f o o d"));

    std::cout << "PARSE TARGET: " << targ.to_s() << std::endl;

    parse = kb.parse(targ,r_list,t_list);

    std::cout << std::endl << std::endl << std::endl << "RESULT: " << parse << std::endl;
    r_l_it = r_list.begin();
    for(;r_l_it != r_list.end();r_l_it++){
        std::cout << std::endl << "Option No. " << r_l_it - r_list.begin()+1 << std::endl;
        std::vector<Rule>::iterator r_it = (*r_l_it).begin();
        for(;r_it != (*r_l_it).end();r_it++){
            std::cout << (*r_it).to_s() << std::endl;
        }
    }
    //*/

    /*
    targ = Rule(std::string("S/I:1->a b a n a n a i s f r u i t"));

    std::cout << "PARSE TARGET: " << targ.to_s() << std::endl;

    parse = kb.parse(targ,r_list,t_list);

    std::cout << std::endl << std::endl << std::endl << "RESULT: " << parse << std::endl;
    r_l_it = r_list.begin();
    for(;r_l_it != r_list.end();r_l_it++){
        std::cout << std::endl << "Option No. " << r_l_it - r_list.begin()+1 << std::endl;
        std::vector<Rule>::iterator r_it = (*r_l_it).begin();
        for(;r_it != (*r_l_it).end();r_it++){
            std::cout << (*r_it).to_s() << std::endl;
        }
    }//*/

    /*
    targ = Rule(std::string("S/I:1->b r e a d i s f o o d"));

    std::cout << "PARSE TARGET: " << targ.to_s() << std::endl;

    parse = kb.parse(targ,r_list,t_list);

    std::cout << std::endl << std::endl << std::endl << "RESULT: " << parse << std::endl;
    r_l_it = r_list.begin();
    for(;r_l_it != r_list.end();r_l_it++){
        std::cout << std::endl << "Option No. " << r_l_it - r_list.begin()+1 << std::endl;
        std::vector<Rule>::iterator r_it = (*r_l_it).begin();
        for(;r_it != (*r_l_it).end();r_it++){
            std::cout << (*r_it).to_s() << std::endl;
        }
    }
    //*/

    /*
    targ = Rule(std::string("S/I:1->b r e a d i s f r u i t"));

    std::cout << "PARSE TARGET: " << targ.to_s() << std::endl;

    parse = kb.parse(targ,r_list,t_list);

    std::cout << std::endl << std::endl << std::endl << "RESULT: " << parse << std::endl;
    r_l_it = r_list.begin();
    for(;r_l_it != r_list.end();r_l_it++){
        std::cout << std::endl << "Option No. " << r_l_it - r_list.begin()+1 << std::endl;
        std::vector<Rule>::iterator r_it = (*r_l_it).begin();
        for(;r_it != (*r_l_it).end();r_it++){
            std::cout << (*r_it).to_s() << std::endl;
        }
    }
    //*/

    return 0;
}
#endif
