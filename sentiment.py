from transformers import AutoModelForSequenceClassification
from transformers import AutoTokenizer
import os
from scipy.special import softmax
from nltk.tokenize import sent_tokenize
from tqdm import tqdm
import pandas as pd
import numpy as np
from collections import defaultdict
# 设置CUDA设备
os.environ["CUDA_VISIBLE_DEVICES"] = "0"
os.environ['HF_DATASETS_OFFLINE'] = '1'
os.environ['HF_HUB_OFFLINE'] = '1'

# 定义路径
source_dirs = {'6G已切分': 'nation', 'lean_news_parts': 'state'}  # 两个CSV文件夹路径及其对应的默认media_type
result_dir = 'sentiment_results'  # 结果保存路径
counter_file = 'article_counter.txt'  # 计数器文件路径

# 确保结果目录存在
os.makedirs(result_dir, exist_ok=True)

# 摇摆州及其对应的缩写
swing_state_abbr = {
    '宾夕法尼亚州': 'PA',
    '内华达州': 'NV',
    '北卡罗来纳州': 'NC',
    '密歇根州': 'MI',
    '威斯康星州': 'WI',
    '佐治亚州': 'GA',
    '亚利桑那州': 'AZ'
}

keywords = {
    'Trump': ['Trump', 'President Trump', 'Former President Trump', 'The Donald', 'MAGA leader'],
    'Harris': ['Harris', 'Vice President Harris', 'Former Senator Harris', 'first female VP'],
    'Republican': ['Republican', 'GOP', 'Conservatives', 'The right', 'Trump supporters'],
    'Democrat': ['Democrat', 'Dems', 'Liberals', 'The left', 'Progressives']
}

def contains_keyword(sent, keywords):
    for key in keywords:
        for term in keywords[key]:
            if term in sent:
                return True
    return False

def data_generator(sentence_list, batch_size=16):
    temp_list = []
    for sent in sentence_list:
        temp_list.append(sent)
        if len(temp_list) == batch_size:
            copy_res = temp_list.copy()
            temp_list = []
            yield copy_res
    if len(temp_list) != 0:
        yield temp_list

def read_article_counter():
    if os.path.exists(counter_file):
        with open(counter_file, 'r') as f:
            return int(f.read().strip())
    else:
        return 0

def write_article_counter(counter):
    with open(counter_file, 'w') as f:
        f.write(str(counter))

def process_files():
    article_id = read_article_counter()  # 读取新闻计数器
    all_results_nation = defaultdict(list)  # nation 结果按日期存储
    all_results_state = defaultdict(list)  # state 结果按州名存储

    for source_dir, default_media_type in source_dirs.items():
        files = [f for f in os.listdir(source_dir) if f.endswith('.csv')]
        
        for f in tqdm(files, desc=f"Processing {source_dir}"):
            file_path = os.path.join(source_dir, f)
            df = pd.read_csv(file_path, encoding='utf-8')

            for index, row in df.iterrows():
                date = row['发布时间']
                if pd.to_datetime(date) < pd.to_datetime('2024-08-01'):
                    continue  # 仅处理2024年8月以后的数据
                
                source = row['数据来源']
                article_id += 1  # 增加新闻计数器

                # 确定 media_type
                media_type = default_media_type
                state_abbr = ''
                if '州名' in row and pd.notna(row['州名']):
                    state_name = row['州名']
                    state_abbr = swing_state_abbr.get(state_name, '')
                    if state_abbr:
                        media_type = state_abbr

                all_sentence = sent_tokenize(row['原文']) + sent_tokenize(row['原文标题'])
                all_sentence = [sent for sent in all_sentence if contains_keyword(sent, keywords)]

                if len(all_sentence) > 0:
                    for sentence_list in data_generator(all_sentence):
                        encoded_input = tokenizer(sentence_list, return_tensors='pt', padding=True, truncation=True, max_length=128).to('cuda')
                        output = model(**encoded_input)
                        scores = output[0].cpu().detach().numpy()
                        scores = softmax(scores, axis=1)
                        rank_list = np.argmax(scores, axis=1)
                        for sen, score, rank in zip(sentence_list, scores, rank_list):
                            temp_item = {
                                'date': date,
                                'color': '',
                                'source': source,
                                'media_type': media_type,
                                'sen': sen,
                                'negative': score[0],
                                'neutral': score[1],
                                'positive': score[2],
                                'rank': rank,
                                'article_id': article_id  # 添加新闻计数器
                            }
                            if state_abbr:
                                temp_item['state'] = state_abbr
                                all_results_state[state_abbr].append(temp_item)
                            else:
                                all_results_nation[date].append(temp_item)

    # 保存 nation 结果
    for date, results in all_results_nation.items():
        result_df = pd.DataFrame(results)
        result_df.to_csv(f'{result_dir}/nation_results_{date}.csv', index=False, encoding='utf_8_sig')

    # 保存 state 结果
    for state, results in all_results_state.items():
        result_df = pd.DataFrame(results)
        result_df.to_csv(f'{result_dir}/state_results_{state}.csv', index=False, encoding='utf_8_sig')

    write_article_counter(article_id)  # 保存新闻计数器
    print("处理完毕，结果已保存到 'sentiment_results' 目录中")

if __name__ == '__main__':
    process_files()
