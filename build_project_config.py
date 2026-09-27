with open("project_config.def") as f:
    #lines = [l.strip() for l in f if l.strip()]
    lines = [l.strip() for l in f ]

# create :
#  fw/config.h
#  rtl/config.vh
reg_addresses = set()

def addr_str_to_int(value):
    return int(value.split("0x")[1], 16)

with open("fw/config.h", "w") as ch, open("rtl/config.vh", "w") as vh:
    ch.write("#ifndef CONFIG_H\n#define CONFIG_H\n\n")
    vh.write("`ifndef CONFIG_VH\n`define CONFIG_VH\n\n")

    for line in lines:
        if "=" in line:
            name, value = line.split("=")
            if "#" in value or "//" in value:
                value = value.split("#")[0].split("//")[0]
            value=value.replace("_","").strip()
        else:
            ch.write(f"\n")
            vh.write(f"\n")
            continue

        # REG_ARRAY_ must be checked before REG_ since it also starts with "REG_"
        if name.strip().startswith("REG_ARRAY_"):
            array_name = name.strip()
            addr_str, count_str = value.split(",", 1)
            addr_str = addr_str.strip()
            count = int(count_str.strip(), 0)
            base_addr = addr_str_to_int(addr_str)

            for i in range(count):
                elem_addr = base_addr + 4*i
                assert elem_addr not in reg_addresses, f"register {array_name}[{i}] address is already in use"
                reg_addresses.add(elem_addr)

            ch.write(f"#define {array_name}_BASE {addr_str}\n")
            ch.write(f"#define {array_name}_SIZE {count}\n")
            ch.write(f"#define {array_name} ((volatile unsigned int*){array_name}_BASE)\n")

            hex_digits = hex(base_addr)[2:]
            if len(hex_digits) <= 8:
                vh_addr = f"32'h{'0'*(8-len(hex_digits))}{hex_digits}"
            else:
                vh_addr = f"64'h{'0'*(16-len(hex_digits))}{hex_digits}"
            vh.write(f"`define {array_name}_BASE {vh_addr}\n")
            vh.write(f"parameter {array_name}_BASE = `{array_name}_BASE ;\n")
            vh.write(f"`define {array_name}_SIZE {count}\n")
            vh.write(f"parameter {array_name}_SIZE = `{array_name}_SIZE ;\n")
            continue

        if name.startswith("REG_"):
            addr_int = addr_str_to_int(value)
            assert addr_int not in reg_addresses, f"register {name} address is already in use"
            reg_addresses.add(addr_int)
            ch.write(f"#define {name} (*(volatile unsigned int*){value})\n")
        else:
            ch.write(f"#define {name} {value}\n")

        if value.startswith("0x"):
            value = hex(int(value.split("0x")[1],16))[2:]
            if len(value) <= 8:
                vh.write(f"`define {name} 32'h{'0'*(8-len(value))}{value}\n")
            else:
                vh.write(f"`define {name} 64'h{'0'*(16-len(value))}{value}\n")
        else:
            vh.write(f"`define {name} {value}\n")
        vh.write(f"parameter {name} = `{name} ;\n")

    ch.write("\n#endif\n")
    vh.write("\n`endif\n")


