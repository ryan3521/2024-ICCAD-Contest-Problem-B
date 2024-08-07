import numpy as np 
import matplotlib.pyplot as plt


# **********************************************************
# *                Original FF Instances                   *
# **********************************************************
# f = open('ff_original.txt', 'r')

# die_w, die_h = [float(x) for x in next(f).split()] # read first line
# ff_num = [int(x) for x in next(f).split()] # read second line

# scale_factor = 100/die_w

# plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))

# fig = plt.gcf()
# ax = fig.gca()

# for line in f:
#     ff_name, coox, cooy, w, h = [word for word in line.split()]

#     coox = float(coox)
#     cooy = float(cooy)
#     w = float(w)
#     h = float(h)

#     patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='r')
#     ax.add_patch(patch)

# plt.xlim([0, die_w*scale_factor])
# plt.ylim([0, die_h*scale_factor])

# plt.savefig('ffs_original.png')
# f.close()


# **********************************************************
# *               Single Bit FF Instances                  *
# **********************************************************
# f = open('ff_single_bit.txt', 'r')

# die_w, die_h = [float(x) for x in next(f).split()] # read first line
# ff_num = [int(x) for x in next(f).split()] # read second line

# scale_factor = 100/die_w

# plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))

# fig = plt.gcf()
# ax = fig.gca()

# for line in f:
#     ff_name, coox, cooy, w, h, canmove, xmax, xmin, ymax, ymin = [word for word in line.split()]

#     coox = float(coox)
#     cooy = float(cooy)
#     w = float(w)
#     h = float(h)
#     canmove = int(canmove)
#     xmax = float(xmax)
#     xmin = float(xmin)
#     ymax = float(ymax)
#     ymin = float(ymin)

#     if canmove == 1:
#         patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='g')
#         ax.add_patch(patch)
#     else:
#         patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='r')
#         ax.add_patch(patch)

#     if canmove == 1:
#         cenx_ = (xmax + xmin)/2
#         ceny_ = (ymax + ymin)/2

#         cenx = (cenx_ - ceny_)*scale_factor/2
#         ceny = (cenx_ + ceny_)*scale_factor/2

#         plt.plot([(coox+w/2)*scale_factor, cenx], [(cooy+h/2)*scale_factor, ceny], marker = 'o', color='b')

#         x1_ = xmax
#         y1_ = ymax
#         x2_ = xmax
#         y2_ = ymin

#         x1 = (x1_ - y1_)*scale_factor/2
#         y1 = (x1_ + y1_)*scale_factor/2
#         x2 = (x2_ - y2_)*scale_factor/2
#         y2 = (x2_ + y2_)*scale_factor/2

#         plt.plot([x1,x2], [y1,y2], color='y')

#         x1_ = xmax
#         y1_ = ymin
#         x2_ = xmin
#         y2_ = ymin

#         x1 = (x1_ - y1_)*scale_factor/2
#         y1 = (x1_ + y1_)*scale_factor/2
#         x2 = (x2_ - y2_)*scale_factor/2
#         y2 = (x2_ + y2_)*scale_factor/2

#         plt.plot([x1,x2], [y1,y2], color='y')

#         x1_ = xmin
#         y1_ = ymin
#         x2_ = xmin
#         y2_ = ymax

#         x1 = (x1_ - y1_)*scale_factor/2
#         y1 = (x1_ + y1_)*scale_factor/2
#         x2 = (x2_ - y2_)*scale_factor/2
#         y2 = (x2_ + y2_)*scale_factor/2

#         plt.plot([x1,x2], [y1,y2], color='y')

#         x1_ = xmin
#         y1_ = ymax
#         x2_ = xmax
#         y2_ = ymax

#         x1 = (x1_ - y1_)*scale_factor/2
#         y1 = (x1_ + y1_)*scale_factor/2
#         x2 = (x2_ - y2_)*scale_factor/2
#         y2 = (x2_ + y2_)*scale_factor/2

