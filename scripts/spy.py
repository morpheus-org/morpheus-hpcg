import numpy as np
from scipy import sparse as sp
import matplotlib.pylab as plt
import argparse
import os

# get filename to process
parser = argparse.ArgumentParser(
    description="Script for plotting the sparsity pattern of a matrix from HPCG"
)

parser.add_argument(
    "--filename",
    type=str,
    required=True,
    help="Absolute filename to the Matrix file to be processed.",
)

parser.add_argument(
    "--nprocs",
    type=int,
    default=4,
    help="Number of processes to be split across.",
)
args = parser.parse_args()

filedir = os.path.dirname(args.filename)
file_extension = "jpg"
outfile = filedir + "/plot." + file_extension

print("*" * 10, " spy.py ", "*" * 10)
print("Input File  : ", args.filename)
print("Output Plot : ", outfile)
print("Decomposing over : ", args.nprocs, " process(es)")
print("*" * 30)

# read matrix from A.dat (file produced during HPCG run with detail debug on)
matrix = np.genfromtxt(args.filename, delimiter=" ")

# split matrix to I, J, V
I, J, V = np.hsplit(matrix, 3)
II = I.reshape(I.shape[0]).astype(int)
JJ = J.reshape(J.shape[0]).astype(int)
VV = V.reshape(V.shape[0])

# convert to CSR
Acsr = sp.csr_matrix((VV, (II, JJ)))

# plot
fig, ax = plt.subplots(tight_layout=True)
plt.spy(Acsr, markersize=1)
Acsr_shape = Acsr.get_shape()

for proc in range(args.nprocs):
    plt.axhline(y=Acsr_shape[0] * proc / args.nprocs, color="r", linestyle="--")
    plt.axvline(x=Acsr_shape[1] * proc / args.nprocs, color="r", linestyle="--")
fig.savefig(outfile, format=file_extension)
