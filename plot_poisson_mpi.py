import numpy as np 
import matplotlib.pyplot as plt

file1s='source.txt'
file2s='field.txt'
file1m='source_mpi.txt'
file2m='field_mpi.txt'

data1s=np.loadtxt(file1s)
data2s=np.loadtxt(file2s)

data1m=np.loadtxt(file1m)
data2m=np.loadtxt(file2m)

plt.figure(figsize=[8,8])
plt.subplot(221)
cx1s=plt.imshow(data1s)
plt.colorbar(cx1s)
plt.title('Source for serial computing')
plt.subplot(222)
cx2s=plt.imshow(data2s)
plt.colorbar(cx2s)
plt.title('Final field for serial computing')
plt.subplot(223)
cx1m=plt.imshow(data1m)
plt.colorbar(cx1m)
plt.title('Source for parallel computing')
plt.subplot(224)
cx2m=plt.imshow(data2m)
plt.colorbar(cx2m)
plt.title('Final field for parallel computing')

plt.tight_layout()


print("Total difference between Seriel and MPI version is {}".format(sum(sum(data2m-data2s))))

plt.show()