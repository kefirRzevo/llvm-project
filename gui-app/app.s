	.file	"app.c"
	.text
	.globl	set_bound_cond                  ; -- Begin function set_bound_cond
	.type	set_bound_cond,@function
set_bound_cond:                         ; @set_bound_cond
; %bb.0:                                ; %entry
	addi	x2, x2, -32
	sd	x1, 24(x2)                      ; 8-byte Folded Spill
	sd	x8, 16(x2)                      ; 8-byte Folded Spill
	addi	x8, x2, 32
	andi	x2, x2, -8
	sd	x10, 8(x2)
	sw	x0, 4(x2)
	jal	x0, .LBB0_1
.LBB0_1:                                ; %for.cond
                                        ; =>This Inner Loop Header: Depth=1
	lwu	x10, 4(x2)
	addi	x11, x0, 30
	beq	x10, x11, .LBB0_7
	jal	x0, .LBB0_2
.LBB0_2:                                ; %for.body
                                        ;   in Loop: Header=BB0_1 Depth=1
	rand	x10
	addi	x11, x0, 63
	srl	x11, x10, x11
	add	x11, x10, x11
	addi	x12, x0, 1
	addi	x12, x12, -2
	and	x11, x11, x12
	sub	x10, x10, x11
	sw	x10, 0(x2)
	lwu	x10, 0(x2)
	beq	x10, x0, .LBB0_4
	jal	x0, .LBB0_3
.LBB0_3:                                ; %if.then
                                        ;   in Loop: Header=BB0_1 Depth=1
	ld	x10, 8(x2)
	lw	x11, 4(x2)
	add	x10, x10, x11
	addi	x11, x0, 1
	sb	x11, 0(x10)
	jal	x0, .LBB0_5
.LBB0_4:                                ; %if.else
                                        ;   in Loop: Header=BB0_1 Depth=1
	ld	x10, 8(x2)
	lw	x11, 4(x2)
	add	x10, x10, x11
	sb	x0, 0(x10)
	jal	x0, .LBB0_5
.LBB0_5:                                ; %if.end
                                        ;   in Loop: Header=BB0_1 Depth=1
	jal	x0, .LBB0_6
.LBB0_6:                                ; %for.inc
                                        ;   in Loop: Header=BB0_1 Depth=1
	lw	x10, 4(x2)
	addi	x10, x10, 1
	sw	x10, 4(x2)
	jal	x0, .LBB0_1
.LBB0_7:                                ; %for.end
	addi	x2, x8, -32
	ld	x8, 16(x2)                      ; 8-byte Folded Reload
	ld	x1, 24(x2)                      ; 8-byte Folded Reload
	addi	x2, x2, 32
	jalr	x0, 0(x1)
.Lfunc_end0:
	.size	set_bound_cond, .Lfunc_end0-set_bound_cond
                                        ; -- End function
	.globl	apply_rule                      ; -- Begin function apply_rule
	.type	apply_rule,@function
apply_rule:                             ; @apply_rule
; %bb.0:                                ; %entry
	addi	x2, x2, -40
	sd	x1, 32(x2)                      ; 8-byte Folded Spill
	sd	x8, 24(x2)                      ; 8-byte Folded Spill
	addi	x8, x2, 40
	andi	x2, x2, -8
	sd	x10, 16(x2)
	sd	x11, 8(x2)
	sw	x0, 4(x2)
	jal	x0, .LBB1_1
.LBB1_1:                                ; %for.cond
                                        ; =>This Inner Loop Header: Depth=1
	lwu	x10, 4(x2)
	addi	x11, x0, 30
	beq	x10, x11, .LBB1_13
	jal	x0, .LBB1_2
