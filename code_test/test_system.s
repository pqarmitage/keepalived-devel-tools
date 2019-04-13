	.file	"test_system.c"
# GNU C11 (GCC) version 7.3.1 20180130 (Red Hat 7.3.1-2) (x86_64-redhat-linux)
#	compiled by GNU C version 7.3.1 20180130 (Red Hat 7.3.1-2), GMP version 6.1.2, MPFR version 3.1.5, MPC version 1.0.2, isl version none
# warning: MPFR header version 3.1.5 differs from library version 3.1.6-p2.
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  test_system.c -mtune=generic -march=x86-64
# -auxbase-strip test_system.s -fverbose-asm
# options enabled:  -faggressive-loop-optimizations
# -fasynchronous-unwind-tables -fauto-inc-dec -fchkp-check-incomplete-type
# -fchkp-check-read -fchkp-check-write -fchkp-instrument-calls
# -fchkp-narrow-bounds -fchkp-optimize -fchkp-store-bounds
# -fchkp-use-static-bounds -fchkp-use-static-const-bounds
# -fchkp-use-wrappers -fcommon -fdelete-null-pointer-checks
# -fdwarf2-cfi-asm -fearly-inlining -feliminate-unused-debug-types
# -ffp-int-builtin-inexact -ffunction-cse -fgcse-lm -fgnu-runtime
# -fgnu-unique -fident -finline-atomics -fira-hoist-pressure
# -fira-share-save-slots -fira-share-spill-slots -fivopts
# -fkeep-static-consts -fleading-underscore -flifetime-dse
# -flto-odr-type-merging -fmath-errno -fmerge-debug-strings -fpeephole
# -fplt -fprefetch-loop-arrays -freg-struct-return
# -fsched-critical-path-heuristic -fsched-dep-count-heuristic
# -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
# -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
# -fsched-stalled-insns-dep -fschedule-fusion -fsemantic-interposition
# -fshow-column -fshrink-wrap-separate -fsigned-zeros
# -fsplit-ivs-in-unroller -fssa-backprop -fstdarg-opt
# -fstrict-volatile-bitfields -fsync-libcalls -ftrapping-math -ftree-cselim
# -ftree-forwprop -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
# -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop
# -ftree-reassoc -ftree-scev-cprop -funit-at-a-time -funwind-tables
# -fverbose-asm -fzero-initialized-in-bss -m128bit-long-double -m64 -m80387
# -malign-stringops -mavx256-split-unaligned-load
# -mavx256-split-unaligned-store -mfancy-math-387 -mfp-ret-in-387 -mfxsr
# -mglibc -mieee-fp -mlong-double-80 -mmmx -mno-sse4 -mpush-args -mred-zone
# -msse -msse2 -mstv -mtls-direct-seg-refs -mvzeroupper

	.text
	.globl	file_name
	.data
	.align 16
	.type	file_name, @object
	.size	file_name, 20
file_name:
	.string	"/tmp/test_system.sh"
	.section	.rodata
.LC0:
	.string	"w"
	.align 8
.LC1:
	.string	"#!/bin/bash\n\n[[ $1 != NO ]] && kill -${1:-TERM} $$ 2>/dev/null\n\nexit ${2:-12}\n"
	.text
	.globl	write_script
	.type	write_script, @function
write_script:
.LFB5:
	.cfi_startproc
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$16, %rsp	#,
# test_system.c:13: 	FILE *fp = fopen(file_name, "w");
	movl	$.LC0, %esi	#,
	movl	$file_name, %edi	#,
	call	fopen	#
	movq	%rax, -8(%rbp)	# tmp88, fp
# test_system.c:14: 	char *script =
	movq	$.LC1, -16(%rbp)	#, script