#         plt.plot([x1,x2], [y1,y2], color='y')


# plt.xlim([0, die_w*scale_factor])
# plt.ylim([0, die_h*scale_factor])

# plt.savefig('ffs_single_bit.png')
# f.close()


# **********************************************************
# *                Multi Bit FF Instances                  *
# **********************************************************
f = open('ff_multi_bit.txt', 'r')

die_w, die_h = [float(x) for x in next(f).split()] # read first line
ff_num = [int(x) for x in next(f).split()] # read second line

scale_factor = 100/die_w

plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))

fig = plt.gcf()
ax = fig.gca()

for line in f:
    ff_name, coox, cooy, w, h, b, canmove, xmax, xmin, ymax, ymin = [word for word in line.split()]

    coox = float(coox)
    cooy = float(cooy)
    w = float(w)
    h = float(h)
    b = int(b)
    canmove = int(canmove)
    xmax = float(xmax)
    xmin = float(xmin)
    ymax = float(ymax)
    ymin = float(ymin)

    if canmove == 1 and b>1:
        patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='g')
        ax.add_patch(patch)
    elif canmove == 1:
        patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='b')
        ax.add_patch(patch)
    else:
        patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='r')
        ax.add_patch(patch)

    if canmove == 1 and b==1:
        cenx_ = (xmax + xmin)/2
        ceny_ = (ymax + ymin)/2

        cenx = (cenx_ - ceny_)*scale_factor/2
        ceny = (cenx_ + ceny_)*scale_factor/2

        plt.plot([(coox+w/2)*scale_factor, cenx], [(cooy+h/2)*scale_factor, ceny], marker = 'o', color='m')

        x1_ = xmax
        y1_ = ymax
        x2_ = xmax
        y2_ = ymin

        x1 = (x1_ - y1_)*scale_factor/2
        y1 = (x1_ + y1_)*scale_factor/2
        x2 = (x2_ - y2_)*scale_factor/2
        y2 = (x2_ + y2_)*scale_factor/2

        plt.plot([x1,x2], [y1,y2], color='y')

        x1_ = xmax
        y1_ = ymin
        x2_ = xmin
        y2_ = ymin

        x1 = (x1_ - y1_)*scale_factor/2
        y1 = (x1_ + y1_)*scale_factor/2
        x2 = (x2_ - y2_)*scale_factor/2
        y2 = (x2_ + y2_)*scale_factor/2

        plt.plot([x1,x2], [y1,y2], color='y')

        x1_ = xmin
        y1_ = ymin
        x2_ = xmin
        y2_ = ymax

        x1 = (x1_ - y1_)*scale_factor/2
        y1 = (x1_ + y1_)*scale_factor/2
        x2 = (x2_ - y2_)*scale_factor/2
        y2 = (x2_ + y2_)*scale_factor/2

        plt.plot([x1,x2], [y1,y2], color='y')

        x1_ = xmin
        y1_ = ymax
        x2_ = xmax
        y2_ = ymax

        x1 = (x1_ - y1_)*scale_factor/2
        y1 = (x1_ + y1_)*scale_factor/2
        x2 = (x2_ - y2_)*scale_factor/2
        y2 = (x2_ + y2_)*scale_factor/2

        plt.plot([x1,x2], [y1,y2], color='y')


plt.xlim([0, die_w*scale_factor])
plt.ylim([0, die_h*scale_factor])

plt.savefig('ffs_multi_bit.png')
f.close()

# **********************************************************
# *                    Gate Instances                      *
# **********************************************************


# f = open('gate.txt', 'r')

# die_w, die_h = [float(x) for x in next(f).split()] # read first line
# ff_num = [int(x) for x in next(f).split()] # read second line

# for line in f:
#     ff_name, coox, cooy, w, h = [word for word in line.split()]

#     coox = float(coox)
#     cooy = float(cooy)
#     w = float(w)
#     h = float(h)

#     patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='g')
#     ax.add_patch(patch)
# plt.savefig('all_original.png')
# f.close()
