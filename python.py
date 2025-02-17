path = 'test.txt'
used_lines = []
with open(path) as f:
    for linha in f:
        if linha.startswith('real'):
            linha = linha.strip().replace("\t", " ")
            used_lines.append(linha)

def convert_to_seconds(time_str):
    parts = time_str.split()
    for part in parts:
        if 'm' in part and 's' in part:
            minutes, seconds = part.split('m')
            seconds = seconds.replace('s', '')
            return int(minutes) * 60 + float(seconds)

seconds_list = [convert_to_seconds(t) for t in used_lines]
print(seconds_list)

import matplotlib.pyplot as plt
labels = ['Sequencial', 'Paralelo']
plt.bar(labels, seconds_list, color=['blue', 'green'])
plt.xlabel('Tipo')
plt.ylabel('Tempo (segundos)')
plt.title('Tempo de Execução')
for i, v in enumerate(seconds_list):
    plt.text(i, v + 1, f"{v:.3f}s", ha='center', fontsize=10)
plt.show()
