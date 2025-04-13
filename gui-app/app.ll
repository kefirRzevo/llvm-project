; ModuleID = 'gui-app/app.c'
source_filename = "gui-app/app.c"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128"
target triple = "riscs"

; Function Attrs: noinline nounwind optnone
define dso_local void @set_bound_cond(ptr noundef %layer) #0 {
entry:
  %layer.addr = alloca ptr, align 8
  %k = alloca i32, align 4
  %need_set = alloca i32, align 4
  store ptr %layer, ptr %layer.addr, align 8
  store i32 0, ptr %k, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %k, align 4
  %cmp = icmp ne i32 %0, 30
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = call i64 @llvm.riscs.rand()
  %rem = srem i64 %1, 2
  %conv = trunc i64 %rem to i32
  store i32 %conv, ptr %need_set, align 4
  %2 = load i32, ptr %need_set, align 4
  %tobool = icmp ne i32 %2, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %3 = load ptr, ptr %layer.addr, align 8
  %4 = load i32, ptr %k, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds i8, ptr %3, i64 %idxprom
  store i8 1, ptr %arrayidx, align 1
  br label %if.end

if.else:                                          ; preds = %for.body
  %5 = load ptr, ptr %layer.addr, align 8
  %6 = load i32, ptr %k, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds i8, ptr %5, i64 %idxprom1
  store i8 0, ptr %arrayidx2, align 1
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %7 = load i32, ptr %k, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %k, align 4
  br label %for.cond, !llvm.loop !3

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind
declare i64 @llvm.riscs.rand() #1

; Function Attrs: noinline nounwind optnone
define dso_local void @apply_rule(ptr noundef %prev, ptr noundef %next) #0 {
entry:
  %prev.addr = alloca ptr, align 8
  %next.addr = alloca ptr, align 8
  %k = alloca i32, align 4
  %neighbors = alloca i32, align 4
  store ptr %prev, ptr %prev.addr, align 8
  store ptr %next, ptr %next.addr, align 8
  store i32 0, ptr %k, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %k, align 4
  %cmp = icmp ne i32 %0, 30
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %neighbors, align 4
  %1 = load ptr, ptr %prev.addr, align 8
  %2 = load i32, ptr %k, align 4
  %sub = sub nsw i32 %2, 1
  %add = add nsw i32 %sub, 30
  %rem = srem i32 %add, 30
  %idxprom = sext i32 %rem to i64
  %arrayidx = getelementptr inbounds i8, ptr %1, i64 %idxprom
  %3 = load i8, ptr %arrayidx, align 1
  %tobool = icmp ne i8 %3, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %4 = load i32, ptr %neighbors, align 4
  %add1 = add nsw i32 %4, 4
  store i32 %add1, ptr %neighbors, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %5 = load ptr, ptr %prev.addr, align 8
  %6 = load i32, ptr %k, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds i8, ptr %5, i64 %idxprom2
  %7 = load i8, ptr %arrayidx3, align 1
  %tobool4 = icmp ne i8 %7, 0
  br i1 %tobool4, label %if.then5, label %if.end7

if.then5:                                         ; preds = %if.end
  %8 = load i32, ptr %neighbors, align 4
  %add6 = add nsw i32 %8, 2
  store i32 %add6, ptr %neighbors, align 4
  br label %if.end7

if.end7:                                          ; preds = %if.then5, %if.end
  %9 = load ptr, ptr %prev.addr, align 8
  %10 = load i32, ptr %k, align 4
  %add8 = add nsw i32 %10, 1
  %rem9 = srem i32 %add8, 30
  %idxprom10 = sext i32 %rem9 to i64
  %arrayidx11 = getelementptr inbounds i8, ptr %9, i64 %idxprom10
  %11 = load i8, ptr %arrayidx11, align 1
  %tobool12 = icmp ne i8 %11, 0
  br i1 %tobool12, label %if.then13, label %if.end15

