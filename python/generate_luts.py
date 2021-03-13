# Generate an LED gamma-correction table
import math

gamma = 2.8 # Correction factor
max_in  = 255 # Top end of INPUT range
max_out = 255 # Top end of OUTPUT range
 
#  without dithering
# print("static const uint8_t gamma_lut[] = {\n   ", end = '')
# for i in range(max_in+1):
#     if i>0:
#         print(', ', end = '')
#     if i>0 and i%16==0:
#         print("\n   ", end = '')
#     x = math.floor(((i/max_in)**gamma)*max_out+0.5)
#     print(f'{x:3}', end = '')
# print("};", end = '')

# with dithering
print("static const uint32_t gamma_lut[] = {\n   ", end = '')
for i in range(max_in+1):
    if i>0:
        print(', ', end = '')
    if i>0 and i%16==0:
        print("\n   ", end = '')
    x = math.floor((((i/max_in)**gamma)*max_out)*(2**16))
    print(f'{x:3}', end = '')
print("};", end = '')