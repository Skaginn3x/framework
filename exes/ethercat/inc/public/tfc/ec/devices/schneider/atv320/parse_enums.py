# Simple cpp code generator to create all enumerations
# assosiated with the atv320

import csv


def fetch_from_inside(some_string, begin_symbol, end_symbol):
    begin = some_string.find(begin_symbol)
    end = some_string.find(end_symbol)
    return some_string[begin+1:end]

def get_cpp_enum(list_of_dict, enum_name):
    cpp_enum_string = f"{enum_name.lower()}_e"
    enum_string = f"// Begin of {cpp_enum_string} enum decleration \n"
    enum_string += f"enum struct {cpp_enum_string}: std::uint16_t "
    enum_string += "{\n"
    for entry in enums[enum]:
        to_be_added = f"   {entry['cpp_name']} ="
        enum_string +=  to_be_added + (" " * (40 - len(to_be_added))) + f" {entry['numerical_value']}, ///< {entry['description']} ({entry['display_name']})" 
        enum_string += "\n"
    enum_string += "};\n"
    return enum_string 

def get_cpp_to_string(list_of_dict, enum_name):
    cpp_enum_string = f"{enum_name.lower()}_e"
    enum_string = f"[[nodiscard]] constexpr auto enum_desc({cpp_enum_string} const enum_value) -> std::string_view {{"
    enum_string += "\n"
    enum_string += "   switch(enum_value)\n"
    enum_string += "   {\n"
    for entry in enums[enum]:
        enum_string += f"      case {cpp_enum_string}::{entry['cpp_name']}:\n"
        enum_string += f"         return \"{entry['display_name']}, {entry['description']}\";\n" #TODO: Might add some more to this. f.e. numerical value
    enum_string += "      default:\n"
    enum_string += "         return \"unknown\";\n"
    enum_string += "   }\n"
    enum_string += "}\n"
    enum_string += f"constexpr auto format_as({cpp_enum_string} const enum_value) -> std::string_view {{ return enum_desc(enum_value); }}\n\n"
    return enum_string

# Move all numbers at the start of a string
# to the back
def move_numbers_to_back(string):
    numbers = ""
    for char in string:
        if char.isdigit():
            numbers += char
        else:
            break
    return string[len(numbers):] + numbers

FILENAME = "enums_atv600.csv"
OUTPUT = "generated_enums.hpp"

if __name__ == "__main__":
    enums = {}
    structure = {}
    with open(FILENAME, 'r') as f:
        reader = csv.reader(f)
        first = True
        enum_name = ""
        for row in reader:
            if first:
                structure = {y: x for x, y in enumerate(row)}
                print(structure)
                first = False
            else:
                if len(row[structure['Code']]) > 0: # New code enumeration
                    if enum_name != "":
                        print(f"{enum_name} parsed!")
                    enum_name = row[structure['Code']].upper()
                    enums[enum_name] = []
                value = row[structure['Values']]
                display = row[structure['Display']]
                description = row[structure['Description']]

                to_be_added = {}
                if value.startswith('16#'):
                    to_be_added['numerical_value']    = int(value.replace('16#', '0x'), 16)
                else:
                    to_be_added['numerical_value']    = int(value)
                to_be_added['display_name'] = display
                # to_be_added['cpp_name'] = fetch_from_inside(display, '[', ']').strip().replace('->','_is_').replace(' & ', '_and_').replace('=','_').replace(' ', '_').lower().replace('/','').replace('+','_plus_').replace('.','').replace('(','').strip();
                to_be_added['freq_name'] = fetch_from_inside(display, '(', ')').strip();
                to_be_added['cpp_name'] = fetch_from_inside(display, '(', ')').strip().lower();

                # try:
                #     int(to_be_added['cpp_name'])
                #     to_be_added['cpp_name'] = "number_" + to_be_added['cpp_name']
                # except Exception as e:
                #     pass
                # # Special cases
                # if enum_name.lower().strip() == "aiol":
                #     print("Special case AIOL")
                #     if to_be_added['numerical_value'] == 0:
                #         to_be_added['cpp_name'] = "positive_only"
                #     if to_be_added['numerical_value'] == 1:
                #         to_be_added['cpp_name'] = "negative_only"
                # # to_be_added['cpp_name'] = move_numbers_to_back(to_be_added['cpp_name'])
                # if len(to_be_added['cpp_name']) == 0:
                #     print(to_be_added, enum_name)
                #     exit(-1)
                # while to_be_added['cpp_name'][0] == '_':
                #     to_be_added['cpp_name'] = to_be_added['cpp_name'][1:]
                # if to_be_added['cpp_name'] == 'auto':
                #     to_be_added['cpp_name'] = 'automatic'
                to_be_added['description']        = description
                enums[enum_name].append(to_be_added)
    sorted(enums)
    source_file = "// ATTENTION: This file is generated by parse_enums.py\n\n// DO NOT EDIT THIS FILE!\n\n"
    source_file += "#pragma once\n\n#include <cstdint>\n#include<string_view>\n\n"
    source_file += "// clang-format off\n"
    for enum in enums:
        source_file += get_cpp_enum(enums[enum], enum)
        source_file += get_cpp_to_string(enums[enum], enum)
    source_file += "// clang-format on\n"
    with open(OUTPUT, 'w') as fd:
        fd.write(source_file)