# create :
#  rtl/register_file.v
ma = '"'
raw_header = [
    f"// this file is autogenerate by build_project_config.py",
    f"// based on project_config.def file",
    f"",
    f"`include {ma}rtl/config.vh{ma}",
    f"",
    f"module register_file #(",
    f"    parameter NB_ADDR          =             32 ,",
    f"    parameter NB_DATA          =             32 ",
    f")(",
    f"    input  wire               i_clk               ,",
    f"    input  wire               i_trigger_reg       ,",
    f"    input  wire [NB_ADDR-1:0] i_reg_offset        ,",
    f"    input  wire [NB_ADDR-1:0] i_reg_address       ,",
    f"    input  wire [NB_DATA-1:0] i_mem_wdata         ,",
    f"",
    f"INPUTS_HERE",
    f"",
    f"OUTPUTS_HERE",
    f"    output wire [NB_DATA-1:0] o_mem_rdata         ",
    f");",
    f"    reg [NB_DATA-1:0]   mem_rdata;",
    f"REGISTERS_HERE",
    f"",
    f"    always @(*) begin",
    f"READ_LOGIC_HERE",
    f"    end",
    f"",
    f"    always @(posedge i_clk) begin",
    f"        if (i_trigger_reg) begin",
    f"WRITE_LOGIC_HERE",
    f"        end",
    f"    end",
    f"    ",
    f"    assign o_mem_rdata = mem_rdata;",
    f"",
    f"ASSIGNS_HERE",
    f"",
    f"endmodule",
]

def is_array_def(l):
    return l.split("=")[0].strip().startswith("REG_ARRAY_")

def is_reg_def(l):
    return l.split("=")[0].strip().startswith("REG_") and not is_array_def(l)

def array_count(l):
    value = l.split("=")[1]
    value = value.split("#")[0].split("//")[0]
    value = value.replace("_","").strip()
    _, count_str = value.split(",", 1)
    return int(count_str.strip(), 0)

reg_lines   = [l for l in lines if is_reg_def(l)]
array_lines = [l for l in lines if is_array_def(l)]

rw_reg_lines = []
r_reg_lines  = []
for l in reg_lines:
    if "read" in l and ("#" in l or "//" in l):
        r_reg_lines.append(l.split("//")[0].split("#")[0])
    else :
        rw_reg_lines.append(l)

rw_array_lines = []
r_array_lines  = []
for l in array_lines:
    if "read" in l and ("#" in l or "//" in l):
        r_array_lines.append(l.split("//")[0].split("#")[0])
    else :
        rw_array_lines.append(l)

print("read writer register")
for l in rw_reg_lines:
    print(f"'{l}'")
print("read only register")
for l in r_reg_lines:
    print(f"'{l}'")
print("read writer register array")
for l in rw_array_lines:
    print(f"'{l}'")
print("read only register array")
for l in r_array_lines:
    print(f"'{l}'")

out_lines = []
output_wires = []
input_wires = []
rw_regs = []

output_array_wires = []
rw_array_regs   = []
rw_array_counts = []

input_array_wires = []
r_array_counts     = []