.LBB1_2:                                ; %for.body
                                        ;   in Loop: Header=BB1_1 Depth=1
	sw	x0, 0(x2)
	ld	x10, 16(x2)
	lw	x11, 4(x2)
	addi	x11, x11, 29
	addiw	x12, x11, 0
	lui	x13, 559241
	addi	x13, x13, -1911
	mul	x12, x12, x13
	addi	x13, x0, 32
	srl	x12, x12, x13
	add	x12, x12, x11
	addi	x13, x0, 1
	and	x13, x12, x13
	addi	x14, x0, 31
	srl	x13, x13, x14
	addiw	x12, x12, 0
	addi	x14, x0, 4
	srl	x12, x12, x14
	add	x12, x12, x13
	addi	x13, x0, 30
	mul	x12, x12, x13
	sub	x11, x11, x12
	addiw	x11, x11, 0
	add	x10, x10, x11
	lbu	x10, 0(x10)
	beq	x10, x0, .LBB1_4
	jal	x0, .LBB1_3
.LBB1_3:                                ; %if.then
                                        ;   in Loop: Header=BB1_1 Depth=1
	lw	x10, 0(x2)
	addi	x10, x10, 4
	sw	x10, 0(x2)
	jal	x0, .LBB1_4
.LBB1_4:                                ; %if.end
                                        ;   in Loop: Header=BB1_1 Depth=1
	ld	x10, 16(x2)
	lw	x11, 4(x2)
	add	x10, x10, x11
	lbu	x10, 0(x10)
	beq	x10, x0, .LBB1_6
	jal	x0, .LBB1_5
.LBB1_5:                                ; %if.then5
                                        ;   in Loop: Header=BB1_1 Depth=1
	lw	x10, 0(x2)
	addi	x10, x10, 2
	sw	x10, 0(x2)
	jal	x0, .LBB1_6
.LBB1_6:                                ; %if.end7
                                        ;   in Loop: Header=BB1_1 Depth=1
	ld	x10, 16(x2)
	lw	x11, 4(x2)
	addi	x11, x11, 1
	addiw	x12, x11, 0
	lui	x13, 559241
	addi	x13, x13, -1911
	mul	x12, x12, x13
	addi	x13, x0, 32
	srl	x12, x12, x13
	add	x12, x12, x11
	addi	x13, x0, 1
	and	x13, x12, x13
	addi	x14, x0, 31
	srl	x13, x13, x14
	addiw	x12, x12, 0
	addi	x14, x0, 4
	srl	x12, x12, x14
	add	x12, x12, x13
	addi	x13, x0, 30
	mul	x12, x12, x13
	sub	x11, x11, x12
	addiw	x11, x11, 0
	add	x10, x10, x11
	lbu	x10, 0(x10)
	beq	x10, x0, .LBB1_8
	jal	x0, .LBB1_7
.LBB1_7:                                ; %if.then13
                                        ;   in Loop: Header=BB1_1 Depth=1
	lw	x10, 0(x2)
	addi	x10, x10, 1
	sw	x10, 0(x2)
	jal	x0, .LBB1_8
.LBB1_8:                                ; %if.end15
                                        ;   in Loop: Header=BB1_1 Depth=1
	lwu	x10, 0(x2)
	addi	x11, x0, 1
	sll	x10, x11, x10
	andi	x10, x10, 110
	beq	x10, x0, .LBB1_10
	jal	x0, .LBB1_9
.LBB1_9:                                ; %if.then17
                                        ;   in Loop: Header=BB1_1 Depth=1
	ld	x10, 8(x2)
	lw	x11, 4(x2)
	add	x10, x10, x11
	addi	x11, x0, 1
	sb	x11, 0(x10)
	jal	x0, .LBB1_11
.LBB1_10:                               ; %if.else
                                        ;   in Loop: Header=BB1_1 Depth=1
	ld	x10, 8(x2)
	lw	x11, 4(x2)
	add	x10, x10, x11
	sb	x0, 0(x10)
	jal	x0, .LBB1_11
.LBB1_11:                               ; %if.end22
                                        ;   in Loop: Header=BB1_1 Depth=1
	jal	x0, .LBB1_12
.LBB1_12:                               ; %for.inc
                                        ;   in Loop: Header=BB1_1 Depth=1
	lw	x10, 4(x2)
	addi	x10, x10, 1
	sw	x10, 4(x2)
	jal	x0, .LBB1_1
