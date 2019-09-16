# netlistfuzz
Fuzz testing uses randomly generated inputs to detect bugs in software. This project apply fuzz testing in the area of hardware.
Netlist is a description of electronic circuit connectivity in hardware description language. Typically, fuzz testing is used in software 
testing. In this project, we apply it to detect bugs in hardware sy nthesis tools. Including crashes and languages features not being 
supported, nine bugs were discovered from a HST called ABC. Quantitative and qualitative methods are used to evaluate the results of this
project. The main contributions of this project is a prototype tool that generates random netlists and uses them to check that the HST is
working correctly. This tool generates test cases with a variety of different features such as generating binary operations 
in a combinational circuit, cascaded connections of a sequential flip-flop and fuzzing finite-state Mealy or Moore machines.
