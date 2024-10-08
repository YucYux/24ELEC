import csv
import os
from datetime import datetime, timedelta

def csv_to_txt(csv_file_path, txt_file_path):
    with open(csv_file_path, mode='r', encoding='utf-8-sig') as csv_file:
        csv_reader = csv.reader(csv_file)
        
        with open(txt_file_path, mode='w', encoding='utf-8') as txt_file:
            for row in csv_reader:
                for cell in row:
                    # 清除单元格内的换行符
                    cleaned_cell = cell.replace('\n', ' ')
                    txt_file.write(cleaned_cell + '\n')  # 每个单元格之间换行
                txt_file.write('\n')  # 每行结束后额外换行

# 如果是处理州数据，取消下面这几行注释
# for state in ['AZ','GA','MI','NC','NV','PA','WI']:
#     csv_to_txt(f'C:\\Users\\NM-on\\OneDrive\\2024election\\0930csv\\res_{state}.csv',
#                f'C:\\Users\\NM-on\\OneDrive\\2024election\\0930csv\\res_{state}.txt')

# exit()

# 批量处理多个文件
folder_path = 'C:\\Users\\NM-on\\OneDrive\\2024election\\0930csv\\'

# 设置日期范围，从2024年8月1日到2024年9月19日
start_date = datetime(2024, 8, 1)
end_date = datetime(2024, 9, 19)

# 生成日期范围内的所有日期
current_date = start_date
while current_date <= end_date:
    # 格式化日期为文件名中的格式
    date_str = current_date.strftime('%Y-%m-%d')
    
    # 构建CSV文件名和TXT文件名
    csv_file_name = f'res_{date_str}.csv'
    txt_file_name = f'res_{date_str}.txt'
    
    # 构建完整的文件路径
    csv_file_path = os.path.join(folder_path, csv_file_name)
    txt_file_path = os.path.join(folder_path, txt_file_name)
    
    # 检查CSV文件是否存在
    if os.path.exists(csv_file_path):
        csv_to_txt(csv_file_path, txt_file_path)
        print(f'Processed {csv_file_name} to {txt_file_name}')
    else:
        print(f'{csv_file_name} does not exist, skipping...')
    
    # 移动到下一天
    current_date += timedelta(days=1)
