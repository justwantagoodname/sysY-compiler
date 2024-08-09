# SysY Compiler ![Build](https://github.com/justwantagoodname/sysY-compiler/actions/workflows/c-cpp.yml/badge.svg)
Chillet Compiler，基于自主研发的 Chillet ir 设计的 SysY 编译器。本项目以 SysY 代码作为输入，生成 Chillet AST、Chillet Triples，并生成 ARM 汇编或 RiscV 汇编。 

![Mirror](https://github.com/justwantagoodname/sysY-compiler/actions/workflows/mirror.yml/badge.svg)

# Usage
## 在 `Ubuntu` 与 `Debian` 发行版下安装编译环境
没有 `make`, 先安装 `make`
```shell
$ make requirements
```

## 启动 HotReload 
此时会自动监听目录下的文件变化, 并自动编译
*不支持新建文件, 需要手动重启*
```shell
$ make dev
```

## 仅编译
```shell
$ make
```

## 清理目录
```shell
$ make clean
```

# Chillet AST

Chillet AST 以 XML 格式、树状结构描述 SysY 语言程序。例如，以下是 `test/hello_and_apb.sysy` 生成的对应 Chillet AST:
```
<CompUnit>
  <Scope name="Global">
    <Decl/>
    <FunctionDef>
      <Function return="Int" name="main">
        <Params/>
        <Scope name="main">
          <Decl>
            <Var name="a" line="2" column="8" type="Int">
              <InitValue>
                <Exp>
                  <Call line="2" name="getint"/>
                </Exp>
              </InitValue>
            </Var>
            <Var name="b" line="2" column="22" type="Int">
              <InitValue>
                <Exp>
                  <Call line="2" name="getint"/>
                </Exp>
              </InitValue>
            </Var>
          </Decl>
          <Block>
            <Var name="a" line="2" column="8" type="Int">
              <InitValue>
                <Exp>
                  <Call line="2" name="getint"/>
                </Exp>
              </InitValue>
            </Var>
            <Var name="b" line="2" column="22" type="Int">
              <InitValue>
                <Exp>
                  <Call line="2" name="getint"/>
                </Exp>
              </InitValue>
            </Var>
            <Exp>
              <Call line="3" name="putf">
                <Param value="a + b: %d\n" type="StringConst"/>
                <Param>
                  <Plus>
                    <Fetch>
                      <Address base="a" line="3" column="24"/>
                    </Fetch>
                    <Fetch>
                      <Address base="b" line="3" column="28"/>
                    </Fetch>
                  </Plus>
                </Param>
              </Call>
            </Exp>
            <Exp>
              <Call line="4" name="putf">
                <Param value="Hello world!\n" type="StringConst"/>
              </Call>
            </Exp>
          </Block>
        </Scope>
      </Function>
    </FunctionDef>
  </Scope>
</CompUnit>
```

# AST Query Language

用于查询 ASTNode 树状结构的语言
### 目前缺点与 TODO
- [ ] 没有转义字符，无法查询包含特殊字符的节点
- [ ] 在子节点顺序匹配中无法使用表达式，目前可以使用ASTNode_querySelectf 函数来实现
- [ ] 子节点顺序匹配中使用切片
- [ ] 支持负数索引
- [ ] 在属性匹配中无法使用表达式和函数，希望加入调用`C/C++`函数能力

## 例子

### 顺序查询
```
/Scope/Main
```

在以上例子中, `/Scope/Main` 表示从根节点开始, 依次查找子节点 `Scope` 和 `Main`

### 通配符查询
```
/Scope/*
```

在以上例子中, `/Scope/*` 表示从根节点开始, 依次查找子节点 `Scope` 下的**所有**子节点

### 后代查询
```
//Main
```

在以上例子中, `//Main` 表示从根节点开始, 依次查找子节点中**所有**的 `Main` 节点

### 属性匹配

> Hint: 可以和顺序查询, 通配符查询, 后代查询一起使用

```
//Var[@name='a',@type='Int']
```

在以上例子中, `//Var[@name='a',@type='Int']` 表示查找所有 `Var` 节点, 节点的 `name` 属性为 `a` **且** `type` 属性为 `Int`

> Note: 同样可以使用 `|` 来表示或

#### 数字属性查询

```
//Exp//Number[@value=1]
```

在以上例子中, `//Exp//Number[@value=1]` 表示查找所有 `Exp` 节点下的所有 `Number` 节点, 节点的 `value` 属性为 `1`

#### 纯属性查询

```
//Exp//*[@value]
```

在以上例子中, `//Exp//*[@value]` 表示查找所有 `Exp` 节点下的所有节点, 节点的 `value` 属性存在


### 子代顺序查询
```
/Scope[0]
```

在以上例子中, `/Scope[0]` 表示从根节点开始, 依次查找子节点中的 `Scope`子节点中第一个子节点

```
//Var[1]
```

在以上例子中, `//Var[1]` 表示查找所有 `Var` 节点, 并取出第二个节点

### 选择器前缀与特殊节点名

### . 选择器

`.`选择器表示当前节点

> Note: 注意此选择器暂时不支持与子代顺序查询一起使用

### .. 选择器

`..`选择器表示父节点

注意该选择器会直接选中父节点，若要对选择的父节点有要求，请使用 `parent::` 选择器

> Note: 注意此选择器暂时不支持与子代顺序查询一起使用

# ARM Stack Translator

# Chillet Triples

Chillet Triples 接受 Chillet AST 作为输入，生成四元组形式的分子级指令动作。例如，以下是 `test/hello_and_apb.sysy` 生成对应的 Chillet Triples:
```
0- 0@main :
1>      {(b0)
2>      call, lib@getint, {}, T0
3>      mov, T0, -, a(0)
4>      var, a(0), #1, Int
5>      call, lib@getint, {}, T0
6>      mov, T0, -, b(1)
7>      var, b(1), #1, Int
8>      load, a(0), -, T1
9>      load, b(1), -, T2
10>     add, T1, T2, T1
11>     call, lib@putf, {"a + b: %d\n", T1}, -
12>     call, lib@putf, {"Hello world!\n"}, -
13>     }(b0)
```

# RiscV Generator