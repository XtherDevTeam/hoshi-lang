# TODO List

- [ ] 异常抛出、捕获的设计和实现
- [x] dyn_cast 关键字实现
- [x] 支持 基本类型 实现 interface
- [x] 加入 `null`
- [x] type_id expression `type_id(expr)` `type_id<type>`
- [x] 加入 `lang.NullInterface`
- [x] 加入 built-in 模组占据 0 号 index
- [x] 编译器为 TypeInfo 结构体开洞
- [x] 为 `import` `export` 增加 `noffi` 关键字
- [x] 加入原生动态创建数组
- [ ] 对因为 `dyn_cast` 造成的额外 `null-check` 进行优化。
    - [ ] 在 IRValueType 中加入 `attrs` 字段用于存放变量属性
    - [ ] 在 llvmCodegen 阶段对 IRValueType 加入 `Nullable` 属性检查，存在的情况下才进行 `null-check`
- [x] 加入可变参数
- [ ] 加入 array 的导出型 FFI wrapper 支持
- [x] 加入 `lang.argv`
- [x] 加入 `interfaceof` `impl` 关键字和 `abstractExpr` 表达式
- [x] 加入 static method 支持
- [x] 加入 interface template 支持
- [ ] 完成 vector 模板的开发
- [ ] 完成 `let [x, y] = depacker()` 的语法设计和实现

# Known issues

- [ ] `||` 算符短路失效，不论条件，结果均为真