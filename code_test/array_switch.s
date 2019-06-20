	.file	"array_switch.c"
# GNU C11 (GCC) version 7.3.1 20180130 (Red Hat 7.3.1-2) (x86_64-redhat-linux)
#	compiled by GNU C version 7.3.1 20180130 (Red Hat 7.3.1-2), GMP version 6.1.2, MPFR version 3.1.5, MPC version 1.0.2, isl version none
# warning: MPFR header version 3.1.5 differs from library version 3.1.6-p2.
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  array_switch.c -mtune=generic -march=x86-64
# -auxbase-strip array_switch.s -O2 -fverbose-asm
# options enabled:  -faggressive-loop-optimizations -falign-labels
# -fasynchronous-unwind-tables -fauto-inc-dec -fbranch-count-reg
# -fcaller-saves -fchkp-check-incomplete-type -fchkp-check-read
# -fchkp-check-write -fchkp-instrument-calls -fchkp-narrow-bounds
# -fchkp-optimize -fchkp-store-bounds -fchkp-use-static-bounds
# -fchkp-use-static-const-bounds -fchkp-use-wrappers -fcode-hoisting
# -fcombine-stack-adjustments -fcommon -fcompare-elim -fcprop-registers
# -fcrossjumping -fcse-follow-jumps -fdefer-pop
# -fdelete-null-pointer-checks -fdevirtualize -fdevirtualize-speculatively
# -fdwarf2-cfi-asm -fearly-inlining -feliminate-unused-debug-types
# -fexpensive-optimizations -fforward-propagate -ffp-int-builtin-inexact
# -ffunction-cse -fgcse -fgcse-lm -fgnu-runtime -fgnu-unique
# -fguess-branch-probability -fhoist-adjacent-loads -fident -fif-conversion
# -fif-conversion2 -findirect-inlining -finline -finline-atomics
# -finline-functions-called-once -finline-small-functions -fipa-bit-cp
# -fipa-cp -fipa-icf -fipa-icf-functions -fipa-icf-variables -fipa-profile
# -fipa-pure-const -fipa-ra -fipa-reference -fipa-sra -fipa-vrp
# -fira-hoist-pressure -fira-share-save-slots -fira-share-spill-slots
# -fisolate-erroneous-paths-dereference -fivopts -fkeep-static-consts
# -fleading-underscore -flifetime-dse -flra-remat -flto-odr-type-merging
# -fmath-errno -fmerge-constants -fmerge-debug-strings
# -fmove-loop-invariants -fomit-frame-pointer -foptimize-sibling-calls
# -foptimize-strlen -fpartial-inlining -fpeephole -fpeephole2 -fplt
# -fprefetch-loop-arrays -free -freg-struct-return -freorder-blocks
# -freorder-functions -frerun-cse-after-loop
# -fsched-critical-path-heuristic -fsched-dep-count-heuristic
# -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
# -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
# -fsched-stalled-insns-dep -fschedule-fusion -fschedule-insns2
# -fsemantic-interposition -fshow-column -fshrink-wrap
# -fshrink-wrap-separate -fsigned-zeros -fsplit-ivs-in-unroller
# -fsplit-wide-types -fssa-backprop -fssa-phiopt -fstdarg-opt
# -fstore-merging -fstrict-aliasing -fstrict-overflow
# -fstrict-volatile-bitfields -fsync-libcalls -fthread-jumps
# -ftoplevel-reorder -ftrapping-math -ftree-bit-ccp -ftree-builtin-call-dce
# -ftree-ccp -ftree-ch -ftree-coalesce-vars -ftree-copy-prop -ftree-cselim
# -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre
# -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
# -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop -ftree-pre
# -ftree-pta -ftree-reassoc -ftree-scev-cprop -ftree-sink -ftree-slsr
# -ftree-sra -ftree-switch-conversion -ftree-tail-merge -ftree-ter
# -ftree-vrp -funit-at-a-time -funwind-tables -fverbose-asm
# -fzero-initialized-in-bss -m128bit-long-double -m64 -m80387
# -malign-stringops -mavx256-split-unaligned-load
# -mavx256-split-unaligned-store -mfancy-math-387 -mfp-ret-in-387 -mfxsr
# -mglibc -mieee-fp -mlong-double-80 -mmmx -mno-sse4 -mpush-args -mred-zone
# -msse -msse2 -mstv -mtls-direct-seg-refs -mvzeroupper

	.text
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	""
.LC1:
	.string	"val %d\n"