.LBB1_13:                               ; %for.end
	addi	x2, x8, -40
	ld	x8, 24(x2)                      ; 8-byte Folded Reload
	ld	x1, 32(x2)                      ; 8-byte Folded Reload
	addi	x2, x2, 40
	jalr	x0, 0(x1)
.Lfunc_end1:
	.size	apply_rule, .Lfunc_end1-apply_rule
                                        ; -- End function
	.globl	app                             ; -- Begin function app
	.type	app,@function
app:                                    ; @app
; %bb.0:                                ; %entry
	addi	x2, x2, -936
	sd	x1, 928(x2)                     ; 8-byte Folded Spill
	sd	x8, 920(x2)                     ; 8-byte Folded Spill
	addi	x8, x2, 936
	sw	x0, -920(x8)
	jal	x0, .LBB2_1
.LBB2_1:                                ; %for.cond
                                        ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB2_3 Depth 2
	lwu	x10, -920(x8)
	addi	x11, x0, 30
	beq	x10, x11, .LBB2_8
	jal	x0, .LBB2_2
.LBB2_2:                                ; %for.body
                                        ;   in Loop: Header=BB2_1 Depth=1
	sw	x0, -924(x8)
	jal	x0, .LBB2_3
.LBB2_3:                                ; %for.cond1
                                        ;   Parent Loop BB2_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	lwu	x10, -924(x8)
	addi	x11, x0, 30
	beq	x10, x11, .LBB2_6
	jal	x0, .LBB2_4
.LBB2_4:                                ; %for.body3
                                        ;   in Loop: Header=BB2_3 Depth=2
	lw	x10, -920(x8)
	addi	x11, x0, 30
	mul	x10, x10, x11
	addi	x11, x8, -916
	add	x10, x11, x10
	lw	x11, -924(x8)
	add	x10, x10, x11
	sb	x0, 0(x10)
	jal	x0, .LBB2_5
.LBB2_5:                                ; %for.inc
                                        ;   in Loop: Header=BB2_3 Depth=2
	lw	x10, -924(x8)
	addi	x10, x10, 1
	sw	x10, -924(x8)
	jal	x0, .LBB2_3
.LBB2_6:                                ; %for.end
                                        ;   in Loop: Header=BB2_1 Depth=1
	jal	x0, .LBB2_7
.LBB2_7:                                ; %for.inc6
                                        ;   in Loop: Header=BB2_1 Depth=1
	lw	x10, -920(x8)
	addi	x10, x10, 1
	sw	x10, -920(x8)
	jal	x0, .LBB2_1
.LBB2_8:                                ; %for.end8
	addi	x10, x8, -916
	call	set_bound_cond
	sw	x0, -928(x8)
	jal	x0, .LBB2_9
.LBB2_9:                                ; %for.cond10
                                        ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB2_12 Depth 2
                                        ;       Child Loop BB2_14 Depth 3
	quit_event	x10
	beq	x10, x0, .LBB2_11
	jal	x0, .LBB2_10
.LBB2_10:                               ; %if.then
	jal	x0, .LBB2_23
.LBB2_11:                               ; %if.end
                                        ;   in Loop: Header=BB2_9 Depth=1
	lw	x11, -928(x8)
	lui	x10, 559241
	addi	x12, x10, -1911
	mul	x10, x11, x12
	addi	x13, x0, 32
	srl	x10, x10, x13
	add	x10, x10, x11
	addi	x14, x0, 1
	and	x15, x10, x14
	addi	x16, x0, 31
	srl	x15, x15, x16
	addiw	x10, x10, 0
	addi	x17, x0, 4
	srl	x10, x10, x17
	add	x10, x10, x15
	addi	x15, x0, 30
	mul	x10, x10, x15
	sub	x10, x11, x10
	addiw	x10, x10, 0
	mul	x10, x10, x15
	addi	x5, x8, -916
	add	x10, x5, x10
	addi	x11, x11, 1
	addiw	x6, x11, 0
	mul	x12, x6, x12
	srl	x12, x12, x13
	add	x12, x12, x11
	and	x13, x12, x14
	srl	x13, x13, x16
	addiw	x12, x12, 0
	srl	x12, x12, x17
	add	x12, x12, x13
	mul	x12, x12, x15
	sub	x11, x11, x12
	addiw	x11, x11, 0
	mul	x11, x11, x15
	add	x11, x5, x11
	call	apply_rule
	sw	x0, -932(x8)
	jal	x0, .LBB2_12
.LBB2_12:                               ; %for.cond19
                                        ;   Parent Loop BB2_9 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB2_14 Depth 3
	lwu	x10, -932(x8)
	addi	x11, x0, 30
	beq	x10, x11, .LBB2_22
	jal	x0, .LBB2_13
.LBB2_13:                               ; %for.body21
                                        ;   in Loop: Header=BB2_12 Depth=2
	sw	x0, -936(x8)
	jal	x0, .LBB2_14
.LBB2_14:                               ; %for.cond23
                                        ;   Parent Loop BB2_9 Depth=1
                                        ;     Parent Loop BB2_12 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	lwu	x10, -936(x8)
	addi	x11, x0, 30
	beq	x10, x11, .LBB2_20
	jal	x0, .LBB2_15
.LBB2_15:                               ; %for.body25
                                        ;   in Loop: Header=BB2_14 Depth=3
	lw	x10, -932(x8)
	addi	x11, x0, 30
	mul	x10, x10, x11
	addi	x11, x8, -916
	add	x10, x11, x10
	lw	x11, -936(x8)
	add	x10, x10, x11
	lbu	x10, 0(x10)
	beq	x10, x0, .LBB2_17
	jal	x0, .LBB2_16
.LBB2_16:                               ; %if.then31
                                        ;   in Loop: Header=BB2_14 Depth=3
	lw	x10, -936(x8)
	lw	x11, -932(x8)
	addi	x12, x0, 255
	set_pixel	x10, x11, x12
	jal	x0, .LBB2_18
.LBB2_17:                               ; %if.else
                                        ;   in Loop: Header=BB2_14 Depth=3
	lw	x10, -936(x8)
	lw	x11, -932(x8)
	addi	x12, x0, 1
	addi	x12, x12, -1
	set_pixel	x10, x11, x12
	jal	x0, .LBB2_18
.LBB2_18:                               ; %if.end35
                                        ;   in Loop: Header=BB2_14 Depth=3
	jal	x0, .LBB2_19
.LBB2_19:                               ; %for.inc36
                                        ;   in Loop: Header=BB2_14 Depth=3
	lw	x10, -936(x8)
	addi	x10, x10, 1
	sw	x10, -936(x8)
	jal	x0, .LBB2_14
.LBB2_20:                               ; %for.end38
                                        ;   in Loop: Header=BB2_12 Depth=2
	jal	x0, .LBB2_21
.LBB2_21:                               ; %for.inc39
                                        ;   in Loop: Header=BB2_12 Depth=2
	lw	x10, -932(x8)
	addi	x10, x10, 1
	sw	x10, -932(x8)
	jal	x0, .LBB2_12
.LBB2_22:                               ; %for.end41
                                        ;   in Loop: Header=BB2_9 Depth=1
	flush	
	lw	x10, -928(x8)
	addi	x10, x10, 1
	sw	x10, -928(x8)
	jal	x0, .LBB2_9
.LBB2_23:                               ; %for.end43
	ld	x8, 920(x2)                     ; 8-byte Folded Reload
	ld	x1, 928(x2)                     ; 8-byte Folded Reload
	addi	x2, x2, 936
	jalr	x0, 0(x1)
.Lfunc_end2:
	.size	app, .Lfunc_end2-app
                                        ; -- End function
	.ident	"clang version 21.0.0git (git@github.com:kefirRzevo/LLVM-course-backend.git 50af51b6822987ee51b27c386c3cf857267a9de7)"
	.section	".note.GNU-stack","",@progbits
