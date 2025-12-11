set style enabled on

layout split
set tui border-kind space 
set disassembly-flavor intel
tui enable

file out/debug/qoi_tool
# b encode
# b decode
b display_ppm_p6
b display_qoi
# b cli
# set args encode -i images/200px-Scribus_logo.ppm -o images/200px-Scribus_logo.qoi 
# set args encode -i test/test.ppm -o test/test.qoi
# set args decode -i test/test.qoi -o test/test.p6 
# set args display -i images/200px-Scribus_logo.ppm 
set args display -i images/200px-Scribus_logo.qoi
r
