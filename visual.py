import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# 设置样式
sns.set_style("whitegrid")
plt.rcParams.update({
    'font.size': 12,
    'font.family': 'Noto Sans SC'
})

# 读取CSV文件
df = pd.read_csv('training_log.csv')

# 创建一个图形，包含两个子图
fig, axes = plt.subplots(2, 1, figsize=(12, 10), sharex=False)
fig.suptitle('俄罗斯方块AI训练进度', fontsize=16)

# 第一个子图：按世代绘制
ax1 = axes[0]
ax1.plot(df['generation'], df['best_score'], 'b-', linewidth=2, label='最佳分数')
ax1.plot(df['generation'], df['average_score'], 'r-', linewidth=2, label='平均分数')
ax1.set_xlabel('世代')
ax1.set_ylabel('分数')
ax1.legend(loc='upper left')
ax1.set_title('按世代的训练进度')
ax1.grid(True)

# 第二个子图：按时间绘制
ax2 = axes[1]
ax2.plot(df['time_elapsed'], df['best_score'], 'b-', linewidth=2, label='最佳分数')
ax2.plot(df['time_elapsed'], df['average_score'], 'r-', linewidth=2, label='平均分数')
ax2.set_xlabel('已用时间')
ax2.set_ylabel('分数')
ax2.legend(loc='upper left')
ax2.set_title('按时间的训练进度')
ax2.grid(True)

# 添加移动平均线(每10代)来显示趋势
window_size = 10
if len(df) >= window_size:
    df['best_score_ma'] = df['best_score'].rolling(window=window_size).mean()
    df['average_score_ma'] = df['average_score'].rolling(window=window_size).mean()
    
    ax1.plot(df['generation'], df['best_score_ma'], 'b--', linewidth=1.5, label='最佳分数趋势')
    ax1.plot(df['generation'], df['average_score_ma'], 'r--', linewidth=1.5, label='平均分数趋势')
    ax1.legend(loc='upper left')
    
    ax2.plot(df['time_elapsed'], df['best_score_ma'], 'b--', linewidth=1.5, label='最佳分数趋势')
    ax2.plot(df['time_elapsed'], df['average_score_ma'], 'r--', linewidth=1.5, label='平均分数趋势')
    ax2.legend(loc='upper left')

# 调整布局并保存图形
plt.tight_layout(rect=[0, 0, 1, 0.95])
plt.savefig('training_progress.png', dpi=300)

# 输出一些基本统计信息
print(f"训练总世代数: {df['generation'].max()}")
print(f"训练总时间: {df['time_elapsed'].max()} 分钟")
print(f"最高分数: {df['best_score'].max()} (世代 {df['generation'][df['best_score'].idxmax()]})")
print(f"最终平均分数: {df['average_score'].iloc[-1]}")
print(f"平均分数增长: {df['average_score'].iloc[-1] - df['average_score'].iloc[0]}")