for l in raw_header:
    if l == "INPUTS_HERE":
        out_lines.append("    // inputs registers")
        for reg in r_reg_lines:
            wire_name = "i_" + reg.split("=")[0].strip().lower()
            input_wires.append(wire_name)
            out_lines.append(f"    input  wire [NB_DATA-1:0] {wire_name} ,")
        out_lines.append("    // input array registers")
        for reg in r_array_lines:
            wire_name = "i_" + reg.split("=")[0].strip().lower()
            count = array_count(reg)
            input_array_wires.append(wire_name)
            r_array_counts.append(count)
            out_lines.append(f"    input  wire [NB_DATA*{count}-1:0] {wire_name} ,")
    elif l == "OUTPUTS_HERE":
        out_lines.append("    // output registers")
        for reg in rw_reg_lines:
            wire_name = "o_" + reg.split("=")[0].strip().lower()
            output_wires.append(wire_name)
            reg_name = reg.split("=")[0].strip().replace("REG_","REGISTER_")
            rw_regs.append(reg_name)
            out_lines.append(f"    output wire [NB_DATA-1:0] {wire_name} ,")
        out_lines.append("    // output array registers")
        for reg in rw_array_lines:
            wire_name = "o_" + reg.split("=")[0].strip().lower()
            count = array_count(reg)
            reg_name = reg.split("=")[0].strip().replace("REG_","REGISTER_")
            output_array_wires.append(wire_name)
            rw_array_regs.append(reg_name)
            rw_array_counts.append(count)
            out_lines.append(f"    output wire [NB_DATA*{count}-1:0] {wire_name} ,")
    elif l == "REGISTERS_HERE":
        out_lines.append("    // registers")
        for reg_name in rw_regs:
            out_lines.append(f"    reg [NB_DATA-1:0] {reg_name} ;")
        out_lines.append("    // array registers")
        for reg_name, count in zip(rw_array_regs, rw_array_counts):
            out_lines.append(f"    reg [NB_DATA-1:0] {reg_name} [0:{count-1}] ;")
    elif l == "READ_LOGIC_HERE":
        out_lines.append("            // read comb RW regs")
        for line, reg_name in zip(rw_reg_lines, rw_regs):
            addr_name = line.split("=")[0].strip()
            out_lines.append(f"            if (i_reg_address == {addr_name} )begin mem_rdata = {reg_name} ; end")
        out_lines.append("            // read comb RW array regs")
        for line, reg_name in zip(rw_array_lines, rw_array_regs):
            addr_name = line.split("=")[0].strip()
            out_lines.append(f"            if (i_reg_address >= {addr_name}_BASE && i_reg_address < ({addr_name}_BASE + {addr_name}_SIZE*4) )begin mem_rdata = {reg_name}[(i_reg_address - {addr_name}_BASE)>>2] ; end")
        out_lines.append("            // read comb R regs")
        for line, input_w in zip(r_reg_lines, input_wires):
            addr_name = line.split("=")[0].strip()
            out_lines.append(f"            if (i_reg_address == {addr_name} )begin mem_rdata = {input_w} ; end")
        out_lines.append("            // read comb R array regs")
        for line, input_w in zip(r_array_lines, input_array_wires):
            addr_name = line.split("=")[0].strip()
            out_lines.append(f"            if (i_reg_address >= {addr_name}_BASE && i_reg_address < ({addr_name}_BASE + {addr_name}_SIZE*4) )begin mem_rdata = {input_w}[NB_DATA*((i_reg_address - {addr_name}_BASE)>>2) +: NB_DATA] ; end")
    elif l == "WRITE_LOGIC_HERE":
        out_lines.append("            // write logic")
        for reg_name, line in zip(rw_regs, rw_reg_lines):
            addr_name = line.split("=")[0].strip()
            out_lines.append(f"            if (i_reg_address == {addr_name} )begin {reg_name} <= i_mem_wdata ; end")
        out_lines.append("            // write logic array regs")
        for reg_name, line in zip(rw_array_regs, rw_array_lines):
            addr_name = line.split("=")[0].strip()
            out_lines.append(f"            if (i_reg_address >= {addr_name}_BASE && i_reg_address < ({addr_name}_BASE + {addr_name}_SIZE*4) )begin {reg_name}[(i_reg_address - {addr_name}_BASE)>>2] <= i_mem_wdata ; end")
    elif l == "ASSIGNS_HERE":
        out_lines.append("    // assignations")
        for wire, reg in zip(output_wires,rw_regs):
            out_lines.append(f"    assign {wire} = {reg} ;")
        out_lines.append("    // array assignations")
        for wire, reg_name, count in zip(output_array_wires, rw_array_regs, rw_array_counts):
            genvar_name = "gv_" + wire[2:]
            out_lines.append(f"    genvar {genvar_name} ;")
            out_lines.append(f"    generate")
            out_lines.append(f"        for ({genvar_name} = 0 ; {genvar_name} < {count} ; {genvar_name} = {genvar_name} + 1) begin : gen_{wire[2:]}")
            out_lines.append(f"            assign {wire}[NB_DATA*{genvar_name} +: NB_DATA] = {reg_name}[{genvar_name}] ;")
            out_lines.append(f"        end")
            out_lines.append(f"    endgenerate")
    else:
        out_lines.append(l)

with open("rtl/register_file.v", "w") as rf:
    rf.writelines("\n".join(out_lines))
