#
# Python script to generate a C LUT that performs a swizzling operation on a colour lookup table (CLUT)
# to transform it into the form that the PS2 expects.
#

indices = []

def add_two_rows(start):
    for i in range(start, start + 8):
        indices.append(i)
    for i in range(start + 16, start + 16 + 8):
        indices.append(i)
    for i in range(start + 8, start + 8 + 8):
        indices.append(i)
    for i in range(start + 24, start + 24 + 8):
        indices.append(i)

def make_indices_array():
    for i in range(256 // 32):
        add_two_rows(32 * i)

def main():
    make_indices_array()
    c_code = "// A LUT to translate a naive colour lookup table index to the swizzled format the ps2 needs\n"
    c_code += "// For the 8 bit index format (IDTEX8) using the more efficient CSM1 CLUT storage mode\n"
    c_code += "// See GS User Manual page 30\n"
    c_code += "unsigned char CLUT_IDTEX8_CSM1_SwizzleTable[256] = {\n"
    for i in range(256 // 16):
        for j in range(i * 16, (i * 16) + 16):
            c_code += f'0x{indices[j]:02x}, '
        c_code += "\n"
    c_code += "};\n"
    print(c_code)


    pass
main()