import numpy as np 
import matplotlib.pyplot as plt


drawGates = True
drawOriginal = False
drawBankingResult = True
markDisplacement = False
drawBinLine = False


# **********************************************************
# *       Draw Original FF Instances and Comb Gates        *
# **********************************************************
if drawOriginal:
    f = open('./input/ff_original.txt', 'r')

    die_w, die_h, bin_w, bin_h = [float(x) for x in next(f).split()] # read first line
    ff_num = [int(x) for x in next(f).split()] # read second line

    scale_factor = 100/die_w

    plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))

    fig = plt.gcf()
    ax = fig.gca()

    for line in f:
        ff_name, size, coox, cooy, w, h = [word for word in line.split()]

        coox = float(coox)
        cooy = float(cooy)
        w = float(w)
        h = float(h)
        size = int(size)
        
        if size == 1:
            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='yellow')
            ax.add_patch(patch)
        elif size == 2:
            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='blue')
            ax.add_patch(patch)
        elif size == 4:
            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='red')
            ax.add_patch(patch)
        

    plt.xlim([0, die_w*scale_factor])
    plt.ylim([0, die_h*scale_factor])

    f = open('./input/gate.txt', 'r')

    die_w, die_h, bin_w, bin_h = [float(x) for x in next(f).split()] # read first line
    ff_num = [int(x) for x in next(f).split()] # read second line

    # Draw Bin Line
    if drawBinLine:
        # Draw Vertical Line
        j = bin_w
        while (j < die_w):
            plt.plot([j*scale_factor, j*scale_factor], [0, die_h*scale_factor], color='b') # plot line
            j = j + bin_w
        
        # Draw Horizontal Line
        i = bin_h
        while (i < die_h):
            plt.plot([0, die_w*scale_factor], [i*scale_factor, i*scale_factor], color='b') # plot line
            i = i + bin_h

    if drawGates:
        for line in f:
            ff_name, coox, cooy, w, h = [word for word in line.split()]

            coox = float(coox)
            cooy = float(cooy)
            w = float(w)
            h = float(h)

            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='g')
            ax.add_patch(patch)


    plt.xlim([0, die_w*scale_factor])
    plt.ylim([0, die_h*scale_factor])


    plt.savefig('./Pic/original.png')
    f.close()




# **********************************************************
# *      Draw Banking Result (with Comb gates)             *
# **********************************************************
if drawBankingResult:
    f = open('./input/placement result.txt', 'r')

    outputFileName = "./Pic/bankingResult.png"
    if markDisplacement: outputFileName = "./Pic/bankingResult_markDisplace.png"

    die_w, die_h, bin_w, bin_h = [float(x) for x in next(f).split()] # read first line
    ff_num = [int(x) for x in next(f).split()] # read second line

    scale_factor = 100/die_w

    plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))

    fig = plt.gcf()
    ax = fig.gca()

    for line in f:
        ff_name, size, coox, cooy, w, h, cenx, ceny, ori_cenx, ori_ceny = [word for word in line.split()]


        coox = float(coox)
        cooy = float(cooy)
        w = float(w)
        h = float(h)
        cenx = float(cenx)*scale_factor
        ceny = float(ceny)*scale_factor
        ori_cenx  = float(ori_cenx)*scale_factor
        ori_ceny  = float(ori_ceny)*scale_factor
        size = int(size)

        if size == 1:
            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='yellow')
            ax.add_patch(patch)
        elif size == 2:
            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='blue')
            ax.add_patch(patch)
        elif size == 4:
            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='red')
            ax.add_patch(patch)

        ##############################      
        # Draw the displacement line #
        ##############################
        if markDisplacement:
            plt.plot(cenx, ceny, 'o', color='b') # plot dot
            plt.plot([cenx, ori_cenx], [ceny, ori_ceny], color='b') # plot line
            

    f.close()

    f = open('./input/gate.txt', 'r')

    die_w, die_h, bin_w, bin_h = [float(x) for x in next(f).split()] # read first line
    ff_num = [int(x) for x in next(f).split()] # read second line

    # Draw Bin Line
    if drawBinLine:
        # Draw Vertical Line
        j = bin_w
        while (j < die_w):
            plt.plot([j*scale_factor, j*scale_factor], [0, die_h*scale_factor], color='b') # plot line
            j = j + bin_w
        
        # Draw Horizontal Line
        i = bin_h
        while (i < die_h):
            plt.plot([0, die_w*scale_factor], [i*scale_factor, i*scale_factor], color='b') # plot line
            i = i + bin_h

    if drawGates:
        for line in f:
            ff_name, coox, cooy, w, h = [word for word in line.split()]

            coox = float(coox)
            cooy = float(cooy)
            w = float(w)
            h = float(h)

            patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=True, width=w*scale_factor, height=h*scale_factor, linewidth=0.5, edgecolor='black', facecolor='g')
            ax.add_patch(patch)


    plt.xlim([0, die_w*scale_factor])
    plt.ylim([0, die_h*scale_factor])

    plt.savefig(outputFileName)
    f.close()