# test_system.c:22: 	fwrite(script, strlen(script), 1, fp);
	movq	-16(%rbp), %rax	# script, tmp89
	movq	%rax, %rdi	# tmp89,
	call	strlen	#
	movq	%rax, %rsi	#, _1
	movq	-8(%rbp), %rdx	# fp, tmp90
	movq	-16(%rbp), %rax	# script, tmp91
	movq	%rdx, %rcx	# tmp90,
	movl	$1, %edx	#,
	movq	%rax, %rdi	# tmp91,
	call	fwrite	#
# test_system.c:23: 	fclose(fp);
	movq	-8(%rbp), %rax	# fp, tmp92
	movq	%rax, %rdi	# tmp92,
	call	fclose	#
# test_system.c:24: }
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	write_script, .-write_script
	.section	.rodata
.LC2:
	.string	"%s %s 2>/dev/null"
	.align 8
.LC3:
	.string	"system() on %s returned exited with "
.LC4:
	.string	"signal %d"
.LC5:
	.string	"exit code %d"
.LC6:
	.string	"return value 0x%x\n"
	.text
	.type	run_command, @function
run_command:
.LFB6:
	.cfi_startproc
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$80, %rsp	#,
	movq	%rdi, -72(%rbp)	# cmd, cmd
	movq	%rsi, -80(%rbp)	# feature, feature
# test_system.c:32: 	sprintf(command_buf, "%s %s 2>/dev/null", file_name, cmd);
	movq	-72(%rbp), %rdx	# cmd, tmp96
	leaq	-64(%rbp), %rax	#, tmp97
	movq	%rdx, %rcx	# tmp96,
	movl	$file_name, %edx	#,
	movl	$.LC2, %esi	#,
	movq	%rax, %rdi	# tmp97,
	movl	$0, %eax	#,
	call	sprintf	#
# test_system.c:33: 	ret = system(command_buf);
	leaq	-64(%rbp), %rax	#, tmp98
	movq	%rax, %rdi	# tmp98,
	call	system	#
	movl	%eax, -4(%rbp)	# tmp99, ret
# test_system.c:34: 	printf("system() on %s returned exited with ", feature);
	movq	-80(%rbp), %rax	# feature, tmp100
	movq	%rax, %rsi	# tmp100,
	movl	$.LC3, %edi	#,
	movl	$0, %eax	#,
	call	printf	#
# test_system.c:35: 	if (WIFSIGNALED(ret))
	movl	-4(%rbp), %eax	# ret, tmp101
	andl	$127, %eax	#, _2
	addl	$1, %eax	#, _3
	sarb	%al	# _5
	testb	%al, %al	# _5
	jle	.L3	#,
# test_system.c:36: 		printf("signal %d", WTERMSIG(ret));
	movl	-4(%rbp), %eax	# ret, tmp102
	andl	$127, %eax	#, _6
	movl	%eax, %esi	# _6,
	movl	$.LC4, %edi	#,
	movl	$0, %eax	#,
	call	printf	#
	jmp	.L4	#
.L3:
# test_system.c:37: 	else if (WIFEXITED(ret))
	movl	-4(%rbp), %eax	# ret, tmp103
	andl	$127, %eax	#, _7
	testl	%eax, %eax	# _7
	jne	.L5	#,
# test_system.c:38: 		printf("exit code %d", WEXITSTATUS(ret));
	movl	-4(%rbp), %eax	# ret, tmp104
	sarl	$8, %eax	#, _8
	movzbl	%al, %eax	# _8, _9
	movl	%eax, %esi	# _9,
	movl	$.LC5, %edi	#,
	movl	$0, %eax	#,
	call	printf	#
	jmp	.L4	#
.L5:
# test_system.c:40: 		printf("return value 0x%x\n", ret);
	movl	-4(%rbp), %eax	# ret, tmp105
	movl	%eax, %esi	# tmp105,
	movl	$.LC6, %edi	#,
	movl	$0, %eax	#,
	call	printf	#
.L4:
# test_system.c:41: 	printf("\n");
	movl	$10, %edi	#,
	call	putchar	#
