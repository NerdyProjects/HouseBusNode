#!/bin/sh
./libcanard/dsdl_compiler/libcanard_dsdlc --outdir dsdl ../dsdl/homeautomation

# Remove source file reference from generated files because they contain absolute paths
find dsdl -type f -name '*.[ch]' -exec sed -i '/Source file:/d' {} \;