if.then13:                                        ; preds = %if.end7
  %12 = load i32, ptr %neighbors, align 4
  %add14 = add nsw i32 %12, 1
  store i32 %add14, ptr %neighbors, align 4
  br label %if.end15

if.end15:                                         ; preds = %if.then13, %if.end7
  %13 = load i32, ptr %neighbors, align 4
  %shl = shl i32 1, %13
  %and = and i32 110, %shl
  %tobool16 = icmp ne i32 %and, 0
  br i1 %tobool16, label %if.then17, label %if.else

if.then17:                                        ; preds = %if.end15
  %14 = load ptr, ptr %next.addr, align 8
  %15 = load i32, ptr %k, align 4
  %idxprom18 = sext i32 %15 to i64
  %arrayidx19 = getelementptr inbounds i8, ptr %14, i64 %idxprom18
  store i8 1, ptr %arrayidx19, align 1
  br label %if.end22

if.else:                                          ; preds = %if.end15
  %16 = load ptr, ptr %next.addr, align 8
  %17 = load i32, ptr %k, align 4
  %idxprom20 = sext i32 %17 to i64
  %arrayidx21 = getelementptr inbounds i8, ptr %16, i64 %idxprom20
  store i8 0, ptr %arrayidx21, align 1
  br label %if.end22

if.end22:                                         ; preds = %if.else, %if.then17
  br label %for.inc

for.inc:                                          ; preds = %if.end22
  %18 = load i32, ptr %k, align 4
  %inc = add nsw i32 %18, 1
  store i32 %inc, ptr %k, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @app() #0 {
entry:
  %data = alloca [30 x [30 x i8]], align 1
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %cur = alloca i32, align 4
  %i18 = alloca i32, align 4
  %j22 = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp ne i32 %0, 30
  br i1 %cmp, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %1 = load i32, ptr %j, align 4
  %cmp2 = icmp ne i32 %1, 30
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %2 = load i32, ptr %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [30 x [30 x i8]], ptr %data, i64 0, i64 %idxprom
  %3 = load i32, ptr %j, align 4
  %idxprom4 = sext i32 %3 to i64
  %arrayidx5 = getelementptr inbounds [30 x i8], ptr %arrayidx, i64 0, i64 %idxprom4
  store i8 0, ptr %arrayidx5, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %4 = load i32, ptr %j, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, ptr %j, align 4
  br label %for.cond1, !llvm.loop !6

for.end:                                          ; preds = %for.cond1
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %5 = load i32, ptr %i, align 4
  %inc7 = add nsw i32 %5, 1
  store i32 %inc7, ptr %i, align 4
  br label %for.cond, !llvm.loop !7

for.end8:                                         ; preds = %for.cond
  %arrayidx9 = getelementptr inbounds [30 x [30 x i8]], ptr %data, i64 0, i64 0
  %arraydecay = getelementptr inbounds [30 x i8], ptr %arrayidx9, i64 0, i64 0
  call void @set_bound_cond(ptr noundef %arraydecay)
  store i32 0, ptr %cur, align 4
  br label %for.cond10

for.cond10:                                       ; preds = %for.end41, %for.end8
  %6 = call i64 @llvm.riscs.quit.event()
  %tobool = icmp ne i64 %6, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %for.cond10
  br label %for.end43

if.end:                                           ; preds = %for.cond10
  %7 = load i32, ptr %cur, align 4
  %rem = srem i32 %7, 30
  %idxprom11 = sext i32 %rem to i64
  %arrayidx12 = getelementptr inbounds [30 x [30 x i8]], ptr %data, i64 0, i64 %idxprom11
  %arraydecay13 = getelementptr inbounds [30 x i8], ptr %arrayidx12, i64 0, i64 0
  %8 = load i32, ptr %cur, align 4
  %add = add nsw i32 %8, 1
  %rem14 = srem i32 %add, 30
  %idxprom15 = sext i32 %rem14 to i64
  %arrayidx16 = getelementptr inbounds [30 x [30 x i8]], ptr %data, i64 0, i64 %idxprom15
  %arraydecay17 = getelementptr inbounds [30 x i8], ptr %arrayidx16, i64 0, i64 0
  call void @apply_rule(ptr noundef %arraydecay13, ptr noundef %arraydecay17)
  store i32 0, ptr %i18, align 4
  br label %for.cond19