.LC2:
	.string	"%u is %s\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB25:
	.cfi_startproc
	pushq	%rbx	#
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
# /usr/include/stdlib.h:257:   return (int) strtol (__nptr, (char **) NULL, 10);
	movq	8(%rsi), %rdi	# MEM[(char * *)argv_3(D) + 8B], MEM[(char * *)argv_3(D) + 8B]
	movl	$10, %edx	#,
	xorl	%esi, %esi	#
	call	strtol	#
# array_switch.c:34: printf("val %d\n", val);fflush(stdout);
	movl	$.LC1, %edi	#,
	movl	%eax, %esi	# _8,
# /usr/include/stdlib.h:257:   return (int) strtol (__nptr, (char **) NULL, 10);
	movq	%rax, %rbx	#, _8
# array_switch.c:34: printf("val %d\n", val);fflush(stdout);
	xorl	%eax, %eax	#
	call	printf	#
	movq	stdout(%rip), %rdi	# stdout,
	call	fflush	#
# array_switch.c:25: 	if (type >= min_val && type - min_val < sizeof(vals) / sizeof(*vals) && vals[type - min_val])
	cmpl	$33, %ebx	#, _8
# array_switch.c:28: 	return "";
	movl	$.LC0, %edx	#, _10
# array_switch.c:25: 	if (type >= min_val && type - min_val < sizeof(vals) / sizeof(*vals) && vals[type - min_val])
	ja	.L2	#,
	movl	%ebx, %eax	# _8, val
	movq	vals.3950(,%rax,8), %rdx	# vals, _10
	testq	%rdx, %rdx	# _10
	je	.L6	#,
.L2:
# array_switch.c:36: printf("%u is %s\n", val, p);
	movl	%ebx, %esi	# _8,
	movl	$.LC2, %edi	#,
	xorl	%eax, %eax	#
	call	printf	#
# array_switch.c:37: }
	xorl	%eax, %eax	#
	popq	%rbx	#
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L6:
	.cfi_restore_state
# array_switch.c:28: 	return "";
	movl	$.LC0, %edx	#, _10
	jmp	.L2	#
	.cfi_endproc
.LFE25:
	.size	main, .-main
	.section	.rodata.str1.1
.LC3:
	.string	"RTM_NEWLINK"
.LC4:
	.string	"RTM_DELLINK"
.LC5:
	.string	"RTM_GETLINK"
.LC6:
	.string	"RTM_NEWADDR"
.LC7:
	.string	"RTM_DELADDR"
.LC8:
	.string	"RTM_GETADDR"
.LC9:
	.string	"RTM_NEWROUTE"
.LC10:
	.string	"RTM_DELROUTE"
.LC11:
	.string	"RTM_NEWRULE"
.LC12:
	.string	"RTM_DELRULE"
	.section	.rodata
	.align 32
	.type	vals.3950, @object
	.size	vals.3950, 272
vals.3950:
	.zero	128
	.quad	.LC3
	.quad	.LC4
	.quad	.LC5
	.zero	8
	.quad	.LC6
	.quad	.LC7
	.quad	.LC8
	.zero	8
	.quad	.LC9
	.quad	.LC10
	.zero	48
	.quad	.LC11
	.quad	.LC12
	.ident	"GCC: (GNU) 7.3.1 20180130 (Red Hat 7.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
