import re
import os
import sys

def search_symbols(symbol_file, proc_file='/proc/kallsyms', output_file='kernel_symbols.h', symbol_width=32, address_width=18):
    # Compile the regular expression to capture the necessary parts of each line
    symbol_pattern = re.compile(r'^([0-9a-fA-F]{16})\s([a-zA-Z])\s([^\t\n]+)')

    # Check if the proc file exists
    if not os.path.exists(proc_file):
        print(f"Error: {proc_file} does not exist.")
        sys.exit(1)

    # Open the output header file and write the initial part
    with open(output_file, 'w') as out_file:
        out_file.write("""// Generated file: Do not modify directly
#ifndef _KERNEL_SYMBOLS_H_
#define _KERNEL_SYMBOLS_H_

""")  # Start of the file

        # Read the list of symbols to search for from the provided file
        try:
            with open(symbol_file, 'r') as f:
                symbols_to_search = [line.strip() for line in f.readlines()]
        except FileNotFoundError:
            print(f"Error: Symbol file '{symbol_file}' not found.")
            sys.exit(1)
        except IOError as e:
            print(f"Error reading symbol file: {e}")
            sys.exit(1)

        # Validate the symbol names in the input file
        for symbol in symbols_to_search:
            if len(symbol) > symbol_width:
                print(f"Error: Symbol '{symbol}' exceeds maximum allowed length of {symbol_width} characters.")
                sys.exit(1)

        # Set up a set to track symbols that are found in /proc/kallsyms
        found_symbols = set()

        # Read the /proc/kallsyms file and search for the symbols
        try:
            with open(proc_file, 'r') as f:
                for line in f:
                    match = symbol_pattern.match(line)
                    if match:
                        address, symbol_type, symbol = match.groups()
                        # Add the symbol to the found set
                        found_symbols.add(symbol)
        except PermissionError:
            print(f"Error: Permission denied when reading {proc_file}. You may need elevated privileges.")
            sys.exit(1)
        except IOError as e:
            print(f"Error reading {proc_file}: {e}")
            sys.exit(1)

        # Check if all input symbols are found in /proc/kallsyms
        for symbol in symbols_to_search:
            if symbol not in found_symbols:
                print(f"Error: Symbol '{symbol}' not found in {proc_file}.")
                sys.exit(1)

        # Get the maximum length of symbol names for alignment
        max_symbol_len = max(len(symbol) for symbol in symbols_to_search)
        max_address_len = address_width  # We want the address part to be fixed width, as set by `address_width`

        # Write the matching symbols and their addresses to the header file
        try:
            with open(proc_file, 'r') as f:
                for line in f:
                    match = symbol_pattern.match(line)
                    if match:
                        address, symbol_type, symbol = match.groups()

                        if symbol in symbols_to_search:
                            # If symbol matches, write the address as a C constant definition with 0x prefix
                            # Align the symbol name and address with ljust() and rjust()
                            symbol_str = f"{symbol.upper()}_ADDR".ljust(max_symbol_len + 5)  # Adding 5 for '_ADDR'
                            address_str = f"0x{address}".rjust(max_address_len)  # Right-align address
                            out_file.write(f"#define {symbol_str} {address_str}\n")
                            symbols_to_search.remove(symbol)  # Remove the symbol from the list once found
                            
                            # If all symbols are found, stop processing
                            if not symbols_to_search:
                                break
        except PermissionError:
            print(f"Error: Permission denied when reading {proc_file}. You may need elevated privileges.")
            sys.exit(1)
        except IOError as e:
            print(f"Error reading {proc_file}: {e}")
            sys.exit(1)

        # Close the header file and finish the definition
        out_file.write("""
#endif /* _KERNEL_SYMBOLS_H_ */
""")
    
    print(f"Header file '{output_file}' has been generated with the matching symbols.")

# Example usage:
symbol_file = 'symbol_names.txt'  # Input file containing the symbol names
search_symbols(symbol_file)

