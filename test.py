import fraglets
from lifelines import KaplanMeierFitter, NelsonAalenFitter
from multiprocessing import Pool
from matplotlib import pyplot as plt

def getIter(t):
    frag = getFrag()
    frag.run(1000000,2000,True)
    print(frag.iter)
    return frag.iter


def getFrag():
    frag = fraglets.fraglets()
    for i in range(20):
        frag.parse("[fork nop z match z split match z fork fork fork nop z * split match z fork fork fork nop z * z]")
    frag.parse("[perm z]")


    # alphabet = "abu"

    # permz = "perm z "
    # for symbol in alphabet:
    #     newMol = permz + symbol
    #     frag.parse(newMol)
    #     print(newMol)
    #     newMol = permz + "z " + symbol
    #     frag.parse(newMol)
    #     print(newMol)

    # permz = "perm z "
    # for symbol in frag.unimolTags:
    #     newMol = permz + symbol
    #     frag.parse(newMol)
    #     print(newMol)
    #     newMol = permz + "z " + symbol
    #     frag.parse(newMol)
    #     print(newMol)


    # for symbol in alphabet:
    #     newMol = permz + "match " + symbol
    #     frag.parse(newMol)
    #     print(newMol)
    #     newMol = permz + "z match " + symbol
    #     frag.parse(newMol)
    #     print(newMol) 
    #     newMol = permz + "matchp " + symbol
    #     frag.parse(newMol)
    #     print(newMol)
    #     newMol = permz + "z matchp " + symbol
    #     frag.parse(newMol)
    #     print(newMol) 

    return frag
    # frag.run(10000,200,True)

# pool = Pool(4)
# data = pool.map(getIter,range(100))

frags = []
data = []

for i in range(200):
    frag=getFrag()
    frag.run(1000,500,True)
    print(frag.iter)
    frags.append(frag)

greatest = frags[0]
for frag in frags:
    if frag.iter > greatest.iter:
        greatest = frag
    data.append(frag.iter)

greatest.drawGraphViz()





print(data)
kmf = KaplanMeierFitter()
kmf.fit(data)
kmf.plot()
plt.show()

naf = NelsonAalenFitter()
naf.fit(data)
naf.plot()
plt.show()
