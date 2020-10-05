# Shavds
Shavds [ʃeɪvz]

"Software Homology Analysis and Vulnerability Detection System based on source code" from "Course Project of Software Security" in HUST-CSE-2020-Fall

# 软件安全课程设计

## 代码同源性检测

1. 使用字符串匹配进行同源性检测
2. 使用控制流程图CFG进行同源性检测

- [x] 跨语言(4种以上)同源性检测
- [x] 分布式任务调度

## 代码漏洞检测

1. 栈缓冲区溢出检测
2. 格式化字符串漏洞检测

- [x] 堆缓冲区溢出检测
- [x] 整数宽度溢出检测
- [x] 整数运算溢出检测
- [x] 整数符号溢出检测
- [x] 空指针引用
- [ ] 竞争性条件

| 栈/堆缓冲区溢出漏洞 | buffer-overflow          |                                         |
| ------------------- | ------------------------ | --------------------------------------- |
| 整数宽度溢出漏洞    | integer-width-overflow   | -fsanitize=implicit-integer-truncation  |
| 整数运算溢出漏洞    | integer-operate-overflow | -fsanitize=signed-integer-overflow      |
| 整数符号溢出漏洞    | integer-sign-overflow    | -fsanitize=implicit-integer-sign-change |
| 空指针引用漏洞      | pointer-null             | -fsanitize=null                         |