import os
import numpy as np

data = np.load(os.path.join("output", "norm_params.npz"))
mean = data["mean"]
std = data["std"]

header_path = os.path.join("include", "norm_params.h")
with open(header_path, "w") as f:
    f.write("#ifndef NORM_PARAMS_H\n")
    f.write("#define NORM_PARAMS_H\n\n")
    f.write("const float norm_mean[6] = {")
    f.write(", ".join(f"{v:.6f}f" for v in mean))
    f.write("};\n")
    f.write("const float norm_std[6] = {")
    f.write(", ".join(f"{v:.6f}f" for v in std))
    f.write("};\n\n")
    f.write("#endif\n")

print(f"Generated {header_path}")
print("mean:", mean)
print("std:", std)