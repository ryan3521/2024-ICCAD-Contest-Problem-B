import numpy as np 
import matplotlib.pyplot as plt


# **********************************************************
# *                Original FF Instances                   *
# **********************************************************
f = open('ff_original.txt', 'r')

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

    patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='r')
    ax.add_patch(patch)

plt.xlim([0, die_w*scale_factor])
plt.ylim([0, die_h*scale_factor])

f = open('gate.txt', 'r')

die_w, die_h = [float(x) for x in next(f).split()] # read first line
ff_num = [int(x) for x in next(f).split()] # read second line

for line in f:
    ff_name, coox, cooy, w, h = [word for word in line.split()]

    coox = float(coox)
    cooy = float(cooy)
    w = float(w)
    h = float(h)

    patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='g')
    ax.add_patch(patch)


plt.xlim([0, die_w*scale_factor])
plt.ylim([0, die_h*scale_factor])


plt.savefig('ffs_original.png')
f.close()




# **********************************************************
# *                Placement Result                        *
# **********************************************************
# f = open('placement result.txt', 'r')

# die_w, die_h = [float(x) for x in next(f).split()] # read first line
# ff_num = [int(x) for x in next(f).split()] # read second line

# scale_factor = 100/die_w

# plt.figure(figsize=(die_w*scale_factor, die_h*scale_factor))

# fig = plt.gcf()
# ax = fig.gca()

# for line in f:
#     ff_name, coox, cooy, w, h, cenx, ceny, ori_cenx, ori_ceny = [word for word in line.split()]


#     coox = float(coox)
#     cooy = float(cooy)
#     w = float(w)
#     h = float(h)
#     cenx = float(cenx)*scale_factor
#     ceny = float(ceny)*scale_factor
#     ori_cenx  = float(ori_cenx)*scale_factor
#     ori_ceny  = float(ori_ceny)*scale_factor


#     patch = plt.Rectangle(xy=(coox*scale_factor, cooy*scale_factor), fill=False, width=w*scale_factor, height=h*scale_factor, color='r')
#     ax.add_patch(patch)

#     # plt.plot(cenx, ceny, 'o', color='b') # plot dot
#     # plt.plot([cenx, ori_cenx], [ceny, ori_ceny], color='b') # plot line

# f.close()

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


# plt.xlim([0, die_w*scale_factor])
# plt.ylim([0, die_h*scale_factor])

# plt.savefig('placement.png')
# f.close()


