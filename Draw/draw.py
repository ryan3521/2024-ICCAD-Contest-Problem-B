import numpy as np 
import matplotlib.pyplot as plt

with open('ff.txt', 'r') as f:
    die_w, die_h = [float(x) for x in next(f).split()] # read first line
    ff_num = [int(x) for x in next(f).split()] # read second line

    scale_factor = 100/die_w

    plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))
    
    fig = plt.gcf()
    ax = fig.gca()

    for line in f:
        ff_name, coox, cooy, w, h = [word for word in line.split()]

        coox = float(coox)
        cooy = float(cooy)
        w = float(w)
        h = float(h)

        patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), width=w*scale_factor, height=h*scale_factor, color='r')
        ax.add_patch(patch)

    plt.xlim([0, die_w*scale_factor])
    plt.ylim([0, die_h*scale_factor])

    plt.savefig('ffs.png')


