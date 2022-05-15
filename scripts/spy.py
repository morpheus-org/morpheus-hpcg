import numpy as np
from scipy import sparse as sp
import matplotlib.pylab as plt
import argparse


def build_matrix(filename):
    mat = np.genfromtxt(filename, delimiter=" ")

    # split matrix to I, J, V
    I, J, V = np.hsplit(mat, 3)
    II = I.reshape(I.shape[0]).astype(int)
    JJ = J.reshape(J.shape[0]).astype(int)
    VV = V.reshape(V.shape[0])

    # convert to CSR
    return sp.csr_matrix((VV, (II, JJ)))


# get filename to process
parser = argparse.ArgumentParser(
    description="Script for plotting the sparsity pattern of a matrix from HPCG"
)

parser.add_argument(
    "--expdir",
    type=str,
    required=True,
    help="Absolute directory to the experiment containing the matrices to be processed.",
)

parser.add_argument(
    "--nprocs",
    type=int,
    default=4,
    help="Number of processes to be split across.",
)

parser.add_argument(
    "--nlevels",
    type=int,
    default=3,
    help="Number of MG levels.",
)
args = parser.parse_args()

file_extension = "jpg"

print("*" * 10, " spy.py ", "*" * 10)
print("Experiment Directory : ", args.expdir)
print("Decomposing over     : ", args.nprocs, " process(es)")
print("Number of MG Levels  : ", args.nlevels)
print("*" * 30)

matrices = {"A": build_matrix(args.expdir + "/matrix-0.txt")}
for lvl in range(1, args.nlevels):
    matrices["Ac" + str(lvl)] = build_matrix(
        args.expdir + "/coarse-" + str(lvl) + "-matrix-0.txt"
    )

for key, mat in matrices.items():
    # plot
    fig, ax = plt.subplots(tight_layout=True)
    plt.spy(mat, markersize=1)
    Acsr_shape = mat.get_shape()

    for proc in range(args.nprocs):
        plt.axhline(y=Acsr_shape[0] * proc / args.nprocs, color="r", linestyle="--")
        plt.axvline(x=Acsr_shape[1] * proc / args.nprocs, color="r", linestyle="--")
    fig.savefig(
        args.expdir + "/plot-" + key + "." + file_extension, format=file_extension
    )