for.cond19:                                       ; preds = %for.inc39, %if.end
  %9 = load i32, ptr %i18, align 4
  %cmp20 = icmp ne i32 %9, 30
  br i1 %cmp20, label %for.body21, label %for.end41

for.body21:                                       ; preds = %for.cond19
  store i32 0, ptr %j22, align 4
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc36, %for.body21
  %10 = load i32, ptr %j22, align 4
  %cmp24 = icmp ne i32 %10, 30
  br i1 %cmp24, label %for.body25, label %for.end38

for.body25:                                       ; preds = %for.cond23
  %11 = load i32, ptr %i18, align 4
  %idxprom26 = sext i32 %11 to i64
  %arrayidx27 = getelementptr inbounds [30 x [30 x i8]], ptr %data, i64 0, i64 %idxprom26
  %12 = load i32, ptr %j22, align 4
  %idxprom28 = sext i32 %12 to i64
  %arrayidx29 = getelementptr inbounds [30 x i8], ptr %arrayidx27, i64 0, i64 %idxprom28
  %13 = load i8, ptr %arrayidx29, align 1
  %tobool30 = icmp ne i8 %13, 0
  br i1 %tobool30, label %if.then31, label %if.else

if.then31:                                        ; preds = %for.body25
  %14 = load i32, ptr %j22, align 4
  %conv = sext i32 %14 to i64
  %15 = load i32, ptr %i18, align 4
  %conv32 = sext i32 %15 to i64
  call void @llvm.riscs.set.pixel(i64 %conv, i64 %conv32, i64 255)
  br label %if.end35

if.else:                                          ; preds = %for.body25
  %16 = load i32, ptr %j22, align 4
  %conv33 = sext i32 %16 to i64
  %17 = load i32, ptr %i18, align 4
  %conv34 = sext i32 %17 to i64
  call void @llvm.riscs.set.pixel(i64 %conv33, i64 %conv34, i64 4294967295)
  br label %if.end35

if.end35:                                         ; preds = %if.else, %if.then31
  br label %for.inc36

for.inc36:                                        ; preds = %if.end35
  %18 = load i32, ptr %j22, align 4
  %inc37 = add nsw i32 %18, 1
  store i32 %inc37, ptr %j22, align 4
  br label %for.cond23, !llvm.loop !8

for.end38:                                        ; preds = %for.cond23
  br label %for.inc39

for.inc39:                                        ; preds = %for.end38
  %19 = load i32, ptr %i18, align 4
  %inc40 = add nsw i32 %19, 1
  store i32 %inc40, ptr %i18, align 4
  br label %for.cond19, !llvm.loop !9

for.end41:                                        ; preds = %for.cond19
  call void @llvm.riscs.flush()
  %20 = load i32, ptr %cur, align 4
  %inc42 = add nsw i32 %20, 1
  store i32 %inc42, ptr %cur, align 4
  br label %for.cond10

for.end43:                                        ; preds = %if.then
  ret void
}

; Function Attrs: nounwind
declare i64 @llvm.riscs.quit.event() #1

; Function Attrs: nounwind
declare void @llvm.riscs.set.pixel(i64, i64, i64) #1

; Function Attrs: nounwind
declare void @llvm.riscs.flush() #1

attributes #0 = { noinline nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"frame-pointer", i32 2}
!2 = !{!"clang version 21.0.0git (git@github.com:kefirRzevo/LLVM-course-backend.git 50af51b6822987ee51b27c386c3cf857267a9de7)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = distinct !{!5, !4}
!6 = distinct !{!6, !4}
!7 = distinct !{!7, !4}
!8 = distinct !{!8, !4}
!9 = distinct !{!9, !4}
