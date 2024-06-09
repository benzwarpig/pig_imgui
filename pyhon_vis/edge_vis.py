import matplotlib.pyplot as plt

def read_data(filepath):
    # 用来存放数据的字典，使用(id1, id2)作为key，值为一个包含(x, y)点的列表
    trajectories = {}
    
    with open(filepath, 'r') as file:
        for line in file:
            parts = line.strip().split()
            x, y = float(parts[0]), float(parts[1])
            id1, id2 = parts[2], parts[3]
            
            # 使用元组(id1, id2)将点分到不同的轨迹
            key = (id1, id2)
            if key not in trajectories:
                trajectories[key] = []
            trajectories[key].append((x, y))

    return trajectories

def plot_trajectories(trajectories):
    plt.figure(figsize=(10, 8))
    
    # 为每个轨迹分配颜色和样式
    for index, ((id1, id2), points) in enumerate(trajectories.items()):
        xs, ys = zip(*points)  # 解包列表中的点到两个列表xs和ys
        plt.plot(xs, ys, label=f'Track {id1}-{id2}', marker='o')
    
    plt.xlabel('X coordinate')
    plt.ylabel('Y coordinate')
    plt.title('Trajectories by ID')
    plt.legend()
    plt.grid(True)  # 添加网格
    plt.show()

if __name__ == "__main__":
    filepath = 'edge_vis.txt' 
    trajectories = read_data(filepath)  # 调用函数读取数据
    plot_trajectories(trajectories)  # 调用函数绘制轨迹
