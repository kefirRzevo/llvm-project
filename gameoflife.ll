; ModuleID = '../../clones/llvm-project-grisc-v/griscv_examples/gameoflife.c'
source_filename = "../../clones/llvm-project-grisc-v/griscv_examples/gameoflife.c"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128"
target triple = "riscs"

; Function Attrs: noinline nounwind optnone
define dso_local void @set_bound_cond(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store ptr %0, ptr %2, align 8
  store i32 0, ptr %3, align 4
  br label %5

5:                                                ; preds = %25, %1
  %6 = load i32, ptr %3, align 4
  %7 = icmp ne i32 %6, 512
  br i1 %7, label %8, label %28

8:                                                ; preds = %5
  %9 = call i64 @llvm.riscs.rand()
  %10 = srem i64 %9, 2
  %11 = trunc i64 %10 to i32
  store i32 %11, ptr %4, align 4
  %12 = load i32, ptr %4, align 4
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %14, label %19

14:                                               ; preds = %8
  %15 = load ptr, ptr %2, align 8
  %16 = load i32, ptr %3, align 4
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds i8, ptr %15, i64 %17
  store i8 1, ptr %18, align 1
  br label %24

19:                                               ; preds = %8
  %20 = load ptr, ptr %2, align 8
  %21 = load i32, ptr %3, align 4
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds i8, ptr %20, i64 %22
  store i8 0, ptr %23, align 1
  br label %24

24:                                               ; preds = %19, %14
  br label %25

25:                                               ; preds = %24
  %26 = load i32, ptr %3, align 4
  %27 = add nsw i32 %26, 1
  store i32 %27, ptr %3, align 4
  br label %5, !llvm.loop !3

28:                                               ; preds = %5
  ret void
}

; Function Attrs: nounwind
declare i64 @llvm.riscs.rand() #1