# test_system.c:42: }
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	run_command, .-run_command
	.section	.rodata
.LC7:
	.string	"%s exists, not overwriting\n"
.LC8:
	.string	"on non existant script"
.LC9:
	.string	"11"
.LC10:
	.string	"non readable script"
.LC11:
	.string	"12"
.LC12:
	.string	"non executable script"
.LC13:
	.string	"13"
	.align 8
.LC14:
	.string	"script terminating with SIGTERM"
.LC15:
	.string	"TERM"
	.align 8
.LC16:
	.string	"script exiting with exit status 10"
.LC17:
	.string	"NO 10"
	.align 8
.LC18:
	.string	"script exiting after invalid signal and exit status 15"
.LC19:
	.string	"65 15"
	.text
	.globl	main
	.type	main, @function
main:
.LFB7:
	.cfi_startproc
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$160, %rsp	#,
	movl	%edi, -148(%rbp)	# argc, argc
	movq	%rsi, -160(%rbp)	# argv, argv
# test_system.c:48: 	if (stat(file_name, &buf) == 0) {
	leaq	-144(%rbp), %rax	#, tmp91
	movq	%rax, %rsi	# tmp91,
	movl	$file_name, %edi	#,
	call	stat	#
	testl	%eax, %eax	# _1
	jne	.L7	#,
# test_system.c:49: 		fprintf(stderr, "%s exists, not overwriting\n", file_name);
	movq	stderr(%rip), %rax	# stderr, stderr.0_2
	movl	$file_name, %edx	#,
	movl	$.LC7, %esi	#,
	movq	%rax, %rdi	# stderr.0_2,
	movl	$0, %eax	#,
	call	fprintf	#
# test_system.c:50: 		exit(1);
	movl	$1, %edi	#,
	call	exit	#
.L7:
# test_system.c:53: 	run_command("11", "on non existant script");
	movl	$.LC8, %esi	#,
	movl	$.LC9, %edi	#,
	call	run_command	#
# test_system.c:55: 	write_script();
	call	write_script	#
# test_system.c:56: 	chmod(file_name, S_IWUSR);
	movl	$128, %esi	#,
	movl	$file_name, %edi	#,
	call	chmod	#
# test_system.c:58: 	run_command("12", "non readable script");
	movl	$.LC10, %esi	#,
	movl	$.LC11, %edi	#,
	call	run_command	#
# test_system.c:60: 	chmod(file_name, S_IRUSR | S_IWUSR);
	movl	$384, %esi	#,
	movl	$file_name, %edi	#,
	call	chmod	#
# test_system.c:62: 	run_command("13", "non executable script");
	movl	$.LC12, %esi	#,
	movl	$.LC13, %edi	#,
	call	run_command	#
# test_system.c:64: 	chmod(file_name, S_IRUSR | S_IWUSR | S_IXUSR);
	movl	$448, %esi	#,
	movl	$file_name, %edi	#,
	call	chmod	#
# test_system.c:66: 	run_command("TERM", "script terminating with SIGTERM");
	movl	$.LC14, %esi	#,
	movl	$.LC15, %edi	#,
	call	run_command	#
# test_system.c:68: 	run_command("NO 10", "script exiting with exit status 10");
	movl	$.LC16, %esi	#,
	movl	$.LC17, %edi	#,
	call	run_command	#
# test_system.c:70: 	run_command("65 15", "script exiting after invalid signal and exit status 15");
	movl	$.LC18, %esi	#,
	movl	$.LC19, %edi	#,
	call	run_command	#
# test_system.c:72: 	unlink(file_name);
	movl	$file_name, %edi	#,
	call	unlink	#
	movl	$0, %eax	#, _17
# test_system.c:73: }
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	main, .-main
	.ident	"GCC: (GNU) 7.3.1 20180130 (Red Hat 7.3.1-2)"
	.section	.note.GNU-stack,"",@progbits
