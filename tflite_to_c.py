import os

tflite_path = os.path.join("output", "gesture_model.tflite")
header_path = os.path.join("include", "gesture_model.h")

os.makedirs("include", exist_ok=True)

with open(tflite_path, "rb") as f:
    data = f.read()

with open(header_path, "w") as f:
    f.write("#ifndef GESTURE_MODEL_H\n")
    f.write("#define GESTURE_MODEL_H\n\n")
    f.write(f"const unsigned int gesture_model_tflite_len = {len(data)};\n")
    f.write("const unsigned char gesture_model_tflite[] = {\n")
    for i, b in enumerate(data):
        if i % 12 == 0:
            f.write("  ")
        f.write(f"0x{b:02x}, ")
        if i % 12 == 11:
            f.write("\n")
    f.write("\n};\n\n")
    f.write("#endif\n")

print(f"Generated {header_path}, size {len(data)} bytes")