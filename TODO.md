# TODO List

- [x] dyn_cast 关键字实现
- [x] 支持 基本类型 实现 interface
- [x] 加入 `null`
- [x] type_id expression `type_id(expr)` `type_id<type>`
- [x] 加入 `lang.NullInterface`
- [x] 加入 built-in 模组占据 0 号 index
- [x] 编译器为 TypeInfo 结构体开洞
- [x] 为 `import` `export` 增加 `noffi` 关键字
- [x] 加入原生动态创建数组
- [x] 对因为 `dyn_cast` 造成的额外 `null-check` 进行优化。
    - [x] 在 IRValueType 中加入 `attrs` 字段用于存放变量属性
    - [x] 在 llvmCodegen 阶段对 IRValueType 加入 `Nullable` 属性检查，存在的情况下才进行 `null-check`
- [x] 加入可变参数
- [ ] 加入 array 的导出型 FFI wrapper 支持
- [x] 加入 `lang.argv`
- [x] 加入 `interfaceof` `impl` 关键字和 `abstractExpr` 表达式
- [x] 加入 static method 支持
- [x] 加入 interface template 支持
- [x] 完成 vector 模板的开发
- [x] 完成 string 的标准库开发
- [x] 完成 constructor 不强制要求 return this 的语法糖
- [x] 加入 lambda
- [ ] 完成 `let [x, y] = depacker()` 的语法设计和实现
- [ ] 改造 `new struct` 的逻辑
- [ ] 有限范围内推断虚函数调用变为正常函数优化
    - [ ] IRValueType改造，支持附加信息
- [x] 加入左值对象和将亡对象判断和处理逻辑
- [x] 在json库加入对 list 和 null 的处理
- [ ] 为type alias加入模板
<!-- - [ ] 加入 intrinstic 函数关键字及对应 llvmCodegen 优化 (dismissed) -->
- [x] 完善 RAII 加入 finalizer
- [ ] 加入 `threading.Mutex`

# Known issues

- [x] `||` 算符短路失效，不论条件，结果均为真
- [x] 没有原生 uint64_t 支持导致 string 实现中的某些操作存在安全隐患
- [x] Interface template foreign specialization 未完成
- [x] 小对象分配和释放占用大量CPU时间，而传参过程中参数均为小对象，考虑允许对函数参数添加 Raw 属性优化函数调用性能
    - [x] 前置任务：函数间控制流分析
- [x] 改造 Nullable 判断条件，在二元计算式，drop掉结果的Nullable标签
- [x] dummy_break, dummy_continue在while for block存在多个block时无法完成替换。
- [x] invoke_virtual由于ensureObject后仍使用arg的属性来判断是否post cleanup导致的memory leak
- [x] dyn_cast 匹配到错误typeid