; Function Attrs: noinline nounwind optnone
define dso_local void @apply_rule(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  store i32 0, ptr %5, align 4
  br label %7

7:                                                ; preds = %61, %2
  %8 = load i32, ptr %5, align 4
  %9 = icmp ne i32 %8, 512
  br i1 %9, label %10, label %64

10:                                               ; preds = %7
  store i32 0, ptr %6, align 4
  %11 = load ptr, ptr %3, align 8
  %12 = load i32, ptr %5, align 4
  %13 = sub nsw i32 %12, 1
  %14 = add nsw i32 %13, 512
  %15 = srem i32 %14, 512
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds i8, ptr %11, i64 %16
  %18 = load i8, ptr %17, align 1
  %19 = icmp ne i8 %18, 0
  br i1 %19, label %20, label %23

20:                                               ; preds = %10
  %21 = load i32, ptr %6, align 4
  %22 = add nsw i32 %21, 4
  store i32 %22, ptr %6, align 4
  br label %23

23:                                               ; preds = %20, %10
  %24 = load ptr, ptr %3, align 8
  %25 = load i32, ptr %5, align 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds i8, ptr %24, i64 %26
  %28 = load i8, ptr %27, align 1
  %29 = icmp ne i8 %28, 0
  br i1 %29, label %30, label %33

30:                                               ; preds = %23
  %31 = load i32, ptr %6, align 4
  %32 = add nsw i32 %31, 2
  store i32 %32, ptr %6, align 4
  br label %33

33:                                               ; preds = %30, %23
  %34 = load ptr, ptr %3, align 8
  %35 = load i32, ptr %5, align 4
  %36 = add nsw i32 %35, 1
  %37 = srem i32 %36, 512
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds i8, ptr %34, i64 %38
  %40 = load i8, ptr %39, align 1
  %41 = icmp ne i8 %40, 0
  br i1 %41, label %42, label %45

42:                                               ; preds = %33
  %43 = load i32, ptr %6, align 4
  %44 = add nsw i32 %43, 1
  store i32 %44, ptr %6, align 4
  br label %45

45:                                               ; preds = %42, %33
  %46 = load i32, ptr %6, align 4
  %47 = shl i32 1, %46
  %48 = and i32 110, %47
  %49 = icmp ne i32 %48, 0
  br i1 %49, label %50, label %55

50:                                               ; preds = %45
  %51 = load ptr, ptr %4, align 8
  %52 = load i32, ptr %5, align 4
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds i8, ptr %51, i64 %53
  store i8 1, ptr %54, align 1
  br label %60

55:                                               ; preds = %45
  %56 = load ptr, ptr %4, align 8
  %57 = load i32, ptr %5, align 4
  %58 = sext i32 %57 to i64
  %59 = getelementptr inbounds i8, ptr %56, i64 %58
  store i8 0, ptr %59, align 1
  br label %60

60:                                               ; preds = %55, %50
  br label %61

61:                                               ; preds = %60
  %62 = load i32, ptr %5, align 4
  %63 = add nsw i32 %62, 1
  store i32 %63, ptr %5, align 4
  br label %7, !llvm.loop !5

64:                                               ; preds = %7
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @app() #0 {
  %1 = alloca [512 x [512 x i8]], align 1
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  call void @llvm.memset.p0.i64(ptr align 1 %1, i8 0, i64 262144, i1 false)
  %5 = getelementptr inbounds [512 x [512 x i8]], ptr %1, i64 0, i64 0
  %6 = getelementptr inbounds [512 x i8], ptr %5, i64 0, i64 0
  call void @set_bound_cond(ptr noundef %6)
  store i32 0, ptr %2, align 4
  br label %7

7:                                                ; preds = %57, %0
  %8 = call i64 @llvm.riscs.quit.event()
  %9 = icmp ne i64 %8, 0
  br i1 %9, label %10, label %11

10:                                               ; preds = %7
  br label %60

11:                                               ; preds = %7
  %12 = load i32, ptr %2, align 4
  %13 = srem i32 %12, 512
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds [512 x [512 x i8]], ptr %1, i64 0, i64 %14
  %16 = getelementptr inbounds [512 x i8], ptr %15, i64 0, i64 0
  %17 = load i32, ptr %2, align 4
  %18 = add nsw i32 %17, 1
  %19 = srem i32 %18, 512
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [512 x [512 x i8]], ptr %1, i64 0, i64 %20
  %22 = getelementptr inbounds [512 x i8], ptr %21, i64 0, i64 0
  call void @apply_rule(ptr noundef %16, ptr noundef %22)
  store i32 0, ptr %3, align 4
  br label %23

23:                                               ; preds = %54, %11
  %24 = load i32, ptr %3, align 4
  %25 = icmp ne i32 %24, 512
  br i1 %25, label %26, label %57

26:                                               ; preds = %23
  store i32 0, ptr %4, align 4
  br label %27

27:                                               ; preds = %50, %26
  %28 = load i32, ptr %4, align 4
  %29 = icmp ne i32 %28, 512
  br i1 %29, label %30, label %53

30:                                               ; preds = %27
  %31 = load i32, ptr %3, align 4
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds [512 x [512 x i8]], ptr %1, i64 0, i64 %32
  %34 = load i32, ptr %4, align 4
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds [512 x i8], ptr %33, i64 0, i64 %35
  %37 = load i8, ptr %36, align 1
  %38 = icmp ne i8 %37, 0
  br i1 %38, label %39, label %44

39:                                               ; preds = %30
  %40 = load i32, ptr %4, align 4
  %41 = sext i32 %40 to i64
  %42 = load i32, ptr %3, align 4
  %43 = sext i32 %42 to i64
  call void @llvm.riscs.set.pixel(i64 %41, i64 %43, i64 255)
  br label %49

44:                                               ; preds = %30
  %45 = load i32, ptr %4, align 4
  %46 = sext i32 %45 to i64
  %47 = load i32, ptr %3, align 4
  %48 = sext i32 %47 to i64
  call void @llvm.riscs.set.pixel(i64 %46, i64 %48, i64 4294967295)
  br label %49

49:                                               ; preds = %44, %39
  br label %50

50:                                               ; preds = %49
  %51 = load i32, ptr %4, align 4
  %52 = add nsw i32 %51, 1
  store i32 %52, ptr %4, align 4
  br label %27, !llvm.loop !6

53:                                               ; preds = %27
  br label %54

54:                                               ; preds = %53
  %55 = load i32, ptr %3, align 4
  %56 = add nsw i32 %55, 1
  store i32 %56, ptr %3, align 4
  br label %23, !llvm.loop !7

57:                                               ; preds = %23
  call void @llvm.riscs.flush()
  %58 = load i32, ptr %2, align 4
  %59 = add nsw i32 %58, 1
  store i32 %59, ptr %2, align 4
  br label %7

60:                                               ; preds = %10
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #2

; Function Attrs: nounwind
declare i64 @llvm.riscs.quit.event() #1

; Function Attrs: nounwind
declare void @llvm.riscs.set.pixel(i64, i64, i64) #1

; Function Attrs: nounwind
declare void @llvm.riscs.flush() #1

attributes #0 = { noinline nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"frame-pointer", i32 2}
!2 = !{!"clang version 21.0.0git (git@github.com:kefirRzevo/LLVM-course-backend.git 3391d2e08a1bb59c459236c08668dd5a2a81dd38)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = distinct !{!5, !4}
!6 = distinct !{!6, !4}
!7 = distinct !{!7, !4